#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

namespace Editor
{
    class Editor : public Jnrlib::ISingletone<Editor>
    {
        MAKE_SINGLETONE_CAPABLE(Editor);

    public:
        void Run();


    private:
        Editor(bool enableValidationLayers);
        ~Editor();

    private:
        void InitWindow();

        CreateInfo::EditorRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

    };

}