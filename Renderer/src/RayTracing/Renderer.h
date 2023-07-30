#pragma once


#include "Scene/Scene.h"
#include "CreateInfo/RayTracingCreateInfo.h"

namespace RayTracing
{
    class Renderer
    {
    public:
        virtual void Render() = 0;
        virtual void TracePixel(uint32_t x, uint32_t y) = 0;
    };

    void RenderScene(std::unique_ptr<Common::Scene>& scene, CreateInfo::RayTracing const& rendererInfo);
}
