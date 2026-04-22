#pragma once

#include <Windows.h>
#include <string>
#include "PaletteData.h"

namespace baresprite
{

struct Project
{
    bool isExistAppConfig;
    std::wstring configPath;
    std::wstring projectPath;
    int imageSize;
    int checkerSize;
    PaletteData palette;
};

} // namespace baresprite