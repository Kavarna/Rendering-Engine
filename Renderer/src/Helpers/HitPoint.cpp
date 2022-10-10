#include "HitPoint.h"

void HitPoint::SetColor(Jnrlib::Color const& color)
{
    mColor = color;
}

Jnrlib::Color const& HitPoint::GetColor() const
{
    return mColor;
}

Jnrlib::Float HitPoint::GetIntersectionPoint(uint32_t index) const
{
    CHECK(index < MAX_INTERSECTION_POINTS) << "Unable to get intersection point " << index << ". Maximum is " << MAX_INTERSECTION_POINTS;
    return mIntersectionPoints[index];
}

std::array<Jnrlib::Float, HitPoint::MAX_INTERSECTION_POINTS> const& HitPoint::GetIntersectionPoints() const
{
    return mIntersectionPoints;
}

void HitPoint::SetIntersectionPoint(Jnrlib::Float t, uint32_t index)
{
    CHECK(index < MAX_INTERSECTION_POINTS) << "Unable to set intersection point " << index << ". Maximum is " << MAX_INTERSECTION_POINTS;
    mIntersectionPoints[index] = t;
}

