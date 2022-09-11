#include "Scene.h"
#include <boost/algorithm/string.hpp>

RendererType GetRendererTypeFromString(std::string const& str)
{
    if (boost::iequals(str, "pathtracing"))
        return RendererType::PathTracing;
    return RendererType::None;
}

std::string GetStringFromRendererType(RendererType rendererType)
{
    switch (rendererType)
    {
        case RendererType::PathTracing:
            return "PathTracing";
        case RendererType::None:
        default:
            return "None";
    }
}

Scene::Scene(SceneCreateInfo const& info) :
    mInfo(info)
{
    LOG(INFO) << "Creating scene with info: " << info;
}

Scene::~Scene()
{ }

RendererType Scene::GetRendererType() const
{
    return mInfo.rendererType;
}

std::string Scene::GetOutputFile() const
{
    return mInfo.outputFile;
}

const Scene::SceneCreateInfo::ImageInfo& Scene::GetImageInfo() const
{
    return mInfo.imageInfo;
}
