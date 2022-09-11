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
    RendererType rendererType = RendererType::None;
    std::string outputFile = "";
    uint64_t width = 0, height = 0;

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
        outputFile = get_js_field<std::string>(js, "output-file");

        auto imageInfo = get_sub_js(js, "image-info");

        width = get_js_field<uint64_t>(imageInfo, "width");
        height = get_js_field<uint64_t>(imageInfo, "height");
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Unable to parse " << path << " as JSON. Error: " << e.what();
        return nullptr;
    }

    LOG(INFO) << "Successfully loaded scene from JSON file" << path;

    Scene::SceneCreateInfo sceneInfo;
    sceneInfo.outputFile = outputFile;
    sceneInfo.rendererType = rendererType;
    
    sceneInfo.imageInfo.width = width;
    sceneInfo.imageInfo.height = height;
    
    std::unique_ptr<Scene> scene = std::make_unique<Scene>(sceneInfo);
    return scene;
}

