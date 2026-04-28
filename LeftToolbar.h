#pragma once

#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class Palette;
class Tools;

class LeftToolbar : public ChildWindow
{
  public:
    LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent, AppState &appState);

    ~LeftToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

    void SelectTool(int index);

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;
    AppState &_appState;

    // Toolbar Components
    std::unique_ptr<Palette> _palette;
    std::unique_ptr<Tools> _tools;

    // Configuration
    static constexpr int WIDTH = 180;

    // WndProc for LeftToolbar
    static LRESULT CALLBACK _LeftToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite