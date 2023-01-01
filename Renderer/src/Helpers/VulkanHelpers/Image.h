#pragma once

#include "Editor/VulkanLoader.h"
#include "vma/vk_mem_alloc.h"

class Image
{
public:
    Image(uint32_t width, uint32_t height, VmaAllocationCreateFlags allocationFlags = 0);

private:
    VkImage mImage;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    bool mMappable;
};

