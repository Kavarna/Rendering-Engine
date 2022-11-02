#include "HitPoint.h"


Jnrlib::Direction const& HitPoint::GetNormal() const
{
    return mNormal;
}

Jnrlib::Float HitPoint::GetIntersectionPoint() const
{
    return mIntersectionPoint;
}

std::shared_ptr<IMaterial> HitPoint::GetMaterial() const
{
    return mMaterial;
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

void HitPoint::SetMaterial(std::shared_ptr<IMaterial> material)
{
    mMaterial = material;
}

