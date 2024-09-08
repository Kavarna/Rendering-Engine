#pragma once

#include <Jnrlib.h>

namespace Common
{
    class Ray;
    class EditorCamera;
}

namespace Common::Components
{
    struct Camera;
    struct Base;
}

namespace Common
{
    class CameraUtils
    {
    public:
        static Ray GetRayForPixel(Common::EditorCamera const*, uint32_t x, uint32_t y);
        static Ray GetRayForPixel(Common::Components::Base const *baseComponent, Common::Components::Camera const*, uint32_t x, uint32_t y);
    };
}
