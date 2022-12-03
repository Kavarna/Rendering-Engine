#include "Renderer.h"
#include "VulkanLoader.h"

#include <unordered_set>
#include <boost/algorithm/string.hpp>

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

    mWindow = info.window;
    CHECK(mWindow != nullptr) << "In order to use the renderer, a window has to be specified";
    LoadFunctions();
    InitInstance(info);
    InitSurface();
    PickPhysicalDevice();
    InitDevice(info);
    InitSwapchain();

    LOG(INFO) << "Vulkan renderer initialised successfully";
}

Renderer::~Renderer()
{
    for (auto const& view : mSwapchainImageViews)
    {
        jnrDestroyImageView(mDevice, view, nullptr);
    }
    jnrDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    jnrDestroySurfaceKHR(mInstance, mRenderingSurface, nullptr);
    jnrDestroyDevice(mDevice, nullptr);
    if (mInstanceExtensions.debugUtils.has_value())
    {
        jnrDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);
    }
    jnrDestroyInstance(mInstance, nullptr);

    LOG(INFO) << "Vulkan renderer uninitialised successfully";
}

VkDevice Renderer::GetDevice()
{
    return mDevice;
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

    bool found = false, foundIntegrated = false;
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

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            LOG(INFO) << "Found intergate GPU with name: " << properties.deviceName;
            mPhysicalDevice = device;
            mPhysicalDeviceFeatures = features;
            mPhysicalDeviceProperties = properties;
            foundIntegrated = true;
        }
    }
    CHECK(found || foundIntegrated) << "Unable to find a GPU good enough";

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
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            mQueueIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        ThrowIfFailed(
            jnrGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mRenderingSurface, &presentSupport)
        );

        if (presentSupport)
        {
            mQueueIndices.presentFamily = i;
        }

        if (mQueueIndices.IsComplete())
            break;
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
            queueInfo.queueCount = sizeof(priorities) / sizeof(priorities[0]);
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

        if (mDeviceExtensions.dynamicRendering.has_value())
        {
            deviceInfo.pNext = &(*mDeviceExtensions.dynamicRendering);
        }
    }

    ThrowIfFailed(
        jnrCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice)
    );

    if (mQueueIndices.graphicsFamily.has_value())
    {
        jnrGetDeviceQueue(mDevice, mQueueIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
    }
    if (mQueueIndices.presentFamily.has_value())
    {
        jnrGetDeviceQueue(mDevice, mQueueIndices.presentFamily.value(), 0, &mPresentQueue);
    }

    LoadFunctionsDevice(mDevice);
}

void Renderer::InitSurface()
{
    ThrowIfFailed(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mRenderingSurface));
}

void Renderer::InitSwapchain()
{
    auto swapchainCapabilities = GetSwapchainCapabilities();

    auto presentMode = SelectBestPresentMode(swapchainCapabilities.presentModes);
    auto surfaceFormat = SelectBestSurfaceFormat(swapchainCapabilities.formats);
    auto extent = SelectSwapchainExtent(swapchainCapabilities.capabilities);

    uint32_t imageCount = 0;
    if (swapchainCapabilities.capabilities.maxImageCount == 0)
    {
        /* If the maxImageCount is 0 => there is no limit, so just stick to the default*/
        imageCount = swapchainCapabilities.capabilities.minImageCount + 1;
    }
    else
    {
        imageCount = glm::clamp(swapchainCapabilities.capabilities.minImageCount + 1,
                                swapchainCapabilities.capabilities.minImageCount, swapchainCapabilities.capabilities.maxImageCount);
    }
    CHECK(imageCount != 0) << "For some reason, the vulkan driver specified that 0 images can be used for the swapchain";

    std::vector<uint32_t> familiesUsingBackbuffer = {
        *mQueueIndices.graphicsFamily,
        *mQueueIndices.presentFamily
    };
    std::sort(familiesUsingBackbuffer.begin(), familiesUsingBackbuffer.end());
    familiesUsingBackbuffer.erase(
        std::unique(familiesUsingBackbuffer.begin(), familiesUsingBackbuffer.end()),
        familiesUsingBackbuffer.end());
    CHECK(familiesUsingBackbuffer.size() != 0) << "Something weird happened and I don't have any families to use with the backbuffer";

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    {
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = mRenderingSurface;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageFormat = surfaceFormat.format;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.minImageCount = imageCount;
        swapchainInfo.oldSwapchain = mSwapchain;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainInfo.imageExtent = extent;
        
        swapchainInfo.imageSharingMode = familiesUsingBackbuffer.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = (uint32_t)familiesUsingBackbuffer.size();
        swapchainInfo.pQueueFamilyIndices = familiesUsingBackbuffer.data();
    }
    
    ThrowIfFailed(jnrCreateSwapchainKHR(mDevice, &swapchainInfo, nullptr, &mSwapchain));

    {
        /* Retrieve images */
        uint32_t count = 0;
        ThrowIfFailed(jnrGetSwapchainImagesKHR(mDevice, mSwapchain, &count, nullptr));

        mSwapchainImages.resize(count);
        ThrowIfFailed(jnrGetSwapchainImagesKHR(mDevice, mSwapchain, &count, mSwapchainImages.data()));
    }

    {
        /* Create views for images */
        mSwapchainImageViews.resize(mSwapchainImages.size());
        
        for (uint32_t i = 0; i < mSwapchainImages.size(); ++i)
        {
            VkImageViewCreateInfo viewInfo = {};
            {
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.flags = 0;
                viewInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
                viewInfo.format = surfaceFormat.format;
                viewInfo.image = mSwapchainImages[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.layerCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            ThrowIfFailed(jnrCreateImageView(mDevice, &viewInfo, nullptr, &mSwapchainImageViews[i]));
        }

    }

}

Renderer::SwapchainSupportDetails Renderer::GetSwapchainCapabilities()
{
    SwapchainSupportDetails result;
    {
        VkSurfaceCapabilitiesKHR capabilities;
        ThrowIfFailed(jnrGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mRenderingSurface, &capabilities));

        result.capabilities = std::move(capabilities);
    }
    
    {
        uint32_t surfaceFormatCount = 0;
        ThrowIfFailed(jnrGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mRenderingSurface, &surfaceFormatCount, nullptr));
        CHECK(surfaceFormatCount != 0) << "The physical device has 0 surface formats. How is this possible?";

        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        surfaceFormats.resize(surfaceFormatCount);
        ThrowIfFailed(jnrGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mRenderingSurface, &surfaceFormatCount, surfaceFormats.data()));

        result.formats = std::move(surfaceFormats);
    }

    {
        uint32_t presentCount = 0;
        ThrowIfFailed(jnrGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mRenderingSurface, &presentCount, nullptr));
        CHECK(presentCount != 0) << "The physical device has 0 present modes. How is this possible?";

        std::vector<VkPresentModeKHR> presentModes;
        presentModes.resize(presentCount);
        ThrowIfFailed(jnrGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mRenderingSurface, &presentCount, presentModes.data()));

        result.presentModes = std::move(presentModes);
    }

    return result;
}

