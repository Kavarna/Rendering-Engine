#include "VulkanLoader.h"
#include <GLFW/glfw3.h>

#define STRINGIFY_INT(a) #a
#define STRINGIFY(a) STRINGIFY_INT(a)
#define PFN(fn) PFN_vk##fn
#define GET_INST_FN_INT(pf, vk, in, inst)                                                                              \
    {                                                                                                                  \
        in = reinterpret_cast<pf>(jnrGetInstanceProcAddr(inst, STRINGIFY(vk)));                                        \
        CHECK(in != nullptr) << "Unable to get a pointer to " #vk;                                                     \
    }
#define GET_INST_FN(fn, inst) GET_INST_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_INST_FN_INT_OPT(pf, vk, in, inst)                                                                          \
    {                                                                                                                  \
        in = reinterpret_cast<pf>(jnrGetInstanceProcAddr(inst, STRINGIFY(vk)));                                        \
        if (in == nullptr)                                                                                             \
            LOG(WARNING) << "Unable to get a pointer to " #vk;                                                         \
    }
#define GET_INST_FN_OPT(fn, inst) GET_INST_FN_INT_OPT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_DEV_FN_INT(pf, vk, in, dev)                                                                                \
    {                                                                                                                  \
        in = reinterpret_cast<pf>(jnrGetDeviceProcAddr(dev, STRINGIFY(vk)));                                           \
        CHECK(in != nullptr) << "Unable to get a pointer to " #vk;                                                     \
    }
#define GET_DEV_FN(fn, inst) GET_DEV_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_DEV_FN_INT_OPT(pf, vk, in, dev)                                                                            \
    {                                                                                                                  \
        in = reinterpret_cast<pf>(jnrGetDeviceProcAddr(dev, STRINGIFY(vk)));                                           \
        if (in == nullptr)                                                                                             \
            LOG(WARNING) << "Unable to get a pointer to " #vk;                                                         \
    }
#define GET_DEV_FN_OPT(fn, inst) GET_DEV_FN_INT_OPT(PFN(fn), vk##fn, jnr##fn, inst)

// Device
JNR_FN(CreateSwapchainKHR);
JNR_FN(DestroySwapchainKHR);
JNR_FN(GetSwapchainImagesKHR);
JNR_FN(CreateImageView);
JNR_FN(DestroyImageView);
JNR_FN(CreateShaderModule);
JNR_FN(DestroyShaderModule);
JNR_FN(CreatePipelineLayout);
JNR_FN(DestroyPipelineLayout);
JNR_FN(CreateGraphicsPipelines);
JNR_FN(DestroyPipeline);
JNR_FN(CreateCommandPool);
JNR_FN(DestroyCommandPool);
JNR_FN(AllocateCommandBuffers);
JNR_FN(ResetCommandPool);
JNR_FN(BeginCommandBuffer);
JNR_FN(EndCommandBuffer);
JNR_FN(CmdBeginRendering);
JNR_FN(CmdEndRendering);
JNR_FN(AcquireNextImageKHR);
JNR_FN(CreateFence);
JNR_FN(DestroyFence);
JNR_FN(CreateSemaphore);
JNR_FN(DestroySemaphore);
JNR_FN(DeviceWaitIdle);
JNR_FN(QueueSubmit);
JNR_FN(QueuePresentKHR);
JNR_FN(CmdPipelineBarrier);
JNR_FN(CmdClearColorImage);
JNR_FN(CmdDrawIndexed);
JNR_FN(CmdBindPipeline);
JNR_FN(CmdDraw);
JNR_FN(CmdSetViewport);
JNR_FN(CmdSetScissor);
JNR_FN(WaitForFences);
JNR_FN(ResetFences);
JNR_FN(CmdBindVertexBuffers);
JNR_FN(CmdCopyBuffer);
JNR_FN(CreateDescriptorPool);
JNR_FN(DestroyDescriptorPool);
JNR_FN(CreateDescriptorSetLayout);
JNR_FN(DestroyDescriptorSetLayout);
JNR_FN(AllocateDescriptorSets);
JNR_FN(UpdateDescriptorSets);
JNR_FN(CmdBindDescriptorSets);
JNR_FN(CmdPushConstants);
JNR_FN(CreateSampler);
JNR_FN(DestroySampler);
JNR_FN(CmdBindIndexBuffer);
JNR_FN(CreatePipelineCache);
JNR_FN(DestroyPipelineCache);
JNR_FN(GetPipelineCacheData);
JNR_FN(AllocateMemory);
JNR_FN(BindBufferMemory);
JNR_FN(BindImageMemory);
JNR_FN(CmdCopyBufferToImage);
JNR_FN(CreateBuffer);
JNR_FN(CreateImage);
JNR_FN(DestroyBuffer);
JNR_FN(DestroyImage);
JNR_FN(MapMemory);
JNR_FN(UnmapMemory);
JNR_FN(FlushMappedMemoryRanges);
JNR_FN(FreeCommandBuffers);
JNR_FN(FreeDescriptorSets);
JNR_FN(FreeMemory);
JNR_FN(GetBufferMemoryRequirements);
JNR_FN(GetImageMemoryRequirements);
JNR_FN(GetPhysicalDeviceMemoryProperties);
JNR_FN(CreateRenderPass);
JNR_FN(DestroyRenderPass);

