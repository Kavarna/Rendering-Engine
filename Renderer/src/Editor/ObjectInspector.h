#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Entity.h"

namespace Common::Components
{
    struct Base;
    struct Sphere;
    struct Camera;
}

namespace Editor
{

    class ObjectInspector : public ImguiWindow
    {
    public:
        ObjectInspector() = default;
        ~ObjectInspector() = default;

        virtual void OnRender() override;

    public:
        void SetEntity(Common::Entity* entity);
        void ClearSelection();

    private:
        void RenderBase(Common::Components::Base& b, bool isUpdatable);
        void RenderSphere(Common::Components::Sphere& s, bool isUpdatable);
        void RenderCamera(Common::Components::Camera& c, bool isUpdatable);

    private:
        Common::Entity* mActiveEntity;

    };
}
