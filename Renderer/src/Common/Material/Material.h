#pragma once


#include "Ray.h"
#include <nlohmann/json.hpp>
#include "CreateInfo/MaterialCreateInfo.h"

namespace Common
{
    class HitPoint;

    struct ScatterInfo
    {
        Ray ray;
        Jnrlib::Color attenuation;
    };

    class IMaterial
    {
    public:
        IMaterial(std::string const& name, uint32_t materialIndex);

    public:
        [[nodiscard]]
        virtual std::optional<ScatterInfo> Scatter(Ray const&, HitPoint const& hp) const = 0;

        std::string const& GetName() const;
        uint32_t GetMaterialIndex() const;

    private:
        std::string mName;
        uint32_t mMaterialIndex;

    };

}