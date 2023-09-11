#pragma once

#include <Jnrlib.h>
#include <string_view>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <nlohmann/json.hpp>

namespace CreateInfo
{

struct VulkanRenderer
{
    struct LayerInfo
    {
        LayerInfo(std::string const &name, bool mandatory = true) : name(name), mandatory(mandatory)
        {
        }
        std::string name;
        bool mandatory = true;
    };

    std::vector<LayerInfo> instanceLayers;
    std::vector<LayerInfo> instanceExtensions;

    std::vector<LayerInfo> deviceLayers;
    std::vector<LayerInfo> deviceExtensions;

    GLFWwindow *window;

    friend std::ostream &operator<<(std::ostream &stream, VulkanRenderer const &info);
    friend std::istream &operator>>(std::ostream &stream, VulkanRenderer const &info);
};

void to_json(nlohmann::json &j, const VulkanRenderer &p);
void from_json(const nlohmann::json &j, VulkanRenderer &p);
} // namespace CreateInfo
