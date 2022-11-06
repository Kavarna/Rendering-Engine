#pragma once


#include "Ray.h"
#include <nlohmann/json.hpp>
#include "CreateInfo/MaterialCreateInfo.h"

class HitPoint;

struct ScatterInfo
{
    Ray ray;
    Jnrlib::Color attenuation;
};

class IMaterial
{
public:
    IMaterial(std::string const& name);

public:
    [[nodiscard]]
    virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const = 0;

    std::string const& GetName() const;

private:
    std::string mName;

};

