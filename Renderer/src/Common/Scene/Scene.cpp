#include "Scene.h"
#include "GeometryHelpers.h"
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

std::optional<HitPoint> Scene::GetClosestHit(Ray const& r) const
{
    return Systems::Intersection::Get()->IntersectRay(r, mEntities);
}

uint32_t Common::Scene::GetNumberOfObjects() const
{
    return (uint32_t)mEntities.size();
}

void Common::Scene::PerformUpdate()
{
    for (auto const& [entity, update] : mEntities.view<Components::Update>().each())
    {
        if (update.dirtyFrames)
        {
            mEntities.patch<Components::Update>(entity, [](Components::Update& up)
            {
                up.dirtyFrames--;
            });
        }
    }
}

entt::registry& Common::Scene::GetEntities() const
{
    return mEntities;
}

void Scene::CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives, bool buildRealtime)
{
    bool hasSphere = false;
    uint32_t currentBufferIndex = 0;
    for (uint32_t i = 0; i < primitives.size(); ++i)
    {
        auto& p = primitives[i];

        auto currentEntity = mEntities.create();
        auto& baseComponent = mEntities.emplace<Components::Base>(currentEntity);
        baseComponent.name = p.name;
        baseComponent.position = p.position;

        switch (p.primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
            {
                baseComponent.scaling = glm::vec3(p.radius, p.radius, p.radius);

                auto& sphereComponent = mEntities.emplace<Components::Sphere>(currentEntity);
                sphereComponent.material = MaterialManager::Get()->GetMaterial(p.materialName);
                CHECK(sphereComponent.material) << "Material " << p.materialName << " is not specified";

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

                    {
                        auto& mesh = mEntities.emplace<Components::Mesh>(currentEntity);
                        mesh = mMeshes["Sphere"];
                    }

                    {
                        auto& update = mEntities.emplace<Components::Update>(currentEntity);
                        update.bufferIndex = currentBufferIndex++;
                        update.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT;
                    }
                }

                break;
            }
        }
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
