#pragma once

#include <Windows.h>

namespace baresprite
{

class CursorRenderer
{
  public:
    CursorRenderer();
    ~CursorRenderer();

    void Render(int cursorSize, int cursorX, int cursorY, HDC hdcMem);
};

} // namespace baresprite