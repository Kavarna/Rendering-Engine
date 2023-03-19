#include "RealtimeRenderSystem.h"

#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/SphereComponent.h"
#include "Scene/Components/UpdateComponent.h"

#include "MaterialManager.h"

#include "glm/gtc/matrix_transform.hpp"

using namespace Common;
using namespace Systems;
using namespace Vulkan;
using namespace Components;

RealtimeRender::RealtimeRender(Scene const* scene, CommandList* cmdList) :
    mScene(scene)
{
    InitRootSignatures();
    InitPerObjectBuffer();
    InitUniformBuffer();
    InitMaterialsBuffer(cmdList);
}

RealtimeRender::~RealtimeRender()
{
}

void RealtimeRender::RenderScene(CommandList* cmdList, uint32_t cmdBufIndex)
{
    auto vertexBuffer = mScene->GetVertexBuffer();
    auto indexBuffer = mScene->GetIndexBuffer();
    cmdList->BindVertexBuffer(vertexBuffer, 0, cmdBufIndex);
    cmdList->BindIndexBuffer(indexBuffer, cmdBufIndex);

    auto const& spheres = mScene->mRegistry.group<const Base, const Components::Update, const Mesh>(entt::get<const Sphere>);
    {
        /* Build rendering buffers */
        for (auto const& [entity, base, update, mesh, sphere] : spheres.each())
        {
            if (update.dirtyFrames > 0)
            {
                PerObjectInfo* objectInfo = (PerObjectInfo*)mPerObjectBuffer->GetElement(update.bufferIndex);

                objectInfo->world = glm::translate(glm::identity<glm::mat4x4>(), base.position);
                objectInfo->world = glm::scale(objectInfo->world, base.scaling);
                objectInfo->materialIndex = sphere.material->GetMaterialIndex();
            }
        }

        {
            UniformBuffer* uniformBuffer = (UniformBuffer*)mUniformBuffer->GetElement();
            if (mCamera)
            {
                if (mCamera->GetDirtyFrames() > 0)
                {
                    uniformBuffer->viewProj = mCamera->GetProjection() * mCamera->GetView();
                }
            }
            else
            {
                uniformBuffer->viewProj = glm::identity<glm::mat4x4>();
            }
        }
    }

    cmdList->BeginRenderingOnImage(mRenderTarget, Jnrlib::Black, mDepthImage, true, cmdBufIndex);
    cmdList->BindPipeline(mDefaultPipeline.get(), cmdBufIndex);
    cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mDefaultRootSignature.get(), cmdBufIndex);

    {
        /* Render */
        for (auto const& [entity, base, update, mesh, sphere] : spheres.each())
        {
             if (mSelectedIndices.find((uint32_t)entity) != mSelectedIndices.end())
                continue;

            /* TODO: pass this as instance */
            uint32_t index = update.bufferIndex;
            cmdList->BindPushRange<uint32_t>(mDefaultRootSignature.get(), 0, 1, &index, VK_SHADER_STAGE_VERTEX_BIT, cmdBufIndex);
            cmdList->DrawIndexedInstanced(mesh.indexCount, mesh.firstIndex, mesh.firstVertex, cmdBufIndex);
        }
    }

    if (mSelectedIndices.size())
    {
        cmdList->BindPipeline(mSelectedObjectsPipeline.get(), cmdBufIndex);
        
        /* Render selected objects */
        for (const auto& entity : mSelectedIndices)
        {
            auto& update = mScene->mRegistry.get<Components::Update>((entt::entity)entity);
            auto& mesh = mScene->mRegistry.get<Mesh>((entt::entity)entity);
            uint32_t index = update.bufferIndex;
            cmdList->BindPushRange<uint32_t>(mDefaultRootSignature.get(), 0, 1, &index, VK_SHADER_STAGE_VERTEX_BIT, cmdBufIndex);
            cmdList->DrawIndexedInstanced(mesh.indexCount, mesh.firstIndex, mesh.firstVertex, cmdBufIndex);
        }

        cmdList->BindPipeline(mOutlinePipeline.get(), cmdBufIndex);
        cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mOutlineRootSignature.get(), cmdBufIndex);
        /* Render the outlines */
        for (const auto& entity : mSelectedIndices)
        {
            auto& update = mScene->mRegistry.get<Components::Update>((entt::entity)entity);
            auto& mesh = mScene->mRegistry.get<Mesh>((entt::entity)entity);
            uint32_t index = update.bufferIndex;

            OutlineVertPushConstants outlineVertPushConstants{.objectIndex = index, .lineWidth = 0.025f};
            cmdList->BindPushRange<OutlineVertPushConstants>(mOutlineRootSignature.get(), 0, 1, &outlineVertPushConstants,
                                                             VK_SHADER_STAGE_VERTEX_BIT, cmdBufIndex);
            cmdList->DrawIndexedInstanced(mesh.indexCount, mesh.firstIndex, mesh.firstVertex, cmdBufIndex);
        }
    }
    cmdList->BindPipeline(mDebugPipeline.get(), cmdBufIndex);
    mBatchRenderer.Begin();
    mBatchRenderer.End(cmdList, cmdBufIndex);
    cmdList->EndRendering(cmdBufIndex);
}

RootSignature* RealtimeRender::GetRootSiganture() const
{
    return mDefaultRootSignature.get();
}

void RealtimeRender::SetCamera(Camera const* camera)
{
    mCamera = camera;
}

void RealtimeRender::SetLight(DirectionalLight const& light)
{
    auto memory = mLightBuffer->GetElement();
    memcpy(memory, &light, sizeof(DirectionalLight));
    ((DirectionalLight*)memory)->direction = glm::normalize(((DirectionalLight*)memory)->direction);
}

void RealtimeRender::SelectIndices(std::unordered_set<uint32_t> const& selectedIndices)
{
    mSelectedIndices = selectedIndices;
}

void RealtimeRender::ClearSelection()
{
    mSelectedIndices.clear();
}

void RealtimeRender::Update(float dt)
{
    mBatchRenderer.Update(dt);
}

void RealtimeRender::OnResize(Image* renderTarget, Image* depthImage, uint32_t width, uint32_t height)
{
    mRenderTarget = renderTarget;
    mDepthImage = depthImage;
    InitPipelines(width, height);
}

void RealtimeRender::AddVertex(glm::vec3 const& position, glm::vec4 const& color, float timeInSeconds)
{
    mBatchRenderer.PersistentVertex(position, color, timeInSeconds);
}

Pipeline* RealtimeRender::GetDefaultPipeline()
{
    return mDefaultPipeline.get();
}

