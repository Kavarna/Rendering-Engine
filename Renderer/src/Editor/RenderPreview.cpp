#include "RenderPreview.h"
#include "PixelInspector.h"
#include "Editor.h"

#include "Scene/Scene.h"
#include "Scene/Systems/RealtimeRenderSystem.h"

#include "CreateInfo/RayTracingCreateInfo.h"
#include "RayTracing/PathTracing.h"
#include "RayTracing/SimpleRayTracing.h"

#include "BufferDumper.h"

#include "Vulkan/Image.h"
#include "Vulkan/CommandList.h"

#include <boost/algorithm/string.hpp>

#include "imgui.h"
#include "imgui_internal.h"

using namespace Common;

Editor::RenderPreview::RenderPreview(Common::Scene* scene, PixelInspector* pixelInspector) :
    mScene(scene),
    mPixelInspector(pixelInspector)
{
    using namespace CreateInfo;

    for (uint32_t type = (uint32_t)RayTracingType::BEGIN; type < (uint32_t)RayTracingType::COUNT; ++type)
    {
        mRendererTypes.push_back(GetStringFromRendererType((RayTracingType)type));
    }
}

Editor::RenderPreview::~RenderPreview()
{ }

void Editor::RenderPreview::SetRenderingContext(RenderingContext const& ctx)
{
    mActiveRenderingContext = ctx;
}

void Editor::RenderPreview::HandleSelect()
{
    if (!ImGui::IsWindowFocused())
    {
        return;
    }
    static bool leftMouseButtonPressed = false;
    if (Editor::Get()->IsMousePressed(GLFW_MOUSE_BUTTON_LEFT) && Editor::Get()->IsMouseEnabled())
    {
        if (!leftMouseButtonPressed)
        {
            leftMouseButtonPressed = true;
            auto mousePosition = ImGui::GetMousePos();
            auto cursorPosition = ImGui::GetCursorScreenPos();
            ImVec2 pos = {mousePosition.x - cursorPosition.x, mousePosition.y - cursorPosition.y};
            if (pos.x > mWidth ||
                pos.y > mHeight ||
                pos.x <= 0 || pos.y <= 0)
            {
                return;
            }

            /* Remap from windows space to image space */
            auto const& imageInfo = mScene->GetImageInfo();
            pos.x = Jnrlib::RemapValueFromIntervalToInterval(pos.x, 0.f, (float)mWidth, 0.f, (float)imageInfo.width);
            pos.y = Jnrlib::RemapValueFromIntervalToInterval(pos.y, 0.f, (float)mHeight, 0.f, (float)imageInfo.height);

            if (!mIsRenderingActive)
            {
                mPixelInspector->CopySelectedRegion((uint32_t)pos.x, (uint32_t)pos.y, mBufferDumper.get(), mRenderer.get(), mActiveRenderingContext.cmdList);
            }
        }
    }
    else
    {
        leftMouseButtonPressed = false;
    }
}

void Editor::RenderPreview::OnRender()
{
    if (!ImGui::Begin("Render Preview"))
    {
        // return;
    }

    auto frameHeight = ImGui::GetFrameHeight();
    float width, height;
    width = ImGui::GetWindowWidth() - 2; /* One pixel on the left, one pixel on the right */
    height = ImGui::GetWindowHeight() - 2 * frameHeight;

    if (width != mWidth || height != mHeight && height != 0)
    {
        OnResize(width, height);
    }

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
    static std::vector<const char*> rendererTypes;
    if (rendererTypes.size() != mRendererTypes.size())
    {
        /* Convert from array of std::strings to array of const char* */
        for (uint32_t i = 0; i < mRendererTypes.size(); ++i)
        {
            rendererTypes.push_back(mRendererTypes[i].c_str());
        }
    }
    if (ImGui::Combo("Renderer type", &mRendererType, rendererTypes.data(), (int)rendererTypes.size()))
    {
        mPixelInspector->CopySelectedRegion(0, 0, nullptr, nullptr, nullptr);
    }

    ShowProgress();

    ImGui::End();
}

void Editor::RenderPreview::OnResize(float newWidth, float newHeight)
{
    mWidth = newWidth;
    mHeight = newHeight;
    VLOG(2) << "Render preview resized to (" << newWidth << "x" << newHeight << ")";
}

void Editor::RenderPreview::StartRendering()
{
    if (mRendererType == (uint32_t)CreateInfo::RayTracingType::PathTracing)
    {
        RenderSimplePathTracing();
        mIsRenderingActive = true;
    }
    else if (mRendererType == (uint32_t)CreateInfo::RayTracingType::SimpleRayTracing)
    {
        RenderSimpleRayTracing();
        mIsRenderingActive = true;
    }
}

void Editor::RenderPreview::ShowProgress()
{
    if (mBufferDumper)
    {
        auto totalWork = (float)mBufferDumper->GetTotalWork();
        auto doneWork = (float)mBufferDumper->GetDoneWork();
        float progress = doneWork / totalWork;
        ImGui::SameLine();
        ImGui::ProgressBar(progress, ImVec2(-FLT_MIN, 0), "Rendering");

        HandleSelect();

        auto frameHeight = ImGui::GetFrameHeight();
        float width, height;
        auto currentCursorPos = ImGui::GetCursorPos();
        width = ImGui::GetWindowWidth() - 2; /* One pixel on the left, one pixel on the right */
        height = ImGui::GetWindowHeight() - currentCursorPos.y - frameHeight; /* One pixel up, one pixel down */

        mBufferDumper->Flush(mActiveRenderingContext.cmdList);

        ImVec2 size;
        size.x = width;
        size.y = height;
        auto image = mBufferDumper->GetImage();
        mActiveRenderingContext.cmdList->TransitionImageToImguiLayout(image);
        ImGui::Image(image->GetTextureID(), size);
    }
}

void Editor::RenderPreview::RenderSimplePathTracing()
{
    auto const& imageInfo = mScene->GetImageInfo();
    mLastBufferDumper = std::move(mBufferDumper);
    mBufferDumper = std::make_unique<BufferDumper>((uint32_t)imageInfo.width, (uint32_t)imageInfo.height);

    mRenderer = std::make_unique<RayTracing::PathTracing>(*(Common::IDumper*)mBufferDumper.get(), *mScene, 100, 50);
    std::thread th([&]()
    {
        mRenderer->Render();
        mIsRenderingActive = false;
    });
    th.detach();
}

void Editor::RenderPreview::RenderSimpleRayTracing()
{
    auto const& imageInfo = mScene->GetImageInfo();
    mLastBufferDumper = std::move(mBufferDumper);
    mBufferDumper = std::make_unique<BufferDumper>((uint32_t)imageInfo.width, (uint32_t)imageInfo.height);

    mRenderer = std::make_unique<RayTracing::SimpleRayTracing>(*(Common::IDumper*)mBufferDumper.get(), *mScene, 10);
    std::thread th([&]()
    {
        mRenderer->Render();
        mIsRenderingActive = false;
    });
    th.detach();
}
