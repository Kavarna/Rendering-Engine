#include "Lambertian.h"

Lambertian::Lambertian(CreateInfo::Material const& info) : 
    IMaterial(info.name)
{
    CHECK((info.mask & CreateInfo::Material::Attenuation) != 0) << "Lambertian material has to have attenuation";
    
    mAttenuation = info.attenuation;
}

std::optional<ScatterInfo> Lambertian::Scatter(Ray const& rIn, HitPoint const& hp) const
{
    Jnrlib::Direction newDirection = hp.GetNormal() + Jnrlib::GetRandomDirectionInHemisphere(hp.GetNormal());
    Jnrlib::Position newPosition = rIn.At(hp.GetIntersectionPoint());

    return ScatterInfo{
        .ray = Ray(newPosition, newDirection),
        .attenuation = mAttenuation
    };
}

