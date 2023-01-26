#pragma once


#include <vector>
#include "Jnrlib.h"
#include "Singletone.h"
#include "Scene.h"


class SceneParser : public Jnrlib::ISingletone<SceneParser>
{
    MAKE_SINGLETONE_CAPABLE(SceneParser);
private:
    SceneParser();
    ~SceneParser();

public:
    struct ParsedScene
    {
        // std::unique_ptr<Scene> scene;
        CreateInfo::Scene sceneInfo;
        CreateInfo::Renderer rendererInfo;
        CreateInfo::Camera cameraInfo;
    };

    std::optional<ParsedScene> LoadSceneFromFile(std::string const& path);

private:
    std::optional<ParsedScene> LoadSceneFromJSON(std::string const& path);


};

