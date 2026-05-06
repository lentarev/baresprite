#pragma once

#include <Windows.h>
#include "AppState.h"

namespace baresprite
{


class MoveRenderer
{
  public:
    MoveRenderer();
    ~MoveRenderer();

    void Render(const Frame &frame, int checkerSize, AppState &appState, HDC hdcMem);
};

} // namespace baresprite