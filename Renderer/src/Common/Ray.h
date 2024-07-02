#pragma once


#include "Jnrlib.h"

namespace Common
{
    class Ray
    {
    public:
        Ray() = default;
        Ray(Jnrlib::Position const& position, Jnrlib::Direction const& direction, Jnrlib::Float t = Jnrlib::Infinity) :
            origin(position), direction(glm::normalize(direction)), maxT(t)
        { }
        Jnrlib::Position At(Jnrlib::Float t) const
        {
            return origin + direction * t;
        }
        Ray TransformedRay(Jnrlib::Matrix4x4 const& inverseWorld)
        {
            Jnrlib::Direction transformedDirection = Jnrlib::Matrix3x3(inverseWorld) * direction;
            Jnrlib::Position transformedOrigin = inverseWorld * glm::vec4(origin, 1.0f);

            return Ray(transformedOrigin, transformedDirection, maxT);
        }

        ~Ray()
        {
            if (mSaveRays) [[unlikely]]
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