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
    static bool PrevFrame(AppState &appState);
    static bool NextFrame(AppState &appState);
    static bool CloneFrame(AppState &appState);
    static bool DeleteFrame(AppState &appState);

    static Frame &GetCurrentFrame(AppState &state);
    static const Frame &GetCurrentFrame(const AppState &state);

    static bool MatchesFilter(const Frame &frame, const std::wstring &filterTag);
};

} // namespace baresprite