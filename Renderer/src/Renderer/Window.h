#pragma once


#include <Jnrlib.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Window : public Jnrlib::ISingletone<Window>
{
    MAKE_SINGLETONE_CAPABLE(Window);
private:
    Window();
    ~Window();

public:
    void Run();

private:
    void InitWindow();

private:
    GLFWwindow *mWindow;

};


