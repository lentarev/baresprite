#pragma once

#include "ChildWindow.h"
#include <Windows.h>

namespace baresprite
{

class RightToolbar : public ChildWindow
{
  public:
    RightToolbar(HWND hWndParent, HINSTANCE hInstanceParent);

    ~RightToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId) override;

  private:
    HWND _hWndParent;
    const HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;

    // Configuration
    static constexpr int WIDTH = 180;
};

} // namespace baresprite