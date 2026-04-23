#pragma once

#include "AppState.h"
#include <Windows.h>
#include <vector>

namespace baresprite
{

class Palette
{
  public:
    Palette(HWND hWndToolbar, HINSTANCE hInstanceParent, AppState &appState);
    ~Palette();

    // Select color by index
    void SelectColor(int index);

    // Get the currently selected color
    COLORREF GetSelectedColor() const;

    int ColorsCount() const;

    // Open the color selection dialog for editing
    bool EditColor(int index, HWND hWndOwner);

  private:
    HWND _hWndToolbar;
    HINSTANCE _hInstanceParent;
    AppState &_appState;

    std::vector<HBITMAP> _paletteBitmaps;
    std::vector<HWND> _paletteButtons;

    // Configuration
    int _startX = 17;
    int _startY = 20;

    const int _BTN_SIZE = 26;
    const int _BORDER_WIDTH = 5;
    const int _SPACING = 4;
    const int _COLS = 5;
    int _selectedIndex = 0;
    const int _colorCount = 25;

    COLORREF _selectedBorder = RGB(255, 108, 0);
    COLORREF _selectedColor = RGB(0, 0, 0);
    COLORREF _defaultBorder = RGB(255, 255, 255);

    //std::vector<COLORREF> _colors;

    // METHODS
    void CreatePalette();
    HBITMAP CreateBitmap(COLORREF color, int width, int height) const;
    COLORREF InvertColor(COLORREF color);
    void ResetColor(int index);

    int _oldIndex = 0;

    // METHODS

    // Hook function for the color picker dialog
    static UINT_PTR CALLBACK _PickerColorDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite