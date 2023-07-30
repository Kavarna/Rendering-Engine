#include "Jnrlib.h"
#include "SimpleRayTracing.h"

#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/BaseComponent.h"
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
    Jnrlib::Color color = Jnrlib::Blue * (Jnrlib::Float)0.2f;
    auto ray = Common::CameraUtils::GetRayForPixel(&cameraComponent, x, y);
    auto hp = mScene.GetClosestHit(ray);

    if (hp.has_value())
    {
        auto material = hp->GetMaterial();

        std::optional<ScatterInfo> scatterInfo = material->Scatter(ray, *hp);
        if (scatterInfo.has_value())
            color = scatterInfo->attenuation;
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

