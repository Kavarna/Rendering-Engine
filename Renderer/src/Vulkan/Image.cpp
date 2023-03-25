#include "Image.h"

#include "Renderer.h"

#include "ImGuiImplementation.h"

using namespace Vulkan;

Image::Image(Info2D const& info):
    mQueueFamilies(info.queueFamilies),
    mFormat(info.format),
    mImageType(VK_IMAGE_TYPE_2D)
{
    mMappable = ((info.allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0 ||
                 (info.allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

    auto allocator = Renderer::Get()->GetAllocator();
    mExtent2D.width = info.width;
    mExtent2D.height = info.height;

    VkImageCreateInfo& imageInfo = mCreateInfo;
    {
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = 0;
        imageInfo.extent = {
            .width = info.width,
            .height = info.height,
            .depth = 1
        };
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.samples = info.samples;
        imageInfo.sharingMode = mQueueFamilies.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.pQueueFamilyIndices = mQueueFamilies.data();
        imageInfo.queueFamilyIndexCount = (uint32_t)mQueueFamilies.size();
        imageInfo.usage = info.usage;
        imageInfo.format = info.format;
        imageInfo.initialLayout = info.initialLayout;
    }

    VmaAllocationCreateInfo allocationInfo{};
    {
        allocationInfo.usage = info.memoryUsage;
        allocationInfo.flags = info.allocationFlags;
    }

    ThrowIfFailed(
        vmaCreateImage(allocator, &imageInfo, &allocationInfo, &mImage, &mAllocation, &mAllocationInfo)
    );

    mLayout = info.initialLayout;

    if (mMappable)
    {
        auto allocator = Renderer::Get()->GetAllocator();

        vmaMapMemory(allocator, mAllocation, (void**)&mData);
    }
}

Image::~Image()
{
    auto device = Renderer::Get()->GetDevice();
    auto allocator = Renderer::Get()->GetAllocator();

    for (const auto& it : mImageViews)
    {
        jnrDestroyImageView(device, it.second, nullptr);
    }
    mImageViews.clear();

    if (mMappable)
    {
        auto allocator = Renderer::Get()->GetAllocator();
        vmaUnmapMemory(allocator, mAllocation);
    }

    if (mImguiTextureID)
    {
        ImGui_ImplVulkan_RemoveTexture(mImguiTextureID);
    }

    vmaDestroyImage(allocator, mImage, mAllocation);
}

VkImageView Image::GetImageView(VkImageAspectFlags aspectMask)
{
    if (auto it = mImageViews.find(aspectMask); it != mImageViews.end())
    {
        return it->second;
    }
    auto renderer = Renderer::Get();
    auto device = renderer->GetDevice();
    VkImageViewCreateInfo viewInfo = {};
    {
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.image = mImage;
        viewInfo.format = mFormat;
        viewInfo.components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A
        };
        viewInfo.subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
    }

    VkImageView imageView;
    ThrowIfFailed(
        jnrCreateImageView(device, &viewInfo, nullptr, &imageView)
    );
    mImageViews[aspectMask] = imageView;

    return imageView;
}

VkFormat Image::GetFormat() const
{
    return mFormat;
}

void Image::SetPixelColor(uint32_t x, uint32_t y, Jnrlib::Color const& color)
{
    CHECK(mData) << "Cannot set pixel color for an unmappable image";

    CHECK(x < mCreateInfo.extent.width) << "X coordinate must be less than width";
    CHECK(y < mCreateInfo.extent.height) << "Y coordinate must be less than height";

    if (mCreateInfo.format == VK_FORMAT_R32G32B32A32_SFLOAT)
    {
        glm::vec4* image = (glm::vec4*)mData;
        image[y * mCreateInfo.extent.width + x] = color;
    }
    else
    {
        CHECK(false) << "Invalid format used to set pixel color";
    }
}

VkExtent2D Image::GetExtent2D() const
{
    return mExtent2D;
}

ImTextureID Image::GetTextureID()
{
    if (mImguiTextureID != VK_NULL_HANDLE)
        return mImguiTextureID;
    auto renderer = Renderer::Get();
    VkSampler sampler = renderer->GetPointSampler();
    mImguiTextureID = ImGui_ImplVulkan_AddTexture(
        sampler, GetImageView(VK_IMAGE_ASPECT_COLOR_BIT), IMGUI_IMAGE_LAYOUT);
    return (ImTextureID)mImguiTextureID;
}
