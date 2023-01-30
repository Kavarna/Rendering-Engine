#include "Pipeline.h"

#include "FileHelpers.h"
#include "Renderer.h"
#include "VulkanLoader.h"

#include "RootSignature.h"
#include "Image.h"

#include <boost/algorithm/string.hpp>

using namespace Vulkan;

Pipeline::Pipeline(std::string const& name) :
    mName(name)
{
    InitDefaultPipelineState();
}

Pipeline::~Pipeline()
{
    VkDevice device = Renderer::Get()->GetDevice();

    for (auto const& shader : mShaderModules)
    {
        jnrDestroyShaderModule(device, shader.module, nullptr);
    }

    if (mPipeline != VK_NULL_HANDLE)
        jnrDestroyPipeline(device, mPipeline, nullptr);
}

void Pipeline::AddBackbufferColorOutput()
{
    VkFormat format = Renderer::Get()->GetBackbufferFormat();
    mColorOutputs.push_back(format);
}

void Pipeline::AddImageColorOutput(Image const* img)
{
    mColorOutputs.push_back(img->GetFormat());
}

void Pipeline::SetBackbufferDepthStencilOutput()
{
    mDepthFormat = Renderer::Get()->GetDefaultDepthFormat();
    mStencilFormat = Renderer::Get()->GetDefaultStencilFormat();
}

void Pipeline::SetDepthImage(Image const* img)
{
    mDepthFormat = img->GetFormat();
}

void Pipeline::InitDefaultPipelineState()
{
    {
        /* Blend State*/
        mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;        
        mBlendState.logicOpEnable = VK_FALSE;
        mBlendState.attachmentCount = 1;
    }

    {
        /* Depth Stencil State */
        mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mDepthStencilState.stencilTestEnable = VK_FALSE;
        mDepthStencilState.depthTestEnable = VK_FALSE;
    }

    {
        /* Dynamic State */
        mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        mDynamicState.dynamicStateCount = 0;
    }

    {
        /* Input Assembly State */
        mInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mInputAssemblyState.primitiveRestartEnable = VK_FALSE;
    }

    {
        /* Multisample State */
        mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mMultisampleState.alphaToOneEnable = VK_FALSE;
        mMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    {
        /* Rasterization State */
        mRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mRasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        mRasterizationState.depthClampEnable = VK_FALSE;
        mRasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        mRasterizationState.lineWidth = 1.0f;
        mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        mRasterizationState.rasterizerDiscardEnable = VK_FALSE;
    }

    {
        /* Tesselation State */
        mTesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        mTesselationState.patchControlPoints = 0;
    }

    {
        /* Vertex Input State */
        mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        mVertexInputState.pVertexBindingDescriptions = 0;
        mVertexInputState.vertexAttributeDescriptionCount = 0;
    }

    {
        /* Viewport State */
        mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        mViewportState.viewportCount = 0;
        mViewportState.scissorCount = 0;
    }

    mPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    mPipelineInfo.flags = 0;
    mPipelineInfo.layout;

    mPipelineInfo.pColorBlendState = &mBlendState;
    mPipelineInfo.pDepthStencilState = &mDepthStencilState;
    mPipelineInfo.pDynamicState = &mDynamicState;
    mPipelineInfo.pInputAssemblyState = &mInputAssemblyState;
    mPipelineInfo.pMultisampleState = &mMultisampleState;
    mPipelineInfo.pRasterizationState = &mRasterizationState;
    // mPipelineInfo.pTessellationState = &mTesselationState;
    mPipelineInfo.pVertexInputState = &mVertexInputState;
    mPipelineInfo.pViewportState = &mViewportState;
}

void Pipeline::AddShader(std::string const& path)
{
    VkShaderStageFlagBits shaderStage = (VkShaderStageFlagBits)0;
    if (boost::contains(path, ".vert."))
        shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
    else if (boost::contains(path, ".frag."))
        shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    for (auto const& shader : mShaderModules)
    {
        CHECK(shader.stage != shaderStage) << "A pipeline can have only a single shader of a certain type";
    }

    CHECK(shaderStage != 0) << "\"" << path << "\" doesn't contain the vertex type";

    VkDevice device = Renderer::Get()->GetDevice();

    auto shaderContent = Jnrlib::ReadWholeFile(path);
    
    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = reinterpret_cast<uint32_t*>(shaderContent.data());
    shaderInfo.codeSize = (uint32_t)shaderContent.size();

    VkShaderModule shaderModule;
    ThrowIfFailed(
        jnrCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule)
    );

    VkPipelineShaderStageCreateInfo pipelineStageInfo = {};
    {
        pipelineStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineStageInfo.module = shaderModule;
        pipelineStageInfo.flags = 0;
        pipelineStageInfo.pName = "main";
        pipelineStageInfo.stage = shaderStage;        
    }

    mShaderModules.push_back(pipelineStageInfo);
}

void Pipeline::SetRootSignature(RootSignature const* rootSignature)
{
    mRootSignature = rootSignature;
}

void Pipeline::Bake()
{
    VkPipelineLayout layout;
    if (mRootSignature)
    {
        layout = mRootSignature->mPipelineLayout;
    }
    else
    {
        layout = Renderer::Get()->GetEmptyPipelineLayout();
    }
    
    auto device = Renderer::Get()->GetDevice();

    {
        mRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        mRenderingInfo.viewMask = 0; // Multiview is not active
        mRenderingInfo.colorAttachmentCount = (uint32_t)mColorOutputs.size();
        mRenderingInfo.pColorAttachmentFormats = mColorOutputs.data();
        mRenderingInfo.depthAttachmentFormat = mDepthFormat;
        mRenderingInfo.stencilAttachmentFormat = mStencilFormat;
    }

    mPipelineInfo.pNext = &mRenderingInfo;

    mPipelineInfo.layout = layout;
    mPipelineInfo.pStages = mShaderModules.data();
    mPipelineInfo.stageCount = (uint32_t)mShaderModules.size();

    ThrowIfFailed(
        jnrCreateGraphicsPipelines(device, VK_NULL_HANDLE,
                1, &mPipelineInfo, nullptr, &mPipeline);
    );

    LOG(INFO) << "Successfully baked pipeline " << mName;
}
