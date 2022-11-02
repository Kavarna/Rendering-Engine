#pragma once

#include <Jnrlib.h>
#include <Singletone.h>

#include "Material.h"


class MaterialFactory : public Jnrlib::ISingletone<MaterialFactory>
{
    MAKE_SINGLETONE_CAPABLE(MaterialFactory);

private:
    MaterialFactory();
    ~MaterialFactory();

public:
    std::shared_ptr<IMaterial> CreateMaterial(MaterialInfo const& matInfo);

private:
    std::shared_ptr<IMaterial> CreateLambertianMaterial(MaterialInfo const& matInfo);

};
