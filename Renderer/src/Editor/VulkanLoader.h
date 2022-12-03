#pragma once

#include <Jnrlib.h>
#include <vulkan/vulkan.h>


#define JNR_FN(FN) PFN_vk##FN jnr##FN

// Device
extern JNR_FN(CreateCommandPool);
extern JNR_FN(CreateSwapchainKHR);
extern JNR_FN(DestroySwapchainKHR);
extern JNR_FN(GetSwapchainImagesKHR);
extern JNR_FN(CreateImageView);
extern JNR_FN(DestroyImageView);
extern JNR_FN(CreateShaderModule);
extern JNR_FN(DestroyShaderModule);
extern JNR_FN(CreatePipelineLayout);
extern JNR_FN(DestroyPipelineLayout);
extern JNR_FN(CreateGraphicsPipelines);
extern JNR_FN(DestroyPipeline);
extern JNR_FN(CreateRenderPass);

// Instance
extern JNR_FN(DestroyInstance);
extern JNR_FN(CreateDebugUtilsMessengerEXT);
extern JNR_FN(DestroyDebugUtilsMessengerEXT);
extern JNR_FN(EnumeratePhysicalDevices);
extern JNR_FN(GetPhysicalDeviceProperties);
extern JNR_FN(GetPhysicalDeviceFeatures);
extern JNR_FN(GetPhysicalDeviceQueueFamilyProperties);
extern JNR_FN(EnumerateDeviceLayerProperties);
extern JNR_FN(CreateDevice);
extern JNR_FN(DestroyDevice);
extern JNR_FN(GetDeviceQueue);
extern JNR_FN(GetDeviceProcAddr);
extern JNR_FN(DestroySurfaceKHR);
extern JNR_FN(GetPhysicalDeviceSurfaceSupportKHR);
extern JNR_FN(EnumerateDeviceExtensionProperties);
extern JNR_FN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
extern JNR_FN(GetPhysicalDeviceSurfaceFormatsKHR);
extern JNR_FN(GetPhysicalDeviceSurfacePresentModesKHR);

// Special cases
extern JNR_FN(CreateInstance);
extern JNR_FN(GetInstanceProcAddr);
extern JNR_FN(EnumerateInstanceLayerProperties);
extern JNR_FN(EnumerateInstanceExtensionProperties);


namespace Editor
{
    void LoadFunctions();
    void LoadFunctionsInstance(VkInstance);
    void LoadFunctionsDevice(VkDevice);
}