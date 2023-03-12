#include "Scene.h"
#include "GeometryHelpers.h"
#include "Vulkan/CommandList.h"
#include "Scene/Systems/IntersectionSystem.h"
#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/SphereComponent.h"
#include "Scene/Components/UpdateComponent.h"
#include "Constants.h"
#include "MaterialManager.h"

using namespace Common;

Scene::Scene(CreateInfo::Scene const& info) :
    mOutputFile(info.outputFile),
    mImageInfo(info.imageInfo)
{
    LOG(INFO) << "Creating scene with info: " << info;
    CreatePrimitives(info.primitives, info.alsoBuildForRealTimeRendering);
}

Scene::~Scene()
{
}

std::string Scene::GetOutputFile() const
{
    return mOutputFile;
}

const CreateInfo::ImageInfo& Scene::GetImageInfo() const
{
    return mImageInfo;
}

void Scene::SetCamera(std::unique_ptr<Camera>&& camera)
{
    mCamera = std::move(camera);
}

Camera& Scene::GetCamera()
{
    return *mCamera;
}

Camera const& Scene::GetCamera() const
{
    return *mCamera;
}

void Common::Scene::InitializeGraphics(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex)
{
    CreateRenderingBuffers(cmdList, cmdBufIndex);
}

Vulkan::Buffer const* Common::Scene::GetVertexBuffer() const
{
    return mVertexBuffer.get();
}

Vulkan::Buffer const* Common::Scene::GetIndexBuffer() const
{
    return mIndexBuffer.get();
}

std::vector<Entity*>& Common::Scene::GetRootEntities()
{
    return mRootEntities;
}

std::vector<std::unique_ptr<Entity>>& Common::Scene::GetEntities()
{
    return mEntities;
}

std::optional<HitPoint> Scene::GetClosestHit(Ray const& r) const
{
    return Systems::Intersection::Get()->IntersectRay(r, mRegistry);
}

uint32_t Common::Scene::GetNumberOfObjects() const
{
    return (uint32_t)mEntities.size();
}

void Common::Scene::PerformUpdate()
{
    for (auto const& [entity, update] : mRegistry.view<Components::Update>().each())
    {
        if (update.dirtyFrames)
        {
            mRegistry.patch<Components::Update>(entity, [](Components::Update& up)
            {
                up.dirtyFrames--;
            });
        }
    }
}

void Scene::CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives, bool buildRealtime)
{
    bool hasSphere = false;
    uint32_t currentBufferIndex = 0;
    mEntities.reserve(primitives.size());
    mRootEntities.reserve(primitives.size());
    for (uint32_t i = 0; i < primitives.size(); ++i)
    {
        auto& p = primitives[i];
        std::unique_ptr<Entity> entity = std::make_unique<Entity>(mRegistry.create(), mRegistry);

        if (p.parentName.size() > 0)
        {
            for (uint32_t j = 0; j < entity->mEntities.size(); ++j)
            {
                auto* parent = mEntities[j].get();
                auto& base = parent->GetComponent<Components::Base>();
                if (base.name == p.parentName)
                {
                    parent->AddChild(entity.get());
                    break;
                }
            }
        }
        else
        {
            mRootEntities.push_back(entity.get());
        }

        switch (p.primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
            {
                entity->AddComponent(
                    Components::Base{.position = p.position, .scaling = glm::vec3(p.radius, p.radius, p.radius), .name = p.name}
                );

                auto material = MaterialManager::Get()->GetMaterial(p.materialName);
                CHECK(material) << "A sphere must have a material";
                entity->AddComponent(Components::Sphere{.material = material});
                

                if (buildRealtime)
                {
                    if (!hasSphere)
                    {
                        Components::Mesh mesh{};
                        mesh.firstVertex = (uint32_t)mVertices.size();
                        mesh.firstIndex = (uint32_t)mIndices.size();
                        uint32_t stackCount = 36;
                        uint32_t sliceCount = 36;
                        Sphere::GetVertices(Jnrlib::One, sliceCount, stackCount, mVertices, mIndices);
                        mesh.indexCount = (uint32_t)mIndices.size() - mesh.firstIndex;
                        mMeshes["Sphere"] = mesh;
                        hasSphere = true;
                    }

                    entity->AddComponent(Components::Mesh(mMeshes["Sphere"]));
                    entity->AddComponent(
                        Components::Update{.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT, .bufferIndex = currentBufferIndex++}
                    );
                }

                break;
            }
        }

        mEntities.push_back(std::move(entity));
    }
}

void Scene::CreateRenderingBuffers(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex)
{
    {
        /* Vertex buffer */
        std::unique_ptr<Vulkan::Buffer> localVertexBuffer =
            std::make_unique<Vulkan::Buffer>(sizeof(Common::Vertex), mVertices.size(),
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        localVertexBuffer->Copy(mVertices.data());

        mVertexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(Common::Vertex), mVertices.size(),
                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        cmdList->CopyBuffer(mVertexBuffer.get(), localVertexBuffer.get());
        cmdList->AddLocalBuffer(std::move(localVertexBuffer));
    }

    {
        /* Index buffer */
        std::unique_ptr<Vulkan::Buffer> localIndexBuffer =
            std::make_unique<Vulkan::Buffer>(sizeof(uint32_t), mIndices.size(),
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        localIndexBuffer->Copy(mIndices.data());

        mIndexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(uint32_t), mIndices.size(),
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        cmdList->CopyBuffer(mIndexBuffer.get(), localIndexBuffer.get());
        cmdList->AddLocalBuffer(std::move(localIndexBuffer));
    }
}
