#include "RealtimeRenderSystem.h"

#include "Scene/Components/Base.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/Sphere.h"
#include "Scene/Components/Update.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/AccelerationStructure.h"
#include "EditorCamera.h"

#include "CameraUtils.h"

#include "MaterialManager.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

using namespace Common;
using namespace Systems;
using namespace Vulkan;
using namespace Components;

RealtimeRender::RealtimeRender(Scene* scene, CommandList* cmdList) :
    mScene(scene)
{
    InitRootSignatures();
    InitPerObjectBuffer();
    InitUniformBuffer();
    InitMaterialsBuffer(cmdList);
    mBatchRenderer.Begin();
}

RealtimeRender::~RealtimeRender()
{
}

void RealtimeRender::DrawAccelerationStructure(Base const& base, AccelerationStructure const& accelerationStructure)
{
    Jnrlib::Vec3 scale;
    Jnrlib::Quaternion rotation;
    Jnrlib::Position translation;
    Jnrlib::Vec3 skew;
    Jnrlib::Vec4 perspective;
    glm::decompose(base.world, scale, rotation, translation, skew, perspective);

    for (auto const& node : accelerationStructure.nodes)
    {
        Jnrlib::BoundingBox bb = node.bounds;
        bb.pMin += translation;
        bb.pMax += translation;
        mBatchRenderer.WireframeBoundingBox(bb, Jnrlib::Cyan);
    }
}

void RealtimeRender::DrawAccelerationStructures()
{
    auto accelerationStructures = mScene->GetRegistry().view<const Base, const AccelerationStructure>();
    for (auto const& [entity, base, accelStructure] : accelerationStructures.each())
    {
        if (accelStructure.shouldRender)
            DrawAccelerationStructure(base, accelStructure);
    }
}

void RealtimeRender::RenderScene(CommandList* cmdList)
{
    auto vertexBuffer = mScene->GetVertexBuffer();
    auto indexBuffer = mScene->GetIndexBuffer();
    cmdList->BindVertexBuffer(vertexBuffer, 0);
    cmdList->BindIndexBuffer(indexBuffer);

    auto const& updatables = mScene->GetRegistry().group<const Base, const Components::Update, const Mesh>();
    /* Build rendering buffers */
    {
        /* TODO: instead of iterating over all entities, build an observer and only iterate over entities that changed */
        for (auto const& [entity, base, update, mesh] : updatables.each())
        {
            if (update.dirtyFrames > 0)
            {
                PerObjectInfo* objectInfo = (PerObjectInfo*)mPerObjectBuffer->GetElement(update.bufferIndex);

                objectInfo->world = base.world;
                auto parent = base.entityPtr->GetParent();
                while (parent != nullptr)
                {
                    auto& parentBase = parent->GetComponent<Base>();
                    objectInfo->world = objectInfo->world * parentBase.world;
                    parent = parent->GetParent();
                }
                objectInfo->materialIndex = mesh.material->GetMaterialIndex();
                /* TODO: Simply this - move the material to base + move radius to scaling */
                if (auto sphere = base.entityPtr->TryGetComponent<Sphere>(); sphere != nullptr) [[unlikely]]
                {
                    objectInfo->world = glm::scale(objectInfo->world, glm::vec3(sphere->radius));
                }
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


    {
        static Jnrlib::Color whiteSkyColor = Jnrlib::Color(Jnrlib::Half);
        static Jnrlib::Color blueSkyColor = Jnrlib::Color(Jnrlib::Quarter, Jnrlib::Quarter, Jnrlib::One, 1.0f);
        static Jnrlib::Color skyColor = 0.5f * whiteSkyColor + 0.5f * blueSkyColor;
        cmdList->BeginRenderingOnImage(mRenderTarget, skyColor, mDepthImage, true);
    }
    cmdList->BindPipeline(mDefaultPipeline.get());
    cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mDefaultRootSignature.get());

    /* Render */
    {
        for (auto const& [entity, base, update, mesh] : updatables.each())
        {
             if (mSelectedEntities.find(base.entityPtr) != mSelectedEntities.end())
                continue;
             if (mesh.hidden)
                continue;

            /* TODO: pass this as instance */
            uint32_t index = update.bufferIndex;
            cmdList->BindPushRange<uint32_t>(mDefaultRootSignature.get(), 0, 1, &index, VK_SHADER_STAGE_VERTEX_BIT);
            cmdList->DrawIndexedInstanced(mesh.indices.indexCount, mesh.indices.firstIndex, mesh.indices.firstVertex);
        }
    }

    DrawAccelerationStructures();

    if (mSelectedEntities.size())
    {
        cmdList->BindPipeline(mSelectedObjectsPipeline.get());
        
        /* Render selected objects */
        for (const auto& entity : mSelectedEntities)
        {
            auto* update = entity->TryGetComponent<Components::Update>();
            auto* mesh = entity->TryGetComponent<Mesh>();
            if (mesh == nullptr || update == nullptr)
                continue;

            uint32_t index = update->bufferIndex;
            cmdList->BindPushRange<uint32_t>(mDefaultRootSignature.get(), 0, 1, &index, VK_SHADER_STAGE_VERTEX_BIT);
            cmdList->DrawIndexedInstanced(mesh->indices.indexCount, mesh->indices.firstIndex, mesh->indices.firstVertex);
        }

        cmdList->BindPipeline(mOutlinePipeline.get());
        cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mOutlineRootSignature.get());
        /* Render the outlines */
        for (const auto& entity : mSelectedEntities)
        {
            auto* update = entity->TryGetComponent<Components::Update>();
            auto* mesh = entity->TryGetComponent<Mesh>();
            if (mesh == nullptr || update == nullptr)
                continue;

            uint32_t index = update->bufferIndex;

            OutlineVertPushConstants outlineVertPushConstants{.objectIndex = index, .lineWidth = 0.025f};
            cmdList->BindPushRange<OutlineVertPushConstants>(mOutlineRootSignature.get(), 0, 1, &outlineVertPushConstants,
                                                             VK_SHADER_STAGE_VERTEX_BIT);
            cmdList->DrawIndexedInstanced(mesh->indices.indexCount, mesh->indices.firstIndex, mesh->indices.firstVertex);
        }
    }

    cmdList->BindPipeline(mDebugPipeline.get());
    cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mDefaultRootSignature.get());
    if (mDrawCameraFrustum)
    {
        DrawCameraEntities();
    }
    mBatchRenderer.End(cmdList);
    cmdList->EndRendering();
    mBatchRenderer.Begin();
}

