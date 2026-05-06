#pragma once

#include <Windows.h>
#include <vector>
#include <string>

namespace baresprite
{

struct AppState;

struct ReorderDlgData
{
    AppState *appState;
    HWND hDlg;
    HWND hScrollArea; 
    HWND hScrollBar;  
    HWND hComboTag;
    WNDPROC oldScrollProc;

    std::vector<int> frameOrder;
    std::vector<int> visibleIndices;
    std::wstring currentFilterTag = L"All";

    const int thumbW = 100;
    const int thumbH = 100;
    const int padding = 10;
    int scrollPos = 0;
    int maxScroll = 0;

    int dragVisIdx = -1;
    int dropVisIdx = -1;
    bool isDragging = false;
};
}
