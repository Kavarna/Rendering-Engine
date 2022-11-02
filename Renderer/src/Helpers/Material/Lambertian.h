#pragma once

#include "Material.h"

class Lambertian : public IMaterial
{
public:
    Lambertian(CreateInfo::Material const& info);

public:
    [[nodiscard]]
    virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const override;

private:
    Jnrlib::Color mAttenuation;

};
