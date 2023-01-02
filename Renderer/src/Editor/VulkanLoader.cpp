#include "VulkanLoader.h"
#include <GLFW/glfw3.h>

#define STRINGIFY_INT(a)                  #a
#define STRINGIFY(a)                      STRINGIFY_INT(a)
#define PFN(fn)                           PFN_vk##fn
#define GET_INST_FN_INT(pf, vk, in, inst) \
{ \
    in = reinterpret_cast<pf>(jnrGetInstanceProcAddr(inst, STRINGIFY(vk))); \
    CHECK(in != nullptr) << "Unable to get a pointer to " #vk; \
}
#define GET_INST_FN(fn, inst)             GET_INST_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_INST_FN_INT_OPT(pf, vk, in, inst) \
{ \
    in = reinterpret_cast<pf>(jnrGetInstanceProcAddr(inst, STRINGIFY(vk))); \
    if (in == nullptr) \
        LOG(WARNING) << "Unable to get a pointer to " #vk; \
}
#define GET_INST_FN_OPT(fn, inst)             GET_INST_FN_INT_OPT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_DEV_FN_INT(pf, vk, in, dev) \
{ \
    in = reinterpret_cast<pf>(jnrGetDeviceProcAddr(dev, STRINGIFY(vk))); \
    CHECK(in != nullptr) << "Unable to get a pointer to " #vk; \
}
#define GET_DEV_FN(fn, inst)             GET_DEV_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

#define GET_DEV_FN_INT_OPT(pf, vk, in, dev) \
{ \
    in = reinterpret_cast<pf>(jnrGetDeviceProcAddr(dev, STRINGIFY(vk))); \
    if (in == nullptr) \
        LOG(WARNING) << "Unable to get a pointer to " #vk; \
}
#define GET_DEV_FN_OPT(fn, inst)             GET_DEV_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

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

void Editor::LoadFunctions()
{
    jnrGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress(NULL, "vkGetInstanceProcAddr"));
    CHECK(jnrGetInstanceProcAddr != nullptr) << "Unable to get a pointer to vkGetInstanceProcAddr";

    GET_INST_FN(CreateInstance, nullptr);
    GET_INST_FN(EnumerateInstanceLayerProperties, nullptr);
    GET_INST_FN(EnumerateInstanceExtensionProperties, nullptr);
}

void Editor::LoadFunctionsInstance(VkInstance instance)
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
}

void Editor::LoadFunctionsDevice(VkDevice device)
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
    GET_DEV_FN(CmdBeginRendering, device);
    GET_DEV_FN(CmdEndRendering, device);
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
}