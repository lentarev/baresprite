#pragma once

#include "AppState.h"
#include <Windows.h>
#include <string>

namespace baresprite
{

class Tools
{
  public:
    Tools(HWND hWndToolbar, HINSTANCE hInstance, AppState &appState);
    ~Tools();

    int ButtonCount() const;
    void SetActive(const int index);

  private:
    HWND _hWndToolbar;
    HINSTANCE _hInstance;
    AppState &_appState;

    // Configuration
    int _startX = 17;
    int _startY = 190;

    const int _BTN_SIZE_W = 68;
    const int _BTN_SIZE_H = 30;

    const int _SPACING = 10;
    const int _COLS = 2;

    // Tool buttons
    std::vector<HWND> _toolButtons;

    // METHODS
    void CreateButtonTools();
};

} // namespace baresprite