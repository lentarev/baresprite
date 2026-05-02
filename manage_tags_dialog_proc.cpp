#include "manage_tags_dialog_proc.h"
#include "AppState.h"
#include "resource.h"
#include <algorithm>
#include <iostream>

namespace baresprite
{
INT_PTR CALLBACK ManageTagsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

        // Init
    case WM_INITDIALOG: {
        {

            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            AppState *appState = reinterpret_cast<AppState *>(lParam);

            if (!appState)
            {

                return TRUE;
            }

            // Инициализируем режим: редактирование выключено
            SetEditMode(hDlg, false);

            // Centered
            {
                RECT rcParent, rcDlg;
                // We get the coordinates of the main window (parent)
                GetWindowRect(GetParent(hDlg), &rcParent);

                // We get the coordinates of the dialogue itself
                GetWindowRect(hDlg, &rcDlg);

                // Calculating the width and height of a dialog
                int dlgW = rcDlg.right - rcDlg.left;
                int dlgH = rcDlg.bottom - rcDlg.top;

                // Calculate coordinates for centering
                int x = rcParent.left + (rcParent.right - rcParent.left) / 2 - dlgW / 2;
                int y = rcParent.top + (rcParent.bottom - rcParent.top) / 2 - dlgH / 2;

                // Apply the position
                SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
            }

            UpdateAddButtonState(hDlg);

            // Filling the tag list.
            HWND hList = GetDlgItem(hDlg, IDC_MANAGE_TAGS_LIST);

            if (hList)
            {
                // Сортируем
                std::sort(appState->availableTags.begin(), appState->availableTags.end(), [](const std::wstring &a, const std::wstring &b) {
                    if (a == L"None" && b != L"None")
                        return true;
                    if (b == L"None" && a != L"None")
                        return false;
                    return a < b;
                });

                // Очищаем список
                SendMessageW(hList, LB_RESETCONTENT, 0, 0);

                // Добавляем все теги из AppState (уже отсортированного!)
                for (const auto &tag : appState->availableTags)
                {
                    SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)tag.c_str());
                }

                // Выделяем первый элемент
                if (!appState->availableTags.empty())
                {
                    SendMessageW(hList, LB_SETCURSEL, 0, 0);
                }

                UpdateRenameButtonState(hDlg, hList);
            }
        }

        return (INT_PTR)TRUE;
    }

    // Command
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam); // For control notifications

        // Read the pointer
        AppState *appState = reinterpret_cast<AppState *>(GetWindowLongPtr(hDlg, DWLP_USER));

        if (!appState)
        {
            return TRUE;
        }

        // Обработка изменения выбора в списке
        if (wmId == IDC_MANAGE_TAGS_LIST && wmEvent == LBN_SELCHANGE)
        {
            HWND hList = GetDlgItem(hDlg, IDC_MANAGE_TAGS_LIST);

            // Если был активен режим редактирования — прерываем его
            if (IsEditMode(hDlg))
            {
                SetEditMode(hDlg, false);
                SetWindowTextW(GetDlgItem(hDlg, IDC_MANAGE_TAGS_EDIT), L"");
                // Событие EN_CHANGE сработает автоматически и заблокирует кнопку Add
            }

            // Обновляем кнопку под новый выбранный элемент
            UpdateRenameButtonState(hDlg, hList);
            return TRUE;
        }

        // Processing changes in Edit (EN_CHANGE)
        if (wmId == IDC_MANAGE_TAGS_EDIT && wmEvent == EN_CHANGE)
        {
            UpdateAddButtonState(hDlg);
            return TRUE;
        }

        // Handle button (Add)
        if (LOWORD(wParam) == IDC_MANAGE_TAGS_BTN_ADD)
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_MANAGE_TAGS_EDIT);
            HWND hList = GetDlgItem(hDlg, IDC_MANAGE_TAGS_LIST);

            // Getting text from the input field
            wchar_t buf[256] = {};
            GetWindowTextW(hEdit, buf, 256);
            std::wstring newTag(buf);

            // Validation
            if (newTag.empty())
            {
                MessageBoxW(hDlg, L"Tag name cannot be empty.", L"Warning", MB_ICONWARNING);
                return TRUE;
            }

            // Валидация: нет ли дубликата? (регистрозависимое сравнение)
            for (const auto &existingTag : appState->availableTags)
            {
                if (existingTag == newTag)
                {

                    std::wstring msg = L"Tag '" + newTag + L"' already exists.";
                    MessageBoxW(hDlg, msg.c_str(), L"Info", MB_ICONINFORMATION);
                    return TRUE;
                }
            }

            // Добавляем тег в AppState
            appState->availableTags.push_back(newTag);

            // Сортируем
            std::sort(appState->availableTags.begin(), appState->availableTags.end(), [](const std::wstring &a, const std::wstring &b) {
                if (a == L"None" && b != L"None")
                    return true;
                if (b == L"None" && a != L"None")
                    return false;
                return a < b;
            });

            // Обновляем ListBox
            SendMessageW(hList, LB_RESETCONTENT, 0, 0);
            for (const auto &tag : appState->availableTags)
            {
                SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)tag.c_str());
            }

            // Очищаем поле и блокируем кнопку
            SetWindowTextW(hEdit, L"");
            EnableWindow(GetDlgItem(hDlg, IDC_MANAGE_TAGS_BTN_ADD), FALSE);

            // Помечаем проект как изменённый
            appState->isDirty = true;

            return TRUE;
        }

        // Handle button (Rename)
        if (LOWORD(wParam) == IDC_MANAGE_TAGS_BTN_RENAME)
        {

            HWND hEdit = GetDlgItem(hDlg, IDC_MANAGE_TAGS_EDIT);
            HWND hList = GetDlgItem(hDlg, IDC_MANAGE_TAGS_LIST);

            if (!IsEditMode(hDlg))
            {
                // === РЕЖИМ: НАЖАЛИ "EDIT" ===
                int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR)
                {
                    MessageBoxW(hDlg, L"Please select a tag to edit.", L"Warning", MB_ICONWARNING);
                    return TRUE;
                }

                // Загружаем тег в Edit
                std::wstring tag = appState->availableTags[sel];
                SetWindowTextW(hEdit, tag.c_str());

                // Выделяем текст и ставим фокус
                SendMessageW(hEdit, EM_SETSEL, 0, -1);
                SetFocus(hEdit);

                // Переключаем в режим редактирования
                SetEditMode(hDlg, true);
                UpdateRenameButtonState(hDlg, hList); // Кнопка станет "Rename"

                return TRUE;
            }
            else
            {
                // === РЕЖИМ: НАЖАЛИ "RENAME" (СОХРАНЕНИЕ) ===

                // Получаем новый текст
                wchar_t buf[256] = {};
                GetWindowTextW(hEdit, buf, 256);
                std::wstring newTag(buf);

                if (newTag.empty())
                {
                    MessageBoxW(hDlg, L"Tag name cannot be empty.", L"Warning", MB_ICONWARNING);
                    return TRUE;
                }

                // Получаем старый тег
                int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR)
                    return TRUE;

                std::wstring oldTag = appState->availableTags[sel];

                // Если не изменился — просто выходим из режима
                if (oldTag == newTag)
                {
                    SetEditMode(hDlg, false);
                    SetWindowTextW(hEdit, L"");
                    UpdateAddButtonState(hDlg);
                    UpdateRenameButtonState(hDlg, hList);
                    return TRUE;
                }

                // Проверка на дубликат
                for (size_t i = 0; i < appState->availableTags.size(); ++i)
                {
                    if (i != (size_t)sel && appState->availableTags[i] == newTag)
                    {
                        std::wstring msg = L"Tag '" + newTag + L"' already exists.";
                        MessageBoxW(hDlg, msg.c_str(), L"Info", MB_ICONINFORMATION);
                        return TRUE;
                    }
                }

                //  Обновляем тег в кадрах
                for (auto &frame : appState->frames)
                {
                    if (frame.tag == oldTag)
                        frame.tag = newTag;
                }

                // Обновляем в списке и фильтре
                appState->availableTags[sel] = newTag;
                if (appState->currentFilterTag == oldTag)
                    appState->currentFilterTag = newTag;

                // Сортируем
                std::sort(appState->availableTags.begin(), appState->availableTags.end(), [](const std::wstring &a, const std::wstring &b) {
                    if (a == L"None" && b != L"None")
                        return true;
                    if (b == L"None" && a != L"None")
                        return false;
                    return a < b;
                });

                // Находим новый индекс после сортировки
                int newIndex = 0;
                for (size_t i = 0; i < appState->availableTags.size(); ++i)
                {
                    if (appState->availableTags[i] == newTag)
                    {
                        newIndex = (int)i;
                        break;
                    }
                }

                // Обновляем ListBox
                SendMessageW(hList, LB_RESETCONTENT, 0, 0);
                for (const auto &tag : appState->availableTags)
                    SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)tag.c_str());
                SendMessageW(hList, LB_SETCURSEL, newIndex, 0);

                // Сброс режима
                SetEditMode(hDlg, false);
                SetWindowTextW(hEdit, L"");
                EnableWindow(GetDlgItem(hDlg, IDC_MANAGE_TAGS_BTN_ADD), FALSE);
                UpdateRenameButtonState(hDlg, hList); // Кнопка вернётся в "Edit"

                appState->isDirty = true;
                return TRUE;
            }
        }

        // Handle button (Delete)
        if (LOWORD(wParam) == IDC_MANAGE_TAGS_BTN_DELETE)
        {
            HWND hList = GetDlgItem(hDlg, IDC_MANAGE_TAGS_LIST);

            // Получаем индекс выбранного элемента
            int selectedIndex = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
            if (selectedIndex == LB_ERR)
            {
                MessageBoxW(hDlg, L"Please select a tag to delete.", L"Warning", MB_ICONWARNING);
                return TRUE;
            }

            // Получаем название тега
            std::wstring tagToDelete = appState->availableTags[selectedIndex];

            // Защита системного тега "None"
            if (tagToDelete == L"None")
            {
                MessageBoxW(hDlg, L"The 'None' tag is system-defined and cannot be removed.", L"Access Denied", MB_ICONWARNING);
                return TRUE;
            }

            // Проверка: используется ли тег в кадрах?
            int usageCount = 0;
            for (const auto &frame : appState->frames)
            {
                if (frame.tag == tagToDelete)
                {
                    usageCount++;
                }
            }

            if (usageCount > 0)
            {
                // Блокируем удаление, если тег используется
                wchar_t msg[512];
                swprintf_s(msg,
                           L"Tag '%s' is used in %d frame(s).\n\n"
                           L"Please clear or change the tag in those frames before deleting.",
                           tagToDelete.c_str(), usageCount);

                MessageBoxW(hDlg, msg, L"Cannot Delete", MB_ICONWARNING | MB_OK);
                return TRUE;
            }

            // Подтверждение удаления (защита от случайного клика)
            wchar_t confirmMsg[256];
            swprintf_s(confirmMsg, L"Delete tag '%s'?", tagToDelete.c_str());

            if (MessageBoxW(hDlg, confirmMsg, L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) != IDYES)
            {
                return TRUE; // Пользователь отменил
            }

            // Удаляем тег из availableTags
            appState->availableTags.erase(appState->availableTags.begin() + selectedIndex);

            // Если этот тег был выбран в фильтре — сбрасываем на "All"
            if (appState->currentFilterTag == tagToDelete)
            {
                appState->currentFilterTag = L"";
            }

            // Обновляем ListBox
            SendMessageW(hList, LB_RESETCONTENT, 0, 0);
            for (const auto &tag : appState->availableTags)
            {
                SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)tag.c_str());
            }

            // Сбрасываем выделение и режим редактирования (если был активен)
            SendMessageW(hList, LB_SETCURSEL, -1, 0);
            if (IsEditMode(hDlg))
            {
                SetEditMode(hDlg, false);
                SetWindowTextW(GetDlgItem(hDlg, IDC_MANAGE_TAGS_EDIT), L"");
            }
            UpdateRenameButtonState(hDlg, hList);
            UpdateAddButtonState(hDlg);

            // Помечаем проект как изменённый
            appState->isDirty = true;

            return TRUE;
        }

        // Handle button (Cancel)
        if (LOWORD(wParam) == IDCANCEL)
        {

            RemovePropW(hDlg, PROP_EDIT_MODE);
            EndDialog(hDlg, LOWORD(wParam));

            return (INT_PTR)TRUE;
        }
    }

    break;

    case WM_CLOSE:
        RemovePropW(hDlg, PROP_EDIT_MODE);
        EndDialog(hDlg, IDCANCEL); // Закрываем диалог
        return TRUE;
    }

    return (INT_PTR)FALSE;
}

