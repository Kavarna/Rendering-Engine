#include "Editor.h"
#include "Renderer.h"
#include "VulkanHelpers/Pipeline.h"
#include "VulkanHelpers/PipelineManager.h"

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 1920;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 1080;

Editor::Editor::Editor(bool enableValidationLayers)
{
    try
    {
        InitWindow();
        Renderer::Get(CreateRendererInfo(enableValidationLayers));
        InitBasicPipeline();

    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << "Error occured when initializing editor " << e.what();
    }
}

Editor::Editor::~Editor()
{
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
    Pipeline basicPipeline("BasicPipeline");
    {
        basicPipeline.AddShader("Shaders/basic.vert.spv");
        basicPipeline.AddShader("Shaders/basic.frag.spv");
    }
    VkViewport vp{};
    VkRect2D sc{};
    auto& viewport = basicPipeline.GetViewportStateCreateInfo();
    {
        vp.width = width; vp.minDepth = 0.0f; vp.x = 0;
        vp.height = height; vp.maxDepth = 1.0f; vp.y = 0;
        sc.offset = {.x = 0, .y = 0}; sc.extent = {.width = (uint32_t)width, .height = (uint32_t)height};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.pViewports = &vp;
        viewport.scissorCount = 1;
        viewport.pScissors = &sc;
    }
    VkPipelineColorBlendAttachmentState attachmentInfo{};
    auto& blendState = basicPipeline.GetColorBlendStateCreateInfo();
    {
        attachmentInfo.blendEnable = VK_FALSE;
        attachmentInfo.colorWriteMask = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendState.attachmentCount = 1;
        blendState.pAttachments = &attachmentInfo;
    }
    basicPipeline.Bake();
}

void Editor::Editor::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        Frame();
        glfwPollEvents();
    }
}

void Editor::Editor::Frame()
{

}
