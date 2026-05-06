#pragma once

#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class FramePanel;
class TagPanel;

class BottomToolbar : public ChildWindow
{
  public:
    BottomToolbar(HWND hWnd, HINSTANCE hInstance, AppState &appState);

    ~BottomToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

    bool OnHScroll(int scrollCode, HWND hSlider);

    HWND GetHWndBottomToolbar() const;

    FramePanel *GetFramePanel() const;

  private:
    HWND _hWnd;
    HINSTANCE _hInstance;
    HWND _hToolbar = nullptr;

    AppState &_appState;

    // Configuration
    static constexpr int HEIGHT = 80;

    // Toolbar Components
    std::unique_ptr<FramePanel> _framePanel;
    std::unique_ptr<TagPanel> _tagPanel;

    // WndProc for BottomToolbar
    static LRESULT CALLBACK _BottomToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite