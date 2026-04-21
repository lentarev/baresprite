#include "LeftToolbar.h"
#include "Palette.h"
#include <iostream>

namespace baresprite
{

LeftToolbar::LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{
    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, WIDTH, 0, hWndParent, nullptr, hInstanceParent, nullptr);

    // Create palette
    _palette = std::make_unique<Palette>(hWndParent, hInstanceParent);
}

LeftToolbar::~LeftToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void LeftToolbar::OnSize(int clientW, int clientH)
{
    if (_hToolbar)
    {
        // Растягиваем тулбар на всю высоту главного окна
        SetWindowPos(_hToolbar, nullptr, 0, 0, WIDTH, clientH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool LeftToolbar::OnCommand(int commandId)
{

    // Палитра: ID от 3001 до 3025
    if (commandId >= 3001 && commandId < 3001 + _palette->ColorsCount())
    {
        int index = commandId - 3001;

        if (_palette)
        {
            _palette->SelectColor(index); // Визуально выделяем
            std::cout << "Palette ID: " << commandId << std::endl;

            // Получаем цвет и сохраняем в проекте (глобально)
            // COLORREF color = _palette->GetSelectedColor();
            // if (gProjectData)
            //{
            // gProjectData->SetBrushColor(color);
            //}
        }
        return true;
    }

    return false;
}

} // namespace baresprite
