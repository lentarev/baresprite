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

    // Read the source pixels into the buffer
    std::vector<uint32_t> srcBuf(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            srcBuf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // New dimensions (width and height are swapped)
    int newW = h;
    int newH = w;

    // We calculate a new position so that the center remains in place
    int centerX = sel.x + w / 2;
    int centerY = sel.y + h / 2;
    int newX = centerX - newW / 2;
    int newY = centerY - newH / 2;

    // Boundary check: if it doesn't fit, cancel the operation.
    if (newX < 0 || newY < 0 || newX + newW > frame.width || newY + newH > frame.height)
    {
        return; // Pixels won't be lost because the canvas hasn't changed.
    }

    // Save to history before changes
    appState.history.Commit(appState.frames, appState.selection.x, appState.selection.y, appState.selection.w, appState.selection.h,
                            appState.selection.isActive);

    // Clearing the old area
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, 0);
        }
    }

    // We write the rotated pixels to a new location
    // 90° CW: new_x = old_h - 1 - old_y,  new_y = old_x
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int dx = h - 1 - y;
            int dy = x;
            frame.SetPixel(newX + dx, newY + dy, srcBuf[y * w + x]);
        }
    }

    // Updating the selection state
    appState.selection.x = newX;
    appState.selection.y = newY;
    appState.selection.w = newW;
    appState.selection.h = newH;
    appState.selection.rotationAngle = 0.0f; // We reset it because it is aligned with the grid again.

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

    // Reading the original pixels
    std::vector<uint32_t> srcBuf(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            srcBuf[y * w + x] = frame.GetPixel(sel.x + x, sel.y + y);
        }
    }

    // New sizes (switch places)
    int newW = h;
    int newH = w;

    // We calculate the new position (the center remains in place)
    int centerX = sel.x + w / 2;
    int centerY = sel.y + h / 2;
    int newX = centerX - newW / 2;
    int newY = centerY - newH / 2;

    // Checking the boundaries
    if (newX < 0 || newY < 0 || newX + newW > frame.width || newY + newH > frame.height)
    {
        return;
    }

    // Save it to history
    appState.history.Commit(appState.frames, appState.selection.x, appState.selection.y, appState.selection.w, appState.selection.h,
                            appState.selection.isActive);

    // Clearing the old area
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            frame.SetPixel(sel.x + x, sel.y + y, 0);
        }
    }

    // We write rotated pixels (CCW)
    // new_x = old_y,  new_y = old_w - 1 - old_x
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int dx = y;
            int dy = w - 1 - x;
            frame.SetPixel(newX + dx, newY + dy, srcBuf[y * w + x]);
        }
    }

    // Refreshing the selection
    appState.selection.x = newX;
    appState.selection.y = newY;
    appState.selection.w = newW;
    appState.selection.h = newH;
    appState.selection.rotationAngle = 0.0f;

    appState.isDirty = true;

    InvalidateRect(hCanvas, nullptr, FALSE);
}

} // namespace baresprite