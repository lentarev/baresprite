#include "RotateService.h"
#include "Frame.h"
#include <vector>

namespace baresprite
{

void RotateService::RotateSelection90R(AppState &appState, HWND hCanvas)
{
    if (!appState.selection.isActive || appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];
    const auto &sel = appState.selection;
    int w = sel.w;
    int h = sel.h;

    // Считываем исходные пиксели в буфер
    std::vector<uint32_t> srcBuf(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            srcBuf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // Новые размеры (ширина и высота меняются местами)
    int newW = h;
    int newH = w;

    // Вычисляем новую позицию, чтобы центр остался на месте
    int centerX = sel.x + w / 2;
    int centerY = sel.y + h / 2;
    int newX = centerX - newW / 2;
    int newY = centerY - newH / 2;

    // ПРОВЕРКА ГРАНИЦ: если не влезает → отменяем операцию
    if (newX < 0 || newY < 0 || newX + newW > frame.width || newY + newH > frame.height)
    {
        return; // Пиксели не потеряются, так как холст не менялся
    }

    // Сохраняем в историю до изменений
    appState.history.Commit(appState.frames);

    // Очищаем старую область
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, 0);
        }
    }

    // Записываем повёрнутые пиксели на новое место
    // Формула 90° CW: new_x = old_h - 1 - old_y,  new_y = old_x
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int dx = h - 1 - y;
            int dy = x;
            frame.SetPixel(newX + dx, newY + dy, srcBuf[y * w + x]);
        }
    }

    // Обновляем состояние выделения
    appState.selection.x = newX;
    appState.selection.y = newY;
    appState.selection.w = newW;
    appState.selection.h = newH;
    appState.selection.rotationAngle = 0.0f; // Сбрасываем, т.к. снова выровнено по сетке

    appState.isDirty = true;

    InvalidateRect(hCanvas, nullptr, FALSE);
}


void RotateService::RotateSelection90L(AppState &appState, HWND hCanvas)
{
    if (!appState.selection.isActive || appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];
    const auto &sel = appState.selection;
    int w = sel.w;
    int h = sel.h;

    // Считываем исходные пиксели
    std::vector<uint32_t> srcBuf(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            srcBuf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // Новые размеры (меняются местами)
    int newW = h;
    int newH = w;

    // Вычисляем новую позицию (центр остается на месте)
    int centerX = sel.x + w / 2;
    int centerY = sel.y + h / 2;
    int newX = centerX - newW / 2;
    int newY = centerY - newH / 2;

    // Проверка границ
    if (newX < 0 || newY < 0 || newX + newW > frame.width || newY + newH > frame.height)
    {
        return;
    }

    // Сохраняем в историю
    appState.history.Commit(appState.frames);

    // Очищаем старую область
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, 0);
        }
    }

    // Записываем повёрнутые пиксели (CCW)
    // Формула: new_x = old_y,  new_y = old_w - 1 - old_x
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int dx = y;
            int dy = w - 1 - x;
            frame.SetPixel(newX + dx, newY + dy, srcBuf[y * w + x]);
        }
    }

    // Обновляем выделение
    appState.selection.x = newX;
    appState.selection.y = newY;
    appState.selection.w = newW;
    appState.selection.h = newH;
    appState.selection.rotationAngle = 0.0f;

    appState.isDirty = true;

    InvalidateRect(hCanvas, nullptr, FALSE);
}

} // namespace baresprite