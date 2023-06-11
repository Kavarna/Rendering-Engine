#pragma once


#include "Jnrlib.h"

namespace Common
{
    class Ray
    {
    public:
        Ray() = default;
        Ray(Jnrlib::Position const& position, Jnrlib::Direction const& direction) :
            origin(position), direction(glm::normalize(direction))
        { }
        Jnrlib::Position At(Jnrlib::Float t) const
        {
            return origin + direction * t;
        }

    public:
        Jnrlib::Position origin = Jnrlib::Position(0.0f);
        Jnrlib::Direction direction= Jnrlib::Direction(0.0f);
        Jnrlib::Float maxT = Jnrlib::Infinity;
    };

}