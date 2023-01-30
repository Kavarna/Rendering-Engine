#pragma once

#include "Jnrlib.h"
#include "PngDumper.h"
#include "Scene/Scene.h"
#include "Ray.h"

namespace Renderer
{

    class PathTracing
    {
    public:
        PathTracing(Common::PngDumper& dumper, Common::Scene const& scene, uint32_t numSamples, uint32_t maxDepth);

        void Render();

    private:
        void SetPixelColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Jnrlib::Position const& upperLeftCorner);

        Jnrlib::Color GetRayColor(Common::Ray const&, uint32_t depth = 1);

    private:
        Common::PngDumper& mDumper;
        Common::Scene const& mScene;

        const uint32_t mNumSamples;
        const uint32_t mMaxDepth;
    };

}