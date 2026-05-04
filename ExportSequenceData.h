#pragma once

#include "AppState.h"
#include <string>

namespace baresprite
{

struct ExportSequenceData
{
    AppState *appState;
    std::wstring selectedTag;
    std::wstring outputFolder;
    bool confirmed = false;
};

} // namespace baresprite