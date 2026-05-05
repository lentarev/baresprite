#include "MoveDrag.h"

namespace baresprite
{

MoveDrag::MoveDrag() = default;
MoveDrag::~MoveDrag() = default;

void MoveDrag::ButtonDown(AppState &appState, HWND hCanvas, int x, int y, int checkerSize)
{

    int logX = x / checkerSize;
    int logY = y / checkerSize;
    const auto &sel = appState.selection;

    if (sel.isActive && logX >= sel.x && logX < sel.x + sel.w && logY >= sel.y && logY < sel.y + sel.h)
    {
        Frame &frame = appState.frames[appState.currentFrameIndex];

        // Сохраняем для Undo
        appState.history.Commit(appState.frames, appState.selection.x, appState.selection.y, appState.selection.w, appState.selection.h,
                                appState.selection.isActive);

        // Копируем пиксели в буфер
        appState.moveDrag.pixelBuffer.resize(sel.w * sel.h);
        for (int dy = 0; dy < sel.h; ++dy)
        {
            for (int dx = 0; dx < sel.w; ++dx)
            {
                appState.moveDrag.pixelBuffer[dy * sel.w + dx] = frame.GetPixel(sel.x + dx, sel.y + dy);
            }
        }

        // Очищаем выделение в кадре (делает дырку прозрачной)
        for (int dy = 0; dy < sel.h; ++dy)
        {
            for (int dx = 0; dx < sel.w; ++dx)
            {
                frame.SetPixel(sel.x + dx, sel.y + dy, 0);
            }
        }

        // Инициализируем драг
        appState.moveDrag.isDragging = true;
        appState.moveDrag.startLogX = logX;
        appState.moveDrag.startLogY = logY;
        appState.moveDrag.origSelX = sel.x;
        appState.moveDrag.origSelY = sel.y;
        appState.moveDrag.w = sel.w;
        appState.moveDrag.h = sel.h;

        InvalidateRect(hCanvas, nullptr, FALSE);
    }
}

void MoveDrag::ButtonMove(AppState &appState, HWND hCanvas, int x, int y, int checkerSize)
{
    int logX = x / checkerSize;
    int logY = y / checkerSize;

    int dx = logX - appState.moveDrag.startLogX;
    int dy = logY - appState.moveDrag.startLogY;

    int newX = appState.moveDrag.origSelX + dx;
    int newY = appState.moveDrag.origSelY + dy;

    Frame &frame = appState.frames[appState.currentFrameIndex];
    int w = appState.moveDrag.w;
    int h = appState.moveDrag.h;

    // Ограничиваем границами холста
    if (newX < 0)
        newX = 0;
    if (newY < 0)
        newY = 0;
    if (newX + w > frame.width)
        newX = frame.width - w;
    if (newY + h > frame.height)
        newY = frame.height - h;

    // Обновляем координаты выделения (используем как превью-позицию)
    appState.selection.x = newX;
    appState.selection.y = newY;

    // Перерисовываем весь холст (фон + кадр + превью выделения)
    InvalidateRect(hCanvas, nullptr, FALSE);
}

void MoveDrag::ButtonUp(AppState &appState, HWND hCanvas)
{
    Frame &frame = appState.frames[appState.currentFrameIndex];
    const auto &drag = appState.moveDrag;
    int finalX = appState.selection.x;
    int finalY = appState.selection.y;

    // Коммитим буфер в кадр на финальную позицию
    for (int cy = 0; cy < drag.h; ++cy)
    {
        for (int cx = 0; cx < drag.w; ++cx)
        {
            int px = finalX + cx;
            int py = finalY + cy;
            if (px >= 0 && px < frame.width && py >= 0 && py < frame.height)
            {
                frame.SetPixel(px, py, drag.pixelBuffer[cy * drag.w + cx]);
            }
        }
    }

    // Очищаем состояние драга
    appState.moveDrag.isDragging = false;
    appState.moveDrag.pixelBuffer.clear();
    appState.moveDrag.pixelBuffer.shrink_to_fit();

    InvalidateRect(hCanvas, nullptr, FALSE);
}

} // namespace baresprite
