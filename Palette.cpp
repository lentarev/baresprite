#include "Palette.h"
#include <commctrl.h>
#include <commdlg.h> // For ChooseColor
#include <iostream>

namespace baresprite
{

Palette::Palette(HWND hWndToolbar, HINSTANCE hInstanceParent, AppState &appState)
    : _hWndToolbar(hWndToolbar), _hInstanceParent(hInstanceParent), _appState(appState)
{

    CreatePalette();
    SelectColor(_appState.palette.index);
}

Palette::~Palette()
{

    for (HWND btn : _paletteButtons)
    {
        if (btn && IsWindow(btn))
        {
            DestroyWindow(btn);
        }
    }

    _paletteButtons.clear();
}

COLORREF Palette::GetSelectedColor() const
{
    return _selectedColor;
}

/// <summary>
/// Select color
/// </summary>
/// <param name="index"></param>
void Palette::SelectColor(int index)
{

    ResetColor(_oldIndex);

    _oldIndex = index;

    _selectedColor = _appState.palette.colors[index];

    HDC hdc = GetDC(_hWndToolbar);
    HDC hdcMem = CreateCompatibleDC(hdc);

    // The button takes ownership of the bitmap. There's no need to delete it; the button will delete it automatically.
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, _BTN_SIZE, _BTN_SIZE);

    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    // We set a rectangle for filling
    RECT rc = {0, 0, _BTN_SIZE, _BTN_SIZE};

    HPEN hPen = CreatePen(PS_SOLID, _BORDER_WIDTH, _selectedBorder);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    HBRUSH hBrush = CreateSolidBrush(_appState.palette.colors[index]);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    Rectangle(hdcMem, 0, 0, _BTN_SIZE, _BTN_SIZE);

    // Restoring the context state (Pen)
    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);

    // We delete what we created ourselves
    DeleteObject(hPen);
    DeleteObject(hBrush);

    // Restore the context state (Bitmap)
    SelectObject(hdcMem, oldObj);

    // Deleting the virtual canvas from memory
    DeleteDC(hdcMem);

    // Freeing the window context obtained via GetDC
    ReleaseDC(_hWndToolbar, hdc);

    HBITMAP hOldBitmap = (HBITMAP)SendMessage(_paletteButtons[index], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

    // Deleting the old bitmap
    if (hOldBitmap && hOldBitmap != hBitmap)
    {
        DeleteObject(hOldBitmap);
    }
}

/// <summary>
/// Reset color
/// </summary>
/// <param name="index"></param>
void Palette::ResetColor(int index)
{
    HDC hdc = GetDC(_hWndToolbar);

    HDC hdcMem = CreateCompatibleDC(hdc);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, _BTN_SIZE, _BTN_SIZE);

    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    RECT rc = {0, 0, _BTN_SIZE, _BTN_SIZE};

    HPEN hPen = CreatePen(PS_SOLID, _BORDER_WIDTH, _defaultBorder);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    HBRUSH hBrush = CreateSolidBrush(_appState.palette.colors[index]);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    Rectangle(hdcMem, 0, 0, _BTN_SIZE, _BTN_SIZE);

    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);

    DeleteObject(hPen);
    DeleteObject(hBrush);

    SelectObject(hdcMem, oldObj);

    DeleteDC(hdcMem);

    ReleaseDC(_hWndToolbar, hdc);

    HBITMAP hOldBitmap = (HBITMAP)SendMessage(_paletteButtons[index], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

    if (hOldBitmap && hOldBitmap != hBitmap)
    {
        DeleteObject(hOldBitmap);
    }
}

void Palette::CreatePalette()
{

    int idCounter = 3001;

    for (int i = 0; i < _colorCount; ++i)
    {
        int col = i % _COLS;
        int row = i / _COLS;

        int x = _startX + col * (_BTN_SIZE + _SPACING);
        int y = _startY + row * (_BTN_SIZE + _SPACING);

        HBITMAP hBmp = CreateBitmap(_appState.palette.colors[i], _BTN_SIZE, _BTN_SIZE);

        HWND hBtn = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT | BS_NOTIFY, x, y, _BTN_SIZE, _BTN_SIZE, _hWndToolbar,
                                    (HMENU)(INT_PTR)idCounter++, _hInstanceParent, nullptr);

        _paletteButtons.push_back(hBtn);
        SendMessage(hBtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
    }
}

HBITMAP Palette::CreateBitmap(COLORREF color, int width, int height) const
{
    // Parent window device context
    HDC hdc = GetDC(_hWndToolbar);

    // Create a virtual canvas in memory
    HDC hdcMem = CreateCompatibleDC(hdc);

    // Create the bitmap itself (empty) of the required size
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);

    // We select our new bitmap in the device context
    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    // We set a rectangle for filling
    RECT rc = {0, 0, width, height};

    HPEN hPen = CreatePen(PS_SOLID, _BORDER_WIDTH, _defaultBorder);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    // Create a brush of the desired color
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    // Fill a rectangle in DC memory with our brush
    // FillRect(hdcMem, &rc, hBrush);

    Rectangle(hdcMem, 0, 0, _BTN_SIZE, _BTN_SIZE);

    // Restoring the context state (Pen)
    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);

    // We delete only what we have created.
    DeleteObject(hPen);
    DeleteObject(hBrush);

    // Return the old object back to hdcMem
    SelectObject(hdcMem, oldObj);

    // Deleting the virtual canvas from memory
    DeleteDC(hdcMem);

    // Freeing the window context obtained via GetDC
    ReleaseDC(_hWndToolbar, hdc);

    return hBitmap;
}

COLORREF Palette::InvertColor(COLORREF color)
{
    return RGB(255 - GetRValue(color), 255 - GetGValue(color), 255 - GetBValue(color));
}

int Palette::ColorsCount() const
{
    return _colorCount;
}

bool Palette::EditColor(int index, HWND hWndOwner)
{
    if (index < 0 || index >= static_cast<int>(_appState.palette.colors.size()))
        return false;

    // Setting up the dialogue
    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(CHOOSECOLORW);
    cc.hwndOwner = hWndOwner;
    cc.rgbResult = _appState.palette.colors[index]; // Starting color
    cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
    cc.lpfnHook = _PickerColorDialogHook;

    // Array of custom colors (optional, for selection history)
    static COLORREF custColors[16] = {};
    cc.lpCustColors = custColors;

    // Showing the dialogue
    if (ChooseColorW(&cc))
    {
        // The user has chosen a new color!
        _appState.palette.colors[index] = cc.rgbResult; // Updating the array
        return true;
    }

    return false;
}

UINT_PTR CALLBACK Palette::_PickerColorDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG)
    {

        HWND hMainWindow = GetAncestor(hDlg, GA_ROOTOWNER);

        int x = 250, y = 50;

        if (hMainWindow)
        {
            RECT rcMain;
            if (GetWindowRect(hMainWindow, &rcMain))
            {
                // Just move it from the upper left corner of the main window
                x = rcMain.left + 200;
                y = rcMain.top + 50;
            }
        }

        // Apply the position
        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    return FALSE;
}

} // namespace baresprite
