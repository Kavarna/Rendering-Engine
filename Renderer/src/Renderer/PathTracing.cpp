#include "PathTracing.h"


PathTracing::PathTracing(PngDumper& dumper, Scene const& scene) :
    mDumper(dumper),
    mScene(scene)
{ }

void PathTracing::Render()
{
    for (uint32_t y = 0; y < mDumper.GetHeight(); ++y)
    {
        for (uint32_t x = 0; x < mDumper.GetWidth(); ++x)
        {
            mDumper.SetPixelColor(x, y, 1.0f, 1.0f, 0.0f);
        }
    }
}
