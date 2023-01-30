#include "Material.h"

using namespace Common;

IMaterial::IMaterial(std::string const& name) : 
    mName(name)
{ }

std::string const& IMaterial::GetName() const
{
    return mName;
}
