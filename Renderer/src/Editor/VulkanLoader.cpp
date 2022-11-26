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

#define GET_DEV_FN_INT(pf, vk, in, dev) \
{ \
    in = reinterpret_cast<pf>(jnrGetDeviceProcAddr(dev, STRINGIFY(vk))); \
    CHECK(in != nullptr) << "Unable to get a pointer to " #vk; \
}
#define GET_DEV_FN(fn, inst)             GET_DEV_FN_INT(PFN(fn), vk##fn, jnr##fn, inst)

// Device
JNR_FN(CreateCommandPool);

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
    GET_INST_FN(GetDeviceProcAddr, instance);
    GET_INST_FN(DestroyInstance, instance);
    GET_INST_FN(CreateDebugUtilsMessengerEXT, instance);
    GET_INST_FN(DestroyDebugUtilsMessengerEXT, instance);
    GET_INST_FN(EnumeratePhysicalDevices, instance);
    GET_INST_FN(GetPhysicalDeviceProperties, instance);
    GET_INST_FN(GetPhysicalDeviceFeatures, instance);
    GET_INST_FN(GetPhysicalDeviceQueueFamilyProperties, instance);
    GET_INST_FN(EnumerateDeviceLayerProperties, instance);
    GET_INST_FN(CreateDevice, instance);
    GET_INST_FN(DestroyDevice, instance);
    GET_INST_FN(GetDeviceQueue, instance);
}

void Editor::LoadFunctionsDevice(VkDevice device)
{
    GET_DEV_FN(CreateCommandPool, device)
}