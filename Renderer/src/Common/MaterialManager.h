#pragma once


#include <Jnrlib.h>

#include "Material/Material.h"
#include "Vulkan/Buffer.h"
#include "CreateInfo/MaterialCreateInfo.h"

namespace Common
{

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

        struct ShaderMaterial
        {
            glm::vec4 color;
        };

        std::vector<ShaderMaterial> GetShaderMaterials();

    private:
        std::shared_ptr<IMaterial> CreateMaterial(CreateInfo::Material const& matInfo);

    private:
        std::unordered_map<std::string, std::shared_ptr<IMaterial>> mMaterials;

        std::vector<ShaderMaterial> mShaderMaterials;
    };

}