RootSignature* RealtimeRender::GetRootSiganture() const
{
    return mDefaultRootSignature.get();
}

void RealtimeRender::SetDepthImage(Vulkan::Image* depthImage)
{
    mDepthImage = depthImage;
}

void RealtimeRender::SetRenderTarget(Vulkan::Image * renderTarget)
{
    mRenderTarget = renderTarget;
}

void RealtimeRender::SetCamera(EditorCamera const* camera)
{
    mCamera = camera;
}

void RealtimeRender::SetLight(DirectionalLight const& light)
{
    auto memory = mLightBuffer->GetElement();
    memcpy(memory, &light, sizeof(DirectionalLight));
    ((DirectionalLight*)memory)->direction = glm::normalize(((DirectionalLight*)memory)->direction);
}

void RealtimeRender::SelectEntities(std::unordered_set<Entity*> const& selectedIndices)
{
    mSelectedEntities = selectedIndices;
}

void RealtimeRender::ClearSelection()
{
    mSelectedEntities.clear();
}

bool RealtimeRender::GetDrawCameraFrustum() const
{
    return mDrawCameraFrustum;
}

void RealtimeRender::SetDrawCameraFrustum(bool draw)
{
    mDrawCameraFrustum = draw;
}

void RealtimeRender::Update(float dt)
{
    mBatchRenderer.Update(dt);
}

void RealtimeRender::OnResize(uint32_t width, uint32_t height)
{
    InitPipelines(width, height);
}

void RealtimeRender::AddOneTimeVertex(glm::vec3 const& position, glm::vec4 const& color)
{
    mBatchRenderer.Vertex(position, color);
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
        SetLight(DirectionalLight{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .direction = glm::vec3(0.5f, 0.5f, -1.0f)});
    }
    mDefaultDescriptorSets->BindInputBuffer(mLightBuffer.get(), 3, 0, 0);
}

void RealtimeRender::InitPipelines(uint32_t width, uint32_t height)
{
    CHECK(mRenderTarget != nullptr) << "Cannot initialize pipelines if a render target is not set ;(";

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
        mOutlinePipeline->AddShader("Shaders/yellow.frag.spv");
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
        rasterizerInfo.cullMode = VK_CULL_MODE_NONE;
    }
    mDebugPipeline->SetRootSignature(mDefaultRootSignature.get());
    mDebugPipeline->Bake();
}

void RealtimeRender::DrawCameraEntities()
{
    auto* cameraEntity = mScene->GetCameraEntity();
    auto& cameraComponent = cameraEntity->GetComponent<Camera>();
    if (mSelectedEntities.find(const_cast<Entity*>(cameraEntity)) != mSelectedEntities.end())
    {
        DrawCameraEntity(cameraComponent, true);
    }
    else
    {
        DrawCameraEntity(cameraComponent, false);
    }
}

void RealtimeRender::DrawCameraEntity(Common::Components::Camera const& cameraComponent, bool isSelected)
{
    std::function<void(glm::vec3)> vertex;
    if (isSelected)
    {
        vertex = [&](glm::vec3 pos)
        {
            mBatchRenderer.Vertex(pos, Jnrlib::Yellow);
        };
    }
    else
    {
        vertex = [&](glm::vec3 pos)
        {
            mBatchRenderer.Vertex(pos, Jnrlib::Grey);
        };
    }

    glm::vec2 coordinates[] = {
        {0.0f, 0.0f},
        {0.0f, cameraComponent.projectionSize.x},
        {cameraComponent.projectionSize.y, cameraComponent.projectionSize.x},
        {cameraComponent.projectionSize.y, 0.0f},
    };
    glm::vec3 corners[8];
    for (uint32_t i = 0; i < sizeof(coordinates) / sizeof(coordinates[0]); ++i)
    {
        auto currentRay = CameraUtils::GetRayForPixel(&cameraComponent, (uint32_t)coordinates[i].x, (uint32_t)coordinates[i].y);
        corners[i] = currentRay.origin + currentRay.direction * cameraComponent.focalDistance;
        corners[i + 4] = currentRay.origin + currentRay.direction * 10.0f;
    }

    /* Near plane */
    vertex(corners[0]);
    vertex(corners[1]);

    vertex(corners[1]);
    vertex(corners[2]);

    vertex(corners[2]);
    vertex(corners[3]);

    vertex(corners[3]);
    vertex(corners[0]);

    /* Far plane */
    vertex(corners[4]);
    vertex(corners[5]);

    vertex(corners[5]);
    vertex(corners[6]);

    vertex(corners[6]);
    vertex(corners[7]);

    vertex(corners[7]);
    vertex(corners[4]);

    /* Sides */
    vertex(corners[0]);
    vertex(corners[4]);

    vertex(corners[1]);
    vertex(corners[5]);

    vertex(corners[2]);
    vertex(corners[6]);

    vertex(corners[3]);
    vertex(corners[7]);
}
