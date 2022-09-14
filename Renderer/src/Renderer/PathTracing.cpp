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

    for (uint32_t y = 0; y < height; ++y)
    {
        std::vector<std::shared_ptr<struct Jnrlib::Task>> tasks;
        tasks.reserve(width);
        for (uint32_t x = 0; x < width; ++x)
        {
            tasks.emplace_back(threadPool->ExecuteDeffered(
                [this, x, y, width, height]()
                {
                    float red = (float)x / (width - 1.0f);
                    float green = (float)y / (height - 1.0f);
                    float blue = 0.0f;
                    mDumper.SetPixelColor(x, y, red, green, blue);
                })
            );
        }
        for (auto& task : tasks)
        {
            threadPool->Wait(task);
        }
        mDumper.SetProgress((float)y / (width - 1));
    }
}
