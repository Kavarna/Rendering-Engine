#include "CommandList.h"
#include "VulkanLoader.h"
#include "Renderer.h"
#include "Pipeline.h"
#include "ImGuiImplementation.h"
#include "RootSignature.h"
#include "Image.h"

using namespace Vulkan;

CommandList::CommandList(CommandListType cmdListType) :
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

CommandList::~CommandList()
{
    auto device = Renderer::Get()->GetDevice();
    jnrDestroyCommandPool(device, mCommandPool, nullptr);
}

void CommandList::Init(uint32_t numCommandBuffers)
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

void CommandList::ResetAll()
{
    auto device = Renderer::Get()->GetDevice();
    ThrowIfFailed(jnrResetCommandPool(device, mCommandPool, 0));
}

void CommandList::Begin(uint32_t cmdBufIndex)
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

    /* TODO: To be more correct, we could do this after the CPUSynchronizationObject was triggered */
    mLayoutTracker.Flush();
    mMemoryTracker.Flush();
}

void CommandList::End(uint32_t cmdBufIndex)
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

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer* dst, Vulkan::Buffer* src, uint32_t cmdBufIndex)
{
    CopyBuffer(dst, 0, src, 0, cmdBufIndex);
}

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer * dst, uint32_t dstOffset, Vulkan::Buffer * src, uint32_t cmdBufIndex)
{
    CopyBuffer(dst, dstOffset, src, 0, cmdBufIndex);
}

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer * dst, uint32_t dstOffset, Vulkan::Buffer * src, uint32_t srcOffset, uint32_t cmdBufIndex)
{
    CHECK(dst->mCount >= src->mCount) << "Cannot copy a larger buffer into a smaller one";


    VkBufferCopy copyInfo{};
    {
        copyInfo.srcOffset = srcOffset;
        copyInfo.dstOffset = dstOffset;
        copyInfo.size = src->GetElementSize() * src->mCount;
    }
    jnrCmdCopyBuffer(mCommandBuffers[cmdBufIndex], src->mBuffer, dst->mBuffer, 1, &copyInfo);
}

void Vulkan::CommandList::BindVertexBuffer(Vulkan::Buffer const* buffer, uint32_t firstIndex, uint32_t cmdBufIndex)
{
    VkDeviceSize offsets[] = {0};
    jnrCmdBindVertexBuffers(mCommandBuffers[cmdBufIndex], firstIndex, 1, &buffer->mBuffer, offsets);
}

void Vulkan::CommandList::BindVertexBuffers(Vulkan::Buffer const* buffers[], uint32_t firstIndex, uint32_t cmdBufIndex)
{
    constexpr uint32_t count = sizeof(buffers) / sizeof(buffers[0]);
    VkBuffer _buffers[count] = {};
    VkDeviceSize offsets[] = {0};
    for (uint32_t i = 0; i < count; ++i)
    {
        _buffers[i] = buffers[i]->mBuffer;
    }
    jnrCmdBindVertexBuffers(mCommandBuffers[cmdBufIndex], firstIndex, count, _buffers, offsets);
}

void Vulkan::CommandList::BindIndexBuffer(Vulkan::Buffer const* buffer, uint32_t cmdBufIndex)
{
    VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
    if (buffer->GetElementSize() == sizeof(uint32_t))
        indexType = VK_INDEX_TYPE_UINT32;
    else if (buffer->GetElementSize() == sizeof(uint16_t))
        indexType = VK_INDEX_TYPE_UINT16;
    jnrCmdBindIndexBuffer(mCommandBuffers[cmdBufIndex], buffer->mBuffer, 0, indexType);
}

void CommandList::Draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t cmdBufIndex)
{
    jnrCmdDraw(mCommandBuffers[cmdBufIndex], vertexCount, 1, firstVertex, 0);
}

void Vulkan::CommandList::DrawIndexedInstanced(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t cmdBufIndex)
{
    jnrCmdDrawIndexed(mCommandBuffers[cmdBufIndex], indexCount, 1, firstIndex, vertexOffset, 0);
}

void CommandList::BindPipeline(Pipeline* pipeline, uint32_t cmdBufIndex)
{
    jnrCmdBindPipeline(mCommandBuffers[cmdBufIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->mPipeline);
}

void CommandList::BindDescriptorSet(DescriptorSet* set, uint32_t descriptorSetInstance, RootSignature* rootSignature, uint32_t cmdBufIndex)
{
    jnrCmdBindDescriptorSets(mCommandBuffers[cmdBufIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                             rootSignature->mPipelineLayout, 0, 1,
                             &set->mDescriptorSets[descriptorSetInstance], 0, nullptr);
}

void CommandList::SetScissor(std::vector<VkRect2D> const& scissors, uint32_t cmdBufIndex)
{
    jnrCmdSetScissor(mCommandBuffers[cmdBufIndex], 0, (uint32_t)scissors.size(), scissors.data());
}

void CommandList::SetViewports(std::vector<VkViewport> const& viewports, uint32_t cmdBufIndex)
{
    jnrCmdSetViewport(mCommandBuffers[cmdBufIndex], 0, (uint32_t)viewports.size(), viewports.data());
}

void CommandList::TransitionBackbufferTo(TransitionInfo const& transitionInfo, uint32_t cmdBufIndex)
{
    auto renderer = Renderer::Get();
    VkImageMemoryBarrier imageMemoryBarrier{};
    {
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = transitionInfo.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionInfo.dstAccessMask;
        imageMemoryBarrier.oldLayout = mLayoutTracker.GetBackbufferImageLayout(mImageIndex);
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

    mLayoutTracker.TransitionBackBufferImage(mImageIndex, transitionInfo.newLayout);
}

void CommandList::TransitionImageTo(Image* img, TransitionInfo const& transitionInfo, uint32_t cmdBufIndex)
{
    /* TODO: Do not transition if the old layout is already new layout */
    VkImageAspectFlags aspectMask = 0;
    if (transitionInfo.newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else if (transitionInfo.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
    {
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    VkImageMemoryBarrier imageMemoryBarrier{};
    {
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = transitionInfo.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionInfo.dstAccessMask;
        imageMemoryBarrier.oldLayout = mLayoutTracker.GetImageLayout(img);
        imageMemoryBarrier.newLayout = transitionInfo.newLayout;
        imageMemoryBarrier.image = img->mImage;

        imageMemoryBarrier.subresourceRange = {
            .aspectMask = aspectMask,
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

    mLayoutTracker.TransitionImage(img, transitionInfo.newLayout);
}

void CommandList::TransitionImageToImguiLayout(Image* img, uint32_t cmdBufIndex)
{
    TransitionInfo ti = {};
    {
        ti.newLayout = Image::IMGUI_IMAGE_LAYOUT;
        ti.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        ti.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        ti.srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    TransitionImageTo(img, ti, cmdBufIndex);
}

void CommandList::BeginRenderingOnBackbuffer(Jnrlib::Color const& backgroundColor, uint32_t cmdBufIndex)
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
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;
        renderingInfo.renderArea = {
            .offset = VkOffset2D{.x = 0, .y = 0},
            .extent = renderer->GetBackbufferExtent()
        };
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
    }

    jnrCmdBeginRendering(mCommandBuffers[cmdBufIndex], &renderingInfo);
}

void CommandList::BeginRenderingOnImage(Image* img, Jnrlib::Color const& backgroundColor, Image* depth, bool useStencil, uint32_t cmdBufIndex)
{
    auto renderer = Renderer::Get();
    /* TODO: Maybe batch these transitions? */
    /* Transition the image to color attachment */
    {
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            ti.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        TransitionImageTo(img, ti, cmdBufIndex);
    }
    if (depth)
    {
    /* Transition image to depth */
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            ti.newLayout = useStencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        TransitionImageTo(depth, ti, cmdBufIndex);
    }
    VkRenderingAttachmentInfo colorAttachment{};
    {
        VkClearValue clearValue{};
        clearValue.color.float32[0] = (float)backgroundColor.r;
        clearValue.color.float32[1] = (float)backgroundColor.g;
        clearValue.color.float32[2] = (float)backgroundColor.b;
        clearValue.color.float32[3] = (float)backgroundColor.a;

        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.imageView = img->GetImageView(VK_IMAGE_ASPECT_COLOR_BIT);
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValue;
    }
    VkImageAspectFlags depthAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthAspectFlags |= useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    VkRenderingAttachmentInfo depthAttachment{};
    {
        VkClearValue clearValue{};
        clearValue.depthStencil.depth = 1.0f;
        clearValue.depthStencil.stencil = 0;
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageLayout = useStencil ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.imageView = depth->GetImageView(depthAspectFlags);
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue = clearValue;
    }
    VkRenderingInfo renderingInfo{};
    {
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = depth != nullptr ? &depthAttachment : nullptr;
        renderingInfo.pStencilAttachment = useStencil ? &depthAttachment : nullptr;
        renderingInfo.renderArea = {
            .offset = VkOffset2D{.x = 0, .y = 0},
            .extent = img->GetExtent2D()
        };
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
    }

    jnrCmdBeginRendering(mCommandBuffers[cmdBufIndex], &renderingInfo);
}

void CommandList::EndRendering(uint32_t cmdBufIndex)
{
    jnrCmdEndRendering(mCommandBuffers[cmdBufIndex]);
}

void CommandList::InitImGui(uint32_t cmdBufIndex)
{
    ImGui_ImplVulkan_CreateFontsTexture(mCommandBuffers[cmdBufIndex]);
}

void CommandList::UINewFrame(uint32_t cmdBufIndex)
{
    ImGui_ImplVulkan_NewFrame(mCommandBuffers[cmdBufIndex]);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void CommandList::FlushUI(uint32_t cmdBufIndex)
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

void Vulkan::CommandList::AddLocalBuffer(std::unique_ptr<Buffer>&& buffer)
{
    mMemoryTracker.AddBuffer(std::move(buffer));
}

void Vulkan::CommandList::AddLocalImage(std::unique_ptr<Image>&& image)
{
    mMemoryTracker.AddImage(std::move(image));
}

void CommandList::Submit(CPUSynchronizationObject* signalWhenFinished)
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

void CommandList::SubmitToScreen(CPUSynchronizationObject* signalWhenFinished)
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

void CommandList::SubmitAndWait()
{
    CPUSynchronizationObject cpuWait;
    Submit(&cpuWait);
    cpuWait.Wait();
}
