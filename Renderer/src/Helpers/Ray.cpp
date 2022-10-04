#include "Ray.h"

#include <glm/vec3.hpp>
#include <glm/glm.hpp>


Ray::Ray(Jnrlib::Position const& position, Jnrlib::Direction const& direction) :
    mStartPosition(position), mDirection(glm::normalize(direction))
{ }

Jnrlib::Position const& Ray::GetStartPosition() const
{
    return mStartPosition;
}

Jnrlib::Direction const& Ray::GetDirection() const
{
    return mDirection;
}

Jnrlib::Position Ray::At(Jnrlib::Float t) const
{
    return mStartPosition + mDirection * t;
}
