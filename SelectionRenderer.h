#pragma once

#include <Windows.h>
#include "SelectionState.h"

namespace baresprite
{

class SelectionRenderer
{
  public:
    SelectionRenderer();
    ~SelectionRenderer();

    void Render(HDC hdc, const SelectionState &sel, int checkerSize);

    void DrawDashedLine(HDC hdc, int x1, int y1, int x2, int y2, int offset);
};

} // namespace baresprite