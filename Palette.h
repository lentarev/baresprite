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

    const COLORREF _colors[25] = {
        // Primary colors
        RGB(0, 0, 0),       // Black
        RGB(127, 127, 127), // Gray
        RGB(255, 0, 0),     // Red
        RGB(255, 127, 0),   // Orange
        RGB(255, 255, 0),   // Yellow
        RGB(0, 255, 0),     // Green
        RGB(0, 255, 255),   // Cyan
        RGB(0, 0, 255),     // Blue
        RGB(127, 0, 255),   // Purple
        RGB(255, 255, 255), // White

        // Shades of gray
        RGB(64, 64, 64),    // Dark Gray
        RGB(191, 191, 191), // Light Gray

        // Dark shades
        RGB(127, 0, 0),   // Dark Red
        RGB(0, 127, 0),   // Dark Green
        RGB(0, 0, 127),   // Dark Blue
        RGB(127, 127, 0), // Olive

        // Light shades
        RGB(255, 191, 191), // Light Red/Pink
        RGB(255, 127, 127), // Salmon
        RGB(127, 255, 127), // Light Green
        RGB(127, 127, 255), // Light Blue

        // Additional useful colors
        RGB(127, 64, 0),    // Brown
        RGB(255, 0, 255),   // Magenta
        RGB(0, 127, 127),   // Teal
        RGB(255, 200, 150), // Peach/Skin tone
        RGB(64, 0, 0)       // Maroon
    };

    // METHODS
    void CreatePalette();
    HBITMAP CreateBitmap(COLORREF color, int width, int height) const;
    COLORREF InvertColor(COLORREF color);
    void ResetColor(int index);

    int _oldIndex = 0;
};

} // namespace baresprite