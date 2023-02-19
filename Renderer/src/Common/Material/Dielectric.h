#pragma once

#include "Material.h"

namespace Common
{
    class Dielectric : public IMaterial
    {
    public:
        Dielectric(CreateInfo::Material const& info, uint32_t materialIndex);

    public:
        [[nodiscard]]
        virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const override;

    private:
        Jnrlib::Float mRefractionIndex;

    };
}
