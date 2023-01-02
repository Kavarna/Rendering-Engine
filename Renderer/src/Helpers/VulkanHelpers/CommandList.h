#pragma once

#include <Jnrlib.h>
#include "Buffer.h"
#include "SynchronizationObjects.h"
#include "RootSignature.h"

class Pipeline;
class DescriptorSet;

namespace Editor
{
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

        void BindPipeline(Pipeline* pipeline, uint32_t cmdBufIndex = 0);
        void BindDescriptorSet(DescriptorSet* set, uint32_t descriptorSetInstance, RootSignature* rootSignature, uint32_t cmdBufIndex = 0);
        void SetScissor(std::vector<VkRect2D> const& scissors, uint32_t cmdBufIndex = 0);
        void SetViewports(std::vector<VkViewport> const& viewports, uint32_t cmdBufIndex = 0);
        void Draw(uint32_t vertexCount, uint32_t cmdBufIndex = 0);

        void TransitionBackbufferTo(TransitionInfo const& transitionInfo,  uint32_t cmdBufIndex = 0);

        void BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor, uint32_t cmdBufIndex = 0);
        void EndRendering(uint32_t cmdBufIndex = 0);

        void InitImGui(uint32_t cmdBufIndex = 0);

        void BeginRenderingUI(uint32_t cmdBufIndex = 0);
        void EndRenderingUI(uint32_t cmdBufIndex = 0);

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

        template <typename T>
        void BindVertexBuffer(Buffer<T>* buffer, uint32_t cmdBufIndex = 0)
        {
            VkDeviceSize offsets[] = {0};
            jnrCmdBindVertexBuffers(mCommandBuffers[cmdBufIndex], 0, 1, &buffer->mBuffer, offsets);
        }

        template<typename T>
        void CopyBuffer(Buffer<T>* dst, Buffer<T>* src, uint32_t cmdBufIndex = 0)
        {
            CopyBuffer(dst, 0, src, 0, cmdBufIndex);
        }

        template<typename T>
        void CopyBuffer(Buffer<T>* dst, uint32_t dstOffset, Buffer<T>* src, uint32_t cmdBufIndex = 0)
        {
            CopyBuffer(dst, dstOffset, src, 0, cmdBufIndex);
        }

        template<typename T>
        void CopyBuffer(Buffer<T>* dst, uint32_t dstOffset, Buffer<T>* src, uint32_t srcOffset, uint32_t cmdBufIndex = 0)
        {
            CHECK(dst->mCount >= src->mCount) << "Cannot copy a larger buffer into a smaller one";


            VkBufferCopy copyInfo{};
            {
                copyInfo.srcOffset = srcOffset;
                copyInfo.dstOffset = dstOffset;
                copyInfo.size = sizeof(T) * src->mCount;
            }
            jnrCmdCopyBuffer(mCommandBuffers[cmdBufIndex], src->mBuffer, dst->mBuffer, 1, &copyInfo);
        }

    private:
        VkCommandPool mCommandPool;
        CommandListType mType;

        std::vector<VkCommandBuffer> mCommandBuffers;

        /* TODO: Maybe do something smarter once multiple synchronization objects will be needed */
        std::unique_ptr<GPUSynchronizationObject> mBackbufferAvailable = nullptr;
        std::unique_ptr<GPUSynchronizationObject> mRenderingFinished = nullptr;

        /* Current recording info */
        uint32_t mImageIndex;

    };
}