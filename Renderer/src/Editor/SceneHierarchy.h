#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Scene.h"

namespace Common
{
    class Entity;
}

namespace Editor
{
    class SceneViewer;
    class ObjectInspector;

    class SceneHierarchy : public ImguiWindow
    {
    public:
        SceneHierarchy(Common::Scene* scene, SceneViewer* sceneViewer, ObjectInspector* objInspector);
        ~SceneHierarchy() = default;

        virtual void OnRender() override;

    public:
        void SelectEntity(Common::Entity& entity);
        void ClearSelection();

    private:
        Common::Entity* RenderNode(Common::Entity* entity);

    private:
        Common::Scene* mScene;
        SceneViewer* mSceneViewer;
        ObjectInspector* mObjectInspector;

        std::unordered_set<uint32_t> mSelectedNodes;
    };
}
