#pragma once


#include "Editor/VulkanLoader.h"
#include "VulkanHelpers.h"
#include "Editor/Renderer.h"


template <typename T>
class Buffer
{
    friend class Editor::CommandList;
public:
    Buffer(uint64_t count, VkBufferUsageFlags usage, VmaAllocationCreateFlags allocationFlags = 0) :
        mCount(count)
    {
        mMappable = ((allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0 ||
                     (allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

        auto allocator = Editor::Renderer::Get()->GetAllocator();

        VkBufferCreateInfo bufferInfo{};
        {
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.queueFamilyIndexCount = 1; /* TODO: If needed improve this */
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.size = sizeof(T) * count;
            bufferInfo.usage = usage;
        }


        VmaAllocationCreateInfo allocationInfo{};
        {
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags = allocationFlags;
        }

        ThrowIfFailed(
            vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &mBuffer, &mAllocation, &mAllocationInfo)
        );
    }

    void Copy(T* src)
    {
        CHECK(mMappable) << "In order to copy into a buffer, it must be mappable";
        
        auto allocator = Editor::Renderer::Get()->GetAllocator();

        void* data = nullptr;
        vmaMapMemory(allocator, mAllocation, &data);
        memcpy(data, src, sizeof(T) * mCount);
        vmaUnmapMemory(allocator, mAllocation);
    }

    ~Buffer()
    {
        auto allocator = Editor::Renderer::Get()->GetAllocator();

        vmaDestroyBuffer(allocator, mBuffer, mAllocation);
    }

private:
    VkBuffer mBuffer;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    uint64_t mCount;
    bool mMappable;
};

