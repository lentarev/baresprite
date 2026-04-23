#include "ChessBackground.h"

namespace baresprite
{

ChessBackground::ChessBackground(AppState &appStat) : _appState(appStat)
{
    _hBrushLight = CreateSolidBrush(RGB(245, 245, 245)); // Светло-серый
    _hBrushDark = CreateSolidBrush(RGB(215, 215, 215));  // Тёмно-серый
}

ChessBackground::~ChessBackground()
{
    if (_hBrushLight)
    {
        DeleteObject(_hBrushLight);
    }

    if (_hBrushDark)
    {
        DeleteObject(_hBrushDark);
    }
};

void ChessBackground::Render(const PAINTSTRUCT &ps, HDC hdc) const
{
    // Настройки шахматки
    const int CELL_SIZE = _appState.checkerSize;

    // Вычисляем диапазон клеток только для области обновления (ps.rcPaint)
    const int startX = ps.rcPaint.left / CELL_SIZE;
    const int startY = ps.rcPaint.top / CELL_SIZE;
    const int endX = (ps.rcPaint.right + CELL_SIZE - 1) / CELL_SIZE;
    const int endY = (ps.rcPaint.bottom + CELL_SIZE - 1) / CELL_SIZE;

    // Рисуем только нужные клетки
    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            RECT tile = {x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE};

            RECT drawArea;
            if (IntersectRect(&drawArea, &tile, &ps.rcPaint))
            {
                // Чередование: чётная сумма координат -> светлая, нечётная -> тёмная
                HBRUSH hBrush = ((x + y) % 2 == 0) ? _hBrushLight : _hBrushDark;
                FillRect(hdc, &drawArea, hBrush);
            }
        }
    }
}

} // namespace baresprite