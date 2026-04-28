#include "FramePanel.h"
#include <iostream>

namespace baresprite
{
FramePanel::FramePanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState)
    : _hWndBottomTolbar(hWndBottomTolbar), _hInstance(hInstance), _appState(appState)
{
    CreateControlButtons();
    CreateLabel();
}

FramePanel::~FramePanel()
{
    for (HWND btn : _buttons)
    {
        if (btn && IsWindow(btn))
        {
            DestroyWindow(btn);
        }
    }

    if (_hLabel && IsWindow(_hLabel))
    {
        DestroyWindow(_hLabel);
    }

    _buttons.clear();
}

void FramePanel::OnSize(int clientW, int clientH)
{
    ResizeControlButtons(clientW, clientH);
    ResizeLabel(clientW, clientH);
}

void FramePanel::CreateControlButtons()
{

    int currentX = _startX + _PANEL_OFFSET_X;
    int idCounter = 3041;

    for (int i = 0; i < _labels.size(); ++i)
    {
        HWND hBtn = CreateWindowExW(0, L"BUTTON", _labels[i].c_str(), WS_CHILD | WS_VISIBLE | BS_FLAT | BS_NOTIFY | BS_DEFPUSHBUTTON, currentX, _startY,
                                    _BTN_SIZE_W, _BTN_SIZE_H, _hWndBottomTolbar, (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

        _buttons.push_back(hBtn);

        currentX += _BTN_SIZE_W + _SPACING;

        if (i == 0)
        {
            currentX += GAP_AFTER_FIRST_BTN;
        }
    }
}

void FramePanel::ResizeControlButtons(int clientW, int clientH)
{
    const int freeAreaW = (clientW - LEFT_TOOLBAR_WIDTH - RIGHT_TOOLBAR_WIDTH);
    _startX = LEFT_TOOLBAR_WIDTH + freeAreaW / 2;

    int currentX = _startX + _PANEL_OFFSET_X;

    for (int i = 0; i < _buttons.size(); ++i)
    {
        SetWindowPos(_buttons[i], nullptr, currentX, _startY, _BTN_SIZE_W, _BTN_SIZE_H, SWP_NOZORDER | SWP_NOACTIVATE);

        currentX += _BTN_SIZE_W + _SPACING;

        if (i == 0)
        {
            currentX += GAP_AFTER_FIRST_BTN;
        }
    }
}

void FramePanel::CreateLabel()
{
    // Создаём лейбл временно (позиция будет исправлена в ResizeLabel)
    _hLabel = CreateWindowExW(0, L"STATIC", L"1 / 1", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, LABEL_W, LABEL_H, _hWndBottomTolbar, nullptr,
                              _hInstance, nullptr);

    // Сразу вызываем ResizeLabel для правильной позиции
    RECT rc;
    GetClientRect(_hWndBottomTolbar, &rc);
    ResizeLabel(rc.right - rc.left, rc.bottom - rc.top);
}

void FramePanel::ResizeLabel(int clientW, int clientH) 
{
    if (_buttons.size() < 2 || !_hLabel)
        return;

    // Получаем реальные позиции кнопок "<" и ">"
    RECT rectBtnPrev, rectBtnNext;
    GetWindowRect(_buttons[0], &rectBtnPrev); // Кнопка "<"
    GetWindowRect(_buttons[1], &rectBtnNext); // Кнопка ">"

    // Конвертируем в координаты родителя
    POINT prevRight = {rectBtnPrev.right, rectBtnPrev.top};
    POINT nextLeft = {rectBtnNext.left, rectBtnNext.top};
    ScreenToClient(_hWndBottomTolbar, &prevRight);
    ScreenToClient(_hWndBottomTolbar, &nextLeft);

    // Вычисляем центр между кнопками
    int gap = nextLeft.x - prevRight.x; // Расстояние между кнопками
    int labelX = prevRight.x + (gap - LABEL_W) / 2;

    SetWindowPos(_hLabel, nullptr, labelX, _startY, LABEL_W, LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
}

} // namespace baresprite