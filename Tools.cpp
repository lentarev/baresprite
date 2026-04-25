#include "Tools.h"
#include "ToolType.h"
#include <iostream>

namespace baresprite
{

Tools::Tools(HWND hWndToolbar, HINSTANCE hInstance, AppState &appState) : _hWndToolbar(hWndToolbar), _hInstance(hInstance), _appState(appState)
{
    CreateButtonTools();
    SetActive(0);
}

Tools::~Tools()
{
    for (HWND btn : _toolButtons)
    {
        if (btn && IsWindow(btn))
        {
            DestroyWindow(btn);
        }
    }

    _toolButtons.clear();
}

/// <summary>
/// Button count
/// </summary>
/// <returns></returns>
int Tools::ButtonCount() const
{
    return static_cast<int>(ToolType::Count);
}

/// <summary>
/// Set visually active button
/// </summary>
/// <param name="index"></param>
void Tools::SetActive(const int index)
{

    // Updating the logical state
    _appState.currentTool = static_cast<ToolType>(index);

    for (int i = 0; i < static_cast<int>(_toolButtons.size()); ++i)
    {
        HWND hBtn = _toolButtons[i];

        if (!hBtn)
            continue;

        // Getting current styles
        LONG_PTR style = GetWindowLongPtr(hBtn, GWL_STYLE);

        // Reset selected
        style &= ~BS_DEFPUSHBUTTON;

        // Selected button
        if (i == index)
        {
            style |= BS_DEFPUSHBUTTON;
        }

        // Apply the style and force the button to redraw
        // wParam = new styles, lParam = TRUE (redraw immediately)
        SendMessage(hBtn, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);
    }
}

void Tools::CreateButtonTools()
{

    int idCounter = 3031;

    for (int i = 0; i < static_cast<int>(ToolType::Count); ++i)
    {
        int col = i % _COLS;
        int row = i / _COLS;

        int x = _startX + col * (_BTN_SIZE_W + _SPACING);
        int y = _startY + row * (_BTN_SIZE_H + _SPACING);

        // tool buttons
        HWND hBtn = CreateWindowExW(0, L"BUTTON", GetToolLabel(static_cast<ToolType>(i)), WS_CHILD | WS_VISIBLE | BS_FLAT | BS_NOTIFY | BS_DEFPUSHBUTTON, x, y,
                                    _BTN_SIZE_W, _BTN_SIZE_H, _hWndToolbar, (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

        _toolButtons.push_back(hBtn);
    }
}

} // namespace baresprite