#pragma once

#include "Editor/VulkanLoader.h"
#include "vma/vk_mem_alloc.h"

class Image
{
    friend class LayoutTracker;
public:
    struct Info2D
    {
        uint32_t width, height;
        VkImageUsageFlags usage;
        VkFormat format;
        VkImageLayout initialLayout;

        VmaAllocationCreateFlags allocationFlags = 0;
        std::vector<uint32_t> queueFamilies{};
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    };
public:
    Image(Info2D const& info);
    ~Image();

private:
    VkImageCreateInfo mCreateInfo{};
    VkImage mImage;
    
    std::vector<uint32_t> mQueueFamilies;

    VkImageLayout mLayout;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    bool mMappable;
};

