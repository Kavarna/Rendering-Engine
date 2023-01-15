#include "SceneViewer.h"

#include "imgui.h"

#include "VulkanHelpers/CommandList.h"
#include "VulkanHelpers/Buffer.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Editor::SceneViewer::SceneViewer(uint32_t maxFrames, SceneFactory::ParsedScene const* scene) :
    mMaxFrames(maxFrames)
{
    CHECK(maxFrames >= 1) << "There should be at least one frame";
    mPerFrameResources.resize(maxFrames);
    InitTestVertexBuffer();
    InitDefaultRootSignature();
}

Editor::SceneViewer::~SceneViewer()
{
    mTestVertexBuffer.reset();
    mDefaultPipeline.reset();
    mPerFrameResources.clear();
}

void Editor::SceneViewer::SetRenderingContext(RenderingContext const& ctx)
{
    CHECK(ctx.activeFrame < mMaxFrames) << "Active frame (" << ctx.activeFrame << ") must be less than max frames(" << mMaxFrames << ")";
    mActiveRenderingContext = ctx;
}

void Editor::SceneViewer::OnRender()
{
    ImGui::Begin("Scene viewer");

    auto frameHeight = ImGui::GetFrameHeight();
    float width, height;
    width = ImGui::GetWindowWidth() - frameHeight; /* One pixel on the left, one pixel on the right */
    height = ImGui::GetWindowHeight() - 2 * frameHeight;

    if (width != mWidth || height != mHeight && height != 0)
    {
        OnResize(width, height);
    }
   
    if (mActiveRenderingContext.cmdList)
    {
        RenderScene();

        auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
        auto textureId = currentFrameResources.renderTarget->GetTextureID();
        ImVec2 size;
        size.x = (float)currentFrameResources.renderTarget->GetExtent2D().width;
        size.y = (float)currentFrameResources.renderTarget->GetExtent2D().height;
        ImGui::Image(textureId, size);
    }
    else
    {
        LOG(WARNING) << "Scene viewer doesn't have a proper rendering context set";
    }

    ImGui::End();
}

void Editor::SceneViewer::RenderScene()
{
    if (mActiveRenderingContext.cmdList == nullptr)
        return;

    auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
    auto& cmdList = mActiveRenderingContext.cmdList;
    auto cmdBufIndex = mActiveRenderingContext.cmdBufIndex;

    static float angle = 0.0f;
    angle += 0.00001f;

    /* Update part */
    {
        for (uint32_t i = 0; i < 2; ++i)
        {
            auto* el = currentFrameResources.objectBuffer->GetElement(i);
            int multiplier = 1;
            if (i == 1)
                multiplier = -1;
            el->worldViewProjection = glm::rotate(glm::identity<glm::mat4>(), multiplier * angle * glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
        }

    }

    cmdList->BeginRenderingOnImage(currentFrameResources.renderTarget.get(), Jnrlib::Black, cmdBufIndex);
    {
        cmdList->BindPipeline(mDefaultPipeline.get(), cmdBufIndex);
        cmdList->BindVertexBuffer(mTestVertexBuffer.get(), cmdBufIndex);
        
        /* Draw first square */
        {
            glm::vec4 color = Jnrlib::Yellow;
            cmdList->BindPushRange<glm::vec4>(mDefaultRootSignature.get(), 0, 1, &color, VK_SHADER_STAGE_FRAGMENT_BIT, cmdBufIndex);
            cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 2 * mActiveRenderingContext.activeFrame + 0, mDefaultRootSignature.get(), cmdBufIndex);

            cmdList->Draw(6, 0, cmdBufIndex);
        }

        /* Draw second square */
        {
            glm::vec4 color = Jnrlib::Red;
            cmdList->BindPushRange<glm::vec4>(mDefaultRootSignature.get(), 0, 1, &color, VK_SHADER_STAGE_FRAGMENT_BIT, cmdBufIndex);
            cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 2 * mActiveRenderingContext.activeFrame + 1, mDefaultRootSignature.get(), cmdBufIndex);

            cmdList->Draw(6, 6, cmdBufIndex);
        }

    }
    cmdList->EndRendering(cmdBufIndex);
    cmdList->TransitionImageToImguiLayout(currentFrameResources.renderTarget.get(), cmdBufIndex);
}

void Editor::SceneViewer::OnResize(float newWidth, float newHeight)
{   
    Editor::Renderer::Get()->WaitIdle();

    mWidth = newWidth;
    mHeight = newHeight;
    VLOG(2) << "Scene viewer resized to (" << newWidth << "x" << newHeight << ")";

    InitRenderTargets();
    InitDefaultPipeline();
}

