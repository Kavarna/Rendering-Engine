#pragma once


#include <Jnrlib.h>

#include "CreateInfo/VulkanRendererCreateInfo.h"
#include "Vulkan/CommandList.h"
#include "Scene/SceneParser.h"
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
        Editor(bool enableValidationLayers, std::vector<Common::SceneParser::ParsedScene> const& scenes = {});
        ~Editor();

    public:
        void OnResize(uint32_t width, uint32_t height);

    private:
        void InitWindow();
        void InitCommandLists();
        void InitImguiWindows(Common::SceneParser::ParsedScene const* scene);

        CreateInfo::VulkanRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void ShowDockingSpace();

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

        uint32_t mWidth, mHeight;

        struct PerFrameResources
        {
            std::unique_ptr<Vulkan::CPUSynchronizationObject> commandListIsDone = nullptr;
            std::unique_ptr<Vulkan::CommandList> commandList = nullptr;
        };
        std::array<PerFrameResources, MAX_FRAMES_IN_FLIGHT> mPerFrameResources;
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<Vulkan::CommandList> mInitializationCmdList;

        std::vector<std::unique_ptr<ImguiWindow>> mImguiWindows;

        SceneViewer* mSceneViewer;

        bool mShouldClose = false;
        bool mMinimized = false;
    };

}