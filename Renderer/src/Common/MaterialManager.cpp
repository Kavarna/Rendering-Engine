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

std::shared_ptr<IMaterial> MaterialManager::GetDefaultMaterial() const
{
	return mMaterials.begin()->second;
}

std::vector<MaterialManager::ShaderMaterial> MaterialManager::GetShaderMaterials()
{
	return mShaderMaterials;
}

std::shared_ptr<IMaterial> MaterialManager::CreateMaterial(CreateInfo::Material const& matInfo)
{
	switch (matInfo.type)
	{
		case CreateInfo::MaterialType::Lambertian:
		{
			auto materialPtr = std::make_shared<Lambertian>(matInfo, (uint32_t)mMaterials.size());
			mShaderMaterials.emplace_back(ShaderMaterial{.color = matInfo.attenuation});
			return materialPtr;
		}
		case CreateInfo::MaterialType::Metal:
		{
			auto materialPtr = std::make_shared<Metal>(matInfo, (uint32_t)mMaterials.size());
			mShaderMaterials.emplace_back(ShaderMaterial{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)});
			return materialPtr;
		}
		case CreateInfo::MaterialType::Dieletric:
		{
			auto materialPtr = std::make_shared<Dielectric>(matInfo, (uint32_t)mMaterials.size());
			mShaderMaterials.emplace_back(ShaderMaterial{.color = glm::vec4(0.8f, 0.0f, 0.8f, 1.0f)});
			return materialPtr;
		}
		case CreateInfo::MaterialType::None:
		default:
			return nullptr;
	}
}

