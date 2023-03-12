#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Scene.h"


namespace Editor
{
    class SceneViewer;

    class SceneHierarchy : public ImguiWindow
    {
    public:
        SceneHierarchy(Common::Scene* scene, SceneViewer* sceneViewer);
        ~SceneHierarchy() = default;

        virtual void OnRender() override;

    public:
        uint32_t RenderNode(Entity* entity);

    private:
        Common::Scene* mScene;
        SceneViewer* mSceneViewer;

        std::unordered_set<uint32_t> mSelectedNodes;
    };
}
