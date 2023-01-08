#pragma once

#include "Editor/VulkanLoader.h"
#include "vma/vk_mem_alloc.h"

namespace Editor
{
    class CommandList;
}
class LayoutTracker;


class Image
{
    friend class LayoutTracker;
    friend class Editor::CommandList;
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

    VkImageView GetImageView(VkImageAspectFlags aspectMask);
    VkExtent2D GetExtent2D();

private:
    VkImageCreateInfo mCreateInfo{};
    VkImage mImage;

    std::unordered_map<VkImageAspectFlags, VkImageView> mImageViews;
    
    std::vector<uint32_t> mQueueFamilies;

    VkFormat mFormat;
    VkImageType mImageType;

    VkExtent2D mExtent2D;

    VkImageLayout mLayout;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    bool mMappable;
};

