#pragma once

#include "ClipboardData.h"
#include "Frame.h"
#include "HistoryState.h"
#include "MoveDragState.h"
#include "PaletteData.h"
#include "SelectionState.h"
#include "ToolType.h"
#include <Windows.h>
#include <string>

namespace baresprite
{

class Canvas;

struct AppState
{
    bool isProjectLoadedFromConfig = false;
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
    int numberFramesByTag = 0;
    int startIndexByTag = 0;
    Canvas *canvas = nullptr;

    // Onion Skinning settings
    bool onionSkinEnabled = true;
    int onionSkinPrevFrames = 1;
    int onionSkinNextFrames = 0;
    float onionSkinOpacity = 0.35f;

    // Tagging system
    std::vector<std::wstring> availableTags = {};
    std::wstring selectedTag = L"None";
    std::wstring currentFilterTag = L"";

    // Selection
    SelectionState selection;
    ClipboardData clipboard;

    // Undo/Redo
    HistoryState history;

    // Move
    MoveDragState moveDrag;

    // Animation
    int playbackFPS = 12;
    bool isPlaying = false;
};

} // namespace baresprite