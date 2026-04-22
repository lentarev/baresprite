#pragma once

#include <Windows.h>
#include <vector>

namespace baresprite
{
struct PaletteData
{
    std::vector<COLORREF> colors;
    COLORREF color;
    int index;
};
} // namespace baresprite