#pragma once

#include <vector>
#include <memory>
#include "ImguiWindow.h"

namespace Common
{
    class Scene;
}
namespace Common::Systems
{
    class RealtimeRender;
}
namespace Vulkan
{
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
            uint32_t cmdBufIndex = 0;
        };
    public:
        RenderPreview(Common::Scene* scene, Vulkan::CommandList* cmdList, uint32_t cmdBufIndex = 0);
        ~RenderPreview();

        void SetRenderingContext(RenderingContext const& ctx);

        virtual void OnRender() override;

    private:
        void StartRendering();

    private:
        Common::Scene* mScene;

        RenderingContext mActiveRenderingContext{};

        int32_t mRendererType = 0;
        std::vector<const char*> mRendererTypes;
    };
}

