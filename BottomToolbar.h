#pragma once

#include "ChildWindow.h"
#include <Windows.h>

namespace baresprite
{

class BottomToolbar : public ChildWindow
{
  public:
    BottomToolbar(HWND hWndParent, HINSTANCE hInstanceParent);

    ~BottomToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;

    // Configuration
    static constexpr int HEIGHT = 80;

    // WndProc for BottomToolbar
    static LRESULT CALLBACK _BottomToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite