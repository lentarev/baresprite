#include "CursorRenderer.h"


namespace baresprite
{
CursorRenderer::CursorRenderer()
{
}

CursorRenderer::~CursorRenderer() = default;

void CursorRenderer::Render(int cursorSize, int cursorX, int cursorY, HDC hdcMem)
{
    // Сохраняем состояние DC
    int oldRop = SetROP2(hdcMem, R2_XORPEN);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH)); // Прозрачная заливка

    // Рисуем ПРЯМО НА hdcMem (инверсия фона!)
    Rectangle(hdcMem, cursorX, cursorY, cursorX + cursorSize, cursorY + cursorSize);

    // Восстанавливаем состояние
    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteObject(hPen);
    SetROP2(hdcMem, oldRop);
}

} // namespace baresprite