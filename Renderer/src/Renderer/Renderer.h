#pragma once


#include "Scene/Scene.h"
#include "CreateInfo/RendererCreateInfo.h"

namespace Renderer
{
    void RenderScene(std::unique_ptr<Common::Scene>& scene, CreateInfo::Renderer const& rendererInfo);
}
