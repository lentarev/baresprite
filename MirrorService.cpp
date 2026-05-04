#include "MirrorService.h"

#include "Frame.h"
#include <vector>

namespace baresprite
{
void MirrorService::FlipHorizontal(AppState &appState, HWND hCanvas)
{
    if (!appState.selection.isActive || appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];
    const auto &sel = appState.selection;
    int w = sel.w;
    int h = sel.h;

    // 1. Считываем выделение в буфер
    std::vector<uint32_t> buf(static_cast<size_t>(w) * h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            buf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // 2. Сохраняем историю ДО изменения пикселей
    appState.history.Commit(appState.frames);

    // 3. Записываем обратно с горизонтальным отражением
    // Формула: новый X = w - 1 - старый X
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, buf[y * w + (w - 1 - x)]);
        }
    }

    appState.isDirty = true;

    InvalidateRect(hCanvas, nullptr, FALSE);
}

void MirrorService::FlipVertical(AppState &appState, HWND hCanvas)
{
    if (!appState.selection.isActive || appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];
    const auto &sel = appState.selection;
    int w = sel.w;
    int h = sel.h;

    // 1. Считываем выделение в буфер
    std::vector<uint32_t> buf(static_cast<size_t>(w) * h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            buf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // Сохраняем историю
    appState.history.Commit(appState.frames);

    // Записываем обратно с вертикальным отражением
    // Формула: новый Y = h - 1 - старый Y
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, buf[(h - 1 - y) * w + x]);
        }
    }

    appState.isDirty = true;

    InvalidateRect(hCanvas, nullptr, FALSE);
}

} // namespace baresprite