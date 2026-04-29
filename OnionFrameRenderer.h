#pragma once

#include <Windows.h>
#include "AppState.h"

namespace baresprite
{

class OnionFrameRenderer
{
  public:
    OnionFrameRenderer();
    ~OnionFrameRenderer();

    void Render(int frameIndex, float opacity, const Frame &frame, int checkerSize, HDC hdcMem);
};

} // namespace baresprite