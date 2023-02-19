#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>

#include "CreateInfo/SceneCreateInfo.h"
#include "HitPoint.h"
#include "Vertex.h"
#include "Camera.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandList.h"
#include "Scene/Components/MeshComponent.h"

namespace Common::Systems
{
    class RealtimeRender;
}

namespace Common
{
    class Scene
    {
        friend class Common::Systems::RealtimeRender;
    public:
        Scene(CreateInfo::Scene const& info);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

    public:
        std::string GetOutputFile() const;

        const CreateInfo::ImageInfo& GetImageInfo() const;

        void SetCamera(std::unique_ptr<Camera>&& camera);
        Camera& GetCamera();
        Camera const& GetCamera() const;

        void InitializeGraphics(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);

        Vulkan::Buffer const* GetVertexBuffer() const;
        Vulkan::Buffer const* GetIndexBuffer() const;

    public:
        std::optional<HitPoint> GetClosestHit(Ray const&) const;
        uint32_t GetNumberOfObjects() const;
        void PerformUpdate();

#if DEBUG
        entt::registry& GetEntities() const;
#endif

    private:
        void CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives, bool alsoBuildRealtime);
        void CreateRenderingBuffers(Vulkan::CommandList* cmdList, uint32_t cmdBufIndex);

    private:
        std::string mOutputFile;
        CreateInfo::ImageInfo mImageInfo;

        std::unique_ptr<Camera> mCamera;
        mutable entt::registry mEntities;

        std::unordered_map<std::string, Components::Mesh> mMeshes;

        bool mGraphicsInitialized = false;

        std::unique_ptr<Vulkan::Buffer> mVertexBuffer;
        std::vector<Common::Vertex> mVertices;
        std::unique_ptr<Vulkan::Buffer> mIndexBuffer;
        std::vector<uint32_t> mIndices;
    };
}
