#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Scene.h"

namespace Common
{
    class BufferDumper;
    namespace Systems
    {
        class RealtimeRender;
    }
}
namespace Vulkan
{
    class CommandList;
}
namespace RayTracing
{
    class Renderer;
}

namespace Editor
{
    class SceneViewer;

    class PixelInspector : public ImguiWindow
    {
        static constexpr const uint32_t PREVIEW_WIDTH = 11;
        static constexpr const uint32_t PREVIEW_HEIGHT = 11;
    public:
        PixelInspector();
        ~PixelInspector() = default;

    public:
        void CopySelectedRegion(uint32_t x, uint32_t y, Common::BufferDumper* buffer, RayTracing::Renderer* renderer, Vulkan::CommandList* cmdList);
        void RenderRays(Common::Systems::RealtimeRender& render);

        virtual void OnRender() override;

    private:
        RayTracing::Renderer* mRenderer;

        uint32_t mSelectedX;
        uint32_t mSelectedY;

        std::vector<Jnrlib::Position> mRays;

        std::unique_ptr<Common::BufferDumper> mPreviewImage;
        std::unique_ptr<Common::BufferDumper> mLastPreviewImage;
    };
}
