#pragma once

#include <cstdint>
#include <vector>

namespace baresprite
{

struct MoveDragState
{
    bool isDragging = false;
    int startLogX = 0, startLogY = 0;
    int prevDx = 0, prevDy = 0;
    int currDx = 0, currDy = 0;
    std::vector<uint32_t> pixelBuffer;
    int w = 0, h = 0;
    int origSelX = 0, origSelY = 0;
};

} // namespace baresprite