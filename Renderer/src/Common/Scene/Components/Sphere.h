#pragma once

#include "../../Material/Material.h"

namespace Common::Components
{
    struct Sphere
    {
        Jnrlib::Float radius;

        std::shared_ptr<Common::IMaterial> material = nullptr;
    };
}