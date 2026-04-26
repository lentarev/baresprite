#pragma once
#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>

namespace baresprite
{

class CanvasScrollView : public ChildWindow
{
  public:
    CanvasScrollView(HWND hWndMain, HINSTANCE hInstance, AppState &appState);

    ~CanvasScrollView() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

  private:
    HWND _hWndMain;
    HINSTANCE _hInstance;
    AppState &_appState;

    HWND _hCanvasScrollView = nullptr;

    int _containerW = 512;
    int _containerH = 512;

    int _offsetX = 0;
    int _offsetY = 0;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasScrollViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite