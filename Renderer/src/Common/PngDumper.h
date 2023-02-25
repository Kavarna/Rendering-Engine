#pragma once

/* TODO: Try to remove this define */
#undef max
#include <png++/image.hpp>
#include "Jnrlib.h"

namespace Common
{

    class PngDumper
    {
    public:
        PngDumper(uint32_t width, uint32_t height, std::string const& name);
        ~PngDumper();

    public:
        void SetPixelColor(float u, float v,
                           float r, float g, float b, float a = 1.0f);

        void SetPixelColor(uint32_t x, uint32_t y,
                           float r, float g, float b, float a = 1.0f);

        void SetPixelColor(float u, float v,
                           Jnrlib::Color const&);

        void SetPixelColor(uint32_t x, uint32_t y,
                           Jnrlib::Color const&);

        void SetTotalWork(uint32_t totalWork);
        void AddDoneWork();

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        void Flush();

    private:
        png::image<png::rgba_pixel> mImage;
        std::string mName;
        uint32_t mTotalWork;
        std::atomic<uint32_t> mDoneWork = 0;
    };

}