// Instance
JNR_FN(DestroyInstance);
JNR_FN(CreateDebugUtilsMessengerEXT);
JNR_FN(DestroyDebugUtilsMessengerEXT);
JNR_FN(EnumeratePhysicalDevices);
JNR_FN(GetPhysicalDeviceProperties);
JNR_FN(GetPhysicalDeviceFeatures);
JNR_FN(GetPhysicalDeviceQueueFamilyProperties);
JNR_FN(EnumerateDeviceLayerProperties);
JNR_FN(CreateDevice);
JNR_FN(DestroyDevice);
JNR_FN(GetDeviceQueue);
JNR_FN(GetDeviceProcAddr);
JNR_FN(DestroySurfaceKHR);
JNR_FN(GetPhysicalDeviceSurfaceSupportKHR);
JNR_FN(EnumerateDeviceExtensionProperties);
JNR_FN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
JNR_FN(GetPhysicalDeviceSurfaceFormatsKHR);
JNR_FN(GetPhysicalDeviceSurfacePresentModesKHR);

// Special cases
JNR_FN(CreateInstance);
JNR_FN(GetInstanceProcAddr);
JNR_FN(EnumerateInstanceExtensionProperties);
JNR_FN(EnumerateInstanceLayerProperties);

void Vulkan::LoadFunctions()
{
    jnrGetInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress(NULL, "vkGetInstanceProcAddr"));
    CHECK(jnrGetInstanceProcAddr != nullptr) << "Unable to get a pointer to vkGetInstanceProcAddr";

    GET_INST_FN(CreateInstance, nullptr);
    GET_INST_FN(EnumerateInstanceLayerProperties, nullptr);
    GET_INST_FN(EnumerateInstanceExtensionProperties, nullptr);
}

void Vulkan::LoadFunctionsInstance(VkInstance instance)
{
    GET_INST_FN_OPT(CreateDebugUtilsMessengerEXT, instance);
    GET_INST_FN_OPT(DestroyDebugUtilsMessengerEXT, instance);

    GET_INST_FN(GetDeviceProcAddr, instance);
    GET_INST_FN(DestroyInstance, instance);
    GET_INST_FN(EnumeratePhysicalDevices, instance);
    GET_INST_FN(GetPhysicalDeviceProperties, instance);
    GET_INST_FN(GetPhysicalDeviceFeatures, instance);
    GET_INST_FN(GetPhysicalDeviceQueueFamilyProperties, instance);
    GET_INST_FN(EnumerateDeviceLayerProperties, instance);
    GET_INST_FN(CreateDevice, instance);
    GET_INST_FN(DestroyDevice, instance);
    GET_INST_FN(GetDeviceQueue, instance);
    GET_INST_FN(DestroySurfaceKHR, instance);
    GET_INST_FN(GetPhysicalDeviceSurfaceSupportKHR, instance);
    GET_INST_FN(EnumerateDeviceExtensionProperties, instance);
    GET_INST_FN(GetPhysicalDeviceSurfaceCapabilitiesKHR, instance);
    GET_INST_FN(GetPhysicalDeviceSurfaceFormatsKHR, instance);
    GET_INST_FN(GetPhysicalDeviceSurfacePresentModesKHR, instance);
    GET_INST_FN(GetPhysicalDeviceMemoryProperties, instance);
}

