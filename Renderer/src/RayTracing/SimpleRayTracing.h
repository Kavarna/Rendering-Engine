#pragma once

#include "Jnrlib.h"
#include "IDumper.h"
#include "Scene/Scene.h"
#include "Ray.h"
#include "Renderer.h"

namespace RayTracing
{

    class SimpleRayTracing : public Renderer
    {
    public:
        SimpleRayTracing(Common::IDumper& dumper, Common::Scene& scene, uint32_t maxDepth);

        void Render() override;
        void TracePixel(uint32_t x, uint32_t y) override;

    private:
        void RenderTile(uint32_t x, uint32_t y, uint32_t tileId);

    private:
        Common::IDumper& mDumper;
        Common::Scene& mScene;

        const uint32_t mMaxDepth;
    };

}