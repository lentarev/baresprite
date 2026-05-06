#include "CursorRenderer.h"


namespace baresprite
{
CursorRenderer::CursorRenderer()
{
}

CursorRenderer::~CursorRenderer() = default;

void CursorRenderer::Render(int cursorSize, int cursorX, int cursorY, HDC hdcMem)
{
    // outline
    HPEN hWhitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdcMem, hWhitePen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));

    // White outline
    Rectangle(hdcMem, cursorX - 1, cursorY - 1, cursorX + cursorSize + 1, cursorY + cursorSize + 1);

    // main cursor
    HPEN hBlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    SelectObject(hdcMem, hBlackPen);
    Rectangle(hdcMem, cursorX, cursorY, cursorX + cursorSize, cursorY + cursorSize);

    // Restoring the state
    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteObject(hBlackPen);
    DeleteObject(hWhitePen);
}

} // namespace baresprite