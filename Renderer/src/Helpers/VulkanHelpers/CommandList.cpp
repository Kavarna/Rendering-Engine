#include "CommandList.h"
#include "PipelineManager.h"
#include "Editor/VulkanLoader.h"
#include "Editor/Renderer.h"
#include "VulkanHelpers/Pipeline.h"
#include "Helpers/VulkanHelpers/ImGuiImplementation.h"

using namespace Editor;

Editor::CommandList::CommandList(CommandListType cmdListType) :
    mType(cmdListType)
{
    auto renderer = Renderer::Get();
    uint32_t queueIndex = (uint32_t)(-1);
    if (cmdListType == CommandListType::Graphics)
        queueIndex = renderer->mQueueIndices.graphicsFamily.value();
    CHECK(queueIndex != (uint32_t)(-1)) << "Invalid command list provided";

    VkCommandPoolCreateInfo poolInfo{};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueIndex;
    }

    ThrowIfFailed(jnrCreateCommandPool(renderer->mDevice, &poolInfo, nullptr, &mCommandPool));
}

Editor::CommandList::~CommandList()
{
    auto device = Renderer::Get()->GetDevice();
    jnrDestroyCommandPool(device, mCommandPool, nullptr);
}

void Editor::CommandList::Init(uint32_t numCommandBuffers)
{
    CHECK(numCommandBuffers > 0) << "There should be at least one command buffer initialised";

    auto device = Renderer::Get()->GetDevice();
    ResetAll();

    mCommandBuffers.resize(numCommandBuffers);
    VkCommandBufferAllocateInfo allocInfo{};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = numCommandBuffers;
        allocInfo.commandPool = mCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    ThrowIfFailed(jnrAllocateCommandBuffers(device, &allocInfo, mCommandBuffers.data()));
}

void Editor::CommandList::ResetAll()
{
    auto device = Renderer::Get()->GetDevice();
    ThrowIfFailed(jnrResetCommandPool(device, mCommandPool, 0));
}

