#pragma once


#include "VulkanLoader.h"


namespace Vulkan
{
    class Buffer;
    class Image;

    class MemoryTracker
    {
    public:
        MemoryTracker();
        ~MemoryTracker();

    public:
        void AddBuffer(std::unique_ptr<Buffer>&& buffer);
        void AddImage(std::unique_ptr<Image>&& image);

        void Flush();

    private:
        std::vector<std::unique_ptr<Buffer>> mBuffers;
        std::vector<std::unique_ptr<Image>> mImages;

    };
}
