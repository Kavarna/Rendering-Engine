#include "Dielectric.h"

#include "../HitPoint.h"

using namespace Jnrlib;

Float Reflectance(Float cosine, Float referenceIndex)
{
    // Use Schlick's approximation for reflectance.
    Float r0 = (One - referenceIndex) / (One + referenceIndex);
    r0 = r0 * r0;
    return r0 + (One - r0) * pow((One - cosine), 5);
}

Dielectric::Dielectric(CreateInfo::Material const& info) :
    IMaterial(info.name)
{
    CHECK((info.mask & CreateInfo::Material::RefractionIndex) != 0) << "Dielectric material has to have a index of refraction";

    mRefractionIndex = info.refractionIndex;
}

std::optional<ScatterInfo> Dielectric::Scatter(Ray const& rIn, HitPoint const& hp) const
{

    Float refractionRatio = hp.GetFrontFace() ? One / mRefractionIndex : mRefractionIndex;
    Float cosTheta = fmin(dot(-rIn.GetDirection(), hp.GetNormal()), 1.0f);
    Float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    Position newPosition = rIn.At(hp.GetIntersectionPoint());
    Direction finalRay;

    bool cannotRefract = refractionRatio * sinTheta > 1.0;

    bool reflectance = Reflectance(cosTheta, refractionRatio) > Random::get(Zero, One);

    if (cannotRefract || reflectance)
    {
        finalRay = glm::reflect(rIn.GetDirection(), hp.GetNormal());
    }
    else
    {
        finalRay = glm::refract(rIn.GetDirection(), hp.GetNormal(), refractionRatio);
    }


    return ScatterInfo{
        .ray = Ray(newPosition, finalRay),
        .attenuation = Color(One)
    };
}

