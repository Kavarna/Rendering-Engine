#pragma once

#include <Jnrlib.h>
#include "Buffer.h"
#include "SynchronizationObjects.h"
#include "RootSignature.h"
#include "LayoutTracker.h"
#include "MemoryTracker.h"


namespace Vulkan
{
    class Pipeline;
    class DescriptorSet;
    class Renderer;

    enum class CommandListType
    {
        Graphics = 0,
    };

    class CommandList
    {
    public:
        CommandList(CommandListType cmdListType);
        ~CommandList();
    
    public:
        struct TransitionInfo
        {
            VkImageLayout newLayout;
            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;
            VkAccessFlags srcAccessMask;
            VkAccessFlags dstAccessMask;
        };

    public:
        void Init(uint32_t numCommandBuffers = 1);
        void ResetAll();

        void Begin();
        void End();

        void CopyBuffer(Vulkan::Buffer* dst, Vulkan::Buffer* src);
        void CopyBuffer(Vulkan::Buffer* dst, uint32_t dstOffset, Vulkan::Buffer* src);
        void CopyBuffer(Vulkan::Buffer* dst, uint32_t dstOffset, Vulkan::Buffer* src, uint32_t srcOffset);

        void BindVertexBuffer(Vulkan::Buffer const* buffer, uint32_t firstIndex);
        void BindVertexBuffers(Vulkan::Buffer const* buffers[], uint32_t firstIndex);
        void BindIndexBuffer(Vulkan::Buffer const* buffer);

        void BindPipeline(Pipeline* pipeline);
        void BindDescriptorSet(DescriptorSet* set, uint32_t descriptorSetInstance, RootSignature* rootSignature);
        void SetScissor(std::vector<VkRect2D> const& scissors);
        void SetViewports(std::vector<VkViewport> const& viewports);
        void Draw(uint32_t vertexCount, uint32_t firstVertex);
        void DrawIndexedInstanced(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);

        void TransitionBackbufferTo(TransitionInfo const& transitionInfo);
        void TransitionImageTo(Image* img, TransitionInfo const& transitionInfo);
        void TransitionImageToImguiLayout(Image* img);

        void BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor);
        void BeginRenderingOnImage(Image* img, Jnrlib::Color const& backgroundColor, Image* depth, bool useStencil);
        void EndRendering();

        void InitImGui();

        void UINewFrame();
        void FlushUI();

        void AddLocalBuffer(std::unique_ptr<Buffer>&& buffer);
        void AddLocalImage(std::unique_ptr<Image>&& buffer);

        void Submit(CPUSynchronizationObject* signalWhenFinished);
        void SubmitToScreen(CPUSynchronizationObject* signalWhenFinished = nullptr);
        void SubmitAndWait();

    public:
        template <typename T>
        void BindPushRange(RootSignature* rootSignature, uint32_t offset, uint32_t count, T const* data,
                           VkShaderStageFlags stages = VK_SHADER_STAGE_ALL)
        {
            jnrCmdPushConstants(mCommandBuffers[mActiveCommandIndex], rootSignature->mPipelineLayout, stages,
                                offset, sizeof(T) * count, data);
        }


    private:
        VkCommandPool mCommandPool;
        CommandListType mType;

        uint32_t mActiveCommandIndex = 0;

        std::vector<VkCommandBuffer> mCommandBuffers;

        LayoutTracker mLayoutTracker;
        MemoryTracker mMemoryTracker;

        /* TODO: Maybe do something smarter once multiple synchronization objects will be needed */
        std::unique_ptr<GPUSynchronizationObject> mBackbufferAvailable = nullptr;
        std::unique_ptr<GPUSynchronizationObject> mRenderingFinished = nullptr;

        /* Current recording info */
        uint32_t mImageIndex;

    };
}