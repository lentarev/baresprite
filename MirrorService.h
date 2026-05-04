#pragma once

#include "AppState.h"

namespace baresprite
{

class MirrorService
{
  public:
    MirrorService() = delete;
    ~MirrorService() = delete;
    MirrorService(const MirrorService &) = delete;
    MirrorService &operator=(const MirrorService &) = delete;

    static void FlipHorizontal(AppState &appState, HWND hCanvas);
    static void FlipVertical(AppState &appState, HWND hCanvas);
};

} // namespace baresprite