#include "Scene.h"
#include "GeometryHelpers.h"
#include "Vulkan/CommandList.h"
#include "Scene/Systems/IntersectionSystem.h"
#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/SphereComponent.h"
#include "Scene/Components/UpdateComponent.h"
#include "Scene/Components/CameraComponent.h"
#include "Constants.h"
#include "MaterialManager.h"

using namespace Common;

Scene::Scene(CreateInfo::Scene const& info) :
    mOutputFile(info.outputFile),
    mImageInfo(info.imageInfo)
{
    LOG(INFO) << "Creating scene with info: " << info;
    CreatePrimitives(info.primitives, info.alsoBuildForRealTimeRendering);
    CreateCamera(info.cameraInfo, info.alsoBuildForRealTimeRendering);
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

std::optional<HitPoint> Scene::GetClosestHit(Ray& r) const
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

Entity const* Common::Scene::GetCameraEntity() const
{
    return mCameraEntity;
}

entt::registry& Common::Scene::GetRegistry() const
{
    return mRegistry;
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

        entity->AddComponent(
            Components::Base{.position = p.position, .name = p.name, .entityPtr = entity.get()}
        );

        switch (p.primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
            {
                auto material = MaterialManager::Get()->GetMaterial(p.materialName);
                CHECK(material) << "A sphere must have a material";
                entity->AddComponent(Components::Sphere{.material = material, .radius = p.radius});

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
                    mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(entity.get());
                    mRegistry.on_update<Components::Sphere>().connect<&Entity::UpdateBase>(entity.get());
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
            std::make_unique<Vulkan::Buffer>(sizeof(Common::VertexPositionNormal), mVertices.size(),
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        localVertexBuffer->Copy(mVertices.data());

        mVertexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(Common::VertexPositionNormal), mVertices.size(),
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

void Scene::CreateCamera(CreateInfo::Camera const& cameraInfo, bool alsoBuildRealtime)
{
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(mRegistry.create(), mRegistry);
    entity->AddComponent<Components::Base>(
        Components::Base{
            .position = cameraInfo.position,
            .name = "Camera",
            .entityPtr = entity.get()}
    );

    auto& cameraComponent = entity->AddComponent(Components::Camera(entity->GetComponent<Components::Base>()));
    {
        cameraComponent.primary = true;
        cameraComponent.focalDistance = cameraInfo.focalDistance;
        cameraComponent.projectionSize = glm::vec2(cameraInfo.projectionWidth, cameraInfo.projectionHeight);
        cameraComponent.roll = cameraInfo.roll;
        cameraComponent.pitch = cameraInfo.pitch;
        cameraComponent.yaw = cameraInfo.yaw;
        cameraComponent.viewportSize = glm::vec2(cameraInfo.viewportWidth, cameraInfo.viewportHeight);
        cameraComponent.Update();
    }

    if (alsoBuildRealtime)
    {
        entity->AddComponent(
            Components::Update{.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT, .bufferIndex = (uint32_t)mRegistry.size()}
        );
        mRegistry.on_update<Components::Base>().connect<&Components::Camera::Update>(cameraComponent);
        mRegistry.on_update<Components::Camera>().connect<&Components::Camera::Update>(cameraComponent);

        mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(entity.get());
    }


    mRootEntities.push_back(entity.get());
    mCameraEntity = entity.get();

    mEntities.push_back(std::move(entity));
}
