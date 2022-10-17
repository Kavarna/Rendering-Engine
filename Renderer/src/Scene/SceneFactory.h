#pragma once


#include <vector>
#include "Jnrlib.h"
#include "Singletone.h"
#include "Scene.h"


class SceneFactory : public Jnrlib::ISingletone<SceneFactory>
{
    MAKE_SINGLETONE_CAPABLE(SceneFactory);
private:
    SceneFactory();
    ~SceneFactory();

public:
    struct ParsedScene
    {
        std::unique_ptr<Scene> scene;
        CreateInfo::Renderer rendererInfo;
    };

    std::optional<ParsedScene> LoadSceneFromFile(std::string const& path);

private:
    std::optional<ParsedScene> LoadSceneFromJSON(std::string const& path);


};

