#include "MaterialFactory.h"

MaterialFactory::MaterialFactory()
{ }

MaterialFactory::~MaterialFactory()
{ }

std::shared_ptr<IMaterial> MaterialFactory::CreateMaterial(CreateInfo::Material const& matInfo)
{
    
}

std::shared_ptr<IMaterial> MaterialFactory::CreateLambertianMaterial(MaterialInfo const& matInfo)
{
    
}
