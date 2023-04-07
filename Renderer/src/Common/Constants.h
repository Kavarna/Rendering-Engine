#pragma once

#include <stdint.h>

#include <glm/glm.hpp>

namespace Common::Constants
{
    constexpr const static uint32_t FRAMES_IN_FLIGHT = 2;
    constexpr const static uint32_t MAX_NAME_SIZE = 64;

    constexpr const static glm::vec4 DEFAULT_FORWARD_DIRECTION = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    constexpr const static glm::vec4 DEFAULT_RIGHT_DIRECTION = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    constexpr const static glm::vec4 DEFAULT_UP_DIRECTION = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
}
