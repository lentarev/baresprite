#pragma once

#include <memory>
#include <string>

namespace baresprite
{
struct Frame
{
    int width = 64;
    int height = 64;
    std::unique_ptr<uint32_t[]> pixels;
    int duration = 100;
    bool isVisible = true;
    std::wstring tag;

    Frame(int w = 64, int h = 64) : width(w), height(h)
    {
        pixels = std::make_unique<uint32_t[]>(width * height);
        Clear();
    }

    // Очистка кадра
    void Clear(uint32_t color = 0x00000000)
    {
        if (pixels)
        {
            std::fill(pixels.get(), pixels.get() + width * height, color);
        }
    }

    void SetPixel(int x, int y, uint32_t color)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            pixels[y * width + x] = color;
        }
    }

    uint32_t GetPixel(int x, int y) const
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            return pixels[y * width + x];
        }
        return 0x00000000;
    }
};

} // namespace baresprite
