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
    InitDefaultRootSignature();
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

    auto const& spheres = mScene->mEntities.group<const Base, const Update, const Mesh>(entt::get<const Sphere>);
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
}

void RealtimeRender::InitDefaultRootSignature()
{
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
        SetLight(DirectionalLight{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), .direction = glm::vec3(0.0f, 1.0f, 0.0f)});
    }
    mDefaultDescriptorSets->BindInputBuffer(mLightBuffer.get(), 3, 0, 0);
}


