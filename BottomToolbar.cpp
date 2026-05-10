#include "BottomToolbar.h"
#include "Canvas.h"
#include "FramePanel.h"
#include "TagPanel.h"
#include "manage_tags_dialog_proc.h"
#include "resource.h"
#include <commctrl.h>
#include <iostream>

namespace baresprite
{

BottomToolbar::BottomToolbar(HWND hWnd, HINSTANCE hInstance, AppState &appState) : _hWnd(hWnd), _hInstance(hInstance), _appState(appState)
{

    // Registers the window class for BottomToolbar
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _BottomToolbarWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszClassName = L"BareSpriteBottomToolbarClass";

    RegisterClassExW(&wcex);

    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"BareSpriteBottomToolbarClass", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, nullptr,
                               hInstance, nullptr);

    if (!_hToolbar)
    {
        return; // Failed to create window
    }

    _framePanel = std::make_unique<FramePanel>(_hToolbar, hInstance, appState);
    _tagPanel = std::make_unique<TagPanel>(_hToolbar, hInstance, appState);
}

BottomToolbar::~BottomToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

FramePanel *BottomToolbar::GetFramePanel() const
{
    return _framePanel.get();
}

TagPanel *BottomToolbar::GetTagPanel() const
{
    return _tagPanel.get();
}

void BottomToolbar::OnSize(int clientW, int clientH)
{

    if (_hToolbar)
    {
        SetWindowPos(_hToolbar, nullptr, 0, clientH - HEIGHT, clientW, HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (_framePanel)
    {
        RECT rcFrame = {0, 0, clientW, HEIGHT};
        _framePanel->SetBounds(rcFrame);
    }

    if (_tagPanel && _framePanel)
    {

        int tagPanelStartX = _framePanel->GetRightEdge() + 20; // 20px padding between panels
        RECT rcTag = {tagPanelStartX, 0, clientW, HEIGHT};
        _tagPanel->SetBounds(rcTag);
    }
}

bool BottomToolbar::OnCommand(int commandId, int notifyCode)
{

    if (commandId >= 3041 && commandId < 3041 + 5)
    {
        const int index = commandId - 3041;

        if (notifyCode == BN_CLICKED)
        {
            // Prev <
            if (index == 0)
            {

                if (_framePanel->OnButtonPrev())
                {
                    _tagPanel->UpdateTagSelection();

                    return true;
                }
            }
            // Next >
            else if (index == 1)
            {
                if (_framePanel->OnButtonNext())
                {

                    _tagPanel->UpdateTagSelection();

                    return true;
                }
            }
            // New
            else if (index == 2)
            {
                if (_framePanel->OnButtonNew())
                {

                    return true;
                }
            }
            // Clone
            else if (index == 3)
            {

                if (_framePanel->OnButtonClone())
                {

                    return true;
                }
            }
            // Delete
            else if (index == 4)
            {
                if (_framePanel->OnButtonDelete())
                {
                    _tagPanel->PopulateComboBoxes();
                    return true;
                }
            }
        }
    }

    // Onion command
    if (commandId >= 3051 && commandId < 3052)
    {
        const int index = commandId - 3051;

        // Onion Checkbox
        if (index == 0)
        {
            return _framePanel->OnOnionChecked();
        }
    }

    // Tag panel
    if (commandId >= 3055 && commandId < 3058)
    {
        const int index = commandId - 3055;

        // Filter, Tagging system
        if (notifyCode == CBN_SELCHANGE)
        {
            // Filter list (combobox)
            if (index == 0)
            {
                if (_tagPanel->OnChangeFilter())
                {

                    if (_framePanel)
                    {
                        _framePanel->UpdateFrameLabel(); // Update "Frame 3/10"
                    }

                    _tagPanel->UpdateTagSelection();
                }

                return true;
            }

            // Tag list (combobox)
            if (index == 1)
            {
                if (_tagPanel->OnChangeTag())
                {
                    if (_framePanel)
                    {
                        _framePanel->UpdateFrameLabel();
                    }
                }

                return true;
            }
        }

        // Manage tags (button)
        if (notifyCode == BN_CLICKED)
        {

            if (index == 2)
            {
                // Show manage tags dialog
                INT_PTR manageTagsResult =
                    DialogBoxParam(_hInstance, MAKEINTRESOURCE(IDD_DIALOG_MANAGE_TAGS), _hWnd, ManageTagsDialogProc, reinterpret_cast<LPARAM>(&_appState));

                if (_tagPanel)
                {
                    _tagPanel->PopulateComboBoxes();
                }

                return true;
            }
        }
    }

    // Play command
    if (commandId >= 3071 && commandId < 3072)
    {
        const int index = commandId - 3071;

        // Button - Play
        if (index == 0)
        {
            return _framePanel->OnPlay();
        }
    }

    return false;
}

bool BottomToolbar::OnHScroll(int scrollCode, HWND hSlider)
{
    // We get the control ID from its handle
    int controlId = GetDlgCtrlID(hSlider);

    // Onion slider
    if (controlId >= 3052 && controlId < 3053)
    {
        const int index = controlId - 3052;

        // Onion Trackbar
        if (index == 0)
        {
            if (scrollCode == TB_ENDTRACK || scrollCode == TB_THUMBPOSITION)
            {

                return _framePanel->OnSliderOpacity();
            }
        }
    }

    // Play command
    if (controlId >= 3072 && controlId < 3073)
    {
        const int index = controlId - 3072;

        // Button - Play
        if (index == 0)
        {
            if (scrollCode == TB_ENDTRACK || scrollCode == TB_THUMBTRACK)
            {

                return _framePanel->OnSliderSpeed();
            }
        }
    }

    return false;
}

HWND BottomToolbar::GetHWndBottomToolbar() const
{
    return _hToolbar;
}

LRESULT CALLBACK BottomToolbar::_BottomToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Forwarding notifications from child controls to the parent
    if (message == WM_COMMAND || message == WM_NOTIFY || message == WM_HSCROLL || message == WM_TIMER)
    {
        HWND hParent = GetParent(hWnd);
        if (hParent)
        {
            SendMessage(hParent, message, wParam, lParam);
        }
        return 0; // The message has been processed.
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite
