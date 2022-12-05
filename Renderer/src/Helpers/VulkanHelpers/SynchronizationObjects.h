#pragma once


#include "Editor/VulkanLoader.h"

class GPUSynchronizationObject
{
public:
    GPUSynchronizationObject();
    ~GPUSynchronizationObject();

public:
    VkSemaphore GetSemaphore();

private:
    VkSemaphore mSemaphore;
};

