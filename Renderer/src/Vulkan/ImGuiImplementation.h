#pragma once

#include <stdint.h>
#include <imgui.h>
#include <vulkan/vulkan.h>

#include "backends/imgui_impl_glfw.h"

namespace Vulkan
{
    class CommandList;
    class Image;
    class ImageView;
}

struct ImGui_ImplVulkan_InitInfo
{
    VkDescriptorPool                DescriptorPool;
    uint32_t                        ImageCount;             // >= MinImageCount
    VkSampleCountFlagBits           MSAASamples;            // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
    VkFormat                        BackbufferFormat;
};

IMGUI_IMPL_API bool         ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info);
IMGUI_IMPL_API void         ImGui_ImplVulkan_Shutdown();
IMGUI_IMPL_API void         ImGui_ImplVulkan_NewFrame(Vulkan::CommandList* cmdList);
IMGUI_IMPL_API void         ImGui_ImplVulkan_RenderDrawData(ImDrawData* drawData, Vulkan::CommandList* cmdList);
IMGUI_IMPL_API bool         ImGui_ImplVulkan_CreateFontsTexture(Vulkan::CommandList* cmdList);
IMGUI_IMPL_API ImTextureID  ImGui_ImplVulkan_AddTexture(VkSampler sampler, Vulkan::ImageView imageView);
