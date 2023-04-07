#pragma once

#include <glm/glm.hpp>

namespace Common::Systems
{
    struct DirectionalLight
    {
        glm::vec4 color;
        glm::vec3 direction;
    };
}