void Vulkan::LoadFunctionsDevice(VkDevice device)
{
    GET_DEV_FN(CreateSwapchainKHR, device);
    GET_DEV_FN(DestroySwapchainKHR, device);
    GET_DEV_FN(GetSwapchainImagesKHR, device);
    GET_DEV_FN(CreateImageView, device);
    GET_DEV_FN(DestroyImageView, device);
    GET_DEV_FN(CreateShaderModule, device);
    GET_DEV_FN(DestroyShaderModule, device);
    GET_DEV_FN(CreatePipelineLayout, device);
    GET_DEV_FN(DestroyPipelineLayout, device);
    GET_DEV_FN(CreateGraphicsPipelines, device);
    GET_DEV_FN(DestroyPipeline, device);
    GET_DEV_FN(CreateCommandPool, device);
    GET_DEV_FN(DestroyCommandPool, device);
    GET_DEV_FN(AllocateCommandBuffers, device);
    GET_DEV_FN(ResetCommandPool, device);
    GET_DEV_FN(BeginCommandBuffer, device);
    GET_DEV_FN(EndCommandBuffer, device);
    GET_DEV_FN_OPT(CmdBeginRendering, device);
    GET_DEV_FN_OPT(CmdEndRendering, device);
    GET_DEV_FN(AcquireNextImageKHR, device);
    GET_DEV_FN(CreateFence, device);
    GET_DEV_FN(DestroyFence, device);
    GET_DEV_FN(CreateSemaphore, device);
    GET_DEV_FN(DestroySemaphore, device);
    GET_DEV_FN(DeviceWaitIdle, device);
    GET_DEV_FN(QueueSubmit, device);
    GET_DEV_FN(QueuePresentKHR, device);
    GET_DEV_FN(CmdPipelineBarrier, device);
    GET_DEV_FN(CmdClearColorImage, device);
    GET_DEV_FN(CmdDrawIndexed, device);
    GET_DEV_FN(CmdBindPipeline, device);
    GET_DEV_FN(CmdDraw, device);
    GET_DEV_FN(CmdSetViewport, device);
    GET_DEV_FN(CmdSetScissor, device);
    GET_DEV_FN(WaitForFences, device);
    GET_DEV_FN(ResetFences, device);
    GET_DEV_FN(CmdBindVertexBuffers, device);
    GET_DEV_FN(CmdCopyBuffer, device);
    GET_DEV_FN(CreateDescriptorPool, device);
    GET_DEV_FN(DestroyDescriptorPool, device);
    GET_DEV_FN(CreateDescriptorSetLayout, device);
    GET_DEV_FN(DestroyDescriptorSetLayout, device);
    GET_DEV_FN(AllocateDescriptorSets, device);
    GET_DEV_FN(UpdateDescriptorSets, device);
    GET_DEV_FN(CmdBindDescriptorSets, device);
    GET_DEV_FN(CmdPushConstants, device);
    GET_DEV_FN(CreateSampler, device);
    GET_DEV_FN(DestroySampler, device);
    GET_DEV_FN(CmdBindIndexBuffer, device);
    GET_DEV_FN(CreatePipelineCache, device);
    GET_DEV_FN(DestroyPipelineCache, device);
    GET_DEV_FN(GetPipelineCacheData, device);
    GET_DEV_FN(AllocateMemory, device);
    GET_DEV_FN(BindBufferMemory, device);
    GET_DEV_FN(BindImageMemory, device);
    GET_DEV_FN(CmdCopyBufferToImage, device);
    GET_DEV_FN(CreateBuffer, device);
    GET_DEV_FN(CreateImage, device);
    GET_DEV_FN(DestroyBuffer, device);
    GET_DEV_FN(DestroyImage, device);
    GET_DEV_FN(MapMemory, device);
    GET_DEV_FN(UnmapMemory, device);
    GET_DEV_FN(FlushMappedMemoryRanges, device);
    GET_DEV_FN(FreeCommandBuffers, device);
    GET_DEV_FN(FreeDescriptorSets, device);
    GET_DEV_FN(FreeMemory, device);
    GET_DEV_FN(GetBufferMemoryRequirements, device);
    GET_DEV_FN(GetImageMemoryRequirements, device);
    GET_DEV_FN(CmdCopyBufferToImage, device);
    GET_DEV_FN(CreateRenderPass, device);
    GET_DEV_FN(DestroyRenderPass, device);
}

PFN_vkVoidFunction Vulkan::GetFunctionByName(char const *name, void *userData)
{
    DeviceInstance *deviceInstance = (DeviceInstance *)userData;
    PFN_vkVoidFunction returnValue = jnrGetDeviceProcAddr(deviceInstance->device, name);
    if (returnValue)
    {
        return returnValue;
    }
    return jnrGetInstanceProcAddr(deviceInstance->instance, name);
}
