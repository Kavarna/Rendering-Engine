#pragma once

#include <vulkan/vulkan.h>

namespace Common
{
    struct Vertex
    {
        glm::vec3 position;

        Vertex() = default;
        Vertex(float x, float y, float z) : position(x, y, z)
        { };

        static std::array<VkVertexInputAttributeDescription, 1> GetInputAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};
            {
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].offset = offsetof(Common::Vertex, position);
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