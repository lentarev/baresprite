#pragma once
#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class Canvas;

class CanvasScrollView : public ChildWindow
{
  public:
    CanvasScrollView(HWND hWndMain, HINSTANCE hInstance, AppState &appState);

    ~CanvasScrollView() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

    void RecalculateCanvasCentering();

    Canvas *GetCanvas();

  private:
    HWND _hWndMain;
    HINSTANCE _hInstance;
    AppState &_appState;

    HWND _hCanvasScrollView = nullptr;

    int _containerW = 0;
    int _containerH = 0;

    int _offsetX = 0;
    int _offsetY = 0;

    int _canvasPosX = 0;
    int _canvasPosY = 0;

    int _scrollX = 0;
    int _scrollY = 0;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    // Components
    std::unique_ptr<Canvas> _canvas;

    void UpdateScrollInfo();

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasScrollViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite