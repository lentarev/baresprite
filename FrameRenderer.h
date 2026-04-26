#pragma once

#include <Windows.h>
#include "Frame.h"

namespace baresprite
{

class FrameRenderer
{
  public:
    FrameRenderer();
    ~FrameRenderer();

    void Render(const Frame &frame, int _checkerSize, HDC hdcMem);
};

} // namespace baresprite