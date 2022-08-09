#include "SceneFactory.h"
#include <fstream>
#include <nlohmann/json.hpp> 

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
        throw Jnrlib::Exceptions::FieldNotFound(key, Jnrlib::GetTypeName<T>::value);
    }
}

std::unique_ptr<Scene> SceneFactory::LoadSceneFromJSON(std::string const& path)
{
    LOG(INFO) << "Attempting to load scene from JSON file " << path;

    nlohmann::json js;
    RendererType rendererType = RendererType::None;

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
         rendererType = GetRendererTypeFromString(get_js_field<std::string>(js, "renderer-type"));
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Unable to parse " << path << " as JSON. Error: " << e.what();
        return nullptr;
    }

    LOG(INFO) << "Successfully loaded scene from JSON file" << path;

    std::unique_ptr<Scene> scene = std::make_unique<Scene>(rendererType);
    return scene;
}

