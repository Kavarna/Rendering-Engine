#include "Renderer.h"
#include "VulkanLoader.h"

#include <boost/algorithm/string.hpp>

#include "VulkanHelpers/VulkanHelpers.h"

#include <unordered_set>

using namespace Editor;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG(INFO) << "Validation layer: " << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG(INFO) << "Validation layer: " << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG(WARNING) << "Validation layer: " << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG(ERROR) << "Validation layer: " << pCallbackData->pMessage;
            break;
        default:
            LOG(FATAL) << "How did you trigger this type of validation message?";

    }

    return VK_FALSE;
}


Renderer::Renderer(CreateInfo::EditorRenderer const& info)
{
    LOG(INFO) << "Attempting to initialize vulkan renderer with info" << info;

    LoadFunctions();
    InitInstance(info);
    PickPhysicalDevice();
    InitDevice(info);

    LOG(INFO) << "Vulkan renderer initialised successfully";
}

Renderer::~Renderer()
{
    jnrDestroyDevice(mDevice, nullptr);
    if (mInstanceExtensions.debugUtils.has_value())
    {
        jnrDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);
    }
    jnrDestroyInstance(mInstance, nullptr);

    LOG(INFO) << "Vulkan renderer uninitialised successfully";
}

void Renderer::InitInstance(CreateInfo::EditorRenderer const& info)
{
    VkApplicationInfo appInfo = {};
    {
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pApplicationName = "JNReditor";
        appInfo.pEngineName = "JNRenderer";
    }

    mInstanceLayers = GetEnabledInstanceLayers(info.instanceLayers);
    mInstanceExtensions = HandleEnabledInstanceExtensions(info.instanceExtensions);
    VkInstanceCreateInfo instanceInfo = {};
    {
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.enabledExtensionCount = 0;
        instanceInfo.flags = 0;
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledLayerCount = (uint32_t)mInstanceLayers.size();
        instanceInfo.ppEnabledLayerNames = mInstanceLayers.data();
        instanceInfo.enabledExtensionCount = (uint32_t)mInstanceExtensions.extensionNames.size();
        instanceInfo.ppEnabledExtensionNames = mInstanceExtensions.extensionNames.data();

        if (mInstanceExtensions.debugUtils.has_value())
        {
            instanceInfo.pNext = &(*mInstanceExtensions.debugUtils);
        }

    }
    ThrowIfFailed(jnrCreateInstance(&instanceInfo, nullptr, &mInstance));

    LoadFunctionsInstance(mInstance);

    /* Also create the validation debug utils messenger */
    if (mInstanceExtensions.debugUtils.has_value())
    {
        ThrowIfFailed(
            jnrCreateDebugUtilsMessengerEXT(
                mInstance, &(*mInstanceExtensions.debugUtils), nullptr, &mDebugUtilsMessenger)
        );
    }
    
}

void Renderer::PickPhysicalDevice()
{
    uint32_t count = 0;
    ThrowIfFailed(jnrEnumeratePhysicalDevices(mInstance, &count, nullptr));
    CHECK(count > 0) << "Could not find any vulkan-capable devices";

    std::vector<VkPhysicalDevice> devices;
    devices.resize(count);
    ThrowIfFailed(jnrEnumeratePhysicalDevices(mInstance, &count, devices.data()));

    bool found = false;
    for (auto const& device : devices)
    {
        VkPhysicalDeviceProperties properties = {};
        VkPhysicalDeviceFeatures features = {};

        jnrGetPhysicalDeviceProperties(device, &properties);
        jnrGetPhysicalDeviceFeatures(device, &features);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            LOG(INFO) << "Found discrete GPU with name: " << properties.deviceName;
            mPhysicalDevice = device;
            mPhysicalDeviceFeatures = features;
            mPhysicalDeviceProperties = properties;
            found = true;
            break;
        }
    }
    CHECK(found) << "Unable to find a GPU good enough";

    PickQueueFamilyIndices();
}

void Renderer::PickQueueFamilyIndices()
{
    uint32_t count = 0;
    jnrGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &count, nullptr);
    CHECK(count > 0) << "Selected device doesn't have any queues";

    std::vector<VkQueueFamilyProperties> queues;
    queues.resize(count);
    jnrGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &count, queues.data());

    for (uint32_t i = 0; i < queues.size(); ++i)
    {
        if (queues[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            mQueueIndices.graphicsFamily = i;
        }
    }

    CHECK(!mQueueIndices.IsEmpty()) << "There should be at least one queue";
}

