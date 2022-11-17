#pragma once

#include "Material.h"

class Dielectric : public IMaterial
{
public:
    Dielectric(CreateInfo::Material const& info);

public:
    [[nodiscard]]
    virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const override;

private:
    Jnrlib::Float mRefractionIndex;

};
