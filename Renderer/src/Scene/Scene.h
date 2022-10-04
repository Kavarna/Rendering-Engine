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
    CreateInfo::RendererType GetRendererType() const;
    std::string GetOutputFile() const;

    const CreateInfo::ImageInfo& GetImageInfo() const;

    void SetCamera(std::unique_ptr<Camera>&& camera);
    Camera const& GetCamera() const;

private:
    std::string mOutputFile;
    CreateInfo::RendererType mRendererType;
    CreateInfo::ImageInfo mImageInfo;

    std::unique_ptr<Camera> mCamera;
    std::vector<std::unique_ptr<Primitive>> mObjects;
};


