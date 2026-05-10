#include "FramePanel.h"
#include "Canvas.h"
#include "FrameService.h"
#include <commctrl.h>
#include <cstdio>
#include <iostream>

namespace baresprite
{

FramePanel::FramePanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState)
    : _hWndBottomTolbar(hWndBottomTolbar), _hInstance(hInstance), _appState(appState)
{

    _appState.startIndexByTag = FrameService::FindFirstMatchingFrame(_appState);
    _appState.numberFramesByTag = FrameService::GetNumberFramesByTag(_appState);

    CreateControlButtons();
    CreateLabel();

    UpdateFrameLabel();

    CreateOnionControls();
    UpdateOnionLabel();

    CreatePlayControls();
    UpdateSpeedLabel();
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

    _buttons.clear();

    if (_hLabel && IsWindow(_hLabel))
    {
        DestroyWindow(_hLabel);
    }

    if (_hBtnPlay && IsWindow(_hBtnPlay))
    {
        DestroyWindow(_hBtnPlay);
    }

    if (_hSliderSpeed && IsWindow(_hSliderSpeed))
    {
        DestroyWindow(_hSliderSpeed);
    }

    if (_hLabelSpeed && IsWindow(_hLabelSpeed))
    {
        DestroyWindow(_hLabelSpeed);
    }
}

void FramePanel::SetBounds(const RECT &rc)
{

    _startX = rc.left;

    ResizeControlButtons(rc.right - rc.left, rc.bottom - rc.top);
    ResizeLabel(rc.right - rc.left, rc.bottom - rc.top);
    ResizeOnionControls(rc.right - rc.left, rc.bottom - rc.top);
    ResizePlayControls(rc.right - rc.left, rc.bottom - rc.top);
}

int FramePanel::GetRightEdge() const
{
    if (_buttons.empty())
        return _startX;

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

    _hLabel = CreateWindowExW(0, L"STATIC", L"1 / 1", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, LABEL_W, LABEL_H, _hWndBottomTolbar, nullptr,
                              _hInstance, nullptr);
}

