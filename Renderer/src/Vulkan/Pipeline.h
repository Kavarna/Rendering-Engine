#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <vector>

namespace Vulkan
{
    class RootSignature;

    class Pipeline
    {
        friend class CommandList;
    public:
        Pipeline(std::string const& name);
        ~Pipeline();

        Pipeline(Pipeline const&) = delete;
        Pipeline& operator=(Pipeline const&) = delete;

    public:
        void ClearShaders();
        void AddShader(std::string const& path);
        void AddShader(std::vector<char> const& shaderContent, VkShaderStageFlags shaderStage);

        void SetRootSignature(RootSignature const* rootSignature);

        void AddBackbufferColorOutput();
        void AddImageColorOutput(class Image const* img);
        void AddFormatColorOutput(VkFormat format);
        void SetBackbufferDepthStencilOutput();
        void SetDepthImage(class Image const* img);
        void SetDepthStencilImage(class Image const* img);

        void InitFrom(Pipeline& p);

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

        RootSignature const* mRootSignature = nullptr;

        std::vector<VkFormat> mColorOutputs;
        VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
        VkFormat mStencilFormat = VK_FORMAT_UNDEFINED;
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

}