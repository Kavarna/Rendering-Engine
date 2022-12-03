#include "Pipeline.h"

#include "FileHelpers.h"
#include "Editor/Renderer.h"
#include "Editor/VulkanLoader.h"

#include <boost/algorithm/string.hpp>

static VkPipelineLayout gEmptyPipelineLayout = VK_NULL_HANDLE;

Pipeline::Pipeline(std::string const& name) :
    mName(name)
{
    InitDefaultPipelineState();
}

Pipeline::~Pipeline()
{
    VkDevice device = Editor::Renderer::Get()->GetDevice();

    for (auto const& shader : mShaderModules)
    {
        jnrDestroyShaderModule(device, shader.module, nullptr);
    }

    if (mPipeline != VK_NULL_HANDLE)
        jnrDestroyPipeline(device, mPipeline, nullptr);
}

void Pipeline::InitDefaultPipelineState()
{
    {
        mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        
        /* Blend State*/
        mBlendAttachments[0].blendEnable = VK_FALSE;
        mBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        mBlendState.logicOpEnable = VK_FALSE;
        mBlendState.attachmentCount = 1;
        mBlendState.pAttachments = mBlendAttachments;
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
    mPipelineInfo.layout; // TODO: Fill this later
    mPipelineInfo.renderPass; // TODO: Fill this later

    mPipelineInfo.pColorBlendState = &mBlendState;
    mPipelineInfo.pDepthStencilState = &mDepthStencilState;
    mPipelineInfo.pDynamicState = &mDynamicState;
    mPipelineInfo.pInputAssemblyState = &mInputAssemblyState;
    mPipelineInfo.pMultisampleState = &mMultisampleState;
    mPipelineInfo.pRasterizationState = &mRasterizationState;
    mPipelineInfo.pTessellationState = &mTesselationState;
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

    VkDevice device = Editor::Renderer::Get()->GetDevice();

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

void Pipeline::Bake(std::vector<VkFormat> const& colorOutputFormats, VkFormat depthFormat, VkFormat stencilFormat)
{
    VkPipelineLayout layout;
    if (/* TODO: Is there a pipeline layout provided? */ false)
        return;
    
    layout = Editor::Renderer::Get()->GetEmptyPipelineLayout();
    auto device = Editor::Renderer::Get()->GetDevice();

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    {
        pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingInfo.viewMask = (1u << (uint32_t)colorOutputFormats.size()) - 1u;
        pipelineRenderingInfo.colorAttachmentCount = (uint32_t)colorOutputFormats.size();
        pipelineRenderingInfo.pColorAttachmentFormats = colorOutputFormats.data();
        pipelineRenderingInfo.depthAttachmentFormat = depthFormat;
        pipelineRenderingInfo.stencilAttachmentFormat = stencilFormat;
    }

    mPipelineInfo.pNext = &pipelineRenderingInfo;

    mPipelineInfo.layout = layout;
    mPipelineInfo.pStages = mShaderModules.data();
    mPipelineInfo.stageCount = (uint32_t)mShaderModules.size();

    ThrowIfFailed(
        jnrCreateGraphicsPipelines(device, VK_NULL_HANDLE,
                1, &mPipelineInfo, nullptr, &mPipeline);
    );

    LOG(INFO) << "Successfully baked pipeline " << mName;
}
