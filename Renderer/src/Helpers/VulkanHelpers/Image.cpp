#include "Image.h"

#include "Editor/Renderer.h"

Image::Image(Info2D const& info):
    mQueueFamilies(info.queueFamilies)
{
    mMappable = ((info.allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0 ||
                 (info.allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

    auto allocator = Editor::Renderer::Get()->GetAllocator();

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
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = info.allocationFlags;
    }

    ThrowIfFailed(
        vmaCreateImage(allocator, &imageInfo, &allocationInfo, &mImage, &mAllocation, &mAllocationInfo)
    );

    mLayout = info.initialLayout;
}

Image::~Image()
{
    auto allocator = Editor::Renderer::Get()->GetAllocator();

    vmaDestroyImage(allocator, mImage, mAllocation);
}