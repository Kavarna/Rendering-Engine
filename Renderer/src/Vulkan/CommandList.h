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

        void Begin(uint32_t cmdBufIndex = 0);
        void End(uint32_t cmdBufIndex = 0);

        void CopyBuffer(Vulkan::Buffer* dst, Vulkan::Buffer* src, uint32_t cmdBufIndex = 0);
        void CopyBuffer(Vulkan::Buffer* dst, uint32_t dstOffset, Vulkan::Buffer* src, uint32_t cmdBufIndex = 0);
        void CopyBuffer(Vulkan::Buffer* dst, uint32_t dstOffset, Vulkan::Buffer* src, uint32_t srcOffset, uint32_t cmdBufIndex = 0);

        void BindVertexBuffer(Vulkan::Buffer const* buffer, uint32_t firstIndex, uint32_t cmdBufIndex = 0);
        void BindVertexBuffers(Vulkan::Buffer const* buffers[], uint32_t firstIndex, uint32_t cmdBufIndex = 0);
        void BindIndexBuffer(Vulkan::Buffer const* buffer, uint32_t cmdBufIndex = 0);

        void BindPipeline(Pipeline* pipeline, uint32_t cmdBufIndex = 0);
        void BindDescriptorSet(DescriptorSet* set, uint32_t descriptorSetInstance, RootSignature* rootSignature, uint32_t cmdBufIndex = 0);
        void SetScissor(std::vector<VkRect2D> const& scissors, uint32_t cmdBufIndex = 0);
        void SetViewports(std::vector<VkViewport> const& viewports, uint32_t cmdBufIndex = 0);
        void Draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t cmdBufIndex = 0);
        void DrawIndexedInstanced(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t cmdBufIndex = 0);

        void TransitionBackbufferTo(TransitionInfo const& transitionInfo,  uint32_t cmdBufIndex = 0);
        void TransitionImageTo(Image* img, TransitionInfo const& transitionInfo, uint32_t cmdBufIndex = 0);
        void TransitionImageToImguiLayout(Image* img, uint32_t cmdBufIndex = 0);

        void BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor, uint32_t cmdBufIndex = 0);
        void BeginRenderingOnImage(Image* img, Jnrlib::Color const& backgroundColor, Image* depth, bool useStencil, uint32_t cmdBufIndex = 0);
        void EndRendering(uint32_t cmdBufIndex = 0);

        void InitImGui(uint32_t cmdBufIndex = 0);

        void UINewFrame(uint32_t cmdBufIndex = 0);
        void FlushUI(uint32_t cmdBufIndex = 0);

        void AddLocalBuffer(std::unique_ptr<Buffer>&& buffer);

        void Submit(CPUSynchronizationObject* signalWhenFinished);
        void SubmitToScreen(CPUSynchronizationObject* signalWhenFinished = nullptr);
        void SubmitAndWait();

    public:
        template <typename T>
        void BindPushRange(RootSignature* rootSignature, uint32_t offset, uint32_t count, T const* data,
                           VkShaderStageFlags stages = VK_SHADER_STAGE_ALL, uint32_t cmdBufIndex = 0)
        {
            jnrCmdPushConstants(mCommandBuffers[cmdBufIndex], rootSignature->mPipelineLayout, stages,
                                offset, sizeof(T) * count, data);
        }


    private:
        VkCommandPool mCommandPool;
        CommandListType mType;

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