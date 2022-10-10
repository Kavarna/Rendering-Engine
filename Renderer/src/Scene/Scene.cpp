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

std::optional<HitPoint> Scene::GetClosestHit(Ray const& r) const
{
    std::optional<HitPoint> finalHitPoint;
    Jnrlib::Float closestHitSoFar = 0.0;

    for (const auto& primitive : mObjects)
    {
        std::optional<HitPoint> hp = primitive->IntersectRay(r);
        
        if (!hp.has_value())
            continue;
        
        for (Jnrlib::Float intersectionPoint : hp->GetIntersectionPoints())
        {
            if (intersectionPoint < 0.0f)
                continue;

            if (!finalHitPoint.has_value())
            {
                finalHitPoint = hp;
                closestHitSoFar = intersectionPoint;
            }

            if (intersectionPoint < closestHitSoFar)
            {
                closestHitSoFar = intersectionPoint;
                finalHitPoint = hp;
            }
        }
    }

    return finalHitPoint;
}
