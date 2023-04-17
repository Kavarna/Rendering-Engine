#pragma once

#include <Jnrlib.h>

namespace Common
{
    class IDumper
    {
    public:
        virtual void SetPixelColor(float u, float v,
                           float r, float g, float b, float a = 1.0f) = 0;

        virtual void SetPixelColor(uint32_t x, uint32_t y,
                           float r, float g, float b, float a = 1.0f) = 0;

        virtual void SetPixelColor(float u, float v,
                           Jnrlib::Color const&) = 0;

        virtual void SetPixelColor(uint32_t x, uint32_t y,
                           Jnrlib::Color const&) = 0;

        virtual void SetTotalWork(uint32_t totalWork) = 0;
        virtual void AddDoneWork() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
    };
}
