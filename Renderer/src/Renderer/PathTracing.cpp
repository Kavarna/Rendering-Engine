#include "Jnrlib.h"
#include "PathTracing.h"


PathTracing::PathTracing(PngDumper& dumper, Scene const& scene, uint32_t numSamples, uint32_t maxDepth) :
    mDumper(dumper),
    mScene(scene),
    mNumSamples(numSamples),
    mMaxDepth(maxDepth)
{ }

void PathTracing::Render()
{
    uint32_t width = mDumper.GetWidth();
    uint32_t height = mDumper.GetHeight();

    auto threadPool = Jnrlib::ThreadPool::Get();

    mDumper.SetTotalWork(width * height);

    Jnrlib::Position pos = mScene.GetCamera().GetPosition();
    Jnrlib::Direction forwardDirection = mScene.GetCamera().GetForwardDirection();
    Jnrlib::Direction rightDirection = mScene.GetCamera().GetRightDirection();
    Jnrlib::Direction upDirection = mScene.GetCamera().GetUpDirection();
    Jnrlib::Float focalDistance = mScene.GetCamera().GetFocalDistance();
    Jnrlib::Float halfViewportWidth = Jnrlib::Half * mScene.GetCamera().GetViewportWidth();
    Jnrlib::Float halfViewportHeight = Jnrlib::Half * mScene.GetCamera().GetViewportHeight();

    Jnrlib::Position upperLeftCorner = pos + forwardDirection * focalDistance - rightDirection * halfViewportWidth + upDirection * halfViewportHeight;

    SetPixelColor(122, 339, width, height, upperLeftCorner);

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            threadPool->ExecuteDeffered(
                std::bind(&PathTracing::SetPixelColor, this, x, y, width, height, upperLeftCorner)
            );
        }
    }

    threadPool->WaitForAll();
}

void PathTracing::SetPixelColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Jnrlib::Position const& upperLeftCorner)
{
    Jnrlib::Position pos = mScene.GetCamera().GetPosition();
    Jnrlib::Direction rightDirection = mScene.GetCamera().GetRightDirection();
    Jnrlib::Direction upDirection = mScene.GetCamera().GetUpDirection();

    Jnrlib::Float viewportWidth = mScene.GetCamera().GetViewportWidth();
    Jnrlib::Float viewportHeight = mScene.GetCamera().GetViewportHeight();

    Jnrlib::Color color(Jnrlib::Zero);
    for (uint32_t i = 0; i < mNumSamples; ++i)
    {
        Jnrlib::Float u = ((Jnrlib::Float)x + Jnrlib::Random::get(-Jnrlib::One, Jnrlib::One)) / (width - 1);
        Jnrlib::Float v = ((Jnrlib::Float)y + Jnrlib::Random::get(-Jnrlib::One, Jnrlib::One)) / (width - 1);

        Ray ray(pos, upperLeftCorner + u * rightDirection * viewportWidth - v * upDirection * viewportHeight - pos);
        color += GetRayColor(ray);
    }

    color /= (Jnrlib::Float)mNumSamples;
    
    mDumper.SetPixelColor(x, y, color);
    
    mDumper.AddDoneWork();
}

Jnrlib::Color PathTracing::GetRayColor(Ray const& ray, uint32_t depth)
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
;
        Jnrlib::Color newColor = GetRayColor(scatterInfo->ray, depth + 1);

        return scatterInfo->attenuation * newColor;
    }
    else
    {
        Jnrlib::Float t = Jnrlib::Half * (ray.GetDirection().y + Jnrlib::One);
        Jnrlib::Color whiteSkyColor = Jnrlib::Color(Jnrlib::Half);
        Jnrlib::Color blueSkyColor = Jnrlib::Color(Jnrlib::Half, Jnrlib::Half, Jnrlib::One, 1.0f);
        return t * whiteSkyColor + (Jnrlib::One - t) * blueSkyColor;
    }
}

