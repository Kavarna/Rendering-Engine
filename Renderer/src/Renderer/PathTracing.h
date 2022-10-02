#pragma once

#include "Jnrlib.h"
#include "PngDumper.h"
#include "../Scene/Scene.h"
#include "Utils/Ray.h"

class PathTracing
{
public:
    PathTracing(PngDumper& dumper, Scene const& scene);

    void Render();

private:
    void SetPixelColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Jnrlib::Position const& upperLeftCorner);

    Jnrlib::Color GetRayColor(Renderer::Ray const&);

private:
    PngDumper& mDumper;
    Scene const& mScene;

};

