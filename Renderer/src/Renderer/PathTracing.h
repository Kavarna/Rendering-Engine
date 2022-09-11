#pragma once

#include "Jnrlib.h"
#include "PngDumper.h"
#include "../Scene/Scene.h"

class PathTracing
{
public:
    PathTracing(PngDumper& dumper, Scene const& scene);

    void Render();

private:
    PngDumper& mDumper;
    Scene const& mScene;

};

