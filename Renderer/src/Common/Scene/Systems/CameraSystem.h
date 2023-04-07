#pragma once

#include <entt/entt.hpp>
#include "HitPoint.h"

namespace Common
{
    class Ray;
    class HitPoint;
    class Scene;
}

namespace Common::Components
{
    struct Base;
    struct Camera;
}

namespace Common::Systems
{
    class Camera
    {
    public:
        Camera(Scene* scene);
        ~Camera() = default;
        
    public:
        void Update();

    private:
        void InitializeCameras();
        void UpdateCameras();

    public:
        Scene* mScene = nullptr;
        bool mFirstCall = true;

    };
}