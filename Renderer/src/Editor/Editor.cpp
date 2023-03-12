#include "Editor.h"
#include "Vulkan/Renderer.h"
#include "FileHelpers.h"

#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Vulkan;

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 800;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 600;


/* Redirects for callbacks */
void OnResizeCallback(GLFWwindow* window, int width, int height)
{
    Editor::Editor::Get()->OnResize(width, height);
}

Editor::Editor::Editor(bool enableValidationLayers, std::vector<Common::SceneParser::ParsedScene> const& scenes) : 
    mWidth(DEFAULT_WINDOW_WIDTH), mHeight(DEFAULT_WINDOW_HEIGHT)
{
    CHECK(scenes.size() <= 1) << "Unable to edit multiple scenes at once";
    try
    {
        DeserializeStructures();
        InitWindow();
        Renderer::Get(CreateRendererInfo(enableValidationLayers));
        InitCommandLists();
        InitScene(&scenes[0]);
        InitImguiWindows();
        OnResize(mWidth, mHeight);

        mInitializationCmdList->InitImGui();
        mInitializationCmdList->End();
        mInitializationCmdList->SubmitAndWait();
        mInitializationCmdList.reset();
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error occured when initializing editor " << e.what();
    }
}

Editor::Editor::~Editor()
{
    Renderer::Get()->WaitIdle();

    SerializeWindows();

    mActiveScene.reset();
    
    mImguiWindows.clear();

    for (auto& perResourceFrames : mPerFrameResources)
    {
        perResourceFrames.commandList.reset();
        perResourceFrames.commandListIsDone.reset();
    }

    Renderer::Destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();

    LOG(INFO) << "Successfully destroyed window";
}

void Editor::Editor::OnResize(uint32_t width, uint32_t height)
{
    if (width < 5 || height < 5)
    {
        LOG(INFO) << "The window is too small for a resize. Skipping resize callback";
        mMinimized = true;
        return;
    }
    mMinimized = false;

    mWidth = width;
    mHeight = height;

    Renderer::Get()->WaitIdle();
    Renderer::Get()->OnResize();
}

bool Editor::Editor::IsKeyPressed(int keyCode)
{
    return glfwGetKey(mWindow, keyCode) == GLFW_PRESS;
}

bool Editor::Editor::IsMousePressed(int keyCode)
{
    return glfwGetMouseButton(mWindow, keyCode) == GLFW_PRESS;
}

void Editor::Editor::SetMouseInputMode(bool enable)
{
    if (enable)
    {
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

glm::vec2 Editor::Editor::GetMousePosition()
{
    double xpos = 0.0, ypos = 0.0;
    glfwGetCursorPos(mWindow, &xpos, &ypos);
    return {xpos, ypos};
}

glm::vec2 Editor::Editor::GetWindowDimensions()
{
    return glm::vec2{mWidth, mHeight};
}

void Editor::Editor::DeserializeStructures()
{
    mWindowsDeserializer = std::make_unique<Jnrlib::JnrDeserializer>("windows.jnr");
    mWindowsDeserializer->Read();

    for (uint32_t i = 0; i < mWindowsDeserializer->GetNumStructures(); ++i)
    {
        auto currentStructure = mWindowsDeserializer->GetStructure(i);
        if (currentStructure.index() == 1)
        {
            auto& windowInfo = std::get<Jnrlib::InfoMainWindowV1>(currentStructure);
            mWidth = windowInfo.width;
            mHeight = windowInfo.height;
        }
    }
}

void Editor::Editor::InitWindow()
{
    CHECK(glfwInit() == GLFW_TRUE) << "Unable to initialize GLFW";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    mWindow = glfwCreateWindow(
        mWidth, mHeight,
        "JNReditor", nullptr, nullptr
    );
    CHECK(mWindow != nullptr) << "Unable to create window";

    glfwMakeContextCurrent(mWindow);
    glfwSetWindowSizeCallback(mWindow, OnResizeCallback);

    LOG(INFO) << "Successfully created window";
}

CreateInfo::VulkanRenderer Editor::Editor::CreateRendererInfo(bool enableValidationLayers)
{
    CreateInfo::VulkanRenderer info = {};
    {
        info.window = mWindow;
        info.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        info.deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    }

    {
        uint32_t count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);

        for (uint32_t i = 0; i < count; ++i)
        {
            info.instanceExtensions.push_back(extensions[i]);
        }
    }

    if (enableValidationLayers)
    {
        LOG(INFO) << "Enabling vulkan validation layers";
        info.instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        info.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return info;
}

void Editor::Editor::SerializeWindows()
{
    Jnrlib::JnrSerializer serializer("windows.jnr");
    {
        Jnrlib::InfoWindowsV1 windowsInfo;
        windowsInfo.windows.resize(mImguiWindows.size());
        for (uint32_t i = 0; i < mImguiWindows.size(); ++i)
        {
            windowsInfo.windows[i].isOpen = mImguiWindows[i]->mIsOpen;
        }
        serializer.AddStructure(windowsInfo);
    }
    {
        Jnrlib::InfoMainWindowV1 windowInfo;
        windowInfo.width = mWidth;
        windowInfo.height = mHeight;
        serializer.AddStructure(windowInfo);
    }
    serializer.Flush();
}

void Editor::Editor::ShowDockingSpace()
{
    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id);

    static bool showDebugWindow = false, showDemoWindow = false;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
                LOG(INFO) << "New selected";
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "ALT+F4"))
            {
                mShouldClose = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (mSceneViewer && ImGui::MenuItem("Scene viewer", nullptr, &mSceneViewer->mIsOpen))
            {
                LOG(INFO) << "Scene viewer is shown";
            }

            if (mSceneHierarchy && ImGui::MenuItem("Scene hierarchy", nullptr, &mSceneHierarchy->mIsOpen))
            {
                LOG(INFO) << "Scene hierarchy is shown";
            }

            ImGui::EndMenu();
        }

