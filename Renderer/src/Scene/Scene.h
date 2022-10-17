#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>

#include "CreateInfoUtils.h"

#include "Camera.h"

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
    std::string mOutputFile;
    CreateInfo::ImageInfo mImageInfo;

    std::unique_ptr<Camera> mCamera;
    std::vector<std::unique_ptr<Primitive>> mObjects;
};


