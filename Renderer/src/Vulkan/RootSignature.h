#pragma once


#include <vulkan/vulkan.h>
#include <vector>
#include "Buffer.h"

namespace Vulkan
{
    class Image;
    class ImageView;

    class DescriptorSet
    {
        friend class RootSignature;
        friend class CommandList;

    public:
        DescriptorSet();
        ~DescriptorSet();

        /* Make this non-copyable */
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet& operator=(const DescriptorSet&) = delete;

        void AddSampler(uint32_t binding, std::vector<VkSampler> const& samplers, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);

        void AddCombinedImageSampler(uint32_t binding, VkSampler* sampler, VkShaderStageFlags stages);
        void BindCombinedImageSampler(uint32_t binding, Vulkan::Image* image, VkImageAspectFlags aspectFlags, VkSampler sampler, uint32_t instance = 0);
        void BindCombinedImageSampler(uint32_t binding, Vulkan::ImageView image, VkImageAspectFlags aspectFlags, VkSampler sampler, uint32_t instance = 0);

        void AddStorageBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
        void BindStorageBuffer(Vulkan::Buffer* buffer, uint32_t binding, uint32_t elementIndex = 0, uint32_t instance = 0);

        void AddInputBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
        void BindInputBuffer(Vulkan::Buffer* buffer, uint32_t binding, uint32_t elementIndex = 0, uint32_t instance = 0);

        void BakeLayout();
        /* Bake multiple instances */
        void Bake(uint32_t instances = 1);

    private:
        VkDescriptorSetLayoutCreateInfo mLayoutInfo{};
        VkDescriptorPoolCreateInfo mPoolInfo{};

        uint32_t mInputBufferCount = 0;
        uint32_t mStorageBufferCount = 0;
        uint32_t mSamplerCount = 0;
        uint32_t mCombinedImageSamplerCount = 0;

        std::vector<VkDescriptorSetLayoutBinding> mBindings;
        VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;

        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> mDescriptorSets;
    };

    class RootSignature
    {
        friend class Pipeline;
        friend class CommandList;
    public:
        RootSignature();
        ~RootSignature();

        /* TODO: Should make this non-copyable */

    public:
        template <typename T>
        void AddPushRange(uint32_t offset, uint32_t count = 1, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);
        void AddDescriptorSet(DescriptorSet* descriptorSet);

        void Bake();

    private:
        std::vector<VkPushConstantRange> mPushRanges;
        std::vector<DescriptorSet*> mDescriptorSetLayouts;

        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

    };

}

template<typename T>
inline void Vulkan::RootSignature::AddPushRange(uint32_t offset, uint32_t count, VkShaderStageFlags stages)
{
    VkPushConstantRange pushRange{};
    {
        pushRange.offset = offset;
        pushRange.size = count * sizeof(T);
        pushRange.stageFlags = stages;
    }
    mPushRanges.push_back(pushRange);
}
