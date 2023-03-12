#include "SceneViewer.h"

#include "imgui.h"

#include "Vulkan/CommandList.h"
#include "Vulkan/Buffer.h"

#include "Editor.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Vulkan;

Editor::SceneViewer::SceneViewer(Common::Scene* scene, Vulkan::CommandList* cmdList)
    : mScene(scene)
{
    for (uint32_t i = 0; i < Common::Constants::FRAMES_IN_FLIGHT; ++i)
    {
        mPerFrameResources[i].renderSystem = std::make_unique<Common::Systems::RealtimeRender>(scene, cmdList);
        mPerFrameResources[i].renderSystem->SetCamera(&mScene->GetCamera());
    }
}

Editor::SceneViewer::~SceneViewer()
{
    mDepthImage.reset();
    mDefaultPipeline.reset();
}

void Editor::SceneViewer::SetRenderingContext(RenderingContext const& ctx)
{
    CHECK(ctx.activeFrame < Common::Constants::FRAMES_IN_FLIGHT)
        << "Active frame (" << ctx.activeFrame << ") must be less than max frames(" << Common::Constants::FRAMES_IN_FLIGHT<< ")";
    mActiveRenderingContext = ctx;
}

void Editor::SceneViewer::OnRender()
{    
    ImGui::Begin("Scene viewer");

    auto frameHeight = ImGui::GetFrameHeight();
    float width, height;
    width = ImGui::GetWindowWidth() - frameHeight; /* One pixel on the left, one pixel on the right */
    height = ImGui::GetWindowHeight() - 2 * frameHeight;

    if (width != mWidth || height != mHeight && height != 0)
    {
        OnResize(width, height);
    }
   
    if (mActiveRenderingContext.cmdList)
    {
        if (ImGui::IsWindowFocused())
        {
            Update();
        }

        RenderScene();

        auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
        auto textureId = currentFrameResources.renderTarget->GetTextureID();
        ImVec2 size;
        size.x = (float)currentFrameResources.renderTarget->GetExtent2D().width;
        size.y = (float)currentFrameResources.renderTarget->GetExtent2D().height;
        ImGui::Image(textureId, size);
    }
    else
    {
        LOG(WARNING) << "Scene viewer doesn't have a proper rendering context set";
    }

    ImGui::End();
}

void Editor::SceneViewer::SelectIndices(std::unordered_set<uint32_t> const& selectedIndices)
{
    for (auto& perFrameResources : mPerFrameResources)
    {
        perFrameResources.renderSystem->SelectIndices(selectedIndices);
    }
}

void Editor::SceneViewer::ClearSelection()
{
    for (auto& perFrameResources : mPerFrameResources)
    {
        perFrameResources.renderSystem->ClearSelection();
    }
}

void Editor::SceneViewer::RenderScene()
{
    if (mActiveRenderingContext.cmdList == nullptr)
        return;

    auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
    auto& cmdList = mActiveRenderingContext.cmdList;
    auto cmdBufIndex = mActiveRenderingContext.cmdBufIndex;

    
    currentFrameResources.renderSystem->RenderScene(cmdList, cmdBufIndex);
    cmdList->TransitionImageToImguiLayout(currentFrameResources.renderTarget.get(), cmdBufIndex);

    mScene->PerformUpdate();
    mCamera->PerformUpdate();
}

