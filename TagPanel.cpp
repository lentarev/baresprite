#include "TagPanel.h"
#include "Canvas.h"
#include "FrameService.h"
#include <set>

namespace baresprite
{
TagPanel::TagPanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState)
    : _hWndBottomTolbar(hWndBottomTolbar), _hInstance(hInstance), _appState(appState)
{
    CreateControls();
    PopulateComboBoxes();
}

TagPanel::~TagPanel()
{
    if (_hLabelFilter && IsWindow(_hLabelFilter))
    {
        DestroyWindow(_hLabelFilter);
    }

    if (_hComboFilter && IsWindow(_hComboFilter))
    {
        DestroyWindow(_hComboFilter);
    }

    if (_hLabelTag && IsWindow(_hLabelTag))
    {
        DestroyWindow(_hLabelTag);
    }

    if (_hComboTag && IsWindow(_hComboTag))
    {
        DestroyWindow(_hComboTag);
    }

    if (_hButton && IsWindow(_hButton))
    {
        DestroyWindow(_hButton);
    }
}

void TagPanel::SetBounds(const RECT &rc)
{
    _startX = rc.left;
    ResizeControls(rc.right - rc.left, rc.bottom - rc.top);
}

void TagPanel::CreateControls()
{
    int idCounter = 3055;

    // FILTER
    _hLabelFilter = CreateWindowExW(0, L"STATIC", L"Filter:", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, _LABEL_W, _LABEL_H, _hWndBottomTolbar,
                                    nullptr, _hInstance, nullptr);

    _hComboFilter = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_HASSTRINGS | CBS_AUTOHSCROLL,
                                    0, 0, _COMBO_W, _COMBO_H, _hWndBottomTolbar, (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

    // TAG
    _hLabelTag = CreateWindowExW(0, L"STATIC", L"Tag:", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, _LABEL_W, _LABEL_H, _hWndBottomTolbar,
                                 nullptr, _hInstance, nullptr);

    _hComboTag = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_HASSTRINGS | CBS_AUTOHSCROLL, 0,
                                 0, _COMBO_W, _COMBO_H, _hWndBottomTolbar, (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);

    // Button - Manage tags
    _hButton = CreateWindowExW(0, L"BUTTON", L"Manage tags", WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON, 0, 0, _BTN_SIZE_W, _BTN_SIZE_H, _hWndBottomTolbar,
                               (HMENU)(INT_PTR)idCounter++, _hInstance, nullptr);
}

void TagPanel::ResizeControls(int clientW, int clientH) const
{
    int currentX = _startX;

    // FILTERS
    SetWindowPos(_hLabelFilter, nullptr, currentX, _startY, _LABEL_W, _LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
    currentX += _LABEL_W + _SPACING;

    SetWindowPos(_hComboFilter, nullptr, currentX, _startY + 3, _COMBO_W, _COMBO_H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
    RedrawWindow(_hWndBottomTolbar, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    currentX += _LABEL_W + _SPACING + 70;

    // TAGS
    SetWindowPos(_hLabelTag, nullptr, currentX, _startY, _LABEL_W, _LABEL_H, SWP_NOZORDER | SWP_NOACTIVATE);
    currentX += _LABEL_W + _SPACING;

    SetWindowPos(_hComboTag, nullptr, currentX, _startY + 3, _COMBO_W, _COMBO_H, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
    RedrawWindow(_hWndBottomTolbar, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    currentX += _LABEL_W + _SPACING + 70;

    // BUTTON
    SetWindowPos(_hButton, nullptr, currentX, _startY, _BTN_SIZE_W, _BTN_SIZE_H, SWP_NOZORDER | SWP_NOACTIVATE);
}

void TagPanel::PopulateComboBoxes()
{
    SendMessageW(_hComboFilter, CB_RESETCONTENT, 0, 0);
    SendMessageW(_hComboTag, CB_RESETCONTENT, 0, 0);

    for (const auto &tag : _appState.availableTags)
    {
        SendMessageW(_hComboTag, CB_ADDSTRING, 0, (LPARAM)tag.c_str());
    }

    SendMessageW(_hComboFilter, CB_ADDSTRING, 0, (LPARAM)L"All");

    auto dynamicTags = GetActiveFilterTags();

    for (const auto &tag : dynamicTags)
    {
        SendMessageW(_hComboFilter, CB_ADDSTRING, 0, (LPARAM)tag.c_str());
    }

    UpdateFilterSelection();
    UpdateTagSelection();
}

void TagPanel::UpdateFilterSelection()
{
    std::wstring targetText;

    if (_appState.currentFilterTag.empty())
    {
        targetText = L"All";
    }

    else if (_appState.currentFilterTag == L"None")
    {
        targetText = L"None";
    }

    else
    {
        targetText = _appState.currentFilterTag;
    }

    LRESULT idx = SendMessageW(_hComboFilter, CB_FINDSTRINGEXACT, -1, (LPARAM)targetText.c_str());

    if (idx != CB_ERR)
    {
        SendMessageW(_hComboFilter, CB_SETCURSEL, idx, 0);
    }

    else
    {
        SendMessageW(_hComboFilter, CB_SETCURSEL, 0, 0); // Fallback на All
    }
}

void TagPanel::UpdateTagSelection()
{
    if (_appState.frames.empty())
        return;

    const std::wstring &currentTag = _appState.frames[_appState.currentFrameIndex].tag;

    if (currentTag.empty())
    {
        SendMessageW(_hComboTag, CB_SETCURSEL, 0, 0);
    }
    else
    {
        // Looking for the tag index
        for (size_t i = 0; i < _appState.availableTags.size(); ++i)
        {
            if (_appState.availableTags[i] == currentTag)
            {
                SendMessageW(_hComboTag, CB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}

bool TagPanel::OnComboBoxChange(HWND hWndCtrl, int selIndex)
{
    if (selIndex < 0)
        return false;

    if (hWndCtrl == _hComboFilter)
    {

        wchar_t buf[256] = {};
        SendMessageW(_hComboFilter, CB_GETLBTEXT, selIndex, (LPARAM)buf);

        std::wstring selected(buf);

        if (selected == L"All")
        {
            _appState.currentFilterTag = L"";
        }

        else if (selected == L"None")
        {
            _appState.currentFilterTag = L"None";
        }

        else
        {
            _appState.currentFilterTag = selected;
        }

        _appState.isDirty = true;

        if (!_appState.frames.empty())
        {
            const std::wstring &currentTag = _appState.frames[_appState.currentFrameIndex].tag;
            bool currentMatches = false;

            if (_appState.currentFilterTag.empty())
            {
                currentMatches = true;
            }

            else if (_appState.currentFilterTag == L"None")
            {
                currentMatches = currentTag.empty();
            }

            else
            {
                currentMatches = (currentTag == _appState.currentFilterTag);
            }

            if (!currentMatches)
            {
                int firstMatch = FindFirstMatchingFrame();
                if (firstMatch >= 0)
                {
                    _appState.currentFrameIndex = firstMatch;

                    if (_appState.canvas)
                    {
                        _appState.canvas->LoadFrame(_appState.frames[firstMatch]);
                    }

                    return true;
                }
            }
        }

        return true;
    }
    else if (hWndCtrl == _hComboTag)
    {

        if (selIndex < static_cast<int>(_appState.availableTags.size()))
        {
            _appState.frames[_appState.currentFrameIndex].tag = _appState.availableTags[selIndex];
        }

        _appState.isDirty = true;

        return true;
    }

    return false;
}

bool TagPanel::OnChangeFilter()
{

    int selIndex = (int)SendMessageW(_hComboFilter, CB_GETCURSEL, 0, 0);

    if (selIndex < 0)
    {
        return false;
    }

    if (!OnComboBoxChange(_hComboFilter, selIndex))
    {
        return false;
    }

    if (!_appState.frames.empty())
    {
        const std::wstring &currentTag = _appState.frames[_appState.currentFrameIndex].tag;
        const std::wstring &filter = _appState.currentFilterTag;

        bool currentMatches = false;

        if (filter.empty())
        {
            currentMatches = true;
        }

        else if (filter == L"None")
        {
            currentMatches = currentTag.empty();
        }

        else
        {
            currentMatches = (currentTag == filter);
        }

        if (!currentMatches)
        {
            int firstMatch = FindFirstMatchingFrame();
            if (firstMatch >= 0 && firstMatch != _appState.currentFrameIndex)
            {

                _appState.currentFrameIndex = firstMatch;

                if (_appState.canvas)
                {
                    _appState.canvas->LoadFrame(_appState.frames[firstMatch]);
                }

                return true;
            }
        }
    }

    return true;
}

bool TagPanel::OnChangeTag()
{
    int selIndex = (int)SendMessageW(_hComboTag, CB_GETCURSEL, 0, 0);
    if (selIndex < 0)
    {
        return false;
    }

  
    if (!OnComboBoxChange(_hComboTag, selIndex))
    {
        return false;
    }

   
    if (!_appState.currentFilterTag.empty() && !_appState.frames.empty())
    {
        const Frame &currentFrame = _appState.frames[_appState.currentFrameIndex];
        if (!FrameService::MatchesFilter(currentFrame, _appState.currentFilterTag))
        {
            
            int nearest = -1;
            int current = _appState.currentFrameIndex;
            int size = static_cast<int>(_appState.frames.size());
            const std::wstring &filter = _appState.currentFilterTag;

            
            for (int radius = 1; radius < size; ++radius)
            {
               
                int prev = current - radius;
                if (prev >= 0 && FrameService::MatchesFilter(_appState.frames[prev], filter))
                {
                    nearest = prev;
                    break;
                }
             
                int next = current + radius;
                if (next < size && FrameService::MatchesFilter(_appState.frames[next], filter))
                {
                    nearest = next;
                    break;
                }
            }

           
            if (nearest >= 0)
            {
                _appState.currentFrameIndex = nearest;
                if (_appState.canvas)
                {
                    _appState.canvas->LoadFrame(_appState.frames[nearest]);
                }
            }
            
        }
    }

   
    PopulateComboBoxes();

    return true;
}

/// <summary>
/// Finds the index of the first frame matching the filter
/// Returns -1 if there are no suitable frames.
/// </summary>
/// <returns></returns>
int TagPanel::FindFirstMatchingFrame() const
{

    const std::wstring &filter = _appState.currentFilterTag;

    
    if (filter.empty())
        return 0;

    for (size_t i = 0; i < _appState.frames.size(); ++i)
    {
        const std::wstring &frameTag = _appState.frames[i].tag;

        if (filter == L"None")
        {
           
            if (frameTag.empty())
                return static_cast<int>(i);
        }
        else
        {
            
            if (frameTag == filter)
                return static_cast<int>(i);
        }
    }
    return -1; 
}

std::vector<std::wstring> TagPanel::GetActiveFilterTags() const
{
    std::set<std::wstring> uniqueTags; // set - automatically sorts and removes duplicates
    bool hasNone = false;

    for (const auto &frame : _appState.frames)
    {
        if (frame.tag.empty())
        {
            hasNone = true;
        }

        else
        {
            uniqueTags.insert(frame.tag);
        }
    }

    std::vector<std::wstring> result;
    if (hasNone)
    {
        result.push_back(L"None");
    }

    for (const auto &tag : uniqueTags)
    {
        result.push_back(tag);
    }

    return result;
}

} // namespace baresprite