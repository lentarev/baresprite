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
    /// Copies or cuts the selected area to the clipboard.
    /// </summary>
    static void CopySelectionToClipboard(AppState &appState, bool cut);

    /// <summary>
    /// Pastes the clipboard into the current frame.
    /// </summary>
    /// <param name="appState"></param>
    static void PasteFromClipboard(AppState &appState);
};

} // namespace baresprite