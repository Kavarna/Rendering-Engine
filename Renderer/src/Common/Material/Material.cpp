#include "Material.h"

using namespace Common;

IMaterial::IMaterial(std::string const& name, uint32_t materialIndex) : 
    mName(name),
    mMaterialIndex(materialIndex)
{ }

std::string const& IMaterial::GetName() const
{
    return mName;
}

uint32_t Common::IMaterial::GetMaterialIndex() const
{
    return mMaterialIndex;
}
