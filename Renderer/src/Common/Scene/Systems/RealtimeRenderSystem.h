#pragma once

#include <Jnrlib.h>

#include "Vulkan/RootSignature.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/CommandList.h"

#include "Scene/Scene.h"
#include "Scene/Camera.h"

#include "LightHelpers.h"

namespace Common::Systems
{
    class RealtimeRender : public Jnrlib::ISingletone<RealtimeRender>
    {
    public:
        RealtimeRender(Common::Scene const* scene, Vulkan::CommandList* cmdList);
        ~RealtimeRender();

    public:
        void RenderScene(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);
        Vulkan::RootSignature* GetRootSiganture() const;

        void SetCamera(Camera const* camera);
        void SetLight(DirectionalLight const& light);

        void SelectIndices(std::unordered_set<uint32_t> const& selectedIndices);
        void ClearSelection();

        void OnResize(Vulkan::Image* renderTarget, Vulkan::Image* depthImage, uint32_t width, uint32_t height);

    public:
        Vulkan::Pipeline* GetDefaultPipeline();

    private:
        void InitRootSignatures();
        void InitPerObjectBuffer();
        void InitUniformBuffer();
        void InitMaterialsBuffer(Vulkan::CommandList* cmdList);
        void InitPipelines(uint32_t width, uint32_t height);

    private:
        Camera const* mCamera = nullptr;

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

        Vulkan::Image* mRenderTarget;
        Vulkan::Image* mDepthImage;

        // TODO: Make these a bit more dynamic (?) - take them as parameters
        std::unique_ptr<Vulkan::Buffer> mPerObjectBuffer;
        std::unique_ptr<Vulkan::DescriptorSet> mDefaultDescriptorSets;
        std::unique_ptr<Vulkan::RootSignature> mDefaultRootSignature;
        std::unique_ptr<Vulkan::RootSignature> mOutlineRootSignature;

        mutable Common::Scene const* mScene;
        mutable std::unordered_set<uint32_t> mSelectedIndices;
    };
}
