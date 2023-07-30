#pragma once

#include "Jnrlib.h"
#include "IDumper.h"
#include "Scene/Scene.h"
#include "Ray.h"
#include "Renderer.h"

namespace RayTracing
{

    class PathTracing : public Renderer
    {
    public:
        PathTracing(Common::IDumper& dumper, Common::Scene& scene, uint32_t numSamples, uint32_t maxDepth);

        void Render() override;
        void TracePixel(uint32_t x, uint32_t y) override;

    private:

        Jnrlib::Color GetRayColor(Common::Ray&, uint32_t depth = 1);

    private:
        Common::IDumper& mDumper;
        Common::Scene& mScene;

        uint32_t mWidth;
        uint32_t mHeight;
        Jnrlib::Position mUpperLeftCorner;

        const uint32_t mNumSamples;
        const uint32_t mMaxDepth;
    };

}