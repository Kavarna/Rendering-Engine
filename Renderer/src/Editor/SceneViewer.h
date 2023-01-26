#pragma once


#include "Scene/SceneFactory.h"
#include "ImguiWindow.h"

#include "VulkanHelpers/RootSignature.h"
#include "VulkanHelpers/Image.h"
#include "VulkanHelpers/Pipeline.h"
#include "VulkanHelpers/Buffer.h"

#include "Pool.h"

namespace Editor
{
    class SceneViewer : public ImguiWindow
    {
    public:
        struct RenderingContext
        {
            uint32_t activeFrame = 0;
            CommandList* cmdList = nullptr;
            uint32_t cmdBufIndex = 0;
        };
    public:
        SceneViewer(uint32_t maxFrames, SceneFactory::ParsedScene const* scene);
        ~SceneViewer();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

    private:
        void OnResize(float newWidth, float newHeight);
        void RenderScene();

    private:
        void InitDefaultRootSignature();
        void InitDefaultPipeline();
        void InitRenderTargets();
        void InitTestVertexBuffer();

    private:
        float mWidth = 0.0f;
        float mHeight = 0.0f;

        uint32_t mMaxFrames = 0;

        struct Vertex
        {
            glm::vec3 position;
        };

        RenderingContext mActiveRenderingContext{};

        struct PerObject
        {
            glm::mat4 worldViewProjection;
        };
        struct PerFrameResources
        {
            std::unique_ptr<Image> renderTarget = nullptr;
            std::unique_ptr<Buffer<PerObject>> objectBuffer = nullptr;

            PerFrameResources() = default;
            ~PerFrameResources() = default;

            PerFrameResources(PerFrameResources&&) = default;
            PerFrameResources& operator=(PerFrameResources&&) = default;

            PerFrameResources(PerFrameResources const&) = delete;
            PerFrameResources& operator=(PerFrameResources const&) = delete;
        };
        std::vector<PerFrameResources> mPerFrameResources;

        std::shared_ptr<DescriptorSet> mDefaultDescriptorSets;
        std::unique_ptr<RootSignature> mDefaultRootSignature;
        std::unique_ptr<Pipeline> mDefaultPipeline;

        std::unique_ptr<Image> mDepthImage;

        std::unique_ptr<Buffer<Vertex>> mTestVertexBuffer;
    };
}