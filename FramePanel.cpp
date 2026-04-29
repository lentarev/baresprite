#include "FramePanel.h"
#include "Canvas.h"
#include "FrameService.h"
#include <cstdio>

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

void FramePanel::SetBounds(const RECT &rc)
{
    // rc теперь — это область, выделенная родителем специально для FramePanel
    // Пересчитываем _startX относительно левой границы этой области
    _startX = rc.left;
    //_startY = rc.top;

    // Пересоздаём/перемещаем контролы внутри этих границ
    ResizeControlButtons(rc.right - rc.left, rc.bottom - rc.top);
    ResizeLabel(rc.right - rc.left, rc.bottom - rc.top);
}

int FramePanel::GetRightEdge() const
{
    if (_buttons.empty())
        return _startX;

    // Возвращаем правую границу последней кнопки
    HWND hLastBtn = _buttons.back();
    RECT rc;
    GetWindowRect(hLastBtn, &rc);

    POINT rightEdge = {rc.right, rc.top};
    ScreenToClient(_hWndBottomTolbar, &rightEdge);

    return rightEdge.x;
}

void FramePanel::CreateControlButtons()
{

    int currentX = _startX + _PANEL_OFFSET_X;
    int idCounter = 3041;

    for (int i = 0; i < _labels.size(); ++i)
    {
        HWND hBtn = CreateWindowExW(0, L"BUTTON", _labels[i].c_str(), WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON, currentX, _startY, _BTN_SIZE_W,
                                    _BTN_SIZE_H, _hWndBottomTolbar, (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

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

void FramePanel::UpdateFrameLabel()
{
    if (!_hLabel)
        return; // Защита от нулевого хэндла

    // Берём данные напрямую из AppState
    int current = _appState.currentFrameIndex + 1; // +1 для человеческого отображения
    int total = static_cast<int>(_appState.frames.size());

    // Фолбэк на случай, если список вдруг пуст
    if (total < 1)
        total = 1;
    if (current < 1)
        current = 1;
    if (current > total)
        current = total;

    // Форматируем строку
    wchar_t text[32];
    swprintf_s(text, L"%d / %d", current, total);

    // Обновляем Win32 контрол
    SetWindowTextW(_hLabel, text);
}

/// <summary>
/// New Frame
/// </summary>
/// <returns></returns>
bool FramePanel::OnButtonNew()
{
    if (FrameService::NewFrame(_appState))
    {
        UpdateFrameLabel();

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);
        }

        return true;
    }

    return false;
}

/// <summary>
/// Prev frame
/// </summary>
/// <returns></returns>
bool FramePanel::OnButtonPrev()
{
    if (FrameService::PrevFrame(_appState))
    {
        UpdateFrameLabel();

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);
        }

        return true;
    }

    return false;
}

/// <summary>
/// Next Frame
/// </summary>
/// <returns></returns>
bool FramePanel::OnButtonNext()
{
    if (FrameService::NextFrame(_appState))
    {
        UpdateFrameLabel();

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);
        }

        return true;
    }

    return false;
}

/// <summary>
/// Clone Frame
/// </summary>
/// <returns></returns>
bool FramePanel::OnButtonClone()
{
    if (FrameService::CloneFrame(_appState))
    {
        UpdateFrameLabel();

        if (_appState.canvas)
        {
            const Frame &current = _appState.frames[_appState.currentFrameIndex];
            _appState.canvas->LoadFrame(current);
        }

        return true;
    }

    return false;
}

/// <summary>
/// Delete Frame
/// </summary>
/// <returns></returns>
bool FramePanel::OnButtonDelete()
{
    if (FrameService::DeleteFrame(_appState))
    {
        UpdateFrameLabel();

        if (_appState.canvas)
        {
            const Frame &current = _appState.frames[_appState.currentFrameIndex];
            _appState.canvas->LoadFrame(current);
        }

        return true;
    }

    return false; // Удаление отклонено (остался последний кадр)
}

} // namespace baresprite