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


    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            threadPool->ExecuteDeffered(
                std::bind(&PathTracing::SetPixelColor, this, x, y, width, height)
            );
        }
    }
    LOG(INFO) << "Start waiting";
    threadPool->WaitForAll();
    LOG(INFO) << "Done waiting";
    
}

void PathTracing::SetPixelColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    float red = (float)x / (width - 1);
    float green = (float)y / (width - 1);
    mDumper.SetPixelColor(x, y, red, green, 0.0f);
    mDumper.AddDoneWork();
}
