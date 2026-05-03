#pragma once

#include <vector>

namespace baresprite
{

struct ClipboardData
{

    std::vector<uint32_t> pixels;
    int width = 0;
    int height = 0;
    bool hasData = false;

    void Clear()
    {
        pixels.clear();
        pixels.shrink_to_fit(); // Freeing up allocated RAM
        width = height = 0;
        hasData = false;
    }
};

} // namespace baresprite