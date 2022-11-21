#include "Jnrlib.h"

Jnrlib::Direction Jnrlib::GetRandomPointInUnitSphere()
{
    while (true)
    {
        Jnrlib::Direction randomDirection = {
            Random::get(-Jnrlib::One, Jnrlib::One),
            Random::get(-Jnrlib::One, Jnrlib::One),
            Random::get(-Jnrlib::One, Jnrlib::One)
        };

        if (glm::length(randomDirection) >= Jnrlib::One)
            continue;

        return randomDirection;
    }
}

Jnrlib::Direction Jnrlib::GetRandomDirectionInUnitSphere()
{
    return glm::normalize(GetRandomPointInUnitSphere());
}

Jnrlib::Direction Jnrlib::GetRandomPointInHemisphere(Jnrlib::Direction const& normal)
{
    auto inUnitSphere = GetRandomPointInUnitSphere();
    if (glm::dot(normal, inUnitSphere) > Jnrlib::Zero)
    {
        return inUnitSphere;
    }
    else
    {
        return -inUnitSphere;
    }
}

Jnrlib::Direction Jnrlib::GetRandomDirectionInHemisphere(Jnrlib::Direction const& normal)
{
    auto inUnitSphere = GetRandomDirectionInUnitSphere();
    if (glm::dot(normal, inUnitSphere) > Jnrlib::Zero)
    {
        return inUnitSphere;
    }
    else
    {
        return -inUnitSphere;
    }
}

Jnrlib::Direction Jnrlib::GetRandomInUnitDisk()
{
    while (true)
    {
        Jnrlib::Direction randomDirection = {
            Random::get(-Jnrlib::One, Jnrlib::One),
            Random::get(-Jnrlib::One, Jnrlib::One),
            0.0f
        };
        if (glm::length(randomDirection) >= One)
            continue;

        return randomDirection;
    }
}

