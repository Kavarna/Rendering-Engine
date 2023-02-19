#pragma once

#include <Jnrlib.h>
#include "Vertex.h"

namespace Common
{
    namespace Sphere
    {
        void GetVertices(float radius, uint32_t sliceCount, uint32_t stackCount,
                         std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    }
}