void Editor::SceneViewer::UpdateCamera(float dt)
{
    static bool rightMouseButtonPressed = false;
    if (Editor::Get()->IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (!rightMouseButtonPressed)
        {
            mIsMouseEnabled = !mIsMouseEnabled;
            Editor::Get()->SetMouseInputMode(mIsMouseEnabled);
            mLastMousePosition = Editor::Get()->GetMousePosition();

            rightMouseButtonPressed = true;
        }
    }
    else
    {
        rightMouseButtonPressed = false;
    }

    if (Editor::Get()->IsKeyPressed(GLFW_KEY_W) || Editor::Get()->IsKeyPressed(GLFW_KEY_UP))
    {
        mCamera->MoveForward(dt * 3.0f);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_S) || Editor::Get()->IsKeyPressed(GLFW_KEY_DOWN))
    {
        mCamera->MoveBackward(dt * 3.0f);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_A) || Editor::Get()->IsKeyPressed(GLFW_KEY_LEFT))
    {
        mCamera->StrafeLeft(dt * 3.0f);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_D) || Editor::Get()->IsKeyPressed(GLFW_KEY_RIGHT))
    {
        mCamera->StrafeRight(dt * 3.0f);
    }
    if (!mIsMouseEnabled)
    {
        auto mousePosition = Editor::Get()->GetMousePosition();
        auto mouseMovement = mousePosition - mLastMousePosition;
        if (mouseMovement != glm::vec2{0.0f, 0.0f})
        {
            mouseMovement /= Editor::Get()->GetWindowDimensions();
            mCamera->Pitch(mouseMovement.y);
            if (!Editor::Get()->IsKeyPressed(GLFW_KEY_LEFT_ALT))
            {
                mCamera->Yaw(mouseMovement.x);
            }
            else
            {
                mCamera->Roll(mouseMovement.x);
            }
            mLastMousePosition = mousePosition;
        }
    }
    mCamera->CalculateViewMatrix();
}

void Editor::SceneViewer::Update()
{
    auto dt = ImGui::GetIO().DeltaTime;
    UpdateCamera(dt);
}

void Editor::SceneViewer::OnResize(float newWidth, float newHeight)
{   
    Renderer::Get()->WaitIdle();

    mWidth = newWidth;
    mHeight = newHeight;
    VLOG(2) << "Scene viewer resized to (" << newWidth << "x" << newHeight << ")";

    InitRenderTargets();
    InitCamera();
    for (auto& perFrameResource : mPerFrameResources)
    {
        perFrameResource.renderSystem->OnResize(
            perFrameResource.renderTarget.get(), mDepthImage.get(), (uint32_t)mWidth, (uint32_t)mHeight);
    }
}

void Editor::SceneViewer::InitRenderTargets()
{
    Image::Info2D info;
    {
        info.width = (uint32_t)mWidth;
        info.height = (uint32_t)mHeight;
        info.format = VK_FORMAT_B8G8R8A8_UNORM;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Image::IMGUI_IMAGE_LAYOUT;
    }
    for (auto& frameResources : mPerFrameResources)
    {
        frameResources.renderTarget.reset();
        frameResources.renderTarget = std::make_unique<Image>(info);
    }

    Image::Info2D depthInfo;
    {
        depthInfo.width = (uint32_t)mWidth;
        depthInfo.height = (uint32_t)mHeight;
        depthInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        depthInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    mDepthImage.reset(new Image(depthInfo));
}

void Editor::SceneViewer::InitCamera()
{
    if (mCamera)
    {
        auto currentCamera = *mCamera;
        CreateInfo::Camera cameraInfo;
        {
            cameraInfo.fieldOfView = glm::pi<float>() / 4.0f;
            cameraInfo.focalLength = currentCamera.GetFocalDistance();
            cameraInfo.roll = currentCamera.GetRoll();
            cameraInfo.yaw = currentCamera.GetYaw();
            cameraInfo.pitch = currentCamera.GetPitch();
            cameraInfo.position = currentCamera.GetPosition();
            cameraInfo.RecalculateViewport((uint32_t)mWidth, (uint32_t)mHeight);
        }
        mCamera.reset(new Common::Camera(cameraInfo));
    }
    else
    {
        auto sceneCamera = mScene->GetCamera();
        CreateInfo::Camera cameraInfo;
        {
            cameraInfo.fieldOfView = glm::pi<float>() / 4.0f;
            cameraInfo.focalLength = sceneCamera.GetFocalDistance();
            cameraInfo.roll = sceneCamera.GetRoll();
            cameraInfo.yaw = sceneCamera.GetYaw();
            cameraInfo.pitch = sceneCamera.GetPitch();
            cameraInfo.position = sceneCamera.GetPosition();
            cameraInfo.RecalculateViewport((uint32_t)mWidth, (uint32_t)mHeight);
        }
        mCamera.reset(new Common::Camera(cameraInfo));
    }

    for (auto& perFrameResource : mPerFrameResources)
    {
        perFrameResource.renderSystem->SetCamera(mCamera.get());
    }
}


