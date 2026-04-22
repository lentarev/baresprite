#pragma once
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>
#include "Project.h"

namespace baresprite
{

// Forward declaration
class ChessBackground;


class Canvas : public ChildWindow
{
  public:
    Canvas(HWND hWndParent, HINSTANCE hInstanceParent, Project &projectData);

    ~Canvas() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;

    // Link to the project data structure.
    Project &_projectData;

    HWND _hCanvas = nullptr;

    // Canvas state
    float _zoom = 1.0f;
    int _canvasWidth = 512;
    int _canvasHeight = 512;
    int _offsetX = 0;
    int _offsetY = 0;

    // Canvas Components
    std::unique_ptr<ChessBackground> _chessBackground;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    static constexpr int MIN_CANVAS_SIZE = 512;

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite