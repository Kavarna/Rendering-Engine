#include "Renderer.h"
#include "VulkanLoader.h"

Editor::Renderer::Renderer()
{
    Editor::LoadFunctions();    
    InitInstance();

    LOG(INFO) << "Vulkan renderer initialised successfully";
}

Editor::Renderer::~Renderer()
{
    jnrDestroyInstance(mInstance, nullptr);

    LOG(INFO) << "Vulkan renderer uninitialised successfully";
}

void Editor::Renderer::InitInstance()
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

    VkInstanceCreateInfo instanceInfo = {};
    {
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.enabledExtensionCount = 0;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.flags = 0;
        instanceInfo.pApplicationInfo = &appInfo;
    }

    VkResult res = jnrCreateInstance(&instanceInfo, nullptr, &mInstance);
    CHECK(res == VK_SUCCESS) << "Unable to create a vulkan instance";

    LoadFunctionsInstance(mInstance);
}
