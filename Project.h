#pragma once

#include <Windows.h>
#include <string>

namespace baresprite
{

struct Project
{
    bool isExistAppConfig;
    std::wstring configPath;
    std::wstring projectPath;
    int imageSize;
    int checkerSize;
    COLORREF paletteColor;
};

} // namespace baresprite