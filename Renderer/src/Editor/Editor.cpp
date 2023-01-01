#include "Editor.h"
#include "Renderer.h"
#include "VulkanHelpers/Pipeline.h"
#include "VulkanHelpers/PipelineManager.h"
#include "FileHelpers.h"

#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 1920;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 1080;


/* Redirects for callbacks */
void OnResizeCallback(GLFWwindow* window, int width, int height)
{
    Editor::Editor::Get()->OnResize(width, height);
}

Editor::Editor::Editor(bool enableValidationLayers)
{
    try
    {
        InitWindow();
        Renderer::Get(CreateRendererInfo(enableValidationLayers));
        InitCommandLists();
        InitVertexBuffer();
        OnResize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

        mCommandLists[0]->InitImGui();
        mCommandLists[0]->End();
        mCommandLists[0]->SubmitAndWait();
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error occured when initializing editor " << e.what();
    }
}

Editor::Editor::~Editor()
{
    Renderer::Get()->WaitIdle();
    
    mUniformBuffer.reset();
    mDescriptorSet.reset();
    mRootSignature.reset();
    mVertexBuffer.reset();
    mBasicPipeline.reset();
    
    for (uint32_t i = 0; i < ARRAYSIZE(mCommandLists); ++i)
    {
        mCommandLists[i].reset();
        mCommandListIsDone[i].reset();
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

    InitBasicPipeline();
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

    ImGui::End();
}

void Editor::Editor::InitBasicPipeline()
{
    mDescriptorSet = std::make_unique<DescriptorSet>();
    mDescriptorSet->AddInputBuffer(0, 1);
    mDescriptorSet->Bake(MAX_FRAMES_IN_FLIGHT);

    mRootSignature = std::make_unique<RootSignature>();
    mRootSignature->AddDescriptorSet(mDescriptorSet.get());
    mRootSignature->Bake();

    mBasicPipeline = std::make_unique<Pipeline>("BasicPipeline");
    {
        mBasicPipeline->SetRootSignature(mRootSignature.get());
        mBasicPipeline->AddShader("Shaders/basic.vert.spv");
        mBasicPipeline->AddShader("Shaders/basic.frag.spv");
    }
    VkViewport vp{};
    VkRect2D sc{};
    auto& viewport = mBasicPipeline->GetViewportStateCreateInfo();
    {
        vp.width = (FLOAT)mWidth; vp.minDepth = 0.0f; vp.x = 0;
        vp.height = (FLOAT)mHeight; vp.maxDepth = 1.0f; vp.y = 0;
        sc.offset = {.x = 0, .y = 0}; sc.extent = {.width = (uint32_t)mWidth, .height = (uint32_t)mHeight};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.pViewports = &vp;
        viewport.scissorCount = 1;
        viewport.pScissors = &sc;
    }

    VkVertexInputAttributeDescription attributeDescription{};
    {
        attributeDescription.binding = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.location = 0;
        attributeDescription.offset = offsetof(Vertex, position);
    }

    VkVertexInputBindingDescription bindingDescription{};
    {
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
    }

    auto& vertexInput = mBasicPipeline->GetVertexInputStateCreateInfo();
    {
        vertexInput.vertexAttributeDescriptionCount = 1;
        vertexInput.pVertexAttributeDescriptions = &attributeDescription;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDescription;

    }

    VkPipelineColorBlendAttachmentState attachmentInfo{};
    auto& blendState = mBasicPipeline->GetColorBlendStateCreateInfo();
    {
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }
    
    auto& rasterizerState = mBasicPipeline->GetRasterizationStateCreateInfo();
    {
        rasterizerState.cullMode = VK_CULL_MODE_NONE;
    }

    mBasicPipeline->AddBackbufferColorOutput();
    mBasicPipeline->SetBackbufferDepthStencilOutput();
    mBasicPipeline->Bake();

    mDescriptorSet->AddInputBuffer(mUniformBuffer.get(), 0, 0, 0);
    mDescriptorSet->AddInputBuffer(mUniformBuffer.get(), 0, 0, 1);
}

void Editor::Editor::InitCommandLists()
{
    for (uint32_t i = 0; i < ARRAYSIZE(mCommandLists); ++i)
    {
        mCommandLists[i] = std::make_unique<CommandList>(CommandListType::Graphics);
        mCommandLists[i]->Init();

        mCommandListIsDone[i] = std::make_unique<CPUSynchronizationObject>(true);
    }

    /* The first command list will be used during initialization */
    mCommandLists[0]->Begin();
}

void Editor::Editor::InitVertexBuffer()
{
    Vertex vertices[] = {
        {glm::vec3(0.5, -0.5, 0.0)},
        {glm::vec3(0.5, 0.5, 0.0)},
        {glm::vec3(-0.5, 0.5, 0.0)},
        {glm::vec3(0.5, -0.5, 0.0)},
        {glm::vec3(-0.5, 0.5, 0.0)},
        {glm::vec3(-0.5, -0.5, 0.0)},
    };
    std::unique_ptr<Buffer<Vertex>> localVertexBuffer =
        std::make_unique<Buffer<Vertex>>(sizeof(vertices) / sizeof(vertices[0]),
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    localVertexBuffer->Copy(vertices);

    mVertexBuffer = std::make_unique<Buffer<Vertex>>(sizeof(vertices) / sizeof(vertices[0]),
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    mCommandLists[0]->CopyBuffer(mVertexBuffer.get(), localVertexBuffer.get());

    mUniformBuffer = std::make_unique<Buffer<UniformBuffer>>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                             VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    {
        UniformBuffer* ub = mUniformBuffer->GetElement();
        ub->world = glm::rotate(glm::identity<glm::mat4>(), glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
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
    auto data = mUniformBuffer->GetElement();
    data->world = glm::transpose(data->world);
    data->world = glm::rotate(data->world, glm::half_pi<float>() * 1000, glm::vec3(0.0f, 0.0f, 1.0f));
    data->world = glm::transpose(data->world);


    auto& cmdList = mCommandLists[mCurrentFrame];
    auto& isCmdListDone = mCommandListIsDone[mCurrentFrame];
    isCmdListDone->Wait();
    isCmdListDone->Reset();

    cmdList->Begin();
    {
        cmdList->BeginRenderingOnBackbuffer(Jnrlib::Black);
        {
            cmdList->BeginRenderingUI();
            {
                ShowDockingSpace();
            }
            cmdList->EndRenderingUI();
            /*cmdList->BindPipeline(mBasicPipeline.get());
            cmdList->BindDescriptorSet(mDescriptorSet.get(), mCurrentFrame, mRootSignature.get());
            cmdList->BindVertexBuffer(mVertexBuffer.get());
            cmdList->Draw(6);*/
        }
        cmdList->EndRendering();
    }
    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    Renderer::Get()->WaitIdle();
}
