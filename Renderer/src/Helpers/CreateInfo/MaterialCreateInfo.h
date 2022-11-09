#pragma once

#include "TypeHelpers.h"
#include <Jnrlib.h>
#include <nlohmann/json.hpp>

namespace CreateInfo
{

    enum MaterialType
    {
        Lambertian,
        Metal,
        None,
    };
    MaterialType GetMaterialTypeFromString(std::string const& str);
    std::string GetStringFromMaterialType(MaterialType materialType);

    struct Material
    {
        using FeatureType = uint8_t;

        std::string name;
        Jnrlib::Color attenuation;
        Jnrlib::Float fuziness;
        MaterialType type;

        enum FeaturesMask : FeatureType
        {
            Attenuation = 1,
            Fuzziness = 2,
        };
        FeatureType mask = 0;

        friend std::ostream& operator << (std::ostream& stream, Material const& info);
        friend std::istream& operator >> (std::istream& stream, Material& info);
    };

    void to_json(nlohmann::json& j, const Material& p);
    void from_json(const nlohmann::json& j, Material& p);
}