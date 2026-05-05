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
    std::wstring fileName;
    int columns = 1;
    int padding = 0;
    bool generateJson = true;
    bool confirmed = false;
};

} // namespace baresprite