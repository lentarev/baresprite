#pragma once
#include "ChildWindow.h"
#include <Windows.h>

namespace baresprite
{

class Canvas : public ChildWindow
{
  public:
    Canvas(HWND hWndParent, HINSTANCE hInstanceParent);

    ~Canvas() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hCanvas = nullptr;

    // Canvas state
    float _zoom = 1.0f;
    int _canvasWidth = 512;
    int _canvasHeight = 512;
    int _offsetX = 0;
    int _offsetY = 0;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    static constexpr int MIN_CANVAS_SIZE = 512;

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite