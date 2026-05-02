#include "SelectionRenderer.h"

namespace baresprite
{
SelectionRenderer::SelectionRenderer() = default;
SelectionRenderer::~SelectionRenderer() = default;

void SelectionRenderer::Render(HDC hdc, const SelectionState &sel, int checkerSize)
{
    if (!sel.isActive)
        return;

    // Вычисляем экранные координаты
    int x1 = sel.x * checkerSize;
    int y1 = sel.y * checkerSize;
    int x2 = (sel.x + sel.w) * checkerSize;
    int y2 = (sel.y + sel.h) * checkerSize;

    // Если ширина или высота 0 — добавляем 1 логический пиксель для визуализации
    // Это только для отрисовки, реальные данные не меняем!
    if (x1 == x2)
    {
        x2 = x1 + (sel.w >= 0 ? checkerSize : -checkerSize);
    }
    if (y1 == y2)
    {
        y2 = y1 + (sel.h >= 0 ? checkerSize : -checkerSize);
    }

    // Нормализуем прямоугольник для DrawFocusRect (left < right, top < bottom)
    int left = (x1 < x2) ? x1 : x2;
    int right = (x1 > x2) ? x1 : x2;
    int top = (y1 < y2) ? y1 : y2;
    int bottom = (y1 > y2) ? y1 : y2;

    RECT rect = {left, top, right, bottom};

    // DrawFocusRect сам анимирует пунктир и работает с любым фоном
    DrawFocusRect(hdc, &rect);
}
} // namespace baresprite