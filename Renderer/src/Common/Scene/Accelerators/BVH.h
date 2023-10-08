#pragma once


#include <Jnrlib.h>

#include "Vertex.h"
#include "Scene/Components/AccelerationStructure.h"
#include "Accelerator.h"

namespace Common
{
    namespace Accelerators
    {
        namespace BVH
        {
            enum class SplitType
            {
                SAH,
                Middle,
                EqualCount,
            };
            struct Input
            {
                uint32_t maxPrimsInNode;
                SplitType splitType;

                std::vector<uint32_t> indices;
                std::vector<Common::VertexPositionNormal> vertices;
            };

            struct Output
            {
                Common::Components::AccelerationStructure accelerationStructure;
                std::vector<uint32_t> new_indices;
                /* TODO: Might reorder the vertices to improve cache accesses if requested */
            };

            Output Generate(Input const& input);
        }
    }
}