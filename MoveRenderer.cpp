#include "MoveRenderer.h"
#include "Canvas.h"

namespace baresprite
{

MoveRenderer::MoveRenderer() = default;
MoveRenderer::~MoveRenderer() = default;

void MoveRenderer::Render(const Frame &frame, int checkerSize, AppState &appState, HDC hdcMem)
{
    if (appState.moveDrag.isDragging)
    {
        const auto &drag = appState.moveDrag;
        for (int cy = 0; cy < drag.h; ++cy)
        {
            for (int cx = 0; cx < drag.w; ++cx)
            {
                int px = appState.selection.x + cx;
                int py = appState.selection.y + cy;

                if (px >= 0 && px < frame.width && py >= 0 && py < frame.height)
                {
                    uint32_t pixel = drag.pixelBuffer[cy * drag.w + cx];
                    unsigned char a = (pixel >> 24) & 0xFF;

                    if (a > 0) // Не рисуем полностью прозрачные пиксели
                    {
                        COLORREF col = RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
                        
                        // Рисуем квадратик с учётом зума
                        for (int sy = 0; sy < checkerSize; ++sy)
                        {
                            for (int sx = 0; sx < checkerSize; ++sx)
                            {
                                SetPixelV(hdcMem, px * checkerSize + sx, py * checkerSize + sy, col);
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace baresprite