void Editor::CommandList::Begin(uint32_t cmdBufIndex)
{
    auto renderer = Renderer::Get();

    VkCommandBufferBeginInfo beginInfo = {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    ThrowIfFailed(jnrBeginCommandBuffer(mCommandBuffers[cmdBufIndex], &beginInfo));    
    {
        /* Reset current recording info */
        mImageIndex = -1;
    }

}

void Editor::CommandList::End(uint32_t cmdBufIndex)
{
    auto renderer = Renderer::Get();
    if (mImageIndex != -1)
    {
        /* Then we must have used the back buffer so transition it back */
        {
            TransitionInfo ti{};
            {
                ti.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                ti.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                ti.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                ti.dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }
            TransitionBackbufferTo(ti, cmdBufIndex);
        }

    }

    ThrowIfFailed(jnrEndCommandBuffer(mCommandBuffers[cmdBufIndex]));
}

void Editor::CommandList::Draw(uint32_t vertexCount, uint32_t cmdBufIndex)
{
    jnrCmdDraw(mCommandBuffers[cmdBufIndex], vertexCount, 1, 0, 0);
}

void Editor::CommandList::BindPipeline(Pipeline* pipeline, uint32_t cmdBufIndex)
{
    jnrCmdBindPipeline(mCommandBuffers[cmdBufIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->mPipeline);
}

void Editor::CommandList::SetScissor(std::vector<VkRect2D> const& scissors, uint32_t cmdBufIndex)
{
    jnrCmdSetScissor(mCommandBuffers[cmdBufIndex], 0, (uint32_t)scissors.size(), scissors.data());
}

void Editor::CommandList::SetViewports(std::vector<VkViewport> const& viewports, uint32_t cmdBufIndex)
{
    jnrCmdSetViewport(mCommandBuffers[cmdBufIndex], 0, (uint32_t)viewports.size(), viewports.data());
}

void Editor::CommandList::TransitionBackbufferTo(TransitionInfo const& transitionInfo, uint32_t cmdBufIndex)
{
    auto renderer = Renderer::Get();
    VkImageMemoryBarrier imageMemoryBarrier{};
    {
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = transitionInfo.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionInfo.dstAccessMask;
        imageMemoryBarrier.oldLayout = renderer->mSwapchainImageLayouts[mImageIndex];
        imageMemoryBarrier.newLayout = transitionInfo.newLayout;
        imageMemoryBarrier.image = renderer->mSwapchainImages[mImageIndex];
       
        imageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    jnrCmdPipelineBarrier(
        mCommandBuffers[cmdBufIndex],
        transitionInfo.srcStage,
        transitionInfo.dstStage,
        0, 0, nullptr, 0, nullptr,
        1, &imageMemoryBarrier
    );

    renderer->mSwapchainImageLayouts[mImageIndex] = transitionInfo.newLayout;
}

void Editor::CommandList::BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor, uint32_t cmdBufIndex)
{
    if (mBackbufferAvailable == nullptr)
        mBackbufferAvailable = std::make_unique<GPUSynchronizationObject>();

    auto renderer = Renderer::Get();
    mImageIndex = renderer->AcquireNextImage(mBackbufferAvailable.get());
    {
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            ti.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        TransitionBackbufferTo(ti, cmdBufIndex);
    }

    VkClearValue clearValue{};
    clearValue.color.float32[0] = (float)backgroundColor.r;
    clearValue.color.float32[1] = (float)backgroundColor.g;
    clearValue.color.float32[2] = (float)backgroundColor.b;
    clearValue.color.float32[3] = (float)backgroundColor.a;
    VkRenderingAttachmentInfo colorAttachment{};
    {
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.imageView = renderer->GetSwapchainImageView(mImageIndex);
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValue;
    }
    VkRenderingInfo renderingInfo{};
    {
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = nullptr; /* TODO: Fill this */
        renderingInfo.pStencilAttachment = nullptr; /* TODO: Fill this */
        renderingInfo.renderArea = {
            .offset = VkOffset2D{.x = 0, .y = 0},
            .extent = renderer->GetBackbufferExtent()
        };
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
    }

    jnrCmdBeginRendering(mCommandBuffers[cmdBufIndex], &renderingInfo);
}

void Editor::CommandList::EndRendering(uint32_t cmdBufIndex)
{
    jnrCmdEndRendering(mCommandBuffers[cmdBufIndex]);
}

void Editor::CommandList::InitImGui(uint32_t cmdBufIndex)
{
    ImGui_ImplVulkan_CreateFontsTexture(mCommandBuffers[cmdBufIndex]);
}

void Editor::CommandList::BeginRenderingUI(uint32_t cmdBufIndex)
{
    ImGui_ImplVulkan_NewFrame(mCommandBuffers[cmdBufIndex]);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Editor::CommandList::EndRenderingUI(uint32_t cmdBufIndex)
{
    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCommandBuffers[cmdBufIndex]);

    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

}

void Editor::CommandList::Submit(CPUSynchronizationObject* signalWhenFinished)
{
    auto renderer = Renderer::Get();
    VkSubmitInfo submitInfo{};
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
        submitInfo.pCommandBuffers = mCommandBuffers.data();
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
    }

    if (mType == CommandListType::Graphics)
    {
        ThrowIfFailed(
            jnrQueueSubmit(renderer->mGraphicsQueue, 1, &submitInfo, signalWhenFinished == nullptr ? VK_NULL_HANDLE : signalWhenFinished->GetFence())
        );
    }
}

void Editor::CommandList::SubmitToScreen(CPUSynchronizationObject* signalWhenFinished)
{
    if (mRenderingFinished == nullptr)
        mRenderingFinished = std::make_unique<GPUSynchronizationObject>();
    
    auto renderer = Renderer::Get();
    {
        /* Submit the command buffers */
        VkSemaphore waitSemaphores[] = {mBackbufferAvailable->GetSemaphore()};
        VkSemaphore signalSemaphores[] = {mRenderingFinished->GetSemaphore()};

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo{};
        {
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
            submitInfo.pCommandBuffers = mCommandBuffers.data();
            submitInfo.waitSemaphoreCount = ARRAYSIZE(waitSemaphores);
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.signalSemaphoreCount = ARRAYSIZE(signalSemaphores);
            submitInfo.pSignalSemaphores = signalSemaphores;
        }

        if (mType == CommandListType::Graphics)
        {
            ThrowIfFailed(
                jnrQueueSubmit(renderer->mGraphicsQueue, 1, &submitInfo, signalWhenFinished == nullptr ? VK_NULL_HANDLE : signalWhenFinished->GetFence())
            );
        }
    }

    {
        VkSemaphore waitSemaphores[] = {mRenderingFinished->GetSemaphore()};
        VkSwapchainKHR swapchains[] = {
            renderer->mSwapchain
        };
        uint32_t imageIndices[] = {mImageIndex};

        VkPresentInfoKHR presentInfo{};
        {
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pWaitSemaphores = waitSemaphores;
            presentInfo.waitSemaphoreCount = ARRAYSIZE(waitSemaphores);
            presentInfo.swapchainCount = ARRAYSIZE(swapchains);
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = imageIndices;
        }

        ThrowIfFailed(jnrQueuePresentKHR(renderer->mPresentQueue, &presentInfo));
    }
}

void Editor::CommandList::SubmitAndWait()
{
    CPUSynchronizationObject cpuWait;
    Submit(&cpuWait);
    cpuWait.Wait();
}
