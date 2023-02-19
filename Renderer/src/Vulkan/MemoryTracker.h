#pragma once


#include "VulkanLoader.h"


namespace Vulkan
{
    class Buffer;

    class MemoryTracker
    {
    public:
        MemoryTracker();
        ~MemoryTracker();

    public:
        void AddBuffer(std::unique_ptr<Buffer>&& buffer);

        void Flush();

    private:
        std::vector<std::unique_ptr<Buffer>> mBuffers;

    };
}
