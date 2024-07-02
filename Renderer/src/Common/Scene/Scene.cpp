#include "Scene.h"
#include "GeometryHelpers.h"
#include "Vulkan/CommandList.h"
#include "Scene/Systems/IntersectionSystem.h"
#include "Scene/Components/Base.h"
#include "Scene/Components/Sphere.h"
#include "Scene/Components/Update.h"
#include "Scene/Components/Camera.h"
#include "Constants.h"
#include "MaterialManager.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtx/transform.hpp>

using namespace Common;
using namespace Jnrlib;

namespace Helpers
{
    std::string GetMeshNameFromPath(std::string const& path)
    {
        return std::filesystem::path(path).stem().string();
    }

    /* Helpers */
    class ModelLoader
    {
#define RETURN_IF_FAILURE_FOUND \
{\
if (!mSuccess)\
    return;\
}

        struct MeshProcessContext
        {
            std::vector<VertexPositionNormal> vertices;
            std::vector<uint32_t> indices;
        };

    public:
        ModelLoader(Scene* scene) :
            mScene(scene)
        {
            auto threadPool = ThreadPool::Get();
            mImporters.resize(threadPool->GetNumberOfThreads());
            mTasksToWaitFor.reserve(16); // Simple beginning case
        }

        ~ModelLoader()
        {
            Wait();
        }

        bool Succeeded() const
        {
            return mSuccess;
        }

        std::string GetError() const
        {
            return mFailureMessage;
        }

        void Wait()
        {
            auto threadPool = ThreadPool::Get();
            for (auto const& task : mTasksToWaitFor)
            {
                threadPool->Wait(task);
            }
        }

        void LoadModel(std::string const& path, Entity* ent, std::shared_ptr<IMaterial> material, CreateInfo::AccelerationStructure const& accelerationInfo)
        {
            RETURN_IF_FAILURE_FOUND;

            MarkMesh(path);

            auto threadPool = ThreadPool::Get();
            mTasksToWaitFor.emplace_back(threadPool->ExecuteDeffered(std::bind(&ModelLoader::HandleLoading, this, std::cref(path), ent, material, std::cref(accelerationInfo))));
        }

    private:
        void MarkMesh(std::string const& path)
        {
            std::string name = GetMeshNameFromPath(path);
            Components::Indices indices{};
            mScene->AddMeshIndices(name, indices);
        }

        void HandleLoading(std::string const& path, Entity* ent, std::shared_ptr<IMaterial> material, CreateInfo::AccelerationStructure const& accelerationInfo)
        {
            auto threadPool = ThreadPool::Get();
            uint32_t id = threadPool->GetCurrentThreadId();
            CHECK(id != -1) << "Invalid thread id returned by thread pool. Is this executed on the main thread?";

            if (mImporters[id] == nullptr)
            {
                mImporters[id].reset(new Assimp::Importer());
            }

            RETURN_IF_FAILURE_FOUND;

            auto scene = mImporters[id]->ReadFile(path, aiProcess_GenNormals |
                                                  aiProcess_FlipWindingOrder |
                                                  aiProcess_MakeLeftHanded |
                                                  aiProcess_Triangulate |
                                                  aiProcess_SortByPType |
                                                  aiProcess_OptimizeMeshes);
            if (scene == nullptr && mSuccess)
            {
                std::unique_lock<std::mutex> lock(mMessageMutex);
                mFailureMessage = "Couldn't load mesh from file " + path;
                mSuccess = false;
            }

            RETURN_IF_FAILURE_FOUND;

            MeshProcessContext context;
            ProcessNode(scene, scene->mRootNode, context);

            /* Create acceleration structure */
            Accelerators::BVH::Input bvhAcceleration{};
            bvhAcceleration.splitType = accelerationInfo.splitType;
            bvhAcceleration.maxPrimsInNode = accelerationInfo.maxPrimsInNode;
            bvhAcceleration.indices = context.indices;
            bvhAcceleration.vertices = context.vertices;
            if (auto output = Accelerators::BVH::Generate(bvhAcceleration); !output.accelerationStructure.nodes.empty())
            {
                /* Use the new indices */
                context.indices = std::move(output.new_indices);
                /* Created a valid acceleration structure */
                ent->AddComponent(output.accelerationStructure);
            }

            {
                std::string name = GetMeshNameFromPath(path);
                Components::Mesh mesh;
                mesh.indices.indexCount = (uint32_t)context.indices.size();
                mesh.indices.firstVertex = mScene->AddVertices(std::move(context.vertices));
                mesh.indices.firstIndex = mScene->AddIndices(std::move(context.indices));
                mesh.name = name;
                mesh.material = material;
                
                /* Update the indices */
                auto& indices = mScene->GetMeshIndices(name);
                indices = mesh.indices;

                ent->AddComponent(mesh);
            }
        }

