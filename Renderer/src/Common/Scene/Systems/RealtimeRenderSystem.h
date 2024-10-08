#pragma once

#include <Jnrlib.h>

#include "Vulkan/RootSignature.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/CommandList.h"

#include "Helpers/BatchRenderer.h"

#include "Scene/Scene.h"

#include "LightHelpers.h"

namespace Common
{
    class EditorCamera;
}

namespace Common::Components
{
    struct Base;
    struct Camera;
    struct AccelerationStructure;
}

namespace Common::Systems
{
    class RealtimeRender : public Jnrlib::ISingletone<RealtimeRender>
    {
    public:
        RealtimeRender(Common::Scene* scene, Vulkan::CommandList* cmdList);
        ~RealtimeRender();

    public:
        void RenderScene(Vulkan::CommandList* cmdList);
        Vulkan::RootSignature* GetRootSiganture() const;

        void SetDepthImage(Vulkan::Image* depthImage);
        void SetRenderTarget(Vulkan::Image* renderTarget);

        void SetCamera(EditorCamera const* camera);
        void SetLight(DirectionalLight const& light);

        void SelectEntities(std::unordered_set<Entity*> const& selectedIndices);
        void ClearSelection();

        bool GetDrawCameraFrustum() const;
        void SetDrawCameraFrustum(bool draw = false);

        void Update(float dt);

        void OnResize(uint32_t width, uint32_t height);

        void AddOneTimeVertex(glm::vec3 const& position, glm::vec4 const& color);
        void AddVertex(glm::vec3 const& position, glm::vec4 const& color, float timeInSeconds);

        void ReloadObjects();

    public:
        Vulkan::Pipeline* GetDefaultPipeline();

    private:
        void InitRootSignatures();
        void InitPerObjectBuffer();
        void InitUniformBuffer();
        void InitMaterialsBuffer(Vulkan::CommandList* cmdList);
        void InitPipelines(uint32_t width, uint32_t height);

    private:
        void DrawAccelerationStructure(Common::Components::Base const&, Common::Components::AccelerationStructure const& accelerationStructure);
        void DrawAccelerationStructures();
        void DrawCameraEntities();
        void DrawCameraEntity(Common::Components::Base const &baseComponent, Common::Components::Camera const &cameraComponent, bool isSelected);

    private:
        EditorCamera const* mCamera = nullptr;
        bool mDrawCameraFrustum = false;

        struct UniformBuffer
        {
            glm::mat4x4 viewProj;
        };

        struct OutlineVertPushConstants
        {
            uint32_t objectIndex;
            float lineWidth = 0.5f;
        };

        struct PerObjectInfo
        {
            glm::mat4x4 world;
            uint32_t materialIndex;
            glm::vec3 pad;
        };

        std::unique_ptr<Vulkan::Buffer> mUniformBuffer;
        std::unique_ptr<Vulkan::Buffer> mMaterialsBuffer;

        std::unique_ptr<Vulkan::Buffer> mLightBuffer;

        std::unique_ptr<Vulkan::Pipeline> mDefaultPipeline;
        std::unique_ptr<Vulkan::Pipeline> mSelectedObjectsPipeline;
        std::unique_ptr<Vulkan::Pipeline> mOutlinePipeline;
        std::unique_ptr<Vulkan::Pipeline> mDebugPipeline;

        Vulkan::Image* mRenderTarget;
        Vulkan::Image* mDepthImage;

        Common::BatchRenderer mBatchRenderer;

        std::unique_ptr<Vulkan::Buffer> mPerObjectBuffer;
        std::unique_ptr<Vulkan::Buffer> mOldPerObjectBuffer;
        std::unique_ptr<Vulkan::DescriptorSet> mDefaultDescriptorSets;

        std::unique_ptr<Vulkan::RootSignature> mDefaultRootSignature;
        std::unique_ptr<Vulkan::RootSignature> mOutlineRootSignature;
        
        Common::Scene* mScene;

        mutable std::unordered_set<Entity*> mSelectedEntities;
    };
}
