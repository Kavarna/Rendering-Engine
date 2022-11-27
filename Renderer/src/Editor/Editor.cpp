#include "Editor.h"
#include "Renderer.h"

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 1920;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 1080;

Editor::Editor::Editor(bool enableValidationLayers)
{
    try
    {
        InitWindow();

        Renderer::Get(CreateRendererInfo(enableValidationLayers));
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
