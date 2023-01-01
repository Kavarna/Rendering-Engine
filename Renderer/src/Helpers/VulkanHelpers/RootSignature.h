#pragma once


#include <vulkan/vulkan.h>
#include <vector>
#include "Buffer.h"


class DescriptorSet
{
    friend class RootSignature;
    friend class Editor::CommandList;
public:
    DescriptorSet();
    ~DescriptorSet();

    /* TODO: Should make this non-copyable */

    void AddInputBuffer(uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL);

    template <typename T>
    void AddInputBuffer(Buffer<T>* buffer, uint32_t binding, uint32_t elementIndex = 0, uint32_t instance = 0);

    /* Bake multiple instances */
    void Bake(uint32_t instances = 1);

private:
    VkDescriptorSetLayoutCreateInfo mLayoutInfo;
    VkDescriptorPoolCreateInfo mPoolInfo{};

    uint32_t mInputBufferCount = 0;

    std::vector<VkDescriptorSetLayoutBinding> mBindings;
    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;

    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> mDescriptorSets;
};


class RootSignature
{
    friend class Pipeline;
    friend class Editor::CommandList;
public:
    RootSignature();
    ~RootSignature();

    /* TODO: Should make this non-copyable */

public:
    void AddPushRange(uint32_t size, uint32_t offset, VkShaderStageFlags shaders);
    void AddDescriptorSet(DescriptorSet* descriptorSet);

    void Bake();

private:
    std::vector<VkPushConstantRange> mPushRanges;
    std::vector<DescriptorSet*> mDescriptorSetLayouts;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

};

template<typename T>
inline void DescriptorSet::AddInputBuffer(Buffer<T>* buffer, uint32_t binding, uint32_t elementIndex, uint32_t instance)
{
    auto device = Editor::Renderer::Get()->GetDevice();
    uint32_t dstArrayElement = buffer->mCount == 1 ? 0 : elementIndex;
    VkDescriptorBufferInfo bufferInfo{};
    {
        bufferInfo.buffer = buffer->mBuffer;
        bufferInfo.offset = sizeof(T) * dstArrayElement;
        bufferInfo.range = VK_WHOLE_SIZE; /* TDOO: This might give weird results, when trying to bind only an element from the buffer */
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
