#pragma once

#include <Jnrlib.h>

namespace Common::Components
{
    struct Mesh
    {
        uint32_t firstVertex;
        uint32_t firstIndex;
        uint32_t indexCount;
    };
}
