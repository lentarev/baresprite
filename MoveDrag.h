#pragma once

#include "AppState.h"

namespace baresprite
{

class MoveDrag
{
  public:
    MoveDrag();
    ~MoveDrag();

    void ButtonDown(AppState &appState, HWND hCanvas, int x, int y, int checkerSize);
    void ButtonMove(AppState &appState, HWND hCanvas, int x, int y, int checkerSize);
    void ButtonUp(AppState &appState, HWND hCanvas);
};

} // namespace baresprite