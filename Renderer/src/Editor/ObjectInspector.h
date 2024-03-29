#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Entity.h"

namespace Common::Components
{
    struct Base;
    struct Sphere;
    struct Camera;
    struct Mesh;
    struct AccelerationStructure;
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
        void RenderMesh(Common::Components::Mesh& c, bool isUpdatable);
        void RenderAccelerationStructure(Common::Components::AccelerationStructure& c, bool isUpdatable);

    private:
        Common::Entity* mActiveEntity;

    };
}
