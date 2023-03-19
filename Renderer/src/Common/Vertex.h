#pragma once

#include <vulkan/vulkan.h>

namespace Common
{
    struct VertexPositionNormal
    {
        glm::vec3 position;
        glm::vec3 normal;

        VertexPositionNormal() = default;
        VertexPositionNormal(float x, float y, float z, float nx, float ny, float nz) : position(x, y, z), normal(nx, ny, nz)
        { };

        static std::array<VkVertexInputAttributeDescription, 2> GetInputAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            {
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].offset = offsetof(Common::VertexPositionNormal, position);
            }
            {
                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].offset = offsetof(Common::VertexPositionNormal, normal);
            }
            return attributeDescriptions;
        }

        static std::vector<VkVertexInputBindingDescription> GetInputBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            {
                bindingDescription.binding = 0;
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                bindingDescription.stride = sizeof(VertexPositionNormal);
            }
            return {bindingDescription};
        }
    };

    struct VertexPositionColor
    {
        glm::vec3 position;
        glm::vec4 color;

        VertexPositionColor() = default;
        VertexPositionColor(float x, float y, float z, float r, float g, float b, float a) : position(x, y, z), color(r, g, b, a)
        { };
        VertexPositionColor(glm::vec3 const& pos, glm::vec4 const& col) : position(pos), color(col)
        { };

        static std::array<VkVertexInputAttributeDescription, 2> GetInputAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            {
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].offset = offsetof(Common::VertexPositionColor, position);
            }
            {
                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].offset = offsetof(Common::VertexPositionColor, color);
            }
            return attributeDescriptions;
        }

        static std::vector<VkVertexInputBindingDescription> GetInputBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            {
                bindingDescription.binding = 0;
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                bindingDescription.stride = sizeof(VertexPositionColor);
            }
            return {bindingDescription};
        }
    };
}