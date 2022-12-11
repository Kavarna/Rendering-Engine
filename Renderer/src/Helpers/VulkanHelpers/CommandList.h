#pragma once

#include <Jnrlib.h>
#include "SynchronizationObjects.h"

class Pipeline;

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
        CommandList(VkCommandPool commandPool, CommandListType cmdListType);
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
        void SetScissor(std::vector<VkRect2D> const& scissors, uint32_t cmdBufIndex = 0);
        void SetViewports(std::vector<VkViewport> const& viewports, uint32_t cmdBufIndex = 0);
        void Draw(uint32_t vertexCount, uint32_t cmdBufIndex = 0);

        void TransitionBackbufferTo(TransitionInfo const& transitionInfo,  uint32_t cmdBufIndex = 0);

        void BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor, uint32_t cmdBufIndex = 0);
        void EndRendering(uint32_t cmdBufIndex = 0);

        void SubmitToScreen(CPUSynchronizationObject* signalWhenFinished = nullptr);

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