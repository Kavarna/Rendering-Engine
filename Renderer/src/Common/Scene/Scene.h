#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>

#include "CreateInfo/SceneCreateInfo.h"
#include "HitPoint.h"
#include "Vertex.h"
#include "Vulkan/Buffer.h"
#include "Scene/Components/Mesh.h"
#include "Entity.h"

namespace Vulkan
{
    class CommandList;
}

namespace Common
{
    class Scene
    {
    public:
        Scene(CreateInfo::Scene const& info);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

    public:
        std::string GetOutputFile() const;

        const CreateInfo::ImageInfo& GetImageInfo() const;

        void InitializeGraphics(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);

        Vulkan::Buffer const* GetVertexBuffer() const;
        Vulkan::Buffer const* GetIndexBuffer() const;

        std::vector<Common::VertexPositionNormal> const& GetVertices() const;
        std::vector<uint32_t> const& GetIndices() const;

        std::vector<Entity*>& GetRootEntities();
        std::vector<std::unique_ptr<Entity>>& GetEntities();

        Entity *AddNewEntity(std::string const &name, Jnrlib::Matrix4x4 const world, bool buildRealtime, Entity *parentEntity = nullptr);
        Entity* AddNewEntity(bool buildRealtime, Entity* parentEntity = nullptr);

        void AddSphereComponent(Entity *entity, bool alsoBuildRealtime, std::shared_ptr<IMaterial> material, Jnrlib::Float radius);

    public:
        std::optional<HitPoint> GetClosestHit(Ray&) const;
        uint32_t GetNumberOfObjects() const;
        void PerformUpdate();

        Entity const* GetCameraEntity() const;

        entt::registry& GetRegistry() const;

    public:
        uint32_t AddVertices(std::vector<Common::VertexPositionNormal>&& vertices);
        uint32_t AddIndices(std::vector<uint32_t>&& indices);
        bool IsMeshLoaded(std::string const& meshName) const;
        void AddMeshIndices(std::string const& meshName, Components::Indices const& mesh);
        Components::Indices& GetMeshIndices(std::string const& meshName);

    private:
        void CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives, bool alsoBuildRealtime);
        void CreateRenderingBuffers(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);
        void CreateCamera(CreateInfo::Camera const& cameraInfo, bool alsoBuildRealtime);

    private:
        std::string mOutputFile;
        CreateInfo::ImageInfo mImageInfo;

        mutable entt::registry mRegistry;
        std::vector<std::unique_ptr<Entity>> mEntities;
        std::vector<Entity*> mRootEntities;
        Entity* mCameraEntity;

        std::unordered_map<std::string, Components::Indices> mMeshIndices;

        bool mGraphicsInitialized = false;

        std::unique_ptr<Vulkan::Buffer> mVertexBuffer;
        std::vector<Common::VertexPositionNormal> mVertices;
        std::unique_ptr<Vulkan::Buffer> mIndexBuffer;
        std::vector<uint32_t> mIndices;
    };
}
