#pragma once


#include <Jnrlib.h>
#include <GLFW/glfw3.h>

namespace Editor
{
    class Editor : public Jnrlib::ISingletone<Editor>
    {
        MAKE_SINGLETONE_CAPABLE(Editor);

    public:
        void Run();


    private:
        Editor();
        ~Editor();

    private:
        void InitWindow();

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

    };

}