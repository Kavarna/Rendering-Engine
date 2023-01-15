#include "Editor.h"
#include "Renderer.h"
#include "VulkanHelpers/Pipeline.h"
#include "VulkanHelpers/PipelineManager.h"
#include "FileHelpers.h"

#include "imgui.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 800;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 600;


/* Redirects for callbacks */
void OnResizeCallback(GLFWwindow* window, int width, int height)
{
    Editor::Editor::Get()->OnResize(width, height);
}

Editor::Editor::Editor(bool enableValidationLayers, std::vector<SceneFactory::ParsedScene> const& scenes)
{
    CHECK(scenes.size() <= 1) << "Unable to edit multiple scenes at once";
    try
    {
        InitWindow();
        Renderer::Get(CreateRendererInfo(enableValidationLayers));
        InitCommandLists();
        InitImguiWindows(scenes.size() == 1 ? &scenes[0] : nullptr);
        OnResize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

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
    mWidth = width;
    mHeight = height;

    Renderer::Get()->WaitIdle();
    Renderer::Get()->OnResize();
}

void Editor::Editor::InitWindow()
{
    CHECK(glfwInit() == GLFW_TRUE) << "Unable to initialize GLFW";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    mWindow = glfwCreateWindow(
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        "JNReditor", nullptr, nullptr
    );
    CHECK(mWindow != nullptr) << "Unable to create window";

    glfwSetWindowSizeCallback(mWindow, OnResizeCallback);

    LOG(INFO) << "Successfully created window";
}

VkShaderModule createShaderModule(const std::vector<char>& code)
{
    auto device = Editor::Renderer::Get()->GetDevice();
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (jnrCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

CreateInfo::EditorRenderer Editor::Editor::CreateRendererInfo(bool enableValidationLayers)
{
    CreateInfo::EditorRenderer info = {};
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
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Show debug window", 0, &showDebugWindow);
            ImGui::MenuItem("Show demo window", 0, &showDemoWindow);
            
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

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

void Editor::Editor::InitImguiWindows(SceneFactory::ParsedScene const* scene)
{
    auto sceneViewer = std::make_unique<SceneViewer>(MAX_FRAMES_IN_FLIGHT, scene);
    mSceneViewer = sceneViewer.get();
    mImguiWindows.emplace_back(std::move(sceneViewer));
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
        cmdList->BeginRenderingUI();
        {
            ShowDockingSpace();
        }
        cmdList->BeginRenderingOnBackbuffer(Jnrlib::Black);
        cmdList->EndRenderingUI();
        cmdList->EndRendering();
    }
    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
