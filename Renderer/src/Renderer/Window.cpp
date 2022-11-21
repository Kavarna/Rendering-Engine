#include "Window.h"

constexpr const uint32_t DEFAULT_WINDOW_WIDTH = 1920;
constexpr const uint32_t DEFAULT_WINDOW_HEIGHT = 1080;

Window::Window()
{
    InitWindow();
}

Window::~Window()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Window::InitWindow()
{
    CHECK(glfwInit() == GLFW_TRUE) << "Unable to initialize GLFW";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    mWindow = glfwCreateWindow(
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        "JNRenderer", nullptr, nullptr
    );
    CHECK(mWindow != nullptr) << "Unable to create window";

    LOG(INFO) << "Successfully created window";
}

void Window::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
    }
}
