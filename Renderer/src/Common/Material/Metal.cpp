#include "Metal.h"

#include "../HitPoint.h"

using namespace Common;

Metal::Metal(CreateInfo::Material const& info) :
    IMaterial(info.name)
{
    CHECK((info.mask & CreateInfo::Material::Attenuation) != 0) << "Metal material has to have attenuation";
    CHECK((info.mask & CreateInfo::Material::Fuzziness) != 0) << "Metal material has to have fuziness";

    mAttenuation = info.attenuation;
    mFuziness = glm::clamp(info.fuziness, Jnrlib::Zero, Jnrlib::One);
}

std::optional<ScatterInfo> Metal::Scatter(Ray const& rIn, HitPoint const& hp) const
{
    Jnrlib::Direction reflectedDirection = glm::reflect(rIn.GetDirection(), hp.GetNormal());
    Jnrlib::Position newPosition = rIn.At(hp.GetIntersectionPoint());

    if (glm::dot(reflectedDirection, hp.GetNormal()) < 0)
        return std::nullopt;

    return ScatterInfo{
        .ray = Ray(newPosition, reflectedDirection + mFuziness * Jnrlib::GetRandomDirectionInUnitSphere()),
        .attenuation = mAttenuation
    };
}

