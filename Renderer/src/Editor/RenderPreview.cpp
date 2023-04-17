#include "RenderPreview.h"

#include "Scene/Scene.h"
#include "Scene/Systems/RealtimeRenderSystem.h"

#include "RayTracing/PathTracing.h"

#include "BufferDumper.h"

#include "Vulkan/Image.h"
#include "Vulkan/CommandList.h"

#include <boost/algorithm/string.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#define REALTIME_RENDER_TYPE 0
#define SIMPLE_PATH_TRACING_RENDER_TYPE 1

using namespace Common;

Editor::RenderPreview::RenderPreview(Common::Scene* scene, Vulkan::CommandList* cmdList, uint32_t cmdBufIndex) :
    mScene(scene)
{
    mRendererTypes.push_back("Real time render");
    mRendererTypes.push_back("Simple path tracing");
}

Editor::RenderPreview::~RenderPreview()
{ }

void Editor::RenderPreview::SetRenderingContext(RenderingContext const& ctx)
{
    mActiveRenderingContext = ctx;
}

void Editor::RenderPreview::OnRender()
{
    ImGui::Begin("Render Preview");

    bool isRenderingActive = mIsRenderingActive;
    if (isRenderingActive)
    {
        /* Make the button grayed out as we can't start rendering while one is active already */
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    if (ImGui::Button("Render", ImVec2(0,0)))
    {
        StartRendering();
    }
    if (isRenderingActive)
    {
        /* Pop the imgui states */
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    ImGui::SameLine();
    ImGui::Combo("Renderer type", &mRendererType, mRendererTypes.data(), (int)mRendererTypes.size());

    ShowProgress();

    ImGui::End();
}

void Editor::RenderPreview::StartRendering()
{
    if (mRendererType == SIMPLE_PATH_TRACING_RENDER_TYPE)
    {
        RenderSimplePathTracing();
        mIsRenderingActive = true;
    }
}

void Editor::RenderPreview::ShowProgress()
{
    if (mBufferDumper)
    {
        float totalWork = (float)mBufferDumper->GetTotalWork();
        float doneWork = (float)mBufferDumper->GetDoneWork();
        float progress = doneWork / totalWork;
        ImGui::SameLine();
        ImGui::ProgressBar(progress, ImVec2(-FLT_MIN, 0), "Rendering");

        auto frameHeight = ImGui::GetFrameHeight();
        float width, height;
        auto currentCursorPos = ImGui::GetCursorPos();
        width = ImGui::GetWindowWidth() - 2; /* One pixel on the left, one pixel on the right */
        height = ImGui::GetWindowHeight() - currentCursorPos.y - frameHeight; /* One pixel up, one pixel down */

        static float time = 0.0f;

        time += ImGui::GetIO().DeltaTime;
        if (time >= 5.0f)
        {
            mBufferDumper->Flush(mActiveRenderingContext.cmdList);
            time -= 5.0f;
        }

        ImVec2 size;
        size.x = width;
        size.y = height;
        auto image = mBufferDumper->GetImage();
        auto textureId = image->GetTextureID();
        mActiveRenderingContext.cmdList->TransitionImageToImguiLayout(image);
        ImGui::Image(textureId, size);
    }
}

void Editor::RenderPreview::RenderSimplePathTracing()
{
    auto const& imageInfo = mScene->GetImageInfo();
    mLastBufferDumper = std::move(mBufferDumper);
    mBufferDumper = std::make_unique<BufferDumper>((uint32_t)imageInfo.width, (uint32_t)imageInfo.height);

    mPathTracing = std::make_unique<RayTracing::PathTracing>(*(Common::IDumper*)mBufferDumper.get(), *mScene, 10, 50);
    std::thread th([&]()
    {
        mPathTracing->Render();
        mIsRenderingActive = false;
    });
    th.detach();
}