        void ProcessNode(aiScene const* scene, aiNode* node, MeshProcessContext& ctx)
        {
            for (uint32_t i = 0; i < node->mNumMeshes; ++i)
            {
                ProcessMesh(scene->mMeshes[node->mMeshes[i]], ctx);
            }

            for (uint32_t i = 0; i < node->mNumChildren; ++i)
            {
                ProcessNode(scene, node->mChildren[i], ctx);
            }
        }

        void ProcessMesh(aiMesh* mesh, MeshProcessContext& ctx)
        {
            uint32_t indexOffset = CopyVertices(mesh, ctx);
            CopyIndices(mesh, ctx, indexOffset);
        }

        uint32_t CopyVertices(aiMesh* mesh, MeshProcessContext& ctx)
        {
            uint32_t initialSize = (uint32_t)ctx.vertices.size();
            ctx.vertices.reserve(initialSize + mesh->mNumVertices);

            for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
            {
                VertexPositionNormal vertex{};
                vertex.position = glm::vec3{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
                vertex.normal = glm::vec3{mesh->mNormals[i].z, mesh->mNormals[i].y, mesh->mNormals[i].z};

                CHECK(!(vertex.normal.x == Jnrlib::Zero && vertex.normal.y == Jnrlib::Zero && vertex.normal.z == Jnrlib::Zero)) << "Normal cannot be (0, 0, 0)";

                ctx.vertices.push_back(vertex);
            }

            return initialSize;
        }

        void CopyIndices(aiMesh* mesh, MeshProcessContext& ctx, uint32_t indexOffset)
        {
            uint32_t initialSize = (uint32_t)ctx.indices.size();

            for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
            {
                auto& face = mesh->mFaces[i];
                CHECK(face.mNumIndices == 3) << "Faces should have 3 indices";

                for (uint32_t j = 0; j < face.mNumIndices; ++j)
                {
                    uint32_t index = face.mIndices[j];

                    ctx.indices.push_back(index + indexOffset);
                }
            }
        }

    public:
        Scene* mScene;

        std::vector<std::unique_ptr<Assimp::Importer>> mImporters;
        std::vector<std::shared_ptr<struct Jnrlib::Task>> mTasksToWaitFor;

        std::mutex mMessageMutex;
        bool mSuccess = true;
        std::string mFailureMessage;

    };

}

/* Scene */
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

void Scene::InitializeGraphics(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex)
{
    CreateRenderingBuffers(cmdList, cmdBufIndex);
}

Vulkan::Buffer const* Scene::GetVertexBuffer() const
{
    return mVertexBuffer.get();
}

Vulkan::Buffer const* Scene::GetIndexBuffer() const
{
    return mIndexBuffer.get();
}

std::vector<Common::VertexPositionNormal> const& Common::Scene::GetVertices() const
{
    return mVertices;
}

std::vector<uint32_t> const& Common::Scene::GetIndices() const
{
    return mIndices;
}

std::vector<Entity*>& Scene::GetRootEntities()
{
    return mRootEntities;
}

std::vector<std::unique_ptr<Entity>>& Scene::GetEntities()
{
    return mEntities;
}

std::optional<HitPoint> Scene::GetClosestHit(Ray& r) const
{
    return Systems::Intersection::Get()->IntersectRay(r, mRegistry, this);
}

uint32_t Scene::GetNumberOfObjects() const
{
    return (uint32_t)mEntities.size();
}

void Scene::PerformUpdate()
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

Entity const* Scene::GetCameraEntity() const
{
    return mCameraEntity;
}

entt::registry& Scene::GetRegistry() const
{
    return mRegistry;
}

uint32_t Scene::AddVertices(std::vector<VertexPositionNormal>&& vertices)
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    uint32_t vertexCount = (uint32_t)mVertices.size();
    std::move(vertices.begin(), vertices.end(), std::back_inserter(mVertices));
    
    return vertexCount;
}

uint32_t Scene::AddIndices(std::vector<uint32_t>&& indices)
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    uint32_t indexCount = (uint32_t)mIndices.size();
    std::move(indices.begin(), indices.end(), std::back_inserter(mIndices));

    return indexCount;
}

bool Scene::IsMeshLoaded(std::string const& meshName) const
{
    return mMeshIndices.find(meshName) != mMeshIndices.end();
}

