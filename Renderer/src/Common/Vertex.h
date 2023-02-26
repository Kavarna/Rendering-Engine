#pragma once

#include <vulkan/vulkan.h>

namespace Common
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;

        Vertex() = default;
        Vertex(float x, float y, float z, float nx, float ny, float nz) : position(x, y, z), normal(nx, ny, nz)
        { };

        static std::array<VkVertexInputAttributeDescription, 2> GetInputAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            {
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].offset = offsetof(Common::Vertex, position);
            }
            {
                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].offset = offsetof(Common::Vertex, normal);
            }
            return attributeDescriptions;
        }

        static std::vector<VkVertexInputBindingDescription> GetInputBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            {
                bindingDescription.binding = 0;
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                bindingDescription.stride = sizeof(Common::Vertex);
            }
            return {bindingDescription};
        }
    };
}