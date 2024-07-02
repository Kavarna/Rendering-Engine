#include "Jnrlib.h"
#include "PathTracing.h"

#include "Scene/Components/Camera.h"
#include "Scene/Components/Base.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace RayTracing;
using namespace Common;

PathTracing::PathTracing(IDumper& dumper, Scene& scene, uint32_t numSamples, uint32_t maxDepth) :
    mDumper(dumper),
    mScene(scene),
    mNumSamples(numSamples),
    mMaxDepth(maxDepth)
{ }

void PathTracing::Render()
{
    mWidth = mDumper.GetWidth();
    mHeight = mDumper.GetHeight();

    auto threadPool = Jnrlib::ThreadPool::Get();

    mDumper.SetTotalWork(mWidth * mHeight);

    mUpperLeftCorner = mScene.GetCameraEntity()->GetComponent<Common::Components::Camera>().GetUpperLeftCorner();

    for (uint32_t y = 0; y < mHeight; ++y)
    {
        for (uint32_t x = 0; x < mWidth; ++x)
        {
            threadPool->ExecuteDeffered(
                std::bind(&PathTracing::TracePixel, this, x, y)
            );
        }
    }

    threadPool->WaitForAll();
}

void PathTracing::TracePixel(uint32_t x, uint32_t y)
{
    auto const& cameraComponent = mScene.GetCameraEntity()->GetComponent<Common::Components::Camera>();
    auto const& baseComponent = mScene.GetCameraEntity()->GetComponent<Common::Components::Base>();

    Jnrlib::Vec3 scale;
    Jnrlib::Quaternion rotation;
    Jnrlib::Position translation;
    Jnrlib::Vec3 skew;
    Jnrlib::Vec4 perspective;
    glm::decompose(baseComponent.world, scale, rotation, translation, skew, perspective);

    Jnrlib::Position pos = translation;
    Jnrlib::Direction rightDirection = cameraComponent.GetRightDirection();
    Jnrlib::Direction upDirection = cameraComponent.GetUpDirection();

    Jnrlib::Float viewportWidth = cameraComponent.viewportSize.x;
    Jnrlib::Float viewportHeight = cameraComponent.viewportSize.y;

    Jnrlib::Color color(Jnrlib::Zero);
    for (uint32_t i = 0; i < mNumSamples; ++i)
    {
        Jnrlib::Float u = ((Jnrlib::Float)x + Jnrlib::Random::get(-Jnrlib::One, Jnrlib::One)) / (mWidth - 1);
        Jnrlib::Float v = ((Jnrlib::Float)y + Jnrlib::Random::get(-Jnrlib::One, Jnrlib::One)) / (mHeight - 1);

        Ray ray(pos, mUpperLeftCorner + u * rightDirection * viewportWidth - v * upDirection * viewportHeight - pos);
        color += GetRayColor(ray);
    }

    color /= (Jnrlib::Float)mNumSamples;
    
    mDumper.SetPixelColor(x, y, color);
    
    mDumper.AddDoneWork();
}

Jnrlib::Color PathTracing::GetRayColor(Ray& ray, uint32_t depth)
{
    if (depth >= mMaxDepth)
    {
        return Jnrlib::Color(Jnrlib::Zero);
    }

    if (auto _hp = mScene.GetClosestHit(ray); _hp.has_value())
    {
        HitPoint hp = (*_hp);

        auto material = hp.GetMaterial();
        
        std::optional<ScatterInfo> scatterInfo = material->Scatter(ray, hp);
        if (!scatterInfo.has_value())
            return Jnrlib::Color(Jnrlib::Zero);

        Jnrlib::Color newColor = GetRayColor(scatterInfo->ray, depth + 1);

        return scatterInfo->attenuation * newColor;
    }
    else
    {
        Jnrlib::Float t = Jnrlib::Half * (ray.direction.y + Jnrlib::One);
        Jnrlib::Color whiteSkyColor = Jnrlib::Color(Jnrlib::Half);
        Jnrlib::Color blueSkyColor = Jnrlib::Color(Jnrlib::Quarter, Jnrlib::Quarter, Jnrlib::One, 1.0f);
        return t * whiteSkyColor + (Jnrlib::One - t) * blueSkyColor;
    }
}

