#include "RenderPreview.h"

#include "Common/Scene/Scene.h"
#include "Common/Scene/Systems/RealtimeRenderSystem.h"

#include "Vulkan/Image.h"
#include "Vulkan/CommandList.h"

#include <boost/algorithm/string.hpp>

#include "imgui.h"

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

    if (ImGui::Button("Render"))
    {
        StartRendering();
    }
    ImGui::SameLine();
    ImGui::Combo("Renderer type", &mRendererType, mRendererTypes.data(), (int)mRendererTypes.size());

    auto frameHeight = ImGui::GetFrameHeight();
    float width, height;
    auto currentCursorPos = ImGui::GetCursorPos();
    width = ImGui::GetWindowWidth() - 2; /* One pixel on the left, one pixel on the right */
    height = ImGui::GetWindowHeight() - currentCursorPos.y - frameHeight; /* One pixel up, one pixel down */

    ImVec2 size;
    size.x = width;
    size.y = height;

    ImGui::End();
}

void Editor::RenderPreview::StartRendering()
{
    if (boost::icontains(mRendererTypes[mRendererType], "real") &&
        boost::icontains(mRendererTypes[mRendererType], "time"))
    {

    }
}
