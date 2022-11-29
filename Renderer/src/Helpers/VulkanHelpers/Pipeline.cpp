#include "Pipeline.h"

#include "FileHelpers.h"
#include "Editor/Renderer.h"
#include "Editor/VulkanLoader.h"

#include <boost/algorithm/string.hpp>

Pipeline::Pipeline()
{
}

Pipeline::~Pipeline()
{
    VkDevice device = Editor::Renderer::Get()->GetDevice();

    for (auto const& shader : mShaderModules)
    {
        jnrDestroyShaderModule(device, shader.module, nullptr);
    }
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
