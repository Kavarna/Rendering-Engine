#pragma once 

#include <Jnrlib.h>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{

    struct VulkanRenderer
    {
        std::vector<std::string> instanceLayers;
        std::vector<std::string> instanceExtensions;

        std::vector<std::string> deviceLayers;
        std::vector<std::string> deviceExtensions;

        GLFWwindow *window;

        friend std::ostream& operator << (std::ostream& stream, VulkanRenderer const& info);
        friend std::istream& operator >> (std::istream& stream, VulkanRenderer& info);
    };

    void to_json(nlohmann::json& j, const VulkanRenderer& p);
    void from_json(const nlohmann::json& j, VulkanRenderer& p);
}