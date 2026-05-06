#pragma once

#include "Frame.h"
#include "ReorderDlgData.h"
#include <Windows.h>

namespace baresprite
{
static void DrawFrameThumbnail(HDC hdc, const Frame &frame, RECT &dstRect);

static void UpdateScrollRange(ReorderDlgData *data);

static void UpdateVisibleIndices(ReorderDlgData *data);

static int HitTestVisible(ReorderDlgData *data, int mouseX);

// Message handler for Reorder Frames Dialog box.
INT_PTR CALLBACK ReorderFramesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ScrollAreaSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

} // namespace baresprite