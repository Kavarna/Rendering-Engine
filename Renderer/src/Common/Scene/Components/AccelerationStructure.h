#pragma once

#include <Jnrlib.h>

namespace Common::Components
{
    struct ALIGN(32) LinearBVHNode
    {
        Jnrlib::BoundingBox bounds;
        
        union
        {
            /* TODO: Look into creating a binary tree in which children are stored at 2 * parent (+1) instead of storing the offset */
            uint32_t primitiveOffset;
            uint32_t secondChildOffset;
        };

        uint32_t primitiveCount;
        Jnrlib::Axis axis;
    };

    struct AccelerationStructure
    {
        std::vector<LinearBVHNode> nodes;
        bool shouldRender = false;
    };
}
