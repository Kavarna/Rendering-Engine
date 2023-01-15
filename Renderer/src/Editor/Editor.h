#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

#include "VulkanHelpers/CommandList.h"
#include "Scene/SceneFactory.h"
#include "ImguiWindow.h"

#include "SceneViewer.h"

namespace Editor
{
    class Editor : public Jnrlib::ISingletone<Editor>
    {
        MAKE_SINGLETONE_CAPABLE(Editor);
        constexpr const static uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    public:
        void Run();

    private:
        Editor(bool enableValidationLayers, std::vector<SceneFactory::ParsedScene> const& scenes = {});
        ~Editor();

    public:
        void OnResize(uint32_t width, uint32_t height);

    private:
        void InitWindow();
        void InitCommandLists();
        void InitImguiWindows(SceneFactory::ParsedScene const* scene);

        CreateInfo::EditorRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void ShowDockingSpace();

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

        uint32_t mWidth, mHeight;

        struct PerFrameResources
        {
            std::unique_ptr<CPUSynchronizationObject> commandListIsDone = nullptr;
            std::unique_ptr<CommandList> commandList = nullptr;
        };
        std::array<PerFrameResources, MAX_FRAMES_IN_FLIGHT> mPerFrameResources;
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<CommandList> mInitializationCmdList;

        std::vector<std::unique_ptr<ImguiWindow>> mImguiWindows;

        SceneViewer* mSceneViewer;

        bool mShouldClose = false;
        bool mMinimized = false;
    };

}