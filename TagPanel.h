#pragma once
#include "AppState.h"
#include <Windows.h>

namespace baresprite
{

class TagPanel
{
  public:
    TagPanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState);
    ~TagPanel();

    void SetBounds(const RECT &rc);
    

  private:
    HWND _hWndBottomTolbar;
    HINSTANCE _hInstance;

    // Filter
    HWND _hLabelFilter = nullptr;
    HWND _hComboFilter = nullptr;

    // Tag
    HWND _hLabelTag = nullptr;
    HWND _hComboTag = nullptr;

    // Button
    HWND _hButton = nullptr;


    AppState &_appState;

    // Configuration
    int _startX = 180;
    int _startY = 20;

    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;

    // Константы для лейбла
    static constexpr int _LABEL_W = 40;
    static constexpr int _LABEL_H = 30;
    static constexpr int _SPACING = 4;

    static constexpr int _COMBO_W = 100; 
    static constexpr int _COMBO_H = 30;

    static constexpr int _BTN_SIZE_W = 100;
    static constexpr int _BTN_SIZE_H = 30;

    // METHODS
    void CreateControls();
    void ResizeControls(int clientW, int clientH) const;
};

} // namespace baresprite