#pragma once


#include "Scene/SceneParser.h"
#include "ImguiWindow.h"
#include "Constants.h"

#include "Vulkan/CommandList.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"

#include "Scene/Systems/RealtimeRenderSystem.h"

namespace Common
{
    class Camera;
}


namespace Editor
{
    class SceneHierarchy;

    class SceneViewer : public ImguiWindow
    {
    public:
        struct RenderingContext
        {
            uint32_t activeFrame = 0;
            Vulkan::CommandList* cmdList = nullptr;
        };
    public:
        SceneViewer(Common::Scene* scene, Vulkan::CommandList* cmdList);
        ~SceneViewer();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

        void SelectEntities(std::unordered_set<Common::Entity*> const& selectedIndices);
        void ClearSelection();

        void SetSceneHierarchy(SceneHierarchy* hierarchy);

    private:
        void OnResize(float newWidth, float newHeight);
        
        void UpdatePassive();
        void UpdateActive();
        void RenderScene();

        void AddDebugVertex(glm::vec3 const& pos, glm::vec4 const& color, float time);

        void SelectObject();

    private:
        void UpdateCamera(float dt);

    private:
        void InitRenderTargets();
        void InitCamera();

    private:
        float mWidth = 0.0f;
        float mHeight = 0.0f;
       
        Common::Scene* mScene;
        SceneHierarchy* mSceneHierarchy;

        RenderingContext mActiveRenderingContext{};

        struct PerFrameResources
        {
            std::unique_ptr<Vulkan::Image> renderTarget = nullptr;
        };
        std::array<PerFrameResources, Common::Constants::FRAMES_IN_FLIGHT> mPerFrameResources;

        std::unique_ptr<Common::Systems::RealtimeRender> mRenderSystem;

        std::unique_ptr<Vulkan::Pipeline> mDefaultPipeline;
        std::unique_ptr<Vulkan::Image> mDepthImage;

        std::unique_ptr<Common::EditorCamera> mCamera;

        bool mIsMouseEnabled = true;
        glm::vec2 mLastMousePosition{};
    };
}