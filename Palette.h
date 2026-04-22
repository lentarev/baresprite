#pragma once

#include <Windows.h>
#include <vector>

namespace baresprite
{

class Palette
{
  public:
    Palette(HWND hWndParent, HINSTANCE hInstanceParent);
    ~Palette();

    // Select color by index
    void SelectColor(int index);

    // Get the currently selected color
    COLORREF GetSelectedColor() const;

    int ColorsCount() const;

    // Open the color selection dialog for editing
    bool EditColor(int index, HWND hWndOwner);

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;

    std::vector<HBITMAP> _paletteBitmaps;
    std::vector<HWND> _paletteButtons;

    // Configuration
    int _startX = 22;
    int _startY = 20;

    const int _BTN_SIZE = 24;
    const int _BORDER_WIDTH = 2;
    const int _SPACING = 4;
    const int _COLS = 5;
    int _selectedIndex = 0;
    const int _colorCount = 25;

    COLORREF _colorPen = RGB(255, 255, 255);
    COLORREF _selectedColor = RGB(0, 0, 0);

    std::vector<COLORREF> _colors;

    // METHODS
    void CreatePalette();
    HBITMAP CreateBitmap(COLORREF color, int width, int height) const;
    COLORREF InvertColor(COLORREF color);
    void ResetColor(int index);

    int _oldIndex = 0;
};

} // namespace baresprite