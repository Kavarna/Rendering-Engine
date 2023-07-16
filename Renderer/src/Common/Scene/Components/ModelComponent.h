#pragma once

#include <Jnrlib.h>

namespace Common::Components
{
    struct Model
    {
        std::string name;
        std::shared_ptr<Common::IMaterial> material;
        /* Maybe bounding box? */
    };
}