#if DEBUG
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Show debug window", 0, &showDebugWindow);
            ImGui::MenuItem("Show demo window", 0, &showDemoWindow);
            
            ImGui::EndMenu();
        }
#endif

        ImGui::EndMenuBar();
    }

#if DEBUG
    if (showDebugWindow)
    {
        auto& io = ImGui::GetIO();
        ImGui::Begin("Debug");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
#endif

    for (auto& imguiWindow : mImguiWindows)
    {
        imguiWindow->OnImguiRender();
    }

    ImGui::End();
}

void Editor::Editor::InitCommandLists()
{
    for (auto& frameResources : mPerFrameResources)
    {
        frameResources.commandList = std::make_unique<CommandList>(CommandListType::Graphics);
        frameResources.commandList->Init();

        frameResources.commandListIsDone = std::make_unique<CPUSynchronizationObject>(true);
    }

    mInitializationCmdList = std::make_unique<CommandList>(CommandListType::Graphics);
    mInitializationCmdList->Init();
    mInitializationCmdList->Begin();
}

void Editor::Editor::InitScene(Common::SceneParser::ParsedScene const* parsedScene)
{
    if (parsedScene == nullptr)
    {
        mActiveScene = nullptr;
        return;
    }

    std::unique_ptr<Common::Camera> camera = std::make_unique<Common::Camera>(parsedScene->cameraInfo);
    mActiveScene = std::make_unique<Common::Scene>(parsedScene->sceneInfo);
    mActiveScene->SetCamera(std::move(camera));
    mActiveScene->InitializeGraphics(mInitializationCmdList.get(), 0);
}

void Editor::Editor::InitImguiWindows()
{
    {
        /* Scene viewer */
        auto sceneViewer = std::make_unique<SceneViewer>(mActiveScene.get(), mInitializationCmdList.get());
        mSceneViewer = sceneViewer.get();
        mImguiWindows.emplace_back(std::move(sceneViewer));
    }
    {
        /* Scene hierarchy */
        auto sceneHierarchy = std::make_unique<SceneHierarchy>(mActiveScene.get(), mSceneViewer);
        mSceneHierarchy = sceneHierarchy.get();
        mImguiWindows.emplace_back(std::move(sceneHierarchy));
    }
    
    {
        for (uint32_t j = 0; j < mWindowsDeserializer->GetNumStructures(); ++j)
        {
            auto currentStructure = mWindowsDeserializer->GetStructure(j);
            if (currentStructure.index() == 0)
            {
                auto& windowsInfo = std::get<Jnrlib::InfoWindowsV1>(currentStructure);
                for (uint32_t i = 0; i < windowsInfo.windows.size(); ++i)
                {
                    mImguiWindows[i]->mIsOpen = windowsInfo.windows[i].isOpen;
                }
            }
        }
    }
}

void Editor::Editor::Run()
{
    try
    {
        while (!glfwWindowShouldClose(mWindow) && !mShouldClose)
        {
            Frame();
            glfwPollEvents();
        }
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error occured when running: " << e.what();
    }
    Renderer::Get()->WaitIdle();
}

void Editor::Editor::Frame()
{
    if (mMinimized)
        return;

    auto& cmdList = mPerFrameResources[mCurrentFrame].commandList;
    auto& isCmdListDone = mPerFrameResources[mCurrentFrame].commandListIsDone;

    {
        /* Prepare for rendering */
        isCmdListDone->Wait();
        isCmdListDone->Reset();

        /* Prepare the scene viewer */
        {
            SceneViewer::RenderingContext ctx;
            ctx.activeFrame = mCurrentFrame;
            ctx.cmdBufIndex = 0;
            ctx.cmdList = cmdList.get();
            mSceneViewer->SetRenderingContext(ctx);
        }
    }

    cmdList->Begin();
    {
       
        cmdList->UINewFrame();
        ShowDockingSpace();

        /* Render the UI on the backbuffer */
        cmdList->BeginRenderingOnBackbuffer(Jnrlib::Black);
        cmdList->FlushUI();
        cmdList->EndRendering();
    }
    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % Common::Constants::FRAMES_IN_FLIGHT;
}
