#include "RootSignature.h"
#include "Image.h"
#include "VulkanLoader.h"
#include "Renderer.h"

using namespace Vulkan;

RootSignature::RootSignature()
{ }

RootSignature::~RootSignature()
{
    auto device = Renderer::Get()->GetDevice();
    if (mPipelineLayout != VK_NULL_HANDLE)
    {
        jnrDestroyPipelineLayout(device, mPipelineLayout, nullptr);
    }
}

void RootSignature::AddDescriptorSet(DescriptorSet* descriptorSet)
{
    mDescriptorSetLayouts.push_back(descriptorSet);
}

void RootSignature::Bake()
{
    auto device = Renderer::Get()->GetDevice();

    std::vector<VkDescriptorSetLayout> descriptorSets;
    for (const auto& descriptorSet : mDescriptorSetLayouts)
    {
        descriptorSets.push_back(descriptorSet->mLayout);
    }
    VkPipelineLayoutCreateInfo layoutInfo{};
    {
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.flags = 0;
        layoutInfo.pushConstantRangeCount = (uint32_t)mPushRanges.size();
        layoutInfo.pPushConstantRanges = mPushRanges.data();
        layoutInfo.setLayoutCount = (uint32_t)descriptorSets.size();
        layoutInfo.pSetLayouts = descriptorSets.data();
    }
    
    ThrowIfFailed(
        jnrCreatePipelineLayout(device, &layoutInfo, nullptr, &mPipelineLayout)
    );
}

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::~DescriptorSet()
{
    auto device = Renderer::Get()->GetDevice();
    if (mLayout != VK_NULL_HANDLE)
    {
        jnrDestroyDescriptorSetLayout(device, mLayout, nullptr);
    }
    if (mDescriptorPool != VK_NULL_HANDLE)
    {
        jnrDestroyDescriptorPool(device, mDescriptorPool, nullptr);
    }
}

void DescriptorSet::AddSampler(uint32_t binding, std::vector<VkSampler> const& samplers, VkShaderStageFlags stages)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    {
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = (uint32_t)samplers.size();
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        layoutBinding.pImmutableSamplers = samplers.data();
        layoutBinding.stageFlags = stages;
    }

    mBindings.push_back(layoutBinding);

    mSamplerCount++;
}

void DescriptorSet::AddCombinedImageSampler(uint32_t binding, VkSampler* sampler, VkShaderStageFlags stages)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    {
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.pImmutableSamplers = sampler;
        layoutBinding.stageFlags = stages;
    }

    mBindings.push_back(layoutBinding);

    mCombinedImageSamplerCount++;
}

void DescriptorSet::BindCombinedImageSampler(uint32_t binding, Vulkan::Image* image, VkImageAspectFlags aspectFlags, VkSampler sampler, uint32_t instance)
{
    auto device = Renderer::Get()->GetDevice();
    auto imageView = image->GetImageView(aspectFlags);
    BindCombinedImageSampler(binding, imageView, aspectFlags, sampler, instance);
}

void DescriptorSet::BindCombinedImageSampler(uint32_t binding, Vulkan::ImageView image, VkImageAspectFlags aspectFlags, VkSampler sampler, uint32_t instance)
{
    auto device = Renderer::Get()->GetDevice();
    VkDescriptorImageInfo imageInfo{};
    {
        imageInfo.sampler = sampler;
        imageInfo.imageLayout = image.GetLayout();
        imageInfo.imageView = image.GetView();
    }
    VkWriteDescriptorSet writeDescriptorSet{};
    {
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstSet = mDescriptorSets[instance];
        writeDescriptorSet.pImageInfo = &imageInfo;
    }
    jnrUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::AddStorageBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    {
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = stages;
    }

    mBindings.push_back(layoutBinding);

    mStorageBufferCount++;
}

void DescriptorSet::BindStorageBuffer(Vulkan::Buffer* buffer, uint32_t binding, uint32_t elementIndex, uint32_t instance)
{
    auto device = Renderer::Get()->GetDevice();
    uint32_t dstArrayElement = buffer->mCount == 1 ? 0 : elementIndex;
    VkDescriptorBufferInfo bufferInfo{};
    {
        bufferInfo.buffer = buffer->mBuffer;
        bufferInfo.offset = buffer->GetElementSize() * dstArrayElement;
        bufferInfo.range = VK_WHOLE_SIZE; /* TODO: This might give weird results, when trying to bind only an element from the buffer */
    }
    VkWriteDescriptorSet writeDescriptorSet{};
    {
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.dstArrayElement = dstArrayElement;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstSet = mDescriptorSets[instance];
        writeDescriptorSet.pBufferInfo = &bufferInfo;
    }

    jnrUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::AddInputBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    {
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = stages;
    }

    mBindings.push_back(layoutBinding);

    mInputBufferCount++;
}

void DescriptorSet::BindUniformBuffer(Vulkan::Buffer* buffer, uint32_t binding, uint32_t elementIndex, uint32_t instance)
{
    auto device = Renderer::Get()->GetDevice();
    uint32_t dstArrayElement = buffer->mCount == 1 ? 0 : elementIndex;
    VkDescriptorBufferInfo bufferInfo{};
    {
        bufferInfo.buffer = buffer->mBuffer;
        bufferInfo.offset = buffer->GetElementSize() * dstArrayElement;
        bufferInfo.range = VK_WHOLE_SIZE; /* TODO: This might give weird results, when trying to bind only an element from the buffer */
    }
    VkWriteDescriptorSet writeDescriptorSet{};
    {
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.dstArrayElement = dstArrayElement;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstSet = mDescriptorSets[instance];
        writeDescriptorSet.pBufferInfo = &bufferInfo;
    }

    jnrUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}


void DescriptorSet::BakeLayout()
{
    auto device = Renderer::Get()->GetDevice();

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    {
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pBindings = mBindings.data();
        layoutInfo.bindingCount = (uint32_t)mBindings.size();
        layoutInfo.flags = 0;
    }
    ThrowIfFailed(
        jnrCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mLayout)
    );
}

void DescriptorSet::Bake(uint32_t instances)
{
    auto device = Renderer::Get()->GetDevice();

    if (mLayout == VK_NULL_HANDLE)
        BakeLayout();

    std::vector<VkDescriptorPoolSize> sizes = {};
    if (mInputBufferCount != 0)
    {
        VkDescriptorPoolSize& inputBufferSize = sizes.emplace_back();
        {
            inputBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            inputBufferSize.descriptorCount = mInputBufferCount;
        }
    }
    if (mStorageBufferCount != 0)
    {
        VkDescriptorPoolSize& inputBufferSize = sizes.emplace_back();
        {
            inputBufferSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            inputBufferSize.descriptorCount = mStorageBufferCount;
        }
    }
    if (mSamplerCount != 0)
    {
        VkDescriptorPoolSize& inputBufferSize = sizes.emplace_back();
        {
            inputBufferSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
            inputBufferSize.descriptorCount = mSamplerCount;
        }
    }
    if (mCombinedImageSamplerCount != 0)
    {
        VkDescriptorPoolSize& inputBufferSize = sizes.emplace_back();
        {
            inputBufferSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            inputBufferSize.descriptorCount = mCombinedImageSamplerCount;
        }
    }
    VkDescriptorPoolCreateInfo& poolInfo = mPoolInfo;
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = 0;
        poolInfo.maxSets = (uint32_t)mBindings.size() * instances;
        poolInfo.poolSizeCount = (uint32_t)sizes.size();
        poolInfo.pPoolSizes = sizes.data();
    }
    ThrowIfFailed(
        jnrCreateDescriptorPool(device, &poolInfo, nullptr, &mDescriptorPool)
    );

    mDescriptorSets.resize(mPoolInfo.maxSets); /* Make space for descriptor sets */
    std::vector<VkDescriptorSetLayout> layouts(mPoolInfo.maxSets, mLayout); /* Have enought layouts */
    VkDescriptorSetAllocateInfo allocInfo{};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;
        allocInfo.descriptorSetCount = mPoolInfo.maxSets;
        allocInfo.pSetLayouts = layouts.data();
    }
    jnrAllocateDescriptorSets(device, &allocInfo, mDescriptorSets.data());
}