VkPresentModeKHR Renderer::SelectBestPresentMode(std::vector<VkPresentModeKHR> const& presentModes)
{
    for (auto const& presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;
    }

    // This is guaranteed to be present
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Renderer::SelectBestSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& formats)
{
    for (auto const& format : formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

VkExtent2D Renderer::SelectSwapchainExtent(VkSurfaceCapabilitiesKHR const& capabilities)
{
#undef max
    VkExtent2D extent = {};
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(mWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
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

    std::vector<const char*> enabledLayers = mInstanceLayers; // Copy the existing layers
    enabledLayers.push_back(nullptr); // and prepend the nullptr, to get layer-less extensions
    std::vector<VkExtensionProperties> properties;
    for (auto const& layer : enabledLayers)
    {
        uint32_t count = 0;
        jnrEnumerateInstanceExtensionProperties(layer, &count, nullptr);

        std::vector<VkExtensionProperties> layerProperties;
        layerProperties.resize(count);
        jnrEnumerateInstanceExtensionProperties(layer, &count, layerProperties.data());
        std::move(layerProperties.begin(), layerProperties.end(), std::back_inserter(properties));
    }

    for (const auto& it : expectedExtensions)
    {
        bool found = std::find_if(properties.begin(), properties.end(),
                                  [&](VkExtensionProperties const& properties)
                                  {
                                      return boost::iequals(it.c_str(), properties.extensionName);
                                  }) != properties.end();

        CHECK(found) << "Unable to find extension " << it;

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

        }
        
        extensions.extensionNames.push_back(it.c_str());
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

    std::vector<const char*> enabledLayers = mDeviceLayers; // Copy the existing layers
    enabledLayers.push_back(nullptr); // and prepend the nullptr, to get layer-less extensions
    std::vector<VkExtensionProperties> properties;
    for (auto const& layer : enabledLayers)
    {
        uint32_t count = 0;
        jnrEnumerateDeviceExtensionProperties(mPhysicalDevice, layer, &count, nullptr);

        std::vector<VkExtensionProperties> layerProperties;
        layerProperties.resize(count);
        jnrEnumerateDeviceExtensionProperties(mPhysicalDevice, layer, &count, layerProperties.data());
        std::move(layerProperties.begin(), layerProperties.end(), std::back_inserter(properties));
    }

    for (const auto& it : expectedExtensions)
    {
        bool found = std::find_if(properties.begin(), properties.end(),
                                  [&](VkExtensionProperties const& properties)
                                  {
                                      return boost::iequals(it.c_str(), properties.extensionName);
                                  }) != properties.end();

        CHECK(found) << "Unable to find extension " << it;

        if (strcmp(it.c_str(), VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
        {
            VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering{};
            dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
            dynamicRendering.dynamicRendering = VK_TRUE;

            extensions.dynamicRendering = dynamicRendering;
        }

        extensions.extensionNames.push_back(it.c_str());
    }

    return extensions;
}

VkPipelineLayout Renderer::GetEmptyPipelineLayout()
{
    if (mEmptyPipelineLayout != VK_NULL_HANDLE)
        return mEmptyPipelineLayout;

    VkPipelineLayoutCreateInfo layoutInfo = {};
    {
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.flags = 0;
    }

    ThrowIfFailed(
        jnrCreatePipelineLayout(mDevice, &layoutInfo, nullptr, &mEmptyPipelineLayout)
    );

    return mEmptyPipelineLayout;
}
