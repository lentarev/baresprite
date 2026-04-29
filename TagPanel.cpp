#include "TagPanel.h"

namespace baresprite
{
TagPanel::TagPanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState)
    : _hWndBottomTolbar(hWndBottomTolbar), _hInstance(hInstance), _appState(appState)
{
    CreateControls();
}

TagPanel::~TagPanel()
{
    if (_hLabelFilter && IsWindow(_hLabelFilter))
    {
        DestroyWindow(_hLabelFilter);
    }

    if (_hComboFilter && IsWindow(_hComboFilter))
    {
        DestroyWindow(_hComboFilter);
    }

    if (_hLabelTag && IsWindow(_hLabelTag))
    {
        DestroyWindow(_hLabelTag);
    }

    if (_hComboTag && IsWindow(_hComboTag))
    {
        DestroyWindow(_hComboTag);
    }

    if (_hButton && IsWindow(_hButton))
    {
        DestroyWindow(_hButton);
    }
}

void TagPanel::SetBounds(const RECT &rc)
{
    _startX = rc.left; //  Просто берём левую границу (уже вычисленную в BottomToolbar)
    ResizeControls(rc.right - rc.left, rc.bottom - rc.top);
}

void TagPanel::CreateControls()
{
    // FILTER
    _hLabelFilter = CreateWindowExW(0, L"STATIC", L"Filter:", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, _LABEL_W, _LABEL_H, _hWndBottomTolbar,
                                    nullptr, _hInstance, nullptr);

    _hComboFilter = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_HASSTRINGS | CBS_AUTOHSCROLL,
                                    0, 0, _COMBO_W, _COMBO_H, _hWndBottomTolbar, nullptr, _hInstance, nullptr);

    // TAG
    _hLabelTag = CreateWindowExW(0, L"STATIC", L"Tag:", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, _LABEL_W, _LABEL_H, _hWndBottomTolbar,
                                 nullptr, _hInstance, nullptr);

    _hComboTag = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_HASSTRINGS | CBS_AUTOHSCROLL, 0,
                                 0, _COMBO_W, _COMBO_H, _hWndBottomTolbar, nullptr, _hInstance, nullptr);

    // Button - Manage tags
    _hButton = CreateWindowExW(0, L"BUTTON", L"Manage tags", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON, 0, 0, _BTN_SIZE_W, _BTN_SIZE_H,
                                _hWndBottomTolbar, nullptr, _hInstance, nullptr);
}

void TagPanel::ResizeControls(int clientW, int clientH) const
{
    int currentX = _startX; // Начинаем от переданной позиции

    // Расставляем контролы по порядку

    // FILTERS
    SetWindowPos(_hLabelFilter, nullptr, currentX, _startY, _LABEL_W, _LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
    currentX += _LABEL_W + _SPACING;

    SetWindowPos(_hComboFilter, nullptr, currentX, _startY + 3, _COMBO_W, _COMBO_H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
    RedrawWindow(_hWndBottomTolbar, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    currentX += _LABEL_W + _SPACING + 70;

    // TAGS
    SetWindowPos(_hLabelTag, nullptr, currentX, _startY, _LABEL_W, _LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
    currentX += _LABEL_W + _SPACING;

    SetWindowPos(_hComboTag, nullptr, currentX, _startY + 3, _COMBO_W, _COMBO_H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
    RedrawWindow(_hWndBottomTolbar, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    currentX += _LABEL_W + _SPACING + 70;

    // BUTTON
    SetWindowPos(_hButton, nullptr, currentX, _startY, _BTN_SIZE_W, _BTN_SIZE_H, SWP_NOZORDER | SWP_NOACTIVATE);

}

} // namespace baresprite