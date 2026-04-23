#pragma once

#include <Windows.h>
#include "AppState.h"

namespace baresprite
{

class ChessBackground
{
  public:
    ChessBackground(AppState &appState);

    ~ChessBackground();

    void Render(const PAINTSTRUCT &ps, HDC hdc) const;

  private:
    HBRUSH _hBrushLight;
    HBRUSH _hBrushDark;

    // Link to the project data structure.
    AppState &_appState;
};

} // namespace baresprite