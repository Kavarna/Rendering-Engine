#pragma once

#include "Editor/VulkanLoader.h"
#include "vma/vk_mem_alloc.h"
#include "imgui.h"

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
    static constexpr const VkImageLayout IMGUI_IMAGE_LAYOUT = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

    ImTextureID GetTextureID();

private:
    VkImageCreateInfo mCreateInfo{};
    VkImage mImage;

    /* Pretty much used only to be working with the default back-end */
    VkDescriptorSet mImguiTextureID = VK_NULL_HANDLE;

    std::unordered_map<VkImageAspectFlags, VkImageView> mImageViews;
    VkImageLayout mLayout;

    std::vector<uint32_t> mQueueFamilies;
    VkFormat mFormat;
    VkImageType mImageType;
    VkExtent2D mExtent2D;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    bool mMappable;
};

