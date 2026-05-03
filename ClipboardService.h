#pragma once

#include "AppState.h"

namespace baresprite
{

class ClipboardService
{
  public:
    ClipboardService() = delete;
    ~ClipboardService() = delete;
    ClipboardService(const ClipboardService &) = delete;
    ClipboardService &operator=(const ClipboardService &) = delete;

    /// <summary>
    /// Копирует или вырезает выделенную область в буфер обмена
    /// </summary>
    static void CopySelectionToClipboard(AppState &appState, bool cut);

    /// <summary>
    /// Вставляет буфер обмена в текущий кадр
    /// </summary>
    /// <param name="appState"></param>
    static void PasteFromClipboard(AppState &appState);
};

} // namespace baresprite