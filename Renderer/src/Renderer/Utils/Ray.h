#pragma once


#include "Jnrlib.h"

namespace Renderer
{

    class Ray
    {
    public:
        Ray() = default;
        Ray(Jnrlib::Position const& position, Jnrlib::Direction const& direction);

        Jnrlib::Position const& GetStartPosition() const;
        Jnrlib::Direction const& GetDirection() const;

        Jnrlib::Position At(Jnrlib::Float t) const;

    private:
        Jnrlib::Position mStartPosition = Jnrlib::Position(0.0f);
        Jnrlib::Direction mDirection = Jnrlib::Direction(0.0f);
    };

}