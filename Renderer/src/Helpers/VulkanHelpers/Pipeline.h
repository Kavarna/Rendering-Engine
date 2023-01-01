#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <vector>

namespace Editor
{
    class CommandList;
}
class RootSignature;

class Pipeline
{
    friend class Editor::CommandList;
public:
    Pipeline(std::string const& name);
    ~Pipeline();

public:
    void AddShader(std::string const& path);

    void SetRootSignature(RootSignature const* rootSignature);

    /* TODO: Add color outputs by taking an image as a parameter */
    void AddBackbufferColorOutput();
    void SetBackbufferDepthStencilOutput();

    void Bake();
    

public:
    VkPipelineColorBlendStateCreateInfo& GetColorBlendStateCreateInfo()
    {
        return mBlendState;
    }
    VkPipelineDepthStencilStateCreateInfo& GetDepthStencilStateCreateInfo()
    {
        return mDepthStencilState;
    }
    VkPipelineDynamicStateCreateInfo& GetDynamicStateCreateInfo()
    {
        return mDynamicState;
    }
    VkPipelineInputAssemblyStateCreateInfo& GetInputAssemblyStateCreateInfo()
    {
        return mInputAssemblyState;
    }
    VkPipelineMultisampleStateCreateInfo& GetMultisampleStateCreateInfo()
    {
        return mMultisampleState;
    }
    VkPipelineRasterizationStateCreateInfo& GetRasterizationStateCreateInfo()
    {
        return mRasterizationState;
    }
    VkPipelineTessellationStateCreateInfo& GetTessellationStateCreateInfo()
    {
        return mTesselationState;
    }
    VkPipelineVertexInputStateCreateInfo& GetVertexInputStateCreateInfo()
    {
        return mVertexInputState;
    }
    VkPipelineViewportStateCreateInfo& GetViewportStateCreateInfo()
    {
        return mViewportState;
    }

private:
    void InitDefaultPipelineState();

private:
    std::string mName;

    RootSignature const* mRootSignature;

    std::vector<VkFormat> mColorOutputs;
    VkFormat mDepthFormat;
    VkFormat mStencilFormat;
    std::vector<VkPipelineShaderStageCreateInfo> mShaderModules;

    VkPipelineColorBlendStateCreateInfo mBlendState{};
    VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
    VkPipelineDynamicStateCreateInfo mDynamicState{};
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState{};
    VkPipelineMultisampleStateCreateInfo mMultisampleState{};
    VkPipelineRasterizationStateCreateInfo mRasterizationState{};
    VkPipelineTessellationStateCreateInfo mTesselationState{};
    VkPipelineVertexInputStateCreateInfo mVertexInputState{};
    VkPipelineViewportStateCreateInfo mViewportState{};

    VkPipelineRenderingCreateInfo mRenderingInfo{};
    VkGraphicsPipelineCreateInfo mPipelineInfo{};

    VkPipeline mPipeline = VK_NULL_HANDLE;

};
