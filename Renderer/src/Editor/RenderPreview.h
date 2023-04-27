#pragma once

#include <vector>
#include <memory>
#include "ImguiWindow.h"


namespace RayTracing
{
    class PathTracing;
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
    class RenderPreview : public ImguiWindow
    {
    public:
        struct RenderingContext
        {
            Vulkan::CommandList* cmdList = nullptr;
        };
    public:
        RenderPreview(Common::Scene* scene, Vulkan::CommandList* cmdList, uint32_t cmdBufIndex = 0);
        ~RenderPreview();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

    private:
        void StartRendering();

        void ShowProgress();

    private:
        void RenderSimplePathTracing();

    private:
        Common::Scene* mScene;

        RenderingContext mActiveRenderingContext{};

        std::unique_ptr<Common::BufferDumper> mBufferDumper;
        std::unique_ptr<Common::BufferDumper> mLastBufferDumper;

        std::unique_ptr<RayTracing::PathTracing> mPathTracing;

        int32_t mRendererType = 0;
        std::vector<const char*> mRendererTypes;
        bool mIsRenderingActive = false;
    };
}