void Scene::AddMeshIndices(std::string const& meshName, Components::Indices const& mesh)
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    CHECK(mMeshIndices.find(meshName) == mMeshIndices.end()) << "Mesh " << meshName << " already inserted";

    mMeshIndices.insert({meshName, mesh});
}

Components::Indices& Common::Scene::GetMeshIndices(std::string const& meshName)
{
    CHECK(mMeshIndices.find(meshName) != mMeshIndices.end()) << "Cannot get mesh that was not inserted";

    return mMeshIndices[meshName];
}

void Scene::CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives, bool buildRealtime)
{
    Helpers::ModelLoader loader(this);
    uint32_t currentBufferIndex = 0;
    mEntities.reserve(primitives.size());
    mRootEntities.reserve(primitives.size());
    for (uint32_t i = 0; i < primitives.size(); ++i)
    {
        auto& p = primitives[i];
        std::unique_ptr<Entity> entity = std::make_unique<Entity>(mRegistry.create(), mRegistry);

        /* Handle parent-ship */
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

        /* Handle Base */
        entity->AddComponent(
            Components::Base{.world = glm::translate(p.position), .name = p.name, .entityPtr = entity.get()}
        );

        /* Make sure that there's a material to be used */
        auto material = MaterialManager::Get()->GetMaterial(p.materialName);
        CHECK(material) << "All primitives must have a material";

        if (buildRealtime)
        {
            entity->AddComponent(
                Components::Update{.dirtyFrames = Constants::FRAMES_IN_FLIGHT, .bufferIndex = currentBufferIndex++}
            );
            mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(entity.get());
            entity->UpdateBase();
        }

        switch (p.primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
            {
                if (mMeshIndices.find("Sphere") == mMeshIndices.end())
                {
                    if (buildRealtime)
                    {
                        Components::Indices indices{};
                        indices.firstVertex = (uint32_t)mVertices.size();
                        indices.firstIndex = (uint32_t)mIndices.size();
                        uint32_t stackCount = 36;
                        uint32_t sliceCount = 36;
                        std::vector<VertexPositionNormal> sphereVertices;
                        std::vector<uint32_t> sphereIndices;
                        Sphere::GetVertices(One, sliceCount, stackCount, sphereVertices, sphereIndices);
                        AddVertices(std::move(sphereVertices));
                        AddIndices(std::move(sphereIndices));
                        indices.indexCount = (uint32_t)mIndices.size() - indices.firstIndex;
                        AddMeshIndices("Sphere", indices);

                        mRegistry.on_update<Components::Sphere>().connect<&Entity::UpdateBase>(entity.get());
                    }
                }
                Components::Mesh mesh{};
                mesh.material = material;
                mesh.name = p.name;
                if (buildRealtime)
                {
                    /* They are only needed for realtime rendering */
                    mesh.indices = GetMeshIndices("Sphere");
                }
                entity->AddComponent(mesh);
                entity->AddComponent(Components::Sphere{.radius = p.radius, .material = material});

                break;
            }
            case CreateInfo::PrimitiveType::Mesh:
            { 
                std::string name = Helpers::GetMeshNameFromPath(p.path);
                if (mMeshIndices.find(name) == mMeshIndices.end())
                {
                    loader.LoadModel(p.path, entity.get(), material, p.accelerationInfo);
                }
                else
                {
                    CHECK(false) << "Unable to load the same mesh multiple times";
                }

                break;
            }
        }

        mEntities.push_back(std::move(entity));
    }

    loader.Wait();
}

void Scene::CreateRenderingBuffers(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex)
{
    {
        /* Vertex buffer */
        std::unique_ptr<Vulkan::Buffer> localVertexBuffer =
            std::make_unique<Vulkan::Buffer>(sizeof(VertexPositionNormal), mVertices.size(),
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        localVertexBuffer->Copy(mVertices.data());

        mVertexBuffer = std::make_unique<Vulkan::Buffer>(sizeof(VertexPositionNormal), mVertices.size(),
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
            .world = glm::translate(cameraInfo.position),
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
            Components::Update{.dirtyFrames = Constants::FRAMES_IN_FLIGHT, .bufferIndex = (uint32_t)mRegistry.size()}
        );
        mRegistry.on_update<Components::Base>().connect<&Components::Camera::Update>(cameraComponent);
        mRegistry.on_update<Components::Camera>().connect<&Components::Camera::Update>(cameraComponent);

        mRegistry.on_update<Components::Base>().connect<&Entity::UpdateBase>(entity.get());
        entity->UpdateBase();
    }


    mRootEntities.push_back(entity.get());
    mCameraEntity = entity.get();

    mEntities.push_back(std::move(entity));
}
