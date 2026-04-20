#pragma once

#include "ChildWindow.h"
#include <Windows.h>
#include <vector>

namespace baresprite
{

class LeftToolbar : public ChildWindow
{
  public:
    LeftToolbar(const HWND &hWndParent, const HINSTANCE &hInstanceParent);

    ~LeftToolbar() override;

    void OnSize(int width, int height) override;

    bool OnCommand(int commandId) override;

  private:
    const HWND &_hWndParent;
    const HINSTANCE &_hInstanceParent;
    HWND _hToolbar = nullptr;

    // Configuration
    static constexpr int WIDTH = 180;
};

} // namespace baresprite