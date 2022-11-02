#include "SceneFactory.h"
#include <fstream>
#include <nlohmann/json.hpp> 
#include "CreateInfoUtils.h"
#include "MaterialManager.h"

SceneFactory::SceneFactory()
{ }

SceneFactory::~SceneFactory()
{ }

std::optional<SceneFactory::ParsedScene> SceneFactory::LoadSceneFromFile(std::string const& path)
{
    if (auto parsedScene = LoadSceneFromJSON(path); parsedScene.has_value()) return parsedScene;
    return std::nullopt;
}

template <typename T>
T get_js_field(nlohmann::json& js, const char* key)
{
    try
    {
        return js[key].get<T>();
    }
    catch (...)
    {
        throw Jnrlib::Exceptions::FieldNotFound(key, Jnrlib::GetTypeName<T>());
    }
}

nlohmann::json get_sub_js(nlohmann::json& js, const char* key)
{
    try
    {
        return js[key];
    }
    catch (...)
    {
        throw Jnrlib::Exceptions::FieldNotFound(key, "nlohmann::json");
    }
}


std::optional<SceneFactory::ParsedScene> SceneFactory::LoadSceneFromJSON(std::string const& path)
{
    LOG(INFO) << "Attempting to load scene from JSON file " << path;

    nlohmann::json js;
    CreateInfo::Scene sceneInfo;
    CreateInfo::Camera cameraInfo;
    CreateInfo::Renderer rendererInfo;

    try
    {
        std::ifstream f(path.c_str());
        js = nlohmann::json::parse(f);
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Unable to parse " << path << " as JSON. Error: " << e.what();
        return std::nullopt;
    }

    try
    {
        if (js.contains("materials"))
        {
            LOG(INFO) << "Found materials, parsing materials";
            nlohmann::json materialsJson = js["materials"];
            std::vector<CreateInfo::Material> materials;
            materials.reserve(materialsJson.size());

            for (auto const& material : materialsJson)
            {
                CreateInfo::Material m;
                from_json(material, m);
                materials.emplace_back(m);
            }
            MaterialManager::Get()->AddMaterials(materials);
            LOG(INFO) << "Successfully parsed materials";
        }
        CreateInfo::from_json(js, sceneInfo);
        CreateInfo::from_json(js, rendererInfo);
        if (js.contains("camera"))
        {
            LOG(INFO) << "Found camera, parsing camera";
            CreateInfo::from_json(js["camera"], cameraInfo);
            LOG(INFO) << "Successfully parsed camera";
        }
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error while parsing the scene json: " << e.what();
        return std::nullopt;
    }

    LOG(INFO) << "Successfully loaded scene from JSON file" << path;

    std::unique_ptr<Scene> scene = std::make_unique<Scene>(sceneInfo);
    CreateInfo::ImageInfo outputFile = scene->GetImageInfo();
    cameraInfo.RecalculateViewport((uint32_t)outputFile.width, (uint32_t)outputFile.height);
    std::unique_ptr<Camera> camera = std::make_unique<Camera>(cameraInfo);
    scene->SetCamera(std::move(camera));

    return ParsedScene{
        .scene = std::move(scene),
        .rendererInfo = rendererInfo
    };
}

