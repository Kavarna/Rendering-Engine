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

CPUSynchronizationObject::CPUSynchronizationObject(bool signaled)
{
    auto device = Editor::Renderer::Get()->GetDevice();
    VkFenceCreateInfo fenceInfo{};
    {
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    }
    ThrowIfFailed(jnrCreateFence(device, &fenceInfo, nullptr, &mFence));
}

CPUSynchronizationObject::~CPUSynchronizationObject()
{
    auto device = Editor::Renderer::Get()->GetDevice();
    jnrDestroyFence(device, mFence, nullptr);
}

VkFence CPUSynchronizationObject::GetFence()
{
    return mFence;
}

void CPUSynchronizationObject::Wait()
{
    auto device = Editor::Renderer::Get()->GetDevice();
    ThrowIfFailed(jnrWaitForFences(device, 1, &mFence, VK_TRUE, UINT64_MAX));
    
}

void CPUSynchronizationObject::WaitAndReset()
{
    Wait();
    Reset();
}

void CPUSynchronizationObject::Reset()
{
    auto device = Editor::Renderer::Get()->GetDevice();
    ThrowIfFailed(jnrResetFences(device, 1, &mFence));
}