void FramePanel::ResizeLabel(int clientW, int clientH)
{
    if (_buttons.size() < 2 || !_hLabel)
        return;

    RECT rectBtnPrev, rectBtnNext;
    GetWindowRect(_buttons[0], &rectBtnPrev); // Button "<"
    GetWindowRect(_buttons[1], &rectBtnNext); // Button ">"

    // Convert to parent coordinates
    POINT prevRight = {rectBtnPrev.right, rectBtnPrev.top};
    POINT nextLeft = {rectBtnNext.left, rectBtnNext.top};
    ScreenToClient(_hWndBottomTolbar, &prevRight);
    ScreenToClient(_hWndBottomTolbar, &nextLeft);

    // Calculating the center between buttons
    int gap = nextLeft.x - prevRight.x;
    int labelX = prevRight.x + (gap - LABEL_W) / 2;

    SetWindowPos(_hLabel, nullptr, labelX, _startY, LABEL_W, LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
}

/// <summary>
/// Onion Skin
/// </summary>
void FramePanel::CreateOnionControls()
{
    int idCounter = 3051;

    // Checkbox "Onion Skin"
    _hCheckOnion = CreateWindowExW(0, L"BUTTON", L"Onion Skin", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, _hWndBottomTolbar,
                                   (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);
    // Set value
    SendMessageW(_hCheckOnion, BM_SETCHECK, _appState.onionSkinEnabled ? BST_CHECKED : BST_UNCHECKED, 0);

    // 3. Label for percent
    _hLabelOpacity =
        CreateWindowExW(0, L"STATIC", L"35%", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, 0, 0, _hWndBottomTolbar, nullptr, _hInstance, nullptr);

    // Slider (TrackBar)
    _hSliderOpacity = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | TBS_TOOLTIPS, 0, 0, 0, 0, _hWndBottomTolbar,
                                      (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

    // Setting the range (0 - 100)
    SendMessageW(_hSliderOpacity, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

    // Setting the current position
    int startPos = static_cast<int>(_appState.onionSkinOpacity * 100);
    SendMessageW(_hSliderOpacity, TBM_SETPOS, TRUE, startPos);
}

/// <summary>
/// Resize Onion
/// </summary>
/// <param name="clientW"></param>
/// <param name="clientH"></param>
void FramePanel::ResizeOnionControls(int clientW, int clientH) const
{
    const int offsetY = 37;
    const int width = 90;
    const int size = 50;

    // Checkbox
    int currentX = _startX + _PANEL_OFFSET_X;
    SetWindowPos(_hCheckOnion, nullptr, currentX, _startY + offsetY, width, 20, SWP_NOZORDER | SWP_NOACTIVATE);

    // Label
    currentX += width + _SPACING;
    SetWindowPos(_hLabelOpacity, nullptr, currentX, _startY + offsetY, width - size, 20, SWP_NOZORDER | SWP_NOACTIVATE);

    // Trackbar
    currentX += width - size + _SPACING;
    SetWindowPos(_hSliderOpacity, nullptr, currentX, _startY + offsetY, width + size, 20, SWP_NOZORDER | SWP_NOACTIVATE);
}

/// <summary>
/// Play
/// </summary>
void FramePanel::CreatePlayControls()
{
    int idCounter = 3071;

    // Button - Manage tags
    _hBtnPlay = CreateWindowExW(0, L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON, 0, 0, _BTN_SIZE_W, _BTN_SIZE_H, _hWndBottomTolbar,
                                (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

    // Label for speed
    _hLabelSpeed = CreateWindowExW(0, L"STATIC", L"12fps", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, 0, 0, _hWndBottomTolbar, nullptr,
                                   _hInstance, nullptr);

    // Slider (TrackBar)
    _hSliderSpeed = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | TBS_TOOLTIPS, 0, 0, 0, 0, _hWndBottomTolbar,
                                    (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

    // Setting the range (1 - 100)
    SendMessageW(_hSliderSpeed, TBM_SETRANGE, TRUE, MAKELPARAM(1, 100));
    // Setting the current position
    int startPos = _appState.playbackFPS;
    SendMessageW(_hSliderSpeed, TBM_SETPOS, TRUE, startPos);
}

void FramePanel::ResizePlayControls(int clientW, int clientH) const
{
    const int offsetY = 37;
    const int width = 90;
    const int size = 22;

    // Button
    int currentX = _startX + _PANEL_OFFSET_X + 305;
    SetWindowPos(_hBtnPlay, nullptr, currentX, _startY + offsetY, width - size, 20, SWP_NOZORDER | SWP_NOACTIVATE);

    // Label
    currentX += width - size + _SPACING;
    SetWindowPos(_hLabelSpeed, nullptr, currentX, _startY + offsetY, width + 10, 20, SWP_NOZORDER | SWP_NOACTIVATE);

    // Trackbar
    currentX += width - size + _SPACING + 30;
    SetWindowPos(_hSliderSpeed, nullptr, currentX, _startY + offsetY, width + 50, 20, SWP_NOZORDER | SWP_NOACTIVATE);
}

void FramePanel::UpdateSpeedLabel()
{
    if (!_hLabelSpeed)
        return;

    int fps = _appState.playbackFPS;
    int ms = 1000 / fps;

    wchar_t text[32];
    swprintf_s(text, L"%d fps - %dms", fps, ms);

    SetWindowTextW(_hLabelSpeed, text);
}

/// <summary>
/// Play
/// </summary>
/// <returns></returns>
bool FramePanel::OnPlay()
{
    _appState.isPlaying = !_appState.isPlaying;

    // Updating the button text
    if (_appState.isPlaying)
    {
        SetWindowTextW(_hBtnPlay, L"Stop");

        // Start timter
        int interval = 1000 / _appState.playbackFPS;
        SetTimer(_hWndBottomTolbar, 1, interval, nullptr);
    }
    else
    {
        SetWindowTextW(_hBtnPlay, L"Play");

        // Stop the timer
        KillTimer(_hWndBottomTolbar, 1);
    }

    return true;
}

bool FramePanel::OnSliderSpeed()
{

    int fps = static_cast<int>(SendMessageW(_hSliderSpeed, TBM_GETPOS, 0, 0));

    // We limit from 1 to 60
    if (fps < 1)
    {
        fps = 1;
    }

    if (fps > 60)
    {
        fps = 60;
    }

    _appState.playbackFPS = fps;
    _appState.isDirty = true;

    UpdateSpeedLabel();

    if (_appState.isPlaying)
    {
        int interval = 1000 / fps;
        SetTimer(_hWndBottomTolbar, 1, interval, nullptr);
    }

    return true;
}

/// <summary>
/// Set Onion Checked
/// </summary>
bool FramePanel::OnOnionChecked()
{

    LRESULT isChecked = SendMessageW(_hCheckOnion, BM_GETCHECK, 0, 0);
    bool newState = (isChecked == BST_CHECKED);

    _appState.onionSkinEnabled = newState;

    if (_appState.canvas)
    {
        InvalidateRect(_appState.canvas->GetHWndCanvas(), nullptr, TRUE);
    }

    _appState.isDirty = true;

    return true;
}

/// <summary>
/// Set slider value
/// </summary>
/// <returns></returns>
bool FramePanel::OnSliderOpacity()
{
    int pos = static_cast<int>(SendMessageW(_hSliderOpacity, TBM_GETPOS, 0, 0));
    _appState.onionSkinOpacity = pos / 100.0f;

    UpdateOnionLabel();

    if (_appState.canvas)
    {
        InvalidateRect(_appState.canvas->GetHWndCanvas(), nullptr, TRUE);
    }

    _appState.isDirty = true;

    return true;
}

/// <summary>
/// Update Onion Label
/// </summary>
void FramePanel::UpdateOnionLabel()
{
    if (!_hLabelOpacity)
        return;

    // 0.35 to 35
    int percent = static_cast<int>(_appState.onionSkinOpacity * 100);

    // Format the string
    wchar_t text[32];
    swprintf_s(text, L"%d%%", percent); // %% outputs one character '%'

    // Updating Win32 control
    SetWindowTextW(_hLabelOpacity, text);
}

void FramePanel::UpdateFrameLabel()
{
    if (!_hLabel)
        return;

    int current = _appState.currentFrameIndex + 1;
    int total = static_cast<int>(_appState.frames.size());

    int startIndexByTag = _appState.startIndexByTag;
    int endIndexByTag = _appState.numberFramesByTag + _appState.startIndexByTag;

    int numberFramesByTag = endIndexByTag - startIndexByTag;
    int tagIndex = current - startIndexByTag;

    if (total < 1)
    {
        total = 1;
    }

    if (current < 1)
    {
        current = 1;
    }

    if (current > total)
    {
        current = total;
    }

    // Format the string
    wchar_t text[32];

    if (numberFramesByTag == 0)
    {
        swprintf_s(text, L"%d / %d", current, total);
    }
    else
    {
        swprintf_s(text, L"%d / %d", tagIndex, numberFramesByTag);
    }

    // Updating Win32 control
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

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);

            // Recalculate frames by selected tag
            _appState.numberFramesByTag = FrameService::GetNumberFramesByTag(_appState);
        }

        UpdateFrameLabel();

        _appState.selection.Clear();
        _appState.isDirty = true;

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

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);
        }

        UpdateFrameLabel();

        _appState.selection.Clear();

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

        if (_appState.canvas)
        {
            const Frame &current = FrameService::GetCurrentFrame(_appState);
            _appState.canvas->LoadFrame(current);
        }

        UpdateFrameLabel();

        _appState.selection.Clear();

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

        if (_appState.canvas)
        {
            const Frame &current = _appState.frames[_appState.currentFrameIndex];
            _appState.canvas->LoadFrame(current);

            // Recalculate frames by selected tag
            _appState.numberFramesByTag = FrameService::GetNumberFramesByTag(_appState);
        }

        UpdateFrameLabel();

        _appState.selection.Clear();
        _appState.isDirty = true;

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

        if (_appState.canvas)
        {
            const Frame &current = _appState.frames[_appState.currentFrameIndex];
            _appState.canvas->LoadFrame(current);

            // Recalculate frames by selected tag
            _appState.numberFramesByTag = FrameService::GetNumberFramesByTag(_appState);
        }

        _appState.selection.Clear();
        _appState.isDirty = true;

        UpdateFrameLabel();

        return true;
    }

    return false; // Deletion rejected (last frame remaining)
}

} // namespace baresprite