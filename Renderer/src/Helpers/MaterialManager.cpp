#include "MaterialManager.h"

using json = nlohmann::json;

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
    if (p.mask & Material::Color)
    {
        j["color"] = Jnrlib::to_string(p.color);
    }
}

void from_json(const nlohmann::json& j, Material& p)
{
    j.at("name").get_to(p.name);

    if (j.contains("color"))
    {
        std::string colorString = "";
        j.at("color").get_to(colorString);
        p.color = Jnrlib::to_type<Jnrlib::Color>(colorString);
        p.mask |= Material::Color;
    }
}

void MaterialManager::AddMaterial(Material const& material)
{
    mMaterialMap.insert({material.name, material});
}

void MaterialManager::AddMaterials(std::vector<Material> const& materials)
{
    mMaterialMap.reserve(mMaterialMap.size() + materials.size());
    for (auto const& material : materials)
    {
        AddMaterial(material);
    }
}

Material MaterialManager::GetMaterial(std::string const& name)
{
    auto it = mMaterialMap.find(name);
    CHECK(it != mMaterialMap.end()) << "Looking for material with name " << name << ", but couldn't find it";
    return (*it).second;
}
