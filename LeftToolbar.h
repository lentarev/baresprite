#pragma once

#include "ChildWindow.h"
#include <Windows.h>
#include <memory>
#include "Project.h"

namespace baresprite
{

// Forward declaration
class Palette;

class LeftToolbar : public ChildWindow
{
  public:
    LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent, Project &projectData);

    ~LeftToolbar() override;

    void OnSize(int clientW, int clientH) override;

    bool OnCommand(int commandId, int notifyCode) override;

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;
    HWND _hToolbar = nullptr;
    Project &_projectData;

    //HBRUSH _hToolbarBrush = nullptr;

    // Toolbar Components
    std::unique_ptr<Palette> _palette;

    // Configuration
    static constexpr int WIDTH = 180;

    // WndProc for LeftToolbar
    static LRESULT CALLBACK _LeftToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite