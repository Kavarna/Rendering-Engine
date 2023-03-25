#pragma once

#include "Jnrlib.h"
#include "Vertex.h"

namespace Vulkan
{
    class Buffer;
    class CommandList;
}

namespace Common
{
    class BatchRenderer
    {
    public:
        BatchRenderer(uint32_t initialSize = 1024);
        ~BatchRenderer();

    public:
        void Vertex(float x, float y, float z, float r, float g, float b, float a = 1.0f);
        void Vertex(glm::vec3 const& position, glm::vec4 const& color);
        void PersistentVertex(float x, float y, float z, float r, float g, float b, float a = 1.0f, float time = FLT_MAX);
        void PersistentVertex(glm::vec3 const& position, glm::vec4 const& color, float time = FLT_MAX);

        void Update(float dt);

        void Begin();
        void End(Vulkan::CommandList* cmdList);

    private:
        void Resize(uint32_t size);

    private:
        std::vector<VertexPositionColor> mOverflowVertices;

        std::vector<VertexPositionColor> mPersistentVertices;
        std::vector<float> mPersistentVerticesTimes;
        
        uint32_t mCurrentVertex = 0;
        std::unique_ptr<Vulkan::Buffer> mVertexBuffer;

        uint32_t mCurrentSize = 0;
    };
}

