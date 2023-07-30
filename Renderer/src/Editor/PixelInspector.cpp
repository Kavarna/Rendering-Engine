#include "PixelInspector.h"
#include "BufferDumper.h"
#include "Vulkan/Image.h"
#include "Vulkan/CommandList.h"
#include "RayTracing/Renderer.h"

#include "Scene/Systems/RealtimeRenderSystem.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "Constants.h"

using namespace Editor;
using namespace Common;

PixelInspector::PixelInspector()
{

}

void PixelInspector::CopySelectedRegion(uint32_t srcX, uint32_t srcY, Common::BufferDumper* buffer, RayTracing::Renderer* renderer, Vulkan::CommandList* cmdList)
{
    if (buffer == nullptr || renderer == nullptr)
    {
        mPreviewImage = nullptr;
        mRenderer = nullptr;
        return;
    }
    mRenderer = renderer;
    mSelectedX = srcX;
    mSelectedY = srcY;

    mLastPreviewImage = std::move(mPreviewImage);
    mPreviewImage = std::make_unique<BufferDumper>(PREVIEW_WIDTH, PREVIEW_HEIGHT);

    /* Perform the copy */
    auto srcBuffer = buffer->GetBuffer();
    auto dstBuffer = mPreviewImage->GetBuffer();
    for (uint32_t row = 0; row < PREVIEW_HEIGHT; ++row)
    {
        uint32_t y = row + srcY;
        uint32_t sourceIndex = y * buffer->GetWidth() + srcX - PREVIEW_WIDTH / 2;

        void const* src = srcBuffer->GetElement(sourceIndex);
        void* dst = dstBuffer->GetElement(row * PREVIEW_WIDTH);
        memcpy(dst, src, sizeof(Jnrlib::Color) * PREVIEW_WIDTH);
    }
    mPreviewImage->Flush(cmdList, true);
    cmdList->TransitionImageToImguiLayout(mPreviewImage->GetImage());
}

void PixelInspector::RenderRays(Common::Systems::RealtimeRender& render)
{
    for (auto const& pos : mRays)
    {
        render.AddOneTimeVertex(pos, Jnrlib::Yellow);
    }
}

void PixelInspector::OnRender()
{
    if (!ImGui::Begin("Pixel inspector"))
    {
        // return;
    }

    if (mPreviewImage != nullptr)
    {
        ImVec2 size;
        size.x = PREVIEW_WIDTH * 10;
        size.y = PREVIEW_HEIGHT * 10;
        auto image = mPreviewImage->GetImage();
        ImGui::Image(image->GetTextureID(), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 0, 1));
        if (ImGui::Button("Trace ray"))
        {
            Common::Ray::mPositions.clear();
            Common::Ray::mSaveRays = true;
            Jnrlib::DefferCall call([]()
            {
                Common::Ray::mSaveRays = false;
            });
            mRenderer->TracePixel(mSelectedX, mSelectedY);
            mRays = std::move(Common::Ray::mPositions);
        }
    }

    ImGui::End();
}
