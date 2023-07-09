#include "MaterialCreateInfo.h"

#include <boost/algorithm/string.hpp>

using nlohmann::json;

namespace CreateInfo
{
    MaterialType GetMaterialTypeFromString(std::string const& str)
    {
        auto type = magic_enum::enum_cast<MaterialType>(str);
        if (type.has_value())
        {
            return *type;
        }
        else
            return MaterialType::None;
    }

    std::string GetStringFromMaterialType(MaterialType materialType)
    {
        return std::string(magic_enum::enum_name(materialType));
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
        if (p.mask & Material::Fuzziness)
        {
            j["fuziness"] = Jnrlib::to_string(p.fuziness);
        }
        if (p.mask & Material::RefractionIndex)
        {
            j["refraction-index"] = p.refractionIndex;
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
        if (j.contains("fuziness"))
        {
            j.at("fuziness").get_to(p.fuziness);
            p.mask |= Material::Fuzziness;
        }
        if (j.contains("refraction-index"))
        {
            j.at("refraction-index").get_to(p.refractionIndex);
            p.mask |= Material::RefractionIndex;
        }
    }
}