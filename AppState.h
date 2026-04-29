#pragma once

#include "Frame.h"
#include "PaletteData.h"
#include "ToolType.h"
#include <Windows.h>
#include <string>

namespace baresprite
{

class Canvas;

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
    int currentFrameIndex = 0;
    Canvas *canvas = nullptr;

    // Onion Skinning settings
    bool onionSkinEnabled = true;
    int onionSkinPrevFrames = 1;    
    int onionSkinNextFrames = 0;    
    float onionSkinOpacity = 0.35f;
};

} // namespace baresprite