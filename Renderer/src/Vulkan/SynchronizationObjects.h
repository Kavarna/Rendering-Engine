#pragma once


#include "VulkanLoader.h"

namespace Vulkan
{
    class GPUSynchronizationObject
    {
    public:
        GPUSynchronizationObject();
        ~GPUSynchronizationObject();

    public:
        VkSemaphore GetSemaphore();

    private:
        VkSemaphore mSemaphore;
    };


    class CPUSynchronizationObject
    {
    public:
        CPUSynchronizationObject(bool signaled = false);
        ~CPUSynchronizationObject();

    public:
        VkFence GetFence();

        /// <summary>
        /// Wait for this fence to finish. Note: If batches are needed, this function should not be used
        /// </summary>
        void Wait();

        void WaitAndReset();

        void Reset();

    private:
        VkFence mFence;

    };

}