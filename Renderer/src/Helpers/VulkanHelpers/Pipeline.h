#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <vector>

class Pipeline
{
public:
    Pipeline();
    ~Pipeline();

public:
    void AddShader(std::string const& path);

private:
    std::vector<VkPipelineShaderStageCreateInfo> mShaderModules;
    VkGraphicsPipelineCreateInfo mPipelineInfo;

};
