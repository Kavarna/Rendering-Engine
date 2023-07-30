#pragma once

#include <vector>
#include <memory>
#include "ImguiWindow.h"


namespace RayTracing
{
    class Renderer;
    class PathTracing;
    class SimpleRayTracing;
}

namespace Common
{
    class Scene;
    class BufferDumper;
}
namespace Common::Systems
{
    class RealtimeRender;
}
namespace Vulkan
{
    class Buffer;
    class Image;
    class CommandList;
}

namespace Editor
{
    class PixelInspector;

    class RenderPreview : public ImguiWindow
    {
    public:
        struct RenderingContext
        {
            Vulkan::CommandList* cmdList = nullptr;
        };
    public:
        RenderPreview(Common::Scene* scene, PixelInspector* pixelInspector);
        ~RenderPreview();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

    private:
        void OnResize(float newWidth, float newHeight);

        void StartRendering();

        void ShowProgress();

        void UpdateActive();

    private:
        void RenderSimplePathTracing();
        void RenderSimpleRayTracing();

    private:
        float mWidth = 0.0f;
        float mHeight = 0.0f;

        Common::Scene* mScene;
        PixelInspector* mPixelInspector;

        RenderingContext mActiveRenderingContext{};

        std::unique_ptr<Common::BufferDumper> mBufferDumper;
        std::unique_ptr<Common::BufferDumper> mLastBufferDumper;

        std::unique_ptr<RayTracing::Renderer> mRenderer;

        int32_t mRendererType = 0;
        std::vector<std::string> mRendererTypes;
        bool mIsRenderingActive = false;
    };
}

