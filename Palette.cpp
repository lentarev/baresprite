#include "Palette.h"
#include <commctrl.h>

namespace baresprite
{

Palette::Palette(HWND hWndParent, HINSTANCE hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{

    CreatePalette();
    SelectColor(0);
}

Palette::~Palette()
{
    for (HBITMAP bmp : _paletteBitmaps)
    {
        if (bmp)
            DeleteObject(bmp);
    }
    _paletteBitmaps.clear();
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

    HDC hdc = GetDC(_hWndParent);
    HDC hdcMem = CreateCompatibleDC(hdc);

    // Кнопка принимает владение битмапом. Удалять не нужно, кнопка удалит сама
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, _BTN_SIZE, _BTN_SIZE);

    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    // Задаём прямоугольник для заливки
    RECT rc = {0, 0, _BTN_SIZE, _BTN_SIZE};

    HPEN hPen = CreatePen(PS_SOLID, _BORDER_WIDTH, _colorPen);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    HBRUSH hBrush = CreateSolidBrush(_colors[index]);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    Rectangle(hdcMem, 0, 0, _BTN_SIZE, _BTN_SIZE);

    // Восстанавливаем состояние контекста (Pen)
    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);

    // Удаляем, то что сами создали
    DeleteObject(hPen);
    DeleteObject(hBrush);

    // Восстанавливаем сосотояние контекста (Bitmap)
    SelectObject(hdcMem, oldObj);

    // 2. Удаляем виртуальный холст из памяти
    DeleteDC(hdcMem);

    // 1. Освобождаем контекст окна, полученный через GetDC
    ReleaseDC(_hWndParent, hdc);

    SendMessage(_paletteButtons[index], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
}

/// <summary>
/// Reset color
/// </summary>
/// <param name="index"></param>
void Palette::ResetColor(int index)
{
    HDC hdc = GetDC(_hWndParent);

    HDC hdcMem = CreateCompatibleDC(hdc);

    // Кнопка принимает владение битмапом. Удалять не нужно, кнопка удалит сама
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, _BTN_SIZE, _BTN_SIZE);

    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    // Задаём прямоугольник для заливки
    RECT rc = {0, 0, _BTN_SIZE, _BTN_SIZE};

    HPEN hPen = CreatePen(PS_SOLID, _BORDER_WIDTH, _colors[index]);
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPen);

    HBRUSH hBrush = CreateSolidBrush(_colors[index]);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    Rectangle(hdcMem, 0, 0, _BTN_SIZE, _BTN_SIZE);

    // Восстанавливаем состояние контекста (Pen)
    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBrush);

    // Удаляем, то что сами создали
    DeleteObject(hPen);
    DeleteObject(hBrush);

    // Восстанавливаем сосотояние контекста (Bitmap)
    SelectObject(hdcMem, oldObj);

    // 2. Удаляем виртуальный холст из памяти
    DeleteDC(hdcMem);

    // 1. Освобождаем контекст окна, полученный через GetDC
    ReleaseDC(_hWndParent, hdc);

    SendMessage(_paletteButtons[index], BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
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

        HBITMAP hBmp = CreateBitmap(_colors[i], _BTN_SIZE, _BTN_SIZE);
        _paletteBitmaps.push_back(hBmp);

        HWND hBtn = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_BITMAP | BS_FLAT, x, y, _BTN_SIZE, _BTN_SIZE, _hWndParent,
                                    (HMENU)(INT_PTR)idCounter++, _hInstanceParent, nullptr);

        _paletteButtons.push_back(hBtn);
        SendMessage(hBtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
    }
}

HBITMAP Palette::CreateBitmap(COLORREF color, int width, int height) const
{
    // 1.  Получаем контекст устройства родительского окна
    HDC hdc = GetDC(_hWndParent);

    // 2. Создаём виртуальный холст в памяти
    HDC hdcMem = CreateCompatibleDC(hdc);

    // Создаём сам битмап (пустой) нужного размера
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);

    // Выбираем наш новый битмап в контекст устройства
    HGDIOBJ oldObj = SelectObject(hdcMem, hBitmap);

    // Задаём прямоугольник для заливки
    RECT rc = {0, 0, width, height};

    // Создаём кисть (brush) нужного цвета
    HBRUSH hBrush = CreateSolidBrush(color);

    // Заливаем прямоугольник в памяти DC нашей кистью
    FillRect(hdcMem, &rc, hBrush);

    // Удаляем ТОЛЬКО то, что создали мы
    DeleteObject(hBrush);

    // Возвращаем старый объект обратно в hdcMem
    SelectObject(hdcMem, oldObj);

    // 2. Удаляем виртуальный холст из памяти
    DeleteDC(hdcMem);

    // 1. Освобождаем контекст окна, полученный через GetDC
    ReleaseDC(_hWndParent, hdc);

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

} // namespace baresprite
