#pragma once


#include <Jnrlib.h>

namespace Common
{
    class Entity;

    namespace Components
    {
        struct Base
        {
            Jnrlib::Position position;
            glm::vec3 scaling;

            std::string name;

            Entity* entityPtr;
        };
    }
}