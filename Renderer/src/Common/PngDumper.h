#pragma once

#include <png++/image.hpp>
#include "Jnrlib.h"

#include "IDumper.h"

namespace Common
{

    class PngDumper : public IDumper
    {
    public:
        PngDumper(uint32_t width, uint32_t height, std::string const& name);
        ~PngDumper();

    public:
        void SetPixelColor(float u, float v,
                           float r, float g, float b, float a = 1.0f) override;

        void SetPixelColor(uint32_t x, uint32_t y,
                           float r, float g, float b, float a = 1.0f) override;

        void SetPixelColor(float u, float v,
                           Jnrlib::Color const&) override;

        void SetPixelColor(uint32_t x, uint32_t y,
                           Jnrlib::Color const&) override;

        void SetTotalWork(uint32_t totalWork) override;
        void AddDoneWork() override;

        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        void Flush();

    private:
        png::image<png::rgba_pixel> mImage;
        std::string mName;
        uint32_t mTotalWork;
        std::atomic<uint32_t> mDoneWork = 0;
    };

}