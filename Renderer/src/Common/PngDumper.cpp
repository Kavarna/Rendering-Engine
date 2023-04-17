#include "PngDumper.h"

using namespace Common;

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
    uint32_t x = (uint32_t)(u * GetWidth());
    uint32_t y = (uint32_t)(v * GetHeight());

    SetPixelColor(x, y, Jnrlib::Color(r, g, b, a));
}

void PngDumper::SetPixelColor(uint32_t x, uint32_t y, float r, float g, float b, float a)
{
    SetPixelColor(x, y, Jnrlib::Color(r, g, b, a));
}

void PngDumper::SetPixelColor(float u, float v, Jnrlib::Color const& c)
{
    uint32_t x = (uint32_t)(u * GetWidth());
    uint32_t y = (uint32_t)(v * GetHeight());

    SetPixelColor(x, y, c);
}

void PngDumper::SetPixelColor(uint32_t x, uint32_t y, Jnrlib::Color const& col)
{
    Jnrlib::Color c = glm::clamp(col, Jnrlib::Zero, Jnrlib::One);
    
#define USE_GAMMA
#if defined USE_GAMMA
    /* Apply gamma corrections */
    c = sqrt(c);
#endif
    
    png::byte red = png::byte(c.r * 255.f);
    png::byte green = png::byte(c.g * 255.f);
    png::byte blue = png::byte(c.b * 255.f);
    png::byte alpha = png::byte(c.a * 255.f);

    mImage[y][x] = png::rgba_pixel(red, green, blue, 255);
}

void PngDumper::SetTotalWork(uint32_t totalWork)
{
    mTotalWork = totalWork;
    mDoneWork = 0;
}

void PngDumper::AddDoneWork()
{
    mDoneWork++;
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