void RealtimeRender::InitRootSignatures()
{
    /* Default root signature */
    mDefaultDescriptorSets = std::make_unique<DescriptorSet>();
    {
        mDefaultDescriptorSets->AddStorageBuffer(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mDefaultDescriptorSets->AddInputBuffer(1, 1, VK_SHADER_STAGE_VERTEX_BIT);

        mDefaultDescriptorSets->AddStorageBuffer(2, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        mDefaultDescriptorSets->AddInputBuffer(3, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    mDefaultDescriptorSets->Bake();
    mDefaultRootSignature = std::make_unique<RootSignature>();
    {
        mDefaultRootSignature->AddPushRange<uint32_t>(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mDefaultRootSignature->AddDescriptorSet(mDefaultDescriptorSets.get());
    }
    mDefaultRootSignature->Bake();

    /* Outline root signature */
    mOutlineRootSignature = std::make_unique<RootSignature>();
    {
        mOutlineRootSignature->AddPushRange<OutlineVertPushConstants>(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mOutlineRootSignature->AddDescriptorSet(mDefaultDescriptorSets.get());
    }
    mOutlineRootSignature->Bake();
}

void RealtimeRender::InitPerObjectBuffer()
{
    /* Might make this not host-accessible and just copy in it whenever needed */
    mPerObjectBuffer = std::make_unique<Buffer>(Jnrlib::AlignUp(sizeof(PerObjectInfo), sizeof(glm::vec4)),
        mScene->GetNumberOfObjects(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    mDefaultDescriptorSets->BindStorageBuffer(mPerObjectBuffer.get(), 0, 0, 0);
}

void RealtimeRender::InitUniformBuffer()
{
    mUniformBuffer = std::make_unique<Buffer>(sizeof(UniformBuffer), 1,
                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    mDefaultDescriptorSets->BindInputBuffer(mUniformBuffer.get(), 1, 0, 0);
}

void RealtimeRender::InitMaterialsBuffer(CommandList* cmdList)
{
    auto materialManager = MaterialManager::Get();
    auto materials = materialManager->GetShaderMaterials();

    {
        auto localMaterialsBuffer = std::make_unique<Buffer>(Jnrlib::AlignUp(sizeof(MaterialManager::ShaderMaterial), sizeof(glm::vec4)),
                                                                     materials.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        localMaterialsBuffer->Copy(materials.data());



        mMaterialsBuffer = std::make_unique<Buffer>(Jnrlib::AlignUp(sizeof(MaterialManager::ShaderMaterial), sizeof(glm::vec4)), materials.size(),
                                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        cmdList->CopyBuffer(mMaterialsBuffer.get(), localMaterialsBuffer.get());
        cmdList->AddLocalBuffer(std::move(localMaterialsBuffer));
    }

    mDefaultDescriptorSets->BindStorageBuffer(mMaterialsBuffer.get(), 2, 0, 0);

    {
        mLightBuffer = std::make_unique<Buffer>(Jnrlib::AlignUp(sizeof(DirectionalLight), sizeof(glm::vec4)), 1,
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        SetLight(DirectionalLight{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .direction = glm::vec3(1.0f, 1.0f, 0.0f)});
    }
    mDefaultDescriptorSets->BindInputBuffer(mLightBuffer.get(), 3, 0, 0);
}

void RealtimeRender::InitPipelines(uint32_t width, uint32_t height)
{
    mDefaultPipeline = std::make_unique<Pipeline>("RealtimeRender_DefaultPipeline");
    {
        mDefaultPipeline->AddShader("Shaders/basic.vert.spv");
        mDefaultPipeline->AddShader("Shaders/basic.frag.spv");
    }
    VkViewport vp{};
    VkRect2D sc{};
    auto& viewport = mDefaultPipeline->GetViewportStateCreateInfo();
    {
        vp.width = (FLOAT)width; vp.minDepth = 0.0f; vp.x = 0;
        vp.height = (FLOAT)height; vp.maxDepth = 1.0f; vp.y = 0;
        sc.offset = {.x = 0, .y = 0}; sc.extent = {.width = (uint32_t)width, .height = (uint32_t)height};
        viewport.viewportCount = 1;
        viewport.pViewports = &vp;
        viewport.scissorCount = 1;
        viewport.pScissors = &sc;
    }


    auto vertexPositionNormalAttributeDescription = Common::VertexPositionNormal::GetInputAttributeDescription();
    auto vertexPositionNormalBindingDescription = Common::VertexPositionNormal::GetInputBindingDescription();
    auto vertexPositionColorAttributeDescription = VertexPositionColor::GetInputAttributeDescription();
    auto vertexPositionColorBindingDescription = VertexPositionColor::GetInputBindingDescription();
    {
        auto& vertexInput = mDefaultPipeline->GetVertexInputStateCreateInfo();
        vertexInput.vertexAttributeDescriptionCount = (uint32_t)vertexPositionNormalAttributeDescription.size();
        vertexInput.pVertexAttributeDescriptions = vertexPositionNormalAttributeDescription.data();
        vertexInput.vertexBindingDescriptionCount = (uint32_t)vertexPositionNormalBindingDescription.size();
        vertexInput.pVertexBindingDescriptions = vertexPositionNormalBindingDescription.data();
    }

    VkPipelineColorBlendAttachmentState attachmentInfo{};
    {
        auto& blendState = mDefaultPipeline->GetColorBlendStateCreateInfo();
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }

    {
        auto& rasterizerState = mDefaultPipeline->GetRasterizationStateCreateInfo();
        // rasterizerState.cullMode = VK_CULL_MODE_NONE;
        // rasterizerState.polygonMode = VK_POLYGON_MODE_LINE;
    }

    {
        auto& depthState = mDefaultPipeline->GetDepthStencilStateCreateInfo();
        depthState.depthTestEnable = VK_TRUE;
        depthState.depthWriteEnable = VK_TRUE;
        depthState.depthBoundsTestEnable = VK_TRUE;
        depthState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthState.depthBoundsTestEnable = VK_TRUE;
        depthState.minDepthBounds = 0.0f;
        depthState.maxDepthBounds = 1.0f;
    }

    mDefaultPipeline->SetDepthStencilImage(mDepthImage);
    mDefaultPipeline->SetRootSignature(mDefaultRootSignature.get());
    mDefaultPipeline->AddImageColorOutput(mRenderTarget);
    mDefaultPipeline->Bake();

    mSelectedObjectsPipeline = std::make_unique<Pipeline>("RealtimeRender_SelectedObjectPipeline");
    mSelectedObjectsPipeline->InitFrom(*mDefaultPipeline);
    {
        mSelectedObjectsPipeline->AddShader("Shaders/basic.vert.spv");
        mSelectedObjectsPipeline->AddShader("Shaders/basic.frag.spv");
    }
    {
        auto& depthStencil = mSelectedObjectsPipeline->GetDepthStencilStateCreateInfo();
        depthStencil.stencilTestEnable = VK_TRUE;
        depthStencil.front.reference = 1;
        depthStencil.front.writeMask = 0xFF;
        depthStencil.front.compareMask = 0xFF;
        depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.front.depthFailOp = VK_STENCIL_OP_REPLACE;
        depthStencil.front.failOp = VK_STENCIL_OP_REPLACE;
        depthStencil.front.passOp = VK_STENCIL_OP_REPLACE;
        depthStencil.back = depthStencil.front;
    }
    mSelectedObjectsPipeline->Bake();

    mOutlinePipeline = std::make_unique<Pipeline>("RealtimeRender_OutlinePipeline");
    mOutlinePipeline->InitFrom(*mSelectedObjectsPipeline);
    {
        mOutlinePipeline->AddShader("Shaders/outline.vert.spv");
        mOutlinePipeline->AddShader("Shaders/white.frag.spv");
    }
    {
        auto& depthStencil = mOutlinePipeline->GetDepthStencilStateCreateInfo();
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_TRUE;
        depthStencil.front.reference = 1;
        depthStencil.front.writeMask = 0xFF;
        depthStencil.front.compareMask = 0xFF;
        depthStencil.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
        depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.passOp = VK_STENCIL_OP_ZERO;
        depthStencil.back = depthStencil.front;
    }
    mOutlinePipeline->SetRootSignature(mOutlineRootSignature.get());
    mOutlinePipeline->Bake();

    mDebugPipeline = std::make_unique<Pipeline>("RealtimeRender_DebugPipeline");
    mDebugPipeline->InitFrom(*mDefaultPipeline);
    {
        mDebugPipeline->AddShader("Shaders/color.vert.spv");
        mDebugPipeline->AddShader("Shaders/color.frag.spv");
    }
    {
        auto& vertexStateCreateInfo = mDebugPipeline->GetVertexInputStateCreateInfo();

        vertexStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexPositionColorAttributeDescription.size();
        vertexStateCreateInfo.pVertexAttributeDescriptions = vertexPositionColorAttributeDescription.data();
        vertexStateCreateInfo.vertexBindingDescriptionCount = (uint32_t)vertexPositionColorBindingDescription.size();
        vertexStateCreateInfo.pVertexBindingDescriptions = vertexPositionColorBindingDescription.data();
    }
    {
        auto& inputAssembly = mDebugPipeline->GetInputAssemblyStateCreateInfo();
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    }
    {
        auto& rasterizerInfo = mDebugPipeline->GetRasterizationStateCreateInfo();
        rasterizerInfo.lineWidth = 2.0f;
    }
    mOutlinePipeline->SetRootSignature(mDefaultRootSignature.get());
    mDebugPipeline->Bake();
}


