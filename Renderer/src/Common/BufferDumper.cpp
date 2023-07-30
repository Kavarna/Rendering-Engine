#include "BufferDumper.h"

#include "Vulkan/Buffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/CommandList.h"

using namespace Common;
using namespace Vulkan;

BufferDumper::BufferDumper(uint32_t width, uint32_t height) :
    mWidth(width),
    mHeight(height)
{
    mBuffer = std::make_unique<Vulkan::Buffer>(
        sizeof(Jnrlib::Color), width * height, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
        );
    Vulkan::Image::Info2D imageInfo{};
    {
        imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        imageInfo.width = width;
        imageInfo.height = height;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | Image::IMGUI_IMAGE_USAGE;
    }
    mImage = std::make_unique<Vulkan::Image>(imageInfo);
}

BufferDumper::~BufferDumper()
{
}

void BufferDumper::SetPixelColor(float u, float v, float r, float g, float b, float a)
{
    uint32_t x = (uint32_t)(u * GetWidth());
    uint32_t y = (uint32_t)(v * GetHeight());

    SetPixelColor(x, y, Jnrlib::Color(r, g, b, a));
}

void BufferDumper::SetPixelColor(uint32_t x, uint32_t y, float r, float g, float b, float a)
{
    SetPixelColor(x, y, Jnrlib::Color(r, g, b, a));
}

void BufferDumper::SetPixelColor(float u, float v, Jnrlib::Color const& c)
{
    uint32_t x = (uint32_t)(u * GetWidth());
    uint32_t y = (uint32_t)(v * GetHeight());

    SetPixelColor(x, y, c);
}

void BufferDumper::SetPixelColor(uint32_t x, uint32_t y, Jnrlib::Color const& col)
{
    CHECK(x < mWidth && y < mHeight);
    Jnrlib::Color c = glm::clamp(col, Jnrlib::Zero, Jnrlib::One);

#define USE_GAMMA
#if defined USE_GAMMA
    /* Apply gamma corrections */
    c = sqrt(c);
#endif

    Jnrlib::Color* color = (Jnrlib::Color*)mBuffer->GetElement(y * mHeight + x);
    memcpy(color, &col, sizeof(Jnrlib::Color));

    mNeedsFlush = true;
}

Jnrlib::Color Common::BufferDumper::GetPixelColor(uint32_t x, uint32_t y) const
{
    CHECK(x < mWidth && y < mHeight);
    Jnrlib::Color* color = (Jnrlib::Color*)mBuffer->GetElement(y * mHeight + x);
    return *color;
}

void BufferDumper::SetTotalWork(uint32_t totalWork)
{
    mTotalWork = totalWork;
}

void BufferDumper::AddDoneWork()
{
    mDoneWork++;
}

uint32_t BufferDumper::GetWidth() const
{
    return mWidth;
}

uint32_t BufferDumper::GetHeight() const
{
    return mHeight;
}

uint32_t BufferDumper::GetTotalWork() const
{
    return mTotalWork;
}

uint32_t BufferDumper::GetDoneWork() const
{
    return mDoneWork;
}

bool BufferDumper::NeedsFlush() const
{
    return mNeedsFlush.load();
}

void BufferDumper::Flush(Vulkan::CommandList* cmdList, bool forceFlush)
{
    if (!NeedsFlush() && !forceFlush)
    {
        return;
    }

    CommandList::TransitionInfo ti{};
    {
        ti.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        ti.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        ti.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        ti.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    mImage->EnsureAspect(VK_IMAGE_ASPECT_COLOR_BIT);
    cmdList->TransitionImageTo(mImage.get(), ti);
    cmdList->CopyWholeBufferToImage(mImage.get(), mBuffer.get());
    mNeedsFlush = false;
}

Vulkan::Image* BufferDumper::GetImage() const
{
    return mImage.get();
}

Vulkan::Buffer const* const BufferDumper::GetBuffer() const
{
    return mBuffer.get();
}

Vulkan::Buffer* BufferDumper::GetBuffer()
{
    return mBuffer.get();
}
