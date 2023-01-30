#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>

#include "CreateInfo/SceneCreateInfo.h"
#include "HitPoint.h"

#include "Camera.h"

namespace Common
{
    class Scene
    {
    public:
        Scene(CreateInfo::Scene& info);
        ~Scene();

    public:
        std::string GetOutputFile() const;

        const CreateInfo::ImageInfo& GetImageInfo() const;

        void SetCamera(std::unique_ptr<Camera>&& camera);
        Camera const& GetCamera() const;

    public:
        std::optional<HitPoint> GetClosestHit(Ray const&) const;

    private:
        void CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives);

    private:
        std::string mOutputFile;
        CreateInfo::ImageInfo mImageInfo;

        std::unique_ptr<Camera> mCamera;
        std::vector<std::unique_ptr<Primitive>> mObjects;
    };
}
