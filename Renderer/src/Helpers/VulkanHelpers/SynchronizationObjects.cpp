#include "SynchronizationObjects.h"
#include "Editor/Renderer.h"

GPUSynchronizationObject::GPUSynchronizationObject()
{
    auto device = Editor::Renderer::Get()->GetDevice();
    VkSemaphoreCreateInfo semaphoreInfo{};
    {
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;
    }
    ThrowIfFailed(jnrCreateSemaphore(device, &semaphoreInfo, nullptr, &mSemaphore));

}

GPUSynchronizationObject::~GPUSynchronizationObject()
{
    auto device = Editor::Renderer::Get()->GetDevice();
    jnrDestroySemaphore(device, mSemaphore, nullptr);
}

VkSemaphore GPUSynchronizationObject::GetSemaphore()
{
    return mSemaphore;
}
