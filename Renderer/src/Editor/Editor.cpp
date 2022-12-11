#include "Editor.h"
#include "Renderer.h"
#include "VulkanHelpers/Pipeline.h"
#include "VulkanHelpers/PipelineManager.h"
#include "FileHelpers.h"

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 1920;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 1080;

Editor::Editor::Editor(bool enableValidationLayers)
{
    try
    {
        InitWindow();
        Renderer::Get(CreateRendererInfo(enableValidationLayers));
        InitBasicPipeline();
        InitCommandLists();
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error occured when initializing editor " << e.what();
    }
}

Editor::Editor::~Editor()
{
    Renderer::Get()->WaitIdle();
    
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

void Editor::Editor::InitWindow()
{
    CHECK(glfwInit() == GLFW_TRUE) << "Unable to initialize GLFW";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    mWindow = glfwCreateWindow(
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        "JNReditor", nullptr, nullptr
    );
    CHECK(mWindow != nullptr) << "Unable to create window";

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

void Editor::Editor::InitBasicPipeline()
{
    int32_t width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    mBasicPipeline = std::make_unique<Pipeline>("BasicPipeline");
    {
        mBasicPipeline->AddShader("Shaders/basic.vert.spv");
        mBasicPipeline->AddShader("Shaders/basic.frag.spv");
    }
    VkViewport vp{};
    VkRect2D sc{};
    auto& viewport = mBasicPipeline->GetViewportStateCreateInfo();
    {
        vp.width = (FLOAT)width; vp.minDepth = 0.0f; vp.x = 0;
        vp.height = (FLOAT)height; vp.maxDepth = 1.0f; vp.y = 0;
        sc.offset = {.x = 0, .y = 0}; sc.extent = {.width = (uint32_t)width, .height = (uint32_t)height};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.pViewports = &vp;
        viewport.scissorCount = 1;
        viewport.pScissors = &sc;
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
    mBasicPipeline->AddBackbufferColorOutput();
    mBasicPipeline->SetBackbufferDepthStencilOutput();
    mBasicPipeline->Bake();
}

void Editor::Editor::InitCommandLists()
{
    for (uint32_t i = 0; i < ARRAYSIZE(mCommandLists); ++i)
    {
        mCommandLists[i] = Renderer::Get()->GetCommandList(CommandListType::Graphics);
        mCommandLists[i]->Init();

        mCommandListIsDone[i] = std::make_unique<CPUSynchronizationObject>(true);
    }
}

void Editor::Editor::Run()
{
    try
    {
        while (!glfwWindowShouldClose(mWindow))
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
    auto& cmdList = mCommandLists[mCurrentFrame];
    auto& isCmdListDone = mCommandListIsDone[mCurrentFrame];
    isCmdListDone->Wait();
    isCmdListDone->Reset();

    cmdList->Begin();
    {
        cmdList->BeginRenderingOnBackbuffer(Jnrlib::Black);
        {
            cmdList->BindPipeline(mBasicPipeline.get());
            cmdList->Draw(3);
        }
        cmdList->EndRendering();
    }
    cmdList->End();

    cmdList->SubmitToScreen(isCmdListDone.get());

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
