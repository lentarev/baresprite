#include "TagPanel.h"

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
    _startX = rc.left; //  Просто берём левую границу (уже вычисленную в BottomToolbar)
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
                               nullptr, _hInstance, nullptr);
}

void TagPanel::ResizeControls(int clientW, int clientH) const
{
    int currentX = _startX; // Начинаем от переданной позиции

    // Расставляем контролы по порядку

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
    // Очищаем
    SendMessageW(_hComboFilter, CB_RESETCONTENT, 0, 0);
    SendMessageW(_hComboTag, CB_RESETCONTENT, 0, 0);

    // 1. Filter: добавляем "<All>" первым пунктом
    SendMessageW(_hComboFilter, CB_ADDSTRING, 0, (LPARAM)L"All");

    // 2. Добавляем теги ТОЛЬКО из AppState
    for (const auto &tag : _appState.availableTags)
    {

        SendMessageW(_hComboFilter, CB_ADDSTRING, 0, (LPARAM)tag.c_str());
        SendMessageW(_hComboTag, CB_ADDSTRING, 0, (LPARAM)tag.c_str());
    }

    // 3. Восстанавливаем выбранные значения из AppState
    UpdateFilterSelection();
    UpdateTagSelection();
}

void TagPanel::UpdateFilterSelection()
{
    if (_appState.currentFilterTag.empty())
    {
        SendMessageW(_hComboFilter, CB_SETCURSEL, 0, 0); // <All>
    }
    else
    {
        // Ищем индекс тега в списке
        for (size_t i = 0; i < _appState.availableTags.size(); ++i)
        {
            if (_appState.availableTags[i] == _appState.currentFilterTag)
            {
                SendMessageW(_hComboFilter, CB_SETCURSEL, i + 1, 0); // +1 из-за <All>
                break;
            }
        }
    }
}

void TagPanel::UpdateTagSelection()
{
    if (_appState.frames.empty())
        return;

    const std::wstring &currentTag = _appState.frames[_appState.currentFrameIndex].tag;

    if (currentTag.empty())
    {
        SendMessageW(_hComboTag, CB_SETCURSEL, 0, 0); // Пустой тег = первый пункт
    }
    else
    {
        // Ищем индекс тега
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
        // Меняем фильтр в AppState
        if (selIndex == 0)
        {
            _appState.currentFilterTag = L""; // <All>
        }
        else if (selIndex - 1 < static_cast<int>(_appState.availableTags.size()))
        {
            const std::wstring &selected = _appState.availableTags[selIndex - 1];
            _appState.currentFilterTag = (selected == L"None") ? L"" : selected;
        }
        _appState.isDirty = true;
        return true;
    }
    else if (hWndCtrl == _hComboTag)
    {
        // Меняем тег текущего кадра в AppState
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

    if (OnComboBoxChange(_hComboFilter, selIndex))
    {

        return true;
    }

    return false;
}

bool TagPanel::OnChangeTag()
{
    int selIndex = (int)SendMessageW(_hComboTag, CB_GETCURSEL, 0, 0);

    if (OnComboBoxChange(_hComboTag, selIndex))
    {

        return true;
    }

    return false;
}

} // namespace baresprite