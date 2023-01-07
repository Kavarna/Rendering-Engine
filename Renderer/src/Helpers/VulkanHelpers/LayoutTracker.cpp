#include "LayoutTracker.h"
#include "Editor/Renderer.h"

#include "Image.h"

constexpr const uint32_t PREALLOCATED_ENTRIES = 32;

LayoutTracker::LayoutTracker()
{
    mImageLayouts.reserve(PREALLOCATED_ENTRIES);
}

LayoutTracker::~LayoutTracker()
{ }

void LayoutTracker::TransitionBackBufferImage(uint32_t index, VkImageLayout newLayout)
{
    mBackbufferLayouts[index] = newLayout;
}

void LayoutTracker::TransitionImage(Image * img, VkImageLayout newLayout)
{
    mImageLayouts[img] = newLayout;
}

VkImageLayout LayoutTracker::GetBackbufferImageLayout(uint32_t index)
{
    if (auto it = mBackbufferLayouts.find(index); it != mBackbufferLayouts.end())
        return it->second;

    auto renderer = Editor::Renderer::Get();
    mBackbufferLayouts[index] = renderer->mSwapchainImageLayouts[index];
    return renderer->mSwapchainImageLayouts[index];
}

VkImageLayout LayoutTracker::GetImageLayout(Image* img)
{
    if (auto it = mImageLayouts.find(img); it != mImageLayouts.end())
        return it->second;

    mImageLayouts[img] = img->mLayout;
    return img->mLayout;
}

void LayoutTracker::Flush()
{
    auto renderer = Editor::Renderer::Get();
    for (auto& imageLayout : mImageLayouts)
    {
        auto& [image, layout] = imageLayout;
        image->mLayout = layout;
    }
    mImageLayouts.clear();
    for (auto& backbufferLayout : mBackbufferLayouts)
    {
        auto& [index, layout] = backbufferLayout;
        renderer->mSwapchainImageLayouts[index] = layout;
    }
    mBackbufferLayouts.clear();
}
