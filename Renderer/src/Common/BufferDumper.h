#pragma once

#include "Jnrlib.h"
#include "IDumper.h"

namespace Vulkan
{
    class Buffer;
    class Image;
    class CommandList;
}

namespace Common
{

    class BufferDumper : public IDumper
    {
    public:
        BufferDumper(uint32_t width, uint32_t height);
        ~BufferDumper();

    public:
        void SetPixelColor(float u, float v,
                           float r, float g, float b, float a = 1.0f);

        void SetPixelColor(uint32_t x, uint32_t y,
                           float r, float g, float b, float a = 1.0f);

        void SetPixelColor(float u, float v,
                           Jnrlib::Color const&);

        void SetPixelColor(uint32_t x, uint32_t y,
                           Jnrlib::Color const&);

        Jnrlib::Color GetPixelColor(uint32_t x, uint32_t y) const;

        void SetTotalWork(uint32_t totalWork);
        void AddDoneWork();

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        uint32_t GetTotalWork() const;
        uint32_t GetDoneWork() const;

        bool NeedsFlush() const;
        void Flush(Vulkan::CommandList* cmdList, bool forceFlush = false);

        Vulkan::Image* GetImage() const;

        Vulkan::Buffer const* const GetBuffer() const;
        Vulkan::Buffer* GetBuffer();

    private:
        std::unique_ptr<Vulkan::Buffer> mBuffer;
        std::unique_ptr<Vulkan::Image> mImage;

        uint32_t mWidth, mHeight;

        uint32_t mTotalWork;
        std::atomic<uint32_t> mDoneWork = 0;

        std::atomic<bool> mNeedsFlush = false;
    };

}