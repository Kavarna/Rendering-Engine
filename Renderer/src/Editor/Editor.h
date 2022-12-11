#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

#include "VulkanHelpers/CommandList.h"

namespace Editor
{
    class Editor : public Jnrlib::ISingletone<Editor>
    {
        MAKE_SINGLETONE_CAPABLE(Editor);
        constexpr const static uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    public:
        void Run();

    private:
        Editor(bool enableValidationLayers);
        ~Editor();

    private:
        void InitWindow();
        void InitBasicPipeline();
        void InitCommandLists();

        CreateInfo::EditorRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

        std::vector<VkFramebuffer> swapChainFramebuffers;

        std::unique_ptr<CPUSynchronizationObject> mCommandListIsDone[2];
        std::unique_ptr<CommandList> mCommandLists[2];
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<Pipeline> mBasicPipeline;
    };

}