void Renderer::InitDevice(CreateInfo::EditorRenderer const& info)
{
    auto uniqueQueueIndices = mQueueIndices.GetUniqueFamilyIndices();

    float priorities[] = {1.0f};
    std::vector< VkDeviceQueueCreateInfo> queues = {};
    queues.reserve(uniqueQueueIndices.size());
    for (auto const& index : uniqueQueueIndices)
    {
        VkDeviceQueueCreateInfo queueInfo = {};
        {
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = index;
            queueInfo.flags = 0;
            queueInfo.queueCount = ARRAYSIZE(priorities);
            queueInfo.pQueuePriorities = priorities;
        }
        queues.push_back(queueInfo);
    }

    mDeviceLayers = GetEnabledDeviceLayers(info.deviceLayers);
    mDeviceExtensions = HandleEnabledDeviceExtensions(info.deviceExtensions);

    VkDeviceCreateInfo deviceInfo = {};
    {
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = (uint32_t)uniqueQueueIndices.size();
        deviceInfo.pQueueCreateInfos = queues.data();
        deviceInfo.queueCreateInfoCount = (uint32_t)queues.size();
        deviceInfo.pEnabledFeatures = &mPhysicalDeviceFeatures;
        deviceInfo.enabledLayerCount = (uint32_t)mDeviceLayers.size();
        deviceInfo.ppEnabledLayerNames = mDeviceLayers.data();
        deviceInfo.enabledExtensionCount = (uint32_t)mDeviceExtensions.extensionNames.size();
        deviceInfo.ppEnabledExtensionNames = mDeviceExtensions.extensionNames.data();
    }

    ThrowIfFailed(
        jnrCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice)
    );

    if (mQueueIndices.graphicsFamily.has_value())
    {
        jnrGetDeviceQueue(mDevice, mQueueIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
    }

    LoadFunctionsDevice(mDevice);
}

std::vector<const char*> Renderer::GetEnabledInstanceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& expectedLayers)
{
    uint32_t numLayers = 0;
    std::vector<VkLayerProperties> layers;
    ThrowIfFailed(jnrEnumerateInstanceLayerProperties(&numLayers, nullptr));

    layers.resize(numLayers);
    ThrowIfFailed(
        jnrEnumerateInstanceLayerProperties(&numLayers, &layers[0])
    );

    std::vector<const char*> enabledLayers;
    enabledLayers.reserve(expectedLayers.size());
    for (auto const& expectedLayer : expectedLayers)
    {
        bool found = false;

        for (auto const& layer : layers)
        {
            if (boost::iequals(layer.layerName, expectedLayer.c_str()))
            {
                enabledLayers.push_back(expectedLayer.c_str());
                found = true;
                break;
            }
        }

        CHECK(found) << "Unable to find extension " << expectedLayer;

    }

    return enabledLayers;
}

Renderer::ExtensionsOutput Renderer::HandleEnabledInstanceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& expectedExtensions)
{
    ExtensionsOutput extensions;
    for (const auto& it : expectedExtensions)
    {
        if (strcmp(it.c_str(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugUtils = {};
            debugUtils.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtils.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugUtils.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtils.pfnUserCallback = debugCallback;

            extensions.debugUtils = debugUtils;

            extensions.extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            extensions.extensionNames.push_back(it.c_str());
        }
    }
    return extensions;
}

std::vector<const char*> Renderer::GetEnabledDeviceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& expectedLayers)
{
    uint32_t numLayers = 0;
    std::vector<VkLayerProperties> layers;
    ThrowIfFailed(jnrEnumerateDeviceLayerProperties(mPhysicalDevice , &numLayers, nullptr));

    layers.resize(numLayers);
    ThrowIfFailed(
        jnrEnumerateDeviceLayerProperties(mPhysicalDevice, &numLayers, &layers[0])
    );

    std::vector<const char*> enabledLayers;
    enabledLayers.reserve(expectedLayers.size());
    for (auto const& expectedLayer : expectedLayers)
    {
        bool found = false;

        for (auto const& layer : layers)
        {
            if (boost::iequals(layer.layerName, expectedLayer.c_str()))
            {
                enabledLayers.push_back(expectedLayer.c_str());
                found = true;
                break;
            }
        }

        CHECK(found) << "Unable to find extension " << expectedLayer;
    }

    return enabledLayers;
}

Renderer::ExtensionsOutput Renderer::HandleEnabledDeviceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& expectedExtensions)
{
    ExtensionsOutput extensions;

    for (const auto& it : expectedExtensions)
    {
        extensions.extensionNames.push_back(it.c_str());
    }

    return extensions;
}
