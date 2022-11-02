#include "MaterialCreateInfo.h"

#include <boost/algorithm/string.hpp>

using nlohmann::json;

namespace CreateInfo
{
    MaterialType GetMaterialTypeFromString(std::string const& str)
    {
        if (boost::iequals(str, "lambertian"))
            return MaterialType::Lambertian;
        return MaterialType::None;
    }

    std::string GetStringFromMaterialType(MaterialType materialType)
    {
        switch (materialType)
        {
            case CreateInfo::Lambertian:
                return "Lambertian";
            case CreateInfo::None:
            default:
                return "None";
        }
    }

    std::ostream& operator<<(std::ostream& stream, Material const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, Material& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const Material& p)
    {
        j["name"] = p.name;
        j["type"] = GetStringFromMaterialType(p.type);
        if (p.mask & Material::Attenuation)
        {
            j["attenuation"] = Jnrlib::to_string(p.attenuation);
        }
    }

    void from_json(const nlohmann::json& j, Material& p)
    {
        std::string materialTypeString;
        j.at("name").get_to(p.name);
        j.at("type").get_to(materialTypeString);
        p.type = GetMaterialTypeFromString(materialTypeString);

        if (j.contains("attenuation"))
        {
            std::string attenuationString = "";
            j.at("attenuation").get_to(attenuationString);
            p.attenuation = Jnrlib::to_type<Jnrlib::Color>(attenuationString);
            p.mask |= Material::Attenuation;
        }
    }
}