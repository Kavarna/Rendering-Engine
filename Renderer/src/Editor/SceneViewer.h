#pragma once


#include "Scene/SceneParser.h"
#include "ImguiWindow.h"
#include "Constants.h"

#include "Vulkan/CommandList.h"
#include "Vulkan/RootSignature.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Buffer.h"

#include "Scene/Systems/RealtimeRenderSystem.h"

#include "Vertex.h"

#include "Pool.h"

namespace Editor
{
    class SceneViewer : public ImguiWindow
    {
    public:
        struct RenderingContext
        {
            uint32_t activeFrame = 0;
            Vulkan::CommandList* cmdList = nullptr;
            uint32_t cmdBufIndex = 0;
        };
    public:
        SceneViewer(Common::Scene const* scene, Vulkan::CommandList* cmdList);
        ~SceneViewer();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

    private:
        void OnResize(float newWidth, float newHeight);
        
        void Update();
        void RenderScene();

    private:
        void UpdateCamera(float dt);

    private:
        void InitDefaultPipeline();
        void InitRenderTargets();
        void InitCamera();

    private:
        float mWidth = 0.0f;
        float mHeight = 0.0f;
       
        Common::Scene const* mScene;

        RenderingContext mActiveRenderingContext{};

        struct PerFrameResources
        {
            std::unique_ptr<Vulkan::Image> renderTarget = nullptr;
            std::unique_ptr<Common::Systems::RealtimeRender> renderSystem = nullptr;
        };
        std::array<PerFrameResources, Common::Constants::FRAMES_IN_FLIGHT> mPerFrameResources;

        std::unique_ptr<Vulkan::Pipeline> mDefaultPipeline;
        std::unique_ptr<Vulkan::Image> mDepthImage;

        std::unique_ptr<Common::Camera> mCamera;
    };
}