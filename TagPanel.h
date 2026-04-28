#pragma once
#include "AppState.h"
#include <Windows.h>


namespace baresprite
{

class TagPanel
{
  public:
    TagPanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState);
    ~TagPanel();

    void OnSize(int clientW, int clientH);

  private:
    HWND _hWndBottomTolbar;
    HINSTANCE _hInstance;
    HWND _hTagPanel = nullptr;

    AppState &_appState;
};

} // namespace baresprite