#include "Scene.h"

#include <Sphere.h>

using namespace Common;

Scene::Scene(CreateInfo::Scene& info) :
    mOutputFile(info.outputFile),
    mImageInfo(info.imageInfo)
{
    LOG(INFO) << "Creating scene with info: " << info;
    CreatePrimitives(info.primitives);
}

Scene::~Scene()
{ }

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
#undef max
    std::optional<HitPoint> finalHitPoint;
    Jnrlib::Float closestHitSoFar = std::numeric_limits<Jnrlib::Float>::max();

    for (const auto& primitive : mObjects)
    {
        std::optional<HitPoint> hp = primitive->IntersectRay(r);
        
        if (!hp.has_value())
            continue;
        
        Jnrlib::Float intersectionPoint = hp->GetIntersectionPoint();
        if (intersectionPoint < closestHitSoFar)
        {
            closestHitSoFar = intersectionPoint;
            finalHitPoint = hp.value();
        }
    }

    return finalHitPoint;
}

void Scene::CreatePrimitives(std::vector<CreateInfo::Primitive> const& primitives)
{
    
    mObjects.reserve(primitives.size());
    for (auto const& p : primitives)
    {
        switch (p.primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
            {
                mObjects.emplace_back(
                    std::make_unique<Sphere>(p.name, p.position, p.radius, p.materialName));
                break;
            }
        }
    }
}
