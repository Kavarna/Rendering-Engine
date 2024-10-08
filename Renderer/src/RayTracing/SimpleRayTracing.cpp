#include "Jnrlib.h"
#include "SimpleRayTracing.h"

#include "Scene/Components/Camera.h"
#include "Scene/Components/Base.h"
#include "CameraUtils.h"
#include "SimpleRayTracing.h"

using namespace RayTracing;
using namespace Common;

constexpr uint32_t TILE_SIZE = 64;

SimpleRayTracing::SimpleRayTracing(Common::IDumper& dumper, Common::Scene& scene, uint32_t maxDepth):
    mDumper(dumper),
    mScene(scene),
    mMaxDepth(maxDepth)
{ }

void SimpleRayTracing::Render()
{
    auto threadPool = Jnrlib::ThreadPool::Get();
    auto width = mDumper.GetWidth();
    auto height = mDumper.GetHeight();

    mDumper.SetTotalWork(width * height);

    uint32_t tileId = 0;
    for (uint32_t y = 0; y < height; y += TILE_SIZE)
    {
        for (uint32_t x = 0; x < width; x += TILE_SIZE)
        {
            threadPool->ExecuteDeffered(
                std::bind(&SimpleRayTracing::RenderTile, this, x, y, tileId)
            );
            tileId++;
        }
    }

    threadPool->WaitForAll();
}

void SimpleRayTracing::TracePixel(uint32_t x, uint32_t y)
{
    auto& cameraComponent = mScene.GetCameraEntity()->GetComponent<Common::Components::Camera>();
    auto &cameraBaseComponent = mScene.GetCameraEntity()->GetComponent<Common::Components::Base>();

    auto ray = Common::CameraUtils::GetRayForPixel(&cameraBaseComponent, &cameraComponent, x, y);

    Jnrlib::Float t = Jnrlib::Half * (ray.direction.y + Jnrlib::One);
    Jnrlib::Color whiteSkyColor = Jnrlib::Color(Jnrlib::Half);
    Jnrlib::Color blueSkyColor = Jnrlib::Color(Jnrlib::Quarter, Jnrlib::Quarter, Jnrlib::One, 1.0f);

    Jnrlib::Color color = t * whiteSkyColor + (Jnrlib::One - t) * blueSkyColor;

    if (auto hp = mScene.GetClosestHit(ray); hp.has_value())
    {
        auto material = hp->GetMaterial();

        if (std::optional<ScatterInfo> scatterInfo = material->Scatter(ray, *hp); scatterInfo.has_value())
            color = scatterInfo->attenuation;

        Jnrlib::Float attenuation = std::clamp(glm::dot(hp->GetNormal(), glm::normalize(Jnrlib::Direction(0.5f, 0.5f, -1.0f))) + 0.2f, Jnrlib::Zero, Jnrlib::One);
        color *= attenuation;
    }

    mDumper.SetPixelColor(x, y, color);
    mDumper.AddDoneWork();
}

void SimpleRayTracing::RenderTile(uint32_t _x, uint32_t _y, uint32_t tileId)
{
    auto width = mDumper.GetWidth();
    auto height = mDumper.GetHeight();

    uint32_t actualWidth = std::min(_x + TILE_SIZE, width);
    uint32_t actualHeight = std::min(_y + TILE_SIZE, height);
    for (uint32_t y = _y; y < actualHeight; ++y)
    {
        for (uint32_t x = _x; x < actualWidth; ++x)
        {
            TracePixel(x, y);
        }
    }
}

