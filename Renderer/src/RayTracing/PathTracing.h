#pragma once

#include "Jnrlib.h"
#include "IDumper.h"
#include "Scene/Scene.h"
#include "Ray.h"

namespace RayTracing
{

    class PathTracing
    {
    public:
        PathTracing(Common::IDumper& dumper, Common::Scene& scene, uint32_t numSamples, uint32_t maxDepth);

        void Render();

    private:
        void SetPixelColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Jnrlib::Position const& upperLeftCorner);

        Jnrlib::Color GetRayColor(Common::Ray const&, uint32_t depth = 1);

    private:
        Common::IDumper& mDumper;
        Common::Scene& mScene;

        const uint32_t mNumSamples;
        const uint32_t mMaxDepth;
    };

}