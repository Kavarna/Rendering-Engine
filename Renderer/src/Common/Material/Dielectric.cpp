#include "Dielectric.h"

#include "../HitPoint.h"

using namespace Jnrlib;
using namespace Common;

Float Reflectance(Float cosine, Float referenceIndex)
{
    // Use Schlick's approximation for reflectance.
    Float r0 = (One - referenceIndex) / (One + referenceIndex);
    r0 = r0 * r0;
    return r0 + (One - r0) * (Float)pow((One - cosine), 5);
}

Dielectric::Dielectric(CreateInfo::Material const& info, uint32_t materialIndex) :
    IMaterial(info.name, materialIndex)
{
    CHECK((info.mask & CreateInfo::Material::RefractionIndex) != 0) << "Dielectric material has to have a index of refraction";

    mRefractionIndex = info.refractionIndex;
}

Direction refract(const Direction& direction, const Direction& n, Float referenceIndex)
{
    Float cos_theta = (Float)fmin(dot(-direction, n), 1.0);
    glm::vec3 r_out_perp = referenceIndex * (direction + cos_theta * n);
    glm::vec3 r_out_parallel = -(Float)sqrt(fabs(One - r_out_perp.length() * r_out_perp.length())) * n;
    return r_out_perp + r_out_parallel;
}

std::optional<ScatterInfo> Dielectric::Scatter(Ray const& rIn, HitPoint const& hp) const
{

    Float refractionRatio = hp.GetFrontFace() ? One / mRefractionIndex : mRefractionIndex;
    Float cosTheta = fmin(dot(-rIn.direction, hp.GetNormal()), 1.0f);
    Float sinTheta = sqrt(One - cosTheta * cosTheta);

    Position newPosition = rIn.At(hp.GetIntersectionPoint());
    Direction finalRay;

    bool cannotRefract = refractionRatio * sinTheta > 1.0;
    bool reflectance = Reflectance(cosTheta, refractionRatio) > Random::get(Zero, One);

    if (cannotRefract || reflectance)
    {
        finalRay = glm::reflect(rIn.direction, hp.GetNormal());
    }
    else
    {
        finalRay = refract(rIn.direction, hp.GetNormal(), refractionRatio);
    }


    return ScatterInfo{
        .ray = Ray(newPosition, finalRay),
        .attenuation = Color(One)
    };
}

