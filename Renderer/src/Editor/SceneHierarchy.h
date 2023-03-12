#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Scene.h"


namespace Editor
{
    class SceneHierarchy : public ImguiWindow
    {
    public:
        SceneHierarchy(Common::Scene* scene);
        ~SceneHierarchy() = default;

        virtual void OnRender() override;

    public:
        uint32_t RenderNode(Entity* entity);

    private:
        Common::Scene* mScene;

        std::unordered_set<uint32_t> mSelectedNodes;
    };
}
