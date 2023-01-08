#pragma once


#include <Jnrlib.h>

#include "CreateInfoUtils.h"

#include "VulkanHelpers/CommandList.h"
#include "VulkanHelpers/Buffer.h"
#include "VulkanHelpers/RootSignature.h"
#include "VulkanHelpers/Image.h"

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

        struct PerFrameResources
        {
            std::unique_ptr<CPUSynchronizationObject> commandListIsDone;
            std::unique_ptr<CommandList> commandList;

            std::unique_ptr<Image> renderTarget;
        };
        std::array<PerFrameResources, MAX_FRAMES_IN_FLIGHT> mPerFrameResources;
        uint32_t mCurrentFrame = 0;

        std::unique_ptr<CommandList> mInitializationCmdList;

        std::unique_ptr<Buffer<Vertex>> mVertexBuffer;

        struct UniformBuffer
        {
            glm::mat4 world;
        };
        std::unique_ptr<Buffer<UniformBuffer>> mUniformBuffer;
        std::unique_ptr<DescriptorSet> mDescriptorSet;
        std::unique_ptr<RootSignature> mRootSignature;
        std::unique_ptr<Pipeline> mBasicPipeline;
        bool mShouldClose = false;
    };

}