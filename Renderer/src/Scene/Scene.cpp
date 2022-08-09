#include "Scene.h"
#include <boost/algorithm/string.hpp>

RendererType GetRendererTypeFromString(std::string const& str)
{
    if (boost::iequals(str, "pathtracing"))
        return RendererType::PathTracing;
    return RendererType::None;
}

Scene::Scene(RendererType type) :
    mRendererType(type)
{
}

Scene::~Scene()
{ }

RendererType Scene::GetRendererType() const
{
    return mRendererType;
}
