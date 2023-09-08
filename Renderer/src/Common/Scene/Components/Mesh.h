#pragma once

#include <Jnrlib.h>

namespace Common::Components
{
    struct Indices
    {
        uint32_t firstVertex = 0;
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
    };

    struct Mesh
    {
        std::string name;

        Indices indices;

        bool hidden = false;

        std::shared_ptr<Common::IMaterial> material = nullptr;
    };
}
