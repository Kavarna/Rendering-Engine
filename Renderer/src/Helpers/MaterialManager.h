#pragma once


#include <Jnrlib.h>

#include "Material/Material.h"
#include "CreateInfoUtils.h"

class MaterialManager : public Jnrlib::ISingletone<MaterialManager>
{
    MAKE_SINGLETONE_CAPABLE(MaterialManager);
private:
    MaterialManager() = default;
    ~MaterialManager() = default;

public:
    void AddMaterial(CreateInfo::Material const& material);
    void AddMaterials(std::vector<CreateInfo::Material> const& materials);

    std::shared_ptr<IMaterial> GetMaterial(std::string const& name) const;

private:
    std::shared_ptr<IMaterial> CreateMaterial(CreateInfo::Material const& matInfo);

private:
    std::unordered_map<std::string, std::shared_ptr<IMaterial>> mMaterials;
    
};



