#pragma once


#include "VulkanLoader.h"
#include "VulkanHelpers.h"
#include "Renderer.h"


namespace Vulkan
{
    template <typename T>
    class Buffer
    {
        friend class CommandList;
        friend class DescriptorSet;
    public:
        Buffer(uint64_t count, VkBufferUsageFlags usage, VmaAllocationCreateFlags allocationFlags = 0) :
            mCount(count)
        {
            bool mappable = ((allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0 ||
                             (allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

            auto allocator = Renderer::Get()->GetAllocator();

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

            if (mappable)
            {
                auto allocator = Renderer::Get()->GetAllocator();

                vmaMapMemory(allocator, mAllocation, (void**)&mData);
            }
        }

        void Copy(T* src)
        {
            CHECK(mData) << "In order to copy into a buffer, it must be mappable";
            memcpy(mData, src, sizeof(T) * mCount);
        }

        T* GetElement(uint32_t index = 0)
        {
            CHECK(mData) << "In order to get the address of an element inside the buffer, it must be mappable";
            return &mData[index];
        }

        ~Buffer()
        {
            auto allocator = Renderer::Get()->GetAllocator();

            if (mData != nullptr)
            {
                auto allocator = Renderer::Get()->GetAllocator();
                vmaUnmapMemory(allocator, mAllocation);
            }

            vmaDestroyBuffer(allocator, mBuffer, mAllocation);
        }

    private:
        VkBuffer mBuffer;
        VmaAllocation mAllocation;
        VmaAllocationInfo mAllocationInfo;

        uint64_t mCount;
        T* mData = nullptr;
    };

}