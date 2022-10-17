#pragma once


#include "../Scene/Scene.h"
#include "CreateInfoUtils.h"


void RenderScene(std::unique_ptr<Scene>& scene, CreateInfo::Renderer const& rendererInfo);
