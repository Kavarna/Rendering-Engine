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

JNR_FN(CreateInstance);
JNR_FN(GetInstanceProcAddr);
JNR_FN(DestroyInstance);

void Editor::LoadFunctions()
{
    jnrGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress(NULL, "vkGetInstanceProcAddr"));
    CHECK(jnrGetInstanceProcAddr != nullptr) << "Unable to get a pointer to vkGetInstanceProcAddr";

    GET_INST_FN(CreateInstance, nullptr);    
}

void Editor::LoadFunctionsInstance(VkInstance instance)
{
    GET_INST_FN(DestroyInstance, instance);
}
