#pragma once

#include <Jnrlib.h>

#include "Vulkan/RootSignature.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/CommandList.h"

#include "Scene/Scene.h"
#include "Scene/Camera.h"

namespace Common::Systems
{
    class RealtimeRender : public Jnrlib::ISingletone<RealtimeRender>
    {
    public:
        RealtimeRender(Common::Scene const* scene);
        ~RealtimeRender();

    public:
        void RenderScene(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);
        Vulkan::RootSignature* GetRootSiganture() const;

        void SetCamera(Camera const* camera);

    private:
        void InitDefaultRootSignature();
        void InitPerObjectBuffer();
        void InitUniformBuffer();

    private:

        Camera const* mCamera = nullptr;

        struct UniformBuffer
        {
            glm::mat4x4 viewProj;
        };

        struct PerObjectInfo
        {
            glm::mat4x4 world;
            uint32_t materialIndex;
            glm::vec3 pad;
        };

        std::unique_ptr<Vulkan::Buffer> mUniformBuffer;
        // TODO: Make these a bit more dynamic (?) - take them as parameters
        std::unique_ptr<Vulkan::Buffer> mPerObjectBuffer;
        std::unique_ptr<Vulkan::DescriptorSet> mDefaultDescriptorSets;
        std::unique_ptr<Vulkan::RootSignature> mDefaultRootSignature;

        mutable Common::Scene const* mScene;
    };
}
