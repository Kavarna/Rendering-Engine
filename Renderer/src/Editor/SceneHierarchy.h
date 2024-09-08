#pragma once

#include "ImguiWindow.h"
#include <unordered_set>

namespace Common
{
    class Entity;
    class Scene;
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
        void SelectEntity(Common::Entity* entity);
        void ClearSelection();

    private:
        Common::Entity* RenderNode(Common::Entity* entity);

        void MenuPerNode(Common::Entity *entity);

    private:
        Common::Scene* mScene;
        SceneViewer* mSceneViewer;
        ObjectInspector* mObjectInspector;

        Common::Entity *mEntityToRename = nullptr;
        bool mRenameStarted = false;

        std::unordered_set<Common::Entity*> mSelectedEntities;
    };
}
