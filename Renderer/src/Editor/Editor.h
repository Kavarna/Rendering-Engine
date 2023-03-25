#pragma once


#include <Jnrlib.h>

#include "CreateInfo/VulkanRendererCreateInfo.h"
#include "Vulkan/CommandList.h"
#include "Scene/SceneParser.h"
#include "ImguiWindow.h"
#include "Constants.h"
#include "JnrSerializer.h"

namespace Editor
{
    class SceneViewer;
    class SceneHierarchy;
    class ObjectInspector;
    class RenderPreview;
    class Editor : public Jnrlib::ISingletone<Editor>
    {
        MAKE_SINGLETONE_CAPABLE(Editor);
    public:
        void Run();

    private:
        Editor(bool enableValidationLayers, std::vector<Common::SceneParser::ParsedScene> const& scenes = {});
        ~Editor();

    public:
        void OnResize(uint32_t width, uint32_t height);
        void OnMaximize(int maximized);
        bool IsKeyPressed(int keyCode);
        bool IsMousePressed(int keyCode);

        void SetMouseInputMode(bool enable);

        glm::vec2 GetMousePosition();
        glm::vec2 GetWindowDimensions();

    private:
        void DeserializeStructures();
        void InitWindow();
        void InitCommandLists();
        void InitScene(Common::SceneParser::ParsedScene const* scene = nullptr);
        void InitImguiWindows();

        CreateInfo::VulkanRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void SerializeWindows();

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
        std::array<PerFrameResources, Common::Constants::FRAMES_IN_FLIGHT> mPerFrameResources;
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<Vulkan::CommandList> mInitializationCmdList;

        std::unique_ptr<Common::Scene> mActiveScene;

        std::vector<std::unique_ptr<ImguiWindow>> mImguiWindows;

        std::unique_ptr<Jnrlib::JnrDeserializer> mWindowsDeserializer;

        SceneViewer* mSceneViewer;
        SceneHierarchy* mSceneHierarchy;
        ObjectInspector* mObjectInspector;
        RenderPreview* mRenderPreview;
 
        bool mShouldClose = false;
        bool mMinimized = false;
        bool mMaximized = false;
    };

}