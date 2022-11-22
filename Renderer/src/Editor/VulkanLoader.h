#pragma once

#include <Jnrlib.h>
#include <vulkan/vulkan.h>


#define JNR_FN(FN) PFN_vk##FN jnr##FN

// Device


// Instance
extern JNR_FN(DestroyInstance);

// Special cases
extern JNR_FN(CreateInstance);
extern JNR_FN(GetInstanceProcAddr);


namespace Editor
{
    void LoadFunctions();
    void LoadFunctionsInstance(VkInstance);
}