#include "RealtimeRenderSystem.h"

#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/SphereComponent.h"
#include "Scene/Components/UpdateComponent.h"

#include "glm/gtc/matrix_transform.hpp"

using namespace Common;
using namespace Systems;
using namespace Vulkan;
using namespace Components;

RealtimeRender::RealtimeRender(Common::Scene const* scene) :
    mScene(scene)
{
    InitDefaultRootSignature();
    InitPerObjectBuffer();
    InitUniformBuffer();
}

RealtimeRender::~RealtimeRender()
{
}

void RealtimeRender::RenderScene(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex)
{
    auto vertexBuffer = mScene->GetVertexBuffer();
    auto indexBuffer = mScene->GetIndexBuffer();
    cmdList->BindVertexBuffer(vertexBuffer, 0, cmdBufIndex);
    cmdList->BindIndexBuffer(indexBuffer, cmdBufIndex);
    
    auto const& spheres = mScene->mEntities.group<const Base, const Update, const Mesh>(entt::get<const Sphere>);
    {
        /* Build rendering buffers */
        for (auto const& [entity, base, update, mesh, sphere] : spheres.each())
        {
            PerObjectInfo* objectInfo = (PerObjectInfo*)mPerObjectBuffer->GetElement(update.bufferIndex);


            objectInfo->world = glm::translate(glm::identity<glm::mat4x4>(), base.position);
            objectInfo->world = glm::scale(objectInfo->world, base.scaling);

            /* TODO: Make this flexible */
            objectInfo->materialIndex = 0;
        }

        {
            UniformBuffer* uniformBuffer = (UniformBuffer*)mUniformBuffer->GetElement();
            if (mCamera)
            {
                uniformBuffer->viewProj = mCamera->GetProjection() * mCamera->GetView();
                // uniformBuffer->viewProj = mCamera->GetProjection();
            }
            else
            {
                uniformBuffer->viewProj = glm::identity<glm::mat4x4>();
            }
        }
    }
    cmdList->BindDescriptorSet(mDefaultDescriptorSets.get(), 0, mDefaultRootSignature.get(), cmdBufIndex);

    {
        /* Render */
        for (auto const& [entity, base, update, mesh, sphere] : spheres.each())
        {
            /* TODO: pass this as instance */
            uint32_t index = update.bufferIndex;
            cmdList->BindPushRange<uint32_t>(mDefaultRootSignature.get(), 0, 1, &index, VK_SHADER_STAGE_VERTEX_BIT, cmdBufIndex);
            cmdList->DrawIndexedInstanced(mesh.indexCount, mesh.firstIndex, mesh.firstVertex, cmdBufIndex);
        }
    }
}

Vulkan::RootSignature* Common::Systems::RealtimeRender::GetRootSiganture() const
{
    return mDefaultRootSignature.get();
}

void Common::Systems::RealtimeRender::SetCamera(Camera const* camera)
{
    mCamera = camera;
}

void RealtimeRender::InitDefaultRootSignature()
{
    mDefaultDescriptorSets = std::make_unique<DescriptorSet>();
    {
        mDefaultDescriptorSets->AddStorageBuffer(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mDefaultDescriptorSets->AddInputBuffer(1, 1, VK_SHADER_STAGE_VERTEX_BIT);
    }
    mDefaultDescriptorSets->Bake();
    mDefaultRootSignature = std::make_unique<RootSignature>();
    {
        mDefaultRootSignature->AddPushRange<uint32_t>(0, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mDefaultRootSignature->AddDescriptorSet(mDefaultDescriptorSets.get());
    }
    mDefaultRootSignature->Bake();
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


