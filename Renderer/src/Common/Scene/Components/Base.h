#pragma once


#include <Jnrlib.h>

namespace Common
{
    class Entity;

    namespace Components
    {
        struct Base
        {
            Jnrlib::Matrix4x4 world;
            std::string name;

            /* Should never update inverse world manually, it's automatically updated by Entity::UpdateBase */
            Jnrlib::Matrix4x4 inverseWorld;

            Entity* entityPtr;
        };
    }
}