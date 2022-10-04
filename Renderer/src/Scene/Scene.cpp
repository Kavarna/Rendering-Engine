#include "Scene.h"


Scene::Scene(CreateInfo::Scene& info) :
    mRendererType(info.rendererType),
    mOutputFile(info.outputFile),
    mImageInfo(info.imageInfo),
    mObjects(std::move(info.primitives))
{
    LOG(INFO) << "Creating scene with info: " << info;
}

Scene::~Scene()
{ }

CreateInfo::RendererType Scene::GetRendererType() const
{
    return mRendererType;
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

Camera const& Scene::GetCamera() const
{
    return *mCamera;
}
