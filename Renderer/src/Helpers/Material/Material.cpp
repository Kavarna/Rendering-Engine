#include "Material.h"

IMaterial::IMaterial(std::string const& name) : 
    mName(name)
{ }

std::string const& IMaterial::GetName() const
{
    return mName;
}
