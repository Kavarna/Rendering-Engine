#pragma once


#include <Jnrlib.h>

namespace Common::Components
{
    struct Base
    {
        Jnrlib::Position position;
        glm::vec3 scaling;

        std::string name;
    };
}