void Editor::SceneViewer::InitDefaultRootSignature()
{
    mDefaultDescriptorSets = std::make_unique<DescriptorSet>();
    {
        mDefaultDescriptorSets->AddInputBuffer(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
    }
    mDefaultDescriptorSets->Bake(2 * mMaxFrames);
    mDefaultRootSignature = std::make_unique<RootSignature>();
    {
        mDefaultRootSignature->AddPushRange<glm::vec4>(0, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        mDefaultRootSignature->AddDescriptorSet(mDefaultDescriptorSets.get());
    }
    mDefaultRootSignature->Bake();

    for (uint32_t i = 0; i < mMaxFrames; ++i)
    {
        mPerFrameResources[i].objectBuffer = std::make_shared<Buffer<PerObject>>(
            2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        mDefaultDescriptorSets->AddInputBuffer<PerObject>(mPerFrameResources[i].objectBuffer.get(), 0, 0, 2 * i + 0);
        mDefaultDescriptorSets->AddInputBuffer<PerObject>(mPerFrameResources[i].objectBuffer.get(), 0, 1, 2 * i + 1);
    }
}

void Editor::SceneViewer::InitDefaultPipeline()
{
    mDefaultPipeline = std::make_unique<Pipeline>("SceneViewer_DefaultPipeline");
    {
        mDefaultPipeline->AddShader("Shaders/basic.vert.spv");
        mDefaultPipeline->AddShader("Shaders/basic.frag.spv");
    }
    VkViewport vp{};
    VkRect2D sc{};
    auto& viewport = mDefaultPipeline->GetViewportStateCreateInfo();
    {
        vp.width = (FLOAT)mWidth; vp.minDepth = 0.0f; vp.x = 0;
        vp.height = (FLOAT)mHeight; vp.maxDepth = 1.0f; vp.y = 0;
        sc.offset = {.x = 0, .y = 0}; sc.extent = {.width = (uint32_t)mWidth, .height = (uint32_t)mHeight};
        viewport.viewportCount = 1;
        viewport.pViewports = &vp;
        viewport.scissorCount = 1;
        viewport.pScissors = &sc;
    }
    VkVertexInputAttributeDescription attributeDescription{};
    {
        attributeDescription.binding = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.location = 0;
        attributeDescription.offset = offsetof(Vertex, position);
    }

    VkVertexInputBindingDescription bindingDescription{};
    {
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
    }

    auto& vertexInput = mDefaultPipeline->GetVertexInputStateCreateInfo();
    {
        vertexInput.vertexAttributeDescriptionCount = 1;
        vertexInput.pVertexAttributeDescriptions = &attributeDescription;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDescription;

    }

    VkPipelineColorBlendAttachmentState attachmentInfo{};
    auto& blendState = mDefaultPipeline->GetColorBlendStateCreateInfo();
    {
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }

    auto& rasterizerState = mDefaultPipeline->GetRasterizationStateCreateInfo();
    {
        rasterizerState.cullMode = VK_CULL_MODE_NONE;
    }
    mDefaultPipeline->SetRootSignature(mDefaultRootSignature.get());
    mDefaultPipeline->AddImageColorOutput(mPerFrameResources[0].renderTarget.get());
    mDefaultPipeline->Bake();
}

void Editor::SceneViewer::InitRenderTargets()
{
    Image::Info2D info;
    {
        info.width = (uint32_t)mWidth;
        info.height = (uint32_t)mHeight;
        info.format = VK_FORMAT_B8G8R8A8_UNORM;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Image::IMGUI_IMAGE_LAYOUT;
    }
    for (auto& frameResources : mPerFrameResources)
    {
        frameResources.renderTarget.reset();
        frameResources.renderTarget = std::make_shared<Image>(info);
    }
}

void Editor::SceneViewer::InitTestVertexBuffer()
{
    Vertex vertices[] = {
        /* First quad */
        {glm::vec3(0.5, -0.5, 0.1)},
        {glm::vec3(0.5, 0.5, 0.1)},
        {glm::vec3(-0.5, 0.5, 0.1)},
        {glm::vec3(0.5, -0.5, 0.1)},
        {glm::vec3(-0.5, 0.5, 0.1)},
        {glm::vec3(-0.5, -0.5, 0.1)},

        /* Second quad */
        {glm::vec3(0.5, -0.5, 0.0)},
        {glm::vec3(0.5, 0.5, 0.0)},
        {glm::vec3(-0.5, 0.5, 0.0)},
        {glm::vec3(0.5, -0.5, 0.0)},
        {glm::vec3(-0.5, 0.5, 0.0)},
        {glm::vec3(-0.5, -0.5, 0.0)},
    };

    mTestVertexBuffer = std::make_unique<Buffer<Vertex>>(sizeof(vertices) / sizeof(vertices[0]),
                                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    mTestVertexBuffer->Copy(vertices);

}
