#pragma once


#include "Editor/VulkanLoader.h"

class Image;

class LayoutTracker
{
public:
    LayoutTracker();
    ~LayoutTracker();

public:
    void TransitionBackBufferImage(uint32_t index, VkImageLayout newLayout);
    void TransitionImage(Image* img, VkImageLayout newLayout);

    VkImageLayout GetBackbufferImageLayout(uint32_t index);
    VkImageLayout GetImageLayout(Image* img);

    void Flush();

private:
    std::unordered_map<Image*, VkImageLayout> mImageLayouts;
    std::unordered_map<uint32_t, VkImageLayout> mBackbufferLayouts;

};