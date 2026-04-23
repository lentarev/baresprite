#pragma once

#include <Windows.h>
#include <vector>

namespace baresprite
{
struct PaletteData
{
    std::vector<COLORREF> colors = {RGB(0, 0, 0),     RGB(127, 127, 127), RGB(255, 0, 0),     RGB(255, 127, 0),   RGB(255, 255, 0),
                                    RGB(0, 255, 0),   RGB(0, 255, 255),   RGB(0, 0, 255),     RGB(127, 0, 255),   RGB(255, 255, 255),
                                    RGB(64, 64, 64),  RGB(191, 191, 191), RGB(127, 0, 0),     RGB(0, 127, 0),     RGB(0, 0, 127),
                                    RGB(127, 127, 0), RGB(255, 191, 191), RGB(255, 127, 127), RGB(127, 255, 127), RGB(127, 127, 255),
                                    RGB(127, 64, 0),  RGB(255, 0, 255),   RGB(0, 127, 127),   RGB(255, 200, 150), RGB(64, 0, 0)};

    COLORREF color;
    int index;
};
} // namespace baresprite