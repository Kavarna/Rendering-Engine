#pragma once


#include <vector>
#include "Jnrlib.h"
#include "Singletone.h"
#include "Scene.h"

#include "CreateInfo/SceneCreateInfo.h"
#include "CreateInfo/RayTracingCreateInfo.h"
#include "CreateInfo/MaterialCreateInfo.h"

namespace Common
{
    class SceneParser : public Jnrlib::ISingletone<SceneParser>
    {
        MAKE_SINGLETONE_CAPABLE(SceneParser);
    private:
        SceneParser();
        ~SceneParser();

    public:
        struct ParsedScene
        {
            CreateInfo::Scene sceneInfo;
            CreateInfo::RayTracing rendererInfo;
            std::vector<CreateInfo::Material> materialsInfo;
        };

        std::optional<ParsedScene> LoadSceneFromFile(std::string const& path);

    private:
        std::optional<ParsedScene> LoadSceneFromJSON(std::string const& path);


    };

}