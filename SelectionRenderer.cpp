#include "SelectionRenderer.h"
#include <cmath>

namespace baresprite
{
SelectionRenderer::SelectionRenderer() = default;
SelectionRenderer::~SelectionRenderer() = default;

void SelectionRenderer::Render(HDC hdc, const SelectionState &selection, int checkerSize)
{
    if (!selection.isActive || selection.w <= 0 || selection.h <= 0)
        return;

    int left = selection.x * checkerSize;
    int top = selection.y * checkerSize;
    int right = (selection.x + selection.w) * checkerSize;
    int bottom = (selection.y + selection.h) * checkerSize;

    static ULONGLONG lastTime = GetTickCount64();
    ULONGLONG now = GetTickCount64();

    static int offset = 0;

    // We update the offset every 100ms
    if (now - lastTime > 100)
    {
        offset = (offset + 1) % 8;
        lastTime = now;
    }

    // Draw 4 sides
    DrawDashedLine(hdc, left, top, right, top, offset);       // Top
    DrawDashedLine(hdc, right, top, right, bottom, offset);   // Right
    DrawDashedLine(hdc, right, bottom, left, bottom, offset); // Bottom
    DrawDashedLine(hdc, left, bottom, left, top, offset);     // Left
}

void SelectionRenderer::DrawDashedLine(HDC hdc, int x1, int y1, int x2, int y2, int offset)
{
    bool isHorizontal = (y1 == y2);
    int length = isHorizontal ? std::abs(x2 - x1) : std::abs(y2 - y1);

    int stepX = (x2 > x1) ? 1 : (x2 < x1) ? -1 : 0;
    int stepY = (y2 > y1) ? 1 : (y2 < y1) ? -1 : 0;

    const int dashLen = 4;
    const int gapLen = 4;
    const int patternLen = dashLen + gapLen;

    for (int i = 0; i < length; ++i)
    {
        int pos = (i + offset) % patternLen;

        if (pos < dashLen)
        {
            int curX = x1 + i * stepX;
            int curY = y1 + i * stepY;

            // Contrast outline (white + black)
            int whiteX = curX + (isHorizontal ? 0 : -1);
            int whiteY = curY + (isHorizontal ? -1 : 0);
            SetPixelV(hdc, whiteX, whiteY, RGB(255, 255, 255));

            SetPixelV(hdc, curX, curY, RGB(0, 0, 0));
        }
    }
}

} // namespace baresprite