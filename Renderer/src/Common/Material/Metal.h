#pragma once

#include "Material.h"

namespace Common
{
    class Metal : public IMaterial
    {
    public:
        Metal(CreateInfo::Material const& info, uint32_t materialIndex);

    public:
        [[nodiscard]]
        virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const override;

    private:
        Jnrlib::Color mAttenuation;
        Jnrlib::Float mFuziness;

    };
}