#include "PngDumper.h"

PngDumper::PngDumper(uint32_t width, uint32_t height, std::string const& name):
    mImage(width, height),
    mName(name)
{ }

PngDumper::~PngDumper()
{
    Flush();
}

void PngDumper::SetPixelColor(float u, float v, float r, float g, float b, float a)
{
    png::byte red = png::byte(r * 255.f);
    png::byte green = png::byte(g * 255.f);
    png::byte blue = png::byte(b * 255.f);
    png::byte alpha = png::byte(a * 255.f);

    uint32_t x = u * GetWidth();
    uint32_t y = v * GetHeight();

    mImage[x][y] = png::rgba_pixel(red, green, blue, alpha);
}

void PngDumper::SetPixelColor(uint32_t x, uint32_t y, float r, float g, float b, float a)
{
    png::byte red = png::byte(r * 255.f);
    png::byte green = png::byte(g * 255.f);
    png::byte blue = png::byte(b * 255.f);
    png::byte alpha = png::byte(a * 255.f);

    mImage[x][y] = png::rgba_pixel(red, green, blue, alpha);
}

void PngDumper::SetPixelColor(float u, float v, Jnrlib::Color const& c)
{
    png::byte red = png::byte(c.r * 255.f);
    png::byte green = png::byte(c.g * 255.f);
    png::byte blue = png::byte(c.b * 255.f);
    png::byte alpha = png::byte(c.a * 255.f);

    uint32_t x = u * GetWidth();
    uint32_t y = v * GetHeight();

    mImage[x][y] = png::rgba_pixel(red, green, blue, alpha);
}

void PngDumper::SetPixelColor(uint32_t x, uint32_t y, Jnrlib::Color const& c)
{
    png::byte red = png::byte(c.r * 255.f);
    png::byte green = png::byte(c.g * 255.f);
    png::byte blue = png::byte(c.b * 255.f);
    png::byte alpha = png::byte(c.a * 255.f);

    mImage[x][y] = png::rgba_pixel(red, green, blue, alpha);
}

void PngDumper::SetProgress(float progress)
{
    int barLength = 50;
    int pos = progress * barLength;

    std::cout << "Progress: [";
    for (int i = 0; i != barLength; ++i)
    {
        if (i < pos)
            std::cout << "#";
        else
            std::cout << " ";
    }
    std::cout << "]\r";
}

uint32_t PngDumper::GetWidth() const
{
    return mImage.get_width();
}

uint32_t PngDumper::GetHeight() const
{
    return mImage.get_height();
}

void PngDumper::Flush()
{
    mImage.write(mName.c_str());
}
