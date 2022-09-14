#pragma once

#undef max
#include <png++/image.hpp>
#include "Jnrlib.h"


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

    void SetProgress(float x);

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

    void Flush();

private:
    png::image<png::rgba_pixel> mImage;
    std::string mName;

};


