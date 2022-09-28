#include "SceneFactory.h"
#include <fstream>
#include <nlohmann/json.hpp> 
#include "CreateInfoUtils.h"

SceneFactory::SceneFactory()
{ }

SceneFactory::~SceneFactory()
{ }

std::unique_ptr<Scene> SceneFactory::LoadSceneFromFile(std::string const& path)
{
    if (auto scene = LoadSceneFromJSON(path); scene != nullptr) return scene;
    return nullptr;
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


std::unique_ptr<Scene> SceneFactory::LoadSceneFromJSON(std::string const& path)
{
    LOG(INFO) << "Attempting to load scene from JSON file " << path;

    nlohmann::json js;
    CreateInfo::Scene sceneInfo;
    CreateInfo::Camera cameraInfo;

    try
    {
        std::ifstream f(path.c_str());
        js = nlohmann::json::parse(f);
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Unable to parse " << path << " as JSON. Error: " << e.what();
        return nullptr;
    }

    try
    {
        CreateInfo::from_json(js, sceneInfo);
        if (js.contains("camera"))
        {
            CreateInfo::from_json(js["camera"], cameraInfo);
        }
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error while parsing the scene json: " << e.what();
        return nullptr;
    }

    LOG(INFO) << "Successfully loaded scene from JSON file" << path;

    std::unique_ptr<Scene> scene = std::make_unique<Scene>(sceneInfo);
    std::unique_ptr<Camera> camera = std::make_unique<Camera>(cameraInfo);

    scene->SetCamera(std::move(camera));

    return scene;
}

