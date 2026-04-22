#pragma once

#include <Windows.h>
#include <string>
#include "PaletteData.h"

namespace baresprite
{

struct Project
{
    bool isExistAppConfig;
    bool isDirty;
    std::wstring configPath;
    std::wstring projectPath;
    std::wstring name;
    int imageSize;
    int checkerSize;
    PaletteData palette;
};

} // namespace baresprite