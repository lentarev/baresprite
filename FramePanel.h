#pragma once

#include "AppState.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace baresprite
{

class FramePanel
{
  public:
    FramePanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState);
    ~FramePanel();

    void SetBounds(const RECT &rc);
    int GetRightEdge() const;
    void UpdateFrameLabel();

    bool OnButtonNew();
    bool OnButtonPrev();
    bool OnButtonNext();
    bool OnButtonClone();
    bool OnButtonDelete();

  private:
    HWND _hWndBottomTolbar;
    HINSTANCE _hInstance;

    AppState &_appState;

    std::vector<std::wstring> _labels = {L"<", L">", L"New", L"Clone", L"Delete"};
    std::vector<HWND> _buttons;
    HWND _hLabel = nullptr;

    // Configuration
    int _startX = 180;
    int _startY = 20;

    static constexpr int _BTN_SIZE_W = 68;
    static constexpr int _BTN_SIZE_H = 30;

    static constexpr int _SPACING = 10;
    static constexpr int _COLS = 1;

    // Единый оффсет для центрирования всей панели
    static constexpr int _PANEL_OFFSET_X = -442;

    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;

    // Константы для лейбла
    static constexpr int LABEL_W = 70;
    static constexpr int LABEL_H = _BTN_SIZE_H;
    static constexpr int GAP_AFTER_FIRST_BTN = 70; // Отступ после кнопки "<"

    // METHODS
    void CreateControlButtons();
    void ResizeControlButtons(int clientW, int clientH);

    void CreateLabel();
    void ResizeLabel(int clientW, int clientH);
};

} // namespace baresprite