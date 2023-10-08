#include "SceneViewer.h"
#include "SceneHierarchy.h"
#include "PixelInspector.h"
#include "Editor.h"

#include "imgui.h"

#include "Vulkan/CommandList.h"
#include "Vulkan/Buffer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Scene/Components/Base.h"
#include "Scene/Components/Camera.h"
#include "EditorCamera.h"
#include "CameraUtils.h"

using namespace Vulkan;

Editor::SceneViewer::SceneViewer(Common::Scene* scene, Vulkan::CommandList* cmdList)
    : mScene(scene)
{
    mRenderSystem = std::make_unique<Common::Systems::RealtimeRender>(scene, cmdList);
    mRenderSystem->SetDrawCameraFrustum(true);
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
    if (!ImGui::Begin("Scene viewer", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        return;
    }

    auto frameHeight = ImGui::GetFrameHeight();
    float width, height;
    width = ImGui::GetWindowWidth() - 2; /* One pixel on the left, one pixel on the right */
    height = ImGui::GetWindowHeight() - 2 * frameHeight;

    if (width != mWidth || height != mHeight && height != 0)
    {
        OnResize(width, height);
    }
   
    if (mActiveRenderingContext.cmdList)
    {
        UpdatePassive();
        if (ImGui::IsWindowFocused())
        {
            UpdateActive();
        }

        RenderScene();

        auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
        ImVec2 size;
        size.x = (float)currentFrameResources.renderTarget->GetExtent2D().width;
        size.y = (float)currentFrameResources.renderTarget->GetExtent2D().height;
        ImGui::Image(currentFrameResources.renderTarget->GetTextureID(), size);
    }
    else
    {
        LOG(WARNING) << "Scene viewer doesn't have a proper rendering context set";
    }

    ImGui::End();
}

void Editor::SceneViewer::SelectEntities(std::unordered_set<Common::Entity*> const& selectedIndices)
{
    mRenderSystem->SelectEntities(selectedIndices);
}

void Editor::SceneViewer::ClearSelection()
{
    mRenderSystem->ClearSelection();
}

void Editor::SceneViewer::SetSceneHierarchy(SceneHierarchy* hierarchy)
{
    mSceneHierarchy = hierarchy;
}

void Editor::SceneViewer::SetPixelInspector(PixelInspector* pixelInspector)
{
    mPixelInspector = pixelInspector;
}

void Editor::SceneViewer::RenderScene()
{
    if (mActiveRenderingContext.cmdList == nullptr)
        return;

    auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
    auto& cmdList = mActiveRenderingContext.cmdList;
    
    if (mPixelInspector)
    {
        mPixelInspector->RenderRays(*mRenderSystem);
    }

    mRenderSystem->SetRenderTarget(currentFrameResources.renderTarget.get());
    mRenderSystem->RenderScene(cmdList);
    cmdList->TransitionImageToImguiLayout(currentFrameResources.renderTarget.get());

    mCamera->PerformUpdate();
    /* TODO: Give user the ability not to perform update on the scene */
    mScene->PerformUpdate();
}

void Editor::SceneViewer::AddDebugVertex(glm::vec3 const& pos, glm::vec4 const& color, float time)
{
    mRenderSystem->AddVertex(pos, color, time);
}

void Editor::SceneViewer::SelectObject()
{
    static bool leftMouseButtonPressed = false;
    if (Editor::Get()->IsMousePressed(GLFW_MOUSE_BUTTON_LEFT) && mIsMouseEnabled)
    {
        if (!leftMouseButtonPressed)
        {
            leftMouseButtonPressed = true;
            auto& currentFrameResources = mPerFrameResources[mActiveRenderingContext.activeFrame];
            auto mousePosition = ImGui::GetMousePos();
            auto cursorPosition = ImGui::GetCursorScreenPos();
            ImVec2 pos = {mousePosition.x - cursorPosition.x, mousePosition.y - cursorPosition.y};
            if (pos.x > currentFrameResources.renderTarget->GetExtent2D().width ||
                pos.y > currentFrameResources.renderTarget->GetExtent2D().height ||
                pos.x <= 0 || pos.y <= 0)
            {
                return;
            }


            auto ray = Common::CameraUtils::GetRayForPixel(mCamera.get(), (uint32_t)pos.x, (uint32_t)pos.y);
            auto hp = mScene->GetClosestHit(ray);
            if (hp.has_value())
            {
                mSceneHierarchy->SelectEntity(hp->GetEntity());
            }
            else
            {
                mSceneHierarchy->ClearSelection();
            }
        }
    }
    else
    {
        leftMouseButtonPressed = false;
    }
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

    float speed = 3.0f;
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || Editor::Get()->IsKeyPressed(GLFW_KEY_RIGHT_SHIFT))
    {
        speed *= 3.0f;
    }

    if (Editor::Get()->IsKeyPressed(GLFW_KEY_W) || Editor::Get()->IsKeyPressed(GLFW_KEY_UP))
    {
        mCamera->MoveForward(dt * speed);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_S) || Editor::Get()->IsKeyPressed(GLFW_KEY_DOWN))
    {
        mCamera->MoveBackward(dt * speed);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_A) || Editor::Get()->IsKeyPressed(GLFW_KEY_LEFT))
    {
        mCamera->StrafeLeft(dt * speed);
    }
    if (Editor::Get()->IsKeyPressed(GLFW_KEY_D) || Editor::Get()->IsKeyPressed(GLFW_KEY_RIGHT))
    {
        mCamera->StrafeRight(dt * speed);
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
    mCamera->CalculateUpperLeftCorner();
}

