#include "Scene.h"


Scene::Scene(CreateInfo::Scene const& info) :
    mInfo(info)
{
    LOG(INFO) << "Creating scene with info: " << info;
}

Scene::~Scene()
{ }

CreateInfo::RendererType Scene::GetRendererType() const
{
    return mInfo.rendererType;
}

std::string Scene::GetOutputFile() const
{
    return mInfo.outputFile;
}

const CreateInfo::ImageInfo& Scene::GetImageInfo() const
{
    return mInfo.imageInfo;
}

void Scene::SetCamera(std::unique_ptr<Camera>&& camera)
{
    mCamera = std::move(camera);
}

Camera const& Scene::GetCamera() const
{
    return *mCamera;
}
