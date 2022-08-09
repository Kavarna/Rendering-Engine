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
    std::unique_ptr<Scene> LoadSceneFromFile(std::string const& path);

private:
    std::unique_ptr<Scene> LoadSceneFromJSON(std::string const& path);

};

