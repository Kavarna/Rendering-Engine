#include "Jnrlib.h"
#include "PathTracing.h"


PathTracing::PathTracing(PngDumper& dumper, Scene const& scene) :
    mDumper(dumper),
    mScene(scene)
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
    Jnrlib::Float u = (Jnrlib::Float)x / (width - 1);
    Jnrlib::Float v = (Jnrlib::Float)y / (width - 1);

    Jnrlib::Position pos = mScene.GetCamera().GetPosition();
    Jnrlib::Direction rightDirection = mScene.GetCamera().GetRightDirection();
    Jnrlib::Direction upDirection = mScene.GetCamera().GetUpDirection();

    Ray ray(pos, upperLeftCorner + u * rightDirection - v * upDirection - pos);

    Jnrlib::Color color = GetRayColor(ray);
    
    mDumper.SetPixelColor(x, y, color);
    
    mDumper.AddDoneWork();
}

Jnrlib::Color PathTracing::GetRayColor(Ray const& ray)
{
    Jnrlib::Float t = Jnrlib::Half * (ray.GetDirection().y + Jnrlib::One);
    return (1.0f - t) * Jnrlib::Color(1.0f, 1.0f, 1.0f, 1.0f) + t * Jnrlib::Color(0.5f, 0.7f, 1.0f, 1.0f);
}
