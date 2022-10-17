#include "HitPoint.h"

void HitPoint::SetColor(Jnrlib::Color const& color)
{
    mColor = color;
}

Jnrlib::Color const& HitPoint::GetColor() const
{
    return mColor;
}

Jnrlib::Direction const& HitPoint::GetNormal() const
{
    return mNormal;
}

Jnrlib::Float HitPoint::GetIntersectionPoint() const
{
    return mIntersectionPoint;
}

void HitPoint::SetIntersectionPoint(Jnrlib::Float t)
{
    mIntersectionPoint = t;
}

void HitPoint::SetNormal(Jnrlib::Direction const& normal)
{
    CHECK(!(normal.x == Jnrlib::Zero && normal.y == Jnrlib::Zero && normal.z == Jnrlib::Zero)) << "Normal cannot be (0, 0, 0)";
    mNormal = glm::normalize(normal);
}

