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

    // Reading the selection into the buffer
    std::vector<uint32_t> buf(static_cast<size_t>(w) * h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            buf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // Save the history BEFORE changing pixels
    appState.history.Commit(appState.frames, appState.selection.x, appState.selection.y, appState.selection.w, appState.selection.h,
                            appState.selection.isActive);

    // We write back with horizontal reflection
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

    // Reading the selection into the buffer
    std::vector<uint32_t> buf(static_cast<size_t>(w) * h);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            buf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // Preserving history
    appState.history.Commit(appState.frames, appState.selection.x, appState.selection.y, appState.selection.w, appState.selection.h,
                            appState.selection.isActive);

    // We write back with vertical reflection
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