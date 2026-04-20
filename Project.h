#pragma once

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
};

} // namespace baresprite