#pragma once

#include "PaletteData.h"
#include <Windows.h>
#include <string>
#include "ToolType.h"
#include "Frame.h"

namespace baresprite
{

struct AppState
{
    bool isExistAppConfig;
    bool isDirty;
    std::wstring configPath;
    std::wstring projectPath;
    std::wstring name;
    int imageSize = 64;
    int checkerSize = 8;
    PaletteData palette;
    ToolType currentTool = ToolType::Brush;
    std::vector<Frame> frames;
};

} // namespace baresprite