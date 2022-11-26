#include "Renderer.h"
#include "VulkanLoader.h"

#include <boost/algorithm/string.hpp>

#include "VulkanHelpers/VulkanHelpers.h"

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

    LOG(INFO) << "Vulkan renderer initialised successfully";
}

Renderer::~Renderer()
{
    jnrDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);

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

        if (mInstanceExtensions.debugUtils.enabled)
        {
            instanceInfo.pNext = &mInstanceExtensions.debugUtils.info;
        }

    }
    ThrowIfFailed(jnrCreateInstance(&instanceInfo, nullptr, &mInstance));

    LoadFunctionsInstance(mInstance);

    /* Also create the validation debug utils messenger */
    if (mInstanceExtensions.debugUtils.enabled)
    {
        ThrowIfFailed(
            jnrCreateDebugUtilsMessengerEXT(
                mInstance, &mInstanceExtensions.debugUtils.info, nullptr, &mDebugUtilsMessenger)
        );
    }
    
}

void Renderer::InitDevice()
{
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
            extensions.debugUtils.info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            extensions.debugUtils.info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            extensions.debugUtils.info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            extensions.debugUtils.info.pfnUserCallback = debugCallback;

            extensions.debugUtils.enabled = true;

            extensions.extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            extensions.extensionNames.push_back(it.c_str());
        }
    }
    return extensions;
}
