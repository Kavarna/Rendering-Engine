#pragma once


#include <Jnrlib.h>

#include <nlohmann/json.hpp>

struct Material
{
    using FeatureType = uint8_t;

    std::string name;
    Jnrlib::Color color;

    enum FeaturesMask : FeatureType
    {
        Color = 1,
    };
    FeatureType mask;

    friend std::ostream& operator << (std::ostream& stream, Material const& info);
    friend std::istream& operator >> (std::istream& stream, Material& info);
};

void to_json(nlohmann::json& j, const Material& p);
void from_json(const nlohmann::json& j, Material& p);

class MaterialManager : public Jnrlib::ISingletone<MaterialManager>
{
    MAKE_SINGLETONE_CAPABLE(MaterialManager);
private:
    MaterialManager() = default;
    ~MaterialManager() = default;

public:
    void AddMaterial(Material const& material);
    void AddMaterials(std::vector<Material> const& materials);

private:
    // TODO: Key to be replaced with a hash (or something similar to speed-up look-up)
    std::unordered_map<std::string, Material> mMaterialMap;
};



