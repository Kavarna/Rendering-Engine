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
    class SceneViewer;
    class ObjectInspector : public ImguiWindow
    {
    public:
        ObjectInspector(SceneViewer* sceneViewer);
        ~ObjectInspector() = default;

        virtual void OnRender() override;

    public:
        void SetEntity(Common::Entity* entity, Common::Scene* scene);
        void ClearSelection();

    private:
        void RenderBase(Common::Components::Base& b, bool isUpdatable);
        void RenderSphere(Common::Components::Sphere& s, bool isUpdatable);
        void RenderCamera(Common::Components::Camera& c, bool isUpdatable);
        void RenderMesh(Common::Components::Mesh& c, bool isUpdatable);
        void RenderAccelerationStructure(Common::Components::AccelerationStructure& c, bool isUpdatable);

        void HandleAddComponent();

    private:
        Common::Entity *mActiveEntity = nullptr;
        Common::Scene *mActiveScene = nullptr;
        SceneViewer *mSceneViewer = nullptr;
    };
}
