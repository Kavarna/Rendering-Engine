#pragma once

#include <Jnrlib.h>

namespace Common::Components
{
    struct Mesh
    {
        /* TODO: Merge this with model */
        uint32_t firstVertex;
        uint32_t firstIndex;
        uint32_t indexCount;
    };
}
