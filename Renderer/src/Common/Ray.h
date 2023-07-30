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

        ~Ray()
        {
            if (mSaveRays)
            {
                /* Save the ray for possible replay */
                mPositions.push_back(origin);
                mPositions.push_back(origin + direction * maxT);
            }
        }

        static bool mSaveRays;
        static std::vector<Jnrlib::Position> mPositions;

    public:
        Jnrlib::Position origin = Jnrlib::Position(0.0f);
        Jnrlib::Direction direction= Jnrlib::Direction(0.0f);
        Jnrlib::Float maxT = Jnrlib::Infinity;
    };

}