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
extern JNR_FN(CmdBindVertexBuffers);
extern JNR_FN(CmdCopyBuffer);
extern JNR_FN(CreateDescriptorPool);
extern JNR_FN(DestroyDescriptorPool);
extern JNR_FN(CreateDescriptorSetLayout);
extern JNR_FN(DestroyDescriptorSetLayout);
extern JNR_FN(AllocateDescriptorSets);
extern JNR_FN(UpdateDescriptorSets);
extern JNR_FN(CmdBindDescriptorSets);
extern JNR_FN(CmdPushConstants);
extern JNR_FN(CreateSampler);
extern JNR_FN(DestroySampler);
extern JNR_FN(CmdBindIndexBuffer);
extern JNR_FN(CreatePipelineCache);
extern JNR_FN(DestroyPipelineCache);
extern JNR_FN(GetPipelineCacheData);
extern JNR_FN(AllocateMemory);
extern JNR_FN(BindBufferMemory);
extern JNR_FN(BindImageMemory);
extern JNR_FN(CmdCopyBufferToImage);
extern JNR_FN(CreateBuffer);
extern JNR_FN(CreateImage);
extern JNR_FN(DestroyBuffer);
extern JNR_FN(DestroyImage);
extern JNR_FN(MapMemory);
extern JNR_FN(UnmapMemory);
extern JNR_FN(FlushMappedMemoryRanges);
extern JNR_FN(FreeCommandBuffers);
extern JNR_FN(FreeDescriptorSets);
extern JNR_FN(FreeMemory);
extern JNR_FN(GetBufferMemoryRequirements);
extern JNR_FN(GetImageMemoryRequirements);
extern JNR_FN(GetPhysicalDeviceMemoryProperties);

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


namespace Vulkan
{
    void LoadFunctions();
    void LoadFunctionsInstance(VkInstance);
    void LoadFunctionsDevice(VkDevice);

    struct DeviceInstance
    {
        VkDevice device;
        VkInstance instance;
    };
    PFN_vkVoidFunction GetFunctionByName(char const* name, void* userData);
}