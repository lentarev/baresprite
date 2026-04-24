#include "Tools.h"

namespace baresprite
{

Tools::Tools(HWND hWndToolbar, HINSTANCE hInstance, AppState &appState) : _hWndToolbar(hWndToolbar), _hInstance(hInstance), _appState(appState)
{
    CreateButtonTools();
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

void Tools::CreateButtonTools()
{

    int idCounter = 3031;

    for (int i = 0; i < _labelButtons.size(); ++i)
    {
        int col = i % _COLS;
        int row = i / _COLS;

        int x = _startX + col * (_BTN_SIZE_W + _SPACING);
        int y = _startY + row * (_BTN_SIZE_H + _SPACING);

        // tool buttons
        HWND hBtn = CreateWindowExW(0, L"BUTTON", _labelButtons[i].c_str(), WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT | BS_NOTIFY, x, y, _BTN_SIZE_W,
                                    _BTN_SIZE_H, _hWndToolbar, (HMENU)(INT_PTR)idCounter, _hInstance, nullptr);

        _toolButtons.push_back(hBtn);
    }
}

} // namespace baresprite