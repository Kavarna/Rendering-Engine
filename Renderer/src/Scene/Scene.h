#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>

#include "CreateInfoUtils.h"

#include "Camera.h"

class Scene
{
public:
    Scene(CreateInfo::Scene const& info);
    ~Scene();

public:
    CreateInfo::RendererType GetRendererType() const;
    std::string GetOutputFile() const;

    const CreateInfo::ImageInfo& GetImageInfo() const;

    void SetCamera(std::unique_ptr<Camera>&& camera);

private:
    CreateInfo::Scene mInfo;

    std::unique_ptr<Camera> mCamera;
};


