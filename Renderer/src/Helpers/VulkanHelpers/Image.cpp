#include "Image.h"

#include "Editor/Renderer.h"

Image::Image(uint32_t width, uint32_t height, VmaAllocationCreateFlags allocationFlags)
{
    mMappable = ((allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0 ||
                 (allocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0);

    auto allocator = Editor::Renderer::Get()->GetAllocator();

    VkImageCreateInfo imageInfo{};
    {
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.usage;
    }

    VmaAllocationCreateInfo allocationInfo{};
    {
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = allocationFlags;
    }

    ThrowIfFailed(
        vmaCreateImage(allocator, &imageInfo, &allocationInfo, &mImage, &mAllocation, &mAllocationInfo)
    );
}
