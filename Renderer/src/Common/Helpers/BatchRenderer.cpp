#include "BatchRenderer.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandList.h"

using namespace Vulkan;
using namespace Common;
using namespace Jnrlib;

BatchRenderer::BatchRenderer(uint32_t initialSize)
{
    Resize(initialSize);
}

BatchRenderer::~BatchRenderer()
{
}

void BatchRenderer::Vertex(float x, float y, float z, float r, float g, float b, float a)
{
    Vertex(glm::vec3(x, y, z), glm::vec4(r, g, b, a));
}

void BatchRenderer::Vertex(glm::vec3 const &position, glm::vec4 const &color)
{
    if (mCurrentVertex >= mCurrentSize)
    {
        mOverflowVertices.emplace_back(position, color);
        return;
    }

    auto vertex = (VertexPositionColor *)mVertexBuffer->GetElement(mCurrentVertex);
    vertex->position = position;
    vertex->color = color;

    mCurrentVertex++;
}

void BatchRenderer::PersistentVertex(float x, float y, float z, float r, float g, float b, float a, float time)
{
    PersistentVertex(glm::vec3(x, y, z), glm::vec4(r, g, b, a), time);
}

void BatchRenderer::PersistentVertex(glm::vec3 const &position, glm::vec4 const &color, float time)
{
    Vertex(position, color);

    mPersistentVertices.emplace_back(position, color);
    mPersistentVerticesTimes.emplace_back(time);
}

void BatchRenderer::Update(float dt)
{
    CHECK(mPersistentVertices.size() == mPersistentVerticesTimes.size())
        << "Persistent vertices and persistent vertices times should be the same";

    for (uint32_t i = 0; i < mPersistentVerticesTimes.size(); ++i)
    {
        mPersistentVerticesTimes[i] -= dt;
        if (mPersistentVerticesTimes[i] < Zero)
        {
            mPersistentVertices.erase(mPersistentVertices.begin() + i);
            mPersistentVerticesTimes.erase(mPersistentVerticesTimes.begin() + i);
            i--;
        }
    }
}

void BatchRenderer::Begin()
{
    CHECK(mPersistentVertices.size() == mPersistentVerticesTimes.size())
        << "Persistent vertices and persistent vertices times should be the same";

    if (mOverflowVertices.size() > 0)
    {
        Resize(mCurrentSize + (uint32_t)mOverflowVertices.size());
        memcpy((VertexPositionColor *)mVertexBuffer->GetData() + mCurrentVertex, mOverflowVertices.data(),
               mOverflowVertices.size() * sizeof(decltype(mOverflowVertices)::value_type));
        mCurrentSize = (uint32_t)mVertexBuffer->GetCount();
    }

    // Copy over the persistent vertices
    memcpy(mVertexBuffer->GetData(), mPersistentVertices.data(),
           mPersistentVertices.size() * sizeof(decltype(mPersistentVertices)::value_type));
    mCurrentVertex = (uint32_t)mPersistentVertices.size();
}

void BatchRenderer::End(CommandList *cmdList)
{
    CHECK(mPersistentVertices.size() == mPersistentVerticesTimes.size())
        << "Persistent vertices and persistent vertices times should be the same";

    if (mCurrentVertex > 0)
    {
        cmdList->BindVertexBuffer(mVertexBuffer.get(), 0);
        cmdList->Draw(mCurrentVertex, 0);
    }
}

void BatchRenderer::Resize(uint32_t size)
{
    CHECK(size > mCurrentSize) << "Cannot resize batch renderer with smaller size";
    mCurrentSize = size;

    auto newBuffer = new Buffer(sizeof(VertexPositionColor), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    if (mVertexBuffer != nullptr)
    {
        memcpy(newBuffer->GetData(), mVertexBuffer->GetData(), mVertexBuffer->GetSize());
    }

    mVertexBuffer.reset(newBuffer);
}
