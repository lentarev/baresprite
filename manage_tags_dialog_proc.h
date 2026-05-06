#pragma once

#include <Windows.h>

namespace baresprite
{

constexpr LPCWSTR PROP_EDIT_MODE = L"EditMode";

// Message handler for Manage Tags Dialog box.
INT_PTR CALLBACK ManageTagsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

bool IsEditEmpty(HWND hEdit);
void UpdateAddButtonState(HWND hDlg);
void UpdateRenameButtonState(HWND hDlg, HWND hList);
bool IsEditMode(HWND hDlg);
void SetEditMode(HWND hDlg, bool enabled);


} // namespace baresprite
