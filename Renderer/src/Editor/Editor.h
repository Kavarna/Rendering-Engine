#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

#include "VulkanHelpers/CommandList.h"
#include "VulkanHelpers/Buffer.h"

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

    public:
        void OnResize(uint32_t width, uint32_t height);

    private:
        void InitWindow();
        void InitBasicPipeline();
        void InitCommandLists();
        void InitVertexBuffer();

        CreateInfo::EditorRenderer CreateRendererInfo(bool enableValidationLayers);

    private:
        void ShowDockingSpace();

    private:
        void Frame();

    private:
        GLFWwindow* mWindow;

        uint32_t mWidth, mHeight;

        struct Vertex
        {
            glm::vec3 position;
        };

        std::unique_ptr<Buffer<Vertex>> mVertexBuffer;
        std::unique_ptr<CPUSynchronizationObject> mCommandListIsDone[2];
        std::unique_ptr<CommandList> mCommandLists[2];
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<Pipeline> mBasicPipeline;
        bool mShouldClose = false;
    };

}