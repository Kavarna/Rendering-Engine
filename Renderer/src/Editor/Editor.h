#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

#include "VulkanHelpers/CommandList.h"

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
        void InitBasicPipeline();

        CreateInfo::EditorRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

        std::unique_ptr<CommandList> mCommandList;
        std::unique_ptr<Pipeline> mBasicPipeline;
    };

}