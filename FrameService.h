#pragma once

#include "AppState.h"

namespace baresprite
{

class FrameService
{
  public:
    FrameService() = delete;
    ~FrameService() = delete;
    FrameService(const FrameService &) = delete;
    FrameService &operator=(const FrameService &) = delete;

    static bool NewFrame(AppState &appState);
};

} // namespace baresprite