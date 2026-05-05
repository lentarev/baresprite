#include "ClipboardService.h"
#include "Canvas.h"

namespace baresprite
{

/// <summary>
/// Копирует или вырезает выделенную область в буфер обмена
/// </summary>
/// <param name="appState"></param>
/// <param name="cut"></param>
void ClipboardService::CopySelectionToClipboard(AppState &appState, bool cut)
{
    if (!appState.selection.isActive || appState.selection.w <= 0 || appState.selection.h <= 0)
        return;
    if (appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];

    // 1. Ограничиваем выделение границами кадра
    int startX = (appState.selection.x < 0) ? 0 : appState.selection.x;
    int startY = (appState.selection.y < 0) ? 0 : appState.selection.y;
    int endX = (appState.selection.x + appState.selection.w > frame.width) ? frame.width : appState.selection.x + appState.selection.w;
    int endY = (appState.selection.y + appState.selection.h > frame.height) ? frame.height : appState.selection.y + appState.selection.h;

    int copyW = endX - startX;
    int copyH = endY - startY;
    if (copyW <= 0 || copyH <= 0)
        return;

    // 2. Сохраняем в буфер
    appState.clipboard.pixels.resize(static_cast<size_t>(copyW) * copyH);
    appState.clipboard.width = copyW;
    appState.clipboard.height = copyH;
    appState.clipboard.hasData = true;

    size_t idx = 0;
    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            size_t frameIdx = static_cast<size_t>(y) * frame.width + x;
            appState.clipboard.pixels[idx++] = frame.pixels[frameIdx];

            // Если Cut — очищаем исходник (0 = прозрачный/пустой)
            if (cut)
            {
                frame.pixels[frameIdx] = 0;
            }
        }
    }

    if (cut)
    {

        appState.isDirty = true;

        InvalidateRect(appState.canvas ? appState.canvas->GetHWndCanvas() : nullptr, nullptr, FALSE);
    }
}

/// <summary>
/// Вставляет буфер обмена в текущий кадр
/// </summary>
/// <param name="appState"></param>
void ClipboardService::PasteFromClipboard(AppState &appState)
{
    if (!appState.clipboard.hasData || appState.frames.empty())
        return;

    Frame &frame = appState.frames[appState.currentFrameIndex];

    // Целевая позиция: верхний левый угол выделения (или 0,0 если выделения нет)
    int targetX = appState.selection.isActive ? appState.selection.x : 0;
    int targetY = appState.selection.isActive ? appState.selection.y : 0;

    // Вычисляем область пересечения с кадром
    int pasteStartX = (targetX < 0) ? 0 : targetX;
    int pasteStartY = (targetY < 0) ? 0 : targetY;
    int pasteEndX = (targetX + appState.clipboard.width > frame.width) ? frame.width : targetX + appState.clipboard.width;
    int pasteEndY = (targetY + appState.clipboard.height > frame.height) ? frame.height : targetY + appState.clipboard.height;

    // Смещение внутри буфера (если вставка начинается за пределами кадра слева/сверху)
    int srcOffsetX = (targetX < 0) ? -targetX : 0;
    int srcOffsetY = (targetY < 0) ? -targetY : 0;

    // Копируем пиксели
    for (int y = pasteStartY; y < pasteEndY; ++y)
    {
        for (int x = pasteStartX; x < pasteEndX; ++x)
        {
            int srcX = srcOffsetX + (x - pasteStartX);
            int srcY = srcOffsetY + (y - pasteStartY);

            size_t frameIdx = static_cast<size_t>(y) * frame.width + x;
            size_t clipIdx = static_cast<size_t>(srcY) * appState.clipboard.width + srcX;

            frame.pixels[frameIdx] = appState.clipboard.pixels[clipIdx];
        }
    }

    // Обновляем выделение под вставленную область
    appState.selection.x = targetX;
    appState.selection.y = targetY;
    appState.selection.w = appState.clipboard.width;
    appState.selection.h = appState.clipboard.height;
    appState.selection.isActive = true;
    appState.selection.isDragging = false;

    appState.isDirty = true;

    InvalidateRect(appState.canvas ? appState.canvas->GetHWndCanvas() : nullptr, nullptr, FALSE);
}

} // namespace baresprite