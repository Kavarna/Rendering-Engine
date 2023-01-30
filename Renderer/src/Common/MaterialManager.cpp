#include "MaterialManager.h"
#include "Material/Lambertian.h"
#include "Material/Metal.h"
#include "Material/Dielectric.h"

using namespace Common;

void MaterialManager::AddMaterial(CreateInfo::Material const& material)
{
	mMaterials[material.name] = CreateMaterial(material);
}

void MaterialManager::AddMaterials(std::vector<CreateInfo::Material> const& materials)
{
	mMaterials.reserve(materials.size() + mMaterials.size());
	for (const auto& mat : materials)
	{
		AddMaterial(mat);
	}
}

std::shared_ptr<IMaterial> MaterialManager::GetMaterial(std::string const& name) const
{
	if (auto it = mMaterials.find(name); it != mMaterials.end())
	{
		return (*it).second;
	}
	return nullptr;
}

std::shared_ptr<IMaterial> MaterialManager::CreateMaterial(CreateInfo::Material const& matInfo)
{
	switch (matInfo.type)
	{
		case CreateInfo::MaterialType::Lambertian:
			return std::make_shared<Lambertian>(matInfo);
		case CreateInfo::MaterialType::Metal:
			return std::make_shared<Metal>(matInfo);
		case CreateInfo::MaterialType::Dieletric:
			return std::make_shared<Dielectric>(matInfo);
		case CreateInfo::MaterialType::None:
		default:
			return nullptr;
	}
}

