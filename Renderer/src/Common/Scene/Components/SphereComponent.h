#pragma once

#include "../../Material/Material.h"

namespace Common::Components
{
    struct Sphere
    {
        std::shared_ptr<Common::IMaterial> material;
    };
}