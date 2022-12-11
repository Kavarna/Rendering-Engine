#pragma once

#include <Jnrlib.h>
#include <vulkan/vulkan.h>

#undef CreateSemaphore

#define JNR_FN(FN) PFN_vk##FN jnr##FN

// Device
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
extern JNR_FN(CreateCommandPool);
extern JNR_FN(DestroyCommandPool);
extern JNR_FN(AllocateCommandBuffers);
extern JNR_FN(ResetCommandPool);
extern JNR_FN(BeginCommandBuffer);
extern JNR_FN(EndCommandBuffer);
extern JNR_FN(CmdBeginRendering);
extern JNR_FN(CmdEndRendering);
extern JNR_FN(AcquireNextImageKHR);
extern JNR_FN(CreateFence);
extern JNR_FN(DestroyFence);
extern JNR_FN(CreateSemaphore);
extern JNR_FN(DestroySemaphore);
extern JNR_FN(DeviceWaitIdle);
extern JNR_FN(QueueSubmit);
extern JNR_FN(QueuePresentKHR);
extern JNR_FN(CmdPipelineBarrier);
extern JNR_FN(CmdClearColorImage);
extern JNR_FN(CmdDrawIndexed);
extern JNR_FN(CmdDraw);
extern JNR_FN(CmdBindPipeline);
extern JNR_FN(CmdSetViewport);
extern JNR_FN(CmdSetScissor);
extern JNR_FN(WaitForFences);
extern JNR_FN(ResetFences);

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