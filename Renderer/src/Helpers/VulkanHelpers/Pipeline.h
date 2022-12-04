#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <vector>

class Pipeline
{
public:
    Pipeline(std::string const& name);
    ~Pipeline();

public:
    void AddShader(std::string const& path);
    /* TODO: Add these functions 
    void SetDescriptorSetLayout();
    void AddPushConstants();
    */
    

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

    void Bake(std::vector<VkFormat> const& colorOutputFormats = std::vector<VkFormat>{},
              VkFormat depthFormat = VK_FORMAT_UNDEFINED, VkFormat stencilFormat = VK_FORMAT_UNDEFINED);

private:
    void InitDefaultPipelineState();

private:
    std::string mName;

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

    VkGraphicsPipelineCreateInfo mPipelineInfo{};

    VkPipeline mPipeline = VK_NULL_HANDLE;

};
