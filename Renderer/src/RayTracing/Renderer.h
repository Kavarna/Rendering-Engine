#pragma once


#include "Scene/Scene.h"
#include "CreateInfo/RayTracingCreateInfo.h"

namespace RayTracing
{
    void RenderScene(std::unique_ptr<Common::Scene>& scene, CreateInfo::RayTracing const& rendererInfo);
}
