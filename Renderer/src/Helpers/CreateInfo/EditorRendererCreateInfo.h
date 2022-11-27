#pragma once 

#include <Jnrlib.h>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{

    struct EditorRenderer
    {
        std::vector<std::string> instanceLayers;
        std::vector<std::string> instanceExtensions;

        std::vector<std::string> deviceLayers;
        std::vector<std::string> deviceExtensions;

        GLFWwindow *window;

        friend std::ostream& operator << (std::ostream& stream, EditorRenderer const& info);
        friend std::istream& operator >> (std::istream& stream, EditorRenderer& info);
    };

    void to_json(nlohmann::json& j, const EditorRenderer& p);
    void from_json(const nlohmann::json& j, EditorRenderer& p);
}