// checks if the text in the control is empty
bool IsEditEmpty(HWND hEdit)
{
    wchar_t buf[256];
    GetWindowTextW(hEdit, buf, 256);
    return (buf[0] == L'\0');
}

// updates the state of the Add button
void UpdateAddButtonState(HWND hDlg)
{
    HWND hEdit = GetDlgItem(hDlg, IDC_MANAGE_TAGS_EDIT);
    HWND hBtnAdd = GetDlgItem(hDlg, IDC_MANAGE_TAGS_BTN_ADD);

    if (hEdit && hBtnAdd)
    {
        // Если текст пустой — блокируем кнопку, иначе — разблокируем
        EnableWindow(hBtnAdd, !IsEditEmpty(hEdit));
    }
}

void UpdateRenameButtonState(HWND hDlg, HWND hList)
{
    HWND hBtn = GetDlgItem(hDlg, IDC_MANAGE_TAGS_BTN_RENAME);
    if (!hBtn)
        return;

    int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);

    if (IsEditMode(hDlg))
    {
        // Режим сохранения: кнопка "Rename", активна
        SetWindowTextW(hBtn, L"Rename");
        EnableWindow(hBtn, TRUE);
    }
    else if (sel != LB_ERR)
    {
        // Режим выбора: кнопка "Edit", активна
        SetWindowTextW(hBtn, L"Edit");
        EnableWindow(hBtn, TRUE);
    }
    else
    {
        // Ничего не выбрано: кнопка "Edit", заблокирована
        SetWindowTextW(hBtn, L"Edit");
        EnableWindow(hBtn, FALSE);
    }
}

bool IsEditMode(HWND hDlg)
{
    return GetPropW(hDlg, PROP_EDIT_MODE) != nullptr;
}

void SetEditMode(HWND hDlg, bool enabled)
{
    if (enabled)
    {
        SetPropW(hDlg, PROP_EDIT_MODE, (HANDLE)1);
    }

    else
    {
        RemovePropW(hDlg, PROP_EDIT_MODE);
    }
}

} // namespace baresprite