void Editor::SceneViewer::UpdatePassive()
{
    auto dt = ImGui::GetIO().DeltaTime;
    mRenderSystem->Update(dt);

    /* Fix for when right click doesn't work because the mouse clicked on other window */
    if (!ImGui::IsWindowFocused() && Editor::Get()->IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && !mIsMouseEnabled)
    {
        mIsMouseEnabled = !mIsMouseEnabled;
        Editor::Get()->SetMouseInputMode(mIsMouseEnabled);
    }
}

void Editor::SceneViewer::UpdateActive()
{
    auto dt = ImGui::GetIO().DeltaTime;
    UpdateCamera(dt);
    SelectObject();
}

void Editor::SceneViewer::OnResize(float newWidth, float newHeight)
{   
    Renderer::Get()->WaitIdle();

    mWidth = newWidth;
    mHeight = newHeight;
    VLOG(2) << "Scene viewer resized to (" << newWidth << "x" << newHeight << ")";

    InitRenderTargets();
    InitCamera();

    mRenderSystem->SetDepthImage(mDepthImage.get());
    mRenderSystem->SetRenderTarget(mPerFrameResources[mActiveRenderingContext.activeFrame].renderTarget.get());
    mRenderSystem->OnResize((uint32_t)mWidth, (uint32_t)mHeight);
}

void Editor::SceneViewer::InitRenderTargets()
{
    Image::Info2D info;
    {
        info.width = (uint32_t)mWidth;
        info.height = (uint32_t)mHeight;
        info.format = VK_FORMAT_B8G8R8A8_UNORM;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Image::IMGUI_IMAGE_USAGE;
    }
    for (auto& frameResources : mPerFrameResources)
    {
        frameResources.renderTarget.reset(new Image(info));
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
            cameraInfo.focalDistance = currentCamera.GetFocalDistance();
            cameraInfo.roll = currentCamera.GetRoll();
            cameraInfo.yaw = currentCamera.GetYaw();
            cameraInfo.pitch = currentCamera.GetPitch();
            cameraInfo.position = currentCamera.GetPosition();
            cameraInfo.RecalculateViewport((uint32_t)mWidth, (uint32_t)mHeight);
        }
        mCamera.reset(new Common::EditorCamera(cameraInfo));
    }
    else
    {
        auto cameraEntity = mScene->GetCameraEntity();
        auto& baseComponent = cameraEntity->GetComponent<Common::Components::Base>();
        auto& cameraComponent = cameraEntity->GetComponent<Common::Components::Camera>();
        CreateInfo::Camera cameraInfo;
        {
            cameraInfo.fieldOfView = glm::pi<float>() / 4.0f;
            cameraInfo.focalDistance = cameraComponent.focalDistance;
            cameraInfo.roll = cameraComponent.roll;
            cameraInfo.yaw = cameraComponent.yaw;
            cameraInfo.pitch = cameraComponent.pitch;
            cameraInfo.position = baseComponent.position;
            cameraInfo.RecalculateViewport((uint32_t)mWidth, (uint32_t)mHeight);
        }
        mCamera.reset(new Common::EditorCamera(cameraInfo));
    }

    mRenderSystem->SetCamera(mCamera.get());
}


