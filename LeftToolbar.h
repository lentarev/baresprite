#pragma once

#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class Palette;

class LeftToolbar : public ChildWindow
{
  public:
    LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent);

    ~LeftToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;

    // Toolbar Components
    std::unique_ptr<Palette> _palette;

    // Configuration
    static constexpr int WIDTH = 180;
};

} // namespace baresprite