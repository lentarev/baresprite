#pragma once

#include "ChildWindow.h"
#include <Windows.h>

namespace baresprite
{

class FrameToolbar : public ChildWindow
{
  public:
    FrameToolbar(HWND hWndParent, HINSTANCE hInstanceParent);

    ~FrameToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;

    // Configuration
    static constexpr int HEIGHT = 80;

    // WndProc for FrameToolbar
    static LRESULT CALLBACK _FrameToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite