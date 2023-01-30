#include "RootSignature.h"
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

void DescriptorSet::AddInputBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    {
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    }

    mBindings.push_back(layoutBinding);

    mInputBufferCount++;
}

void DescriptorSet::Bake(uint32_t instances)
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

    VkDescriptorPoolSize sizes[1] = {};
    VkDescriptorPoolSize& inputBufferSize = sizes[0];
    {
        inputBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        inputBufferSize.descriptorCount = mInputBufferCount;
    }
    VkDescriptorPoolCreateInfo& poolInfo = mPoolInfo;
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = 0;
        poolInfo.maxSets = (uint32_t)mBindings.size() * instances;
        poolInfo.poolSizeCount = sizeof(sizes) / sizeof(sizes[0]);
        poolInfo.pPoolSizes = sizes;
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
