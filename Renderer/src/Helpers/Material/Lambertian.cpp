#include "Lambertian.h"

#include "../HitPoint.h"

Lambertian::Lambertian(CreateInfo::Material const& info) : 
    IMaterial(info.name)
{
    CHECK((info.mask & CreateInfo::Material::Attenuation) != 0) << "Lambertian material has to have attenuation";
    
    mAttenuation = info.attenuation;
}

std::optional<ScatterInfo> Lambertian::Scatter(Ray const& rIn, HitPoint const& hp) const
{
    Jnrlib::Direction newDirection = hp.GetNormal() + Jnrlib::GetRandomDirectionInUnitSphere();
    Jnrlib::Position newPosition = rIn.At(hp.GetIntersectionPoint());

    if (newDirection.length() <= Jnrlib::EPSILON)
        newDirection = hp.GetNormal();

    return ScatterInfo{
        .ray = Ray(newPosition, newDirection),
        .attenuation = mAttenuation
    };
}

