#pragma once

#include <Windows.h>
#include <string>
#include "PaletteData.h"

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
};

} // namespace baresprite