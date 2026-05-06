#define NOMINMAX

#include "reorder_frames_dlg_proc.h"
#include "AppState.h"
#include "resource.h"
#include "windowsx.h"
#include <algorithm>
#include <commctrl.h>
#include <iostream>
#include <numeric>
#include <unordered_set>

namespace baresprite
{

INT_PTR CALLBACK ReorderFramesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static ReorderDlgData *data = nullptr;

    switch (message)
    {
    case WM_INITDIALOG: {
        data = reinterpret_cast<ReorderDlgData *>(lParam);

        if (!data)
        {

            return TRUE;
        }

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

        data->hDlg = hDlg;
        data->hScrollArea = GetDlgItem(hDlg, IDC_REORDER_AREA);
        data->hScrollBar = GetDlgItem(hDlg, IDC_REORDER_SCROLL);
        data->hComboTag = GetDlgItem(hDlg, IDC_REORDER_FRAME_COMBO_TAG_FILTER);

        // Инициализация порядка
        data->frameOrder.resize(data->appState->frames.size());
        std::iota(data->frameOrder.begin(), data->frameOrder.end(), 0);

        // >>>>>>>>>>>>>>>>>>>>>>>>>>
        // Dropdown List

        // Всегда первый: All
        ComboBox_AddString(data->hComboTag, L"All");

        // Собираем используемые теги
        std::unordered_set<std::wstring> usedTags;
        bool hasNone = false;

        for (const auto &frame : data->appState->frames)
        {
            std::wstring tag = frame.tag.empty() ? L"None" : frame.tag;
            if (tag != L"All")
            {
                if (tag == L"None")
                    hasNone = true;
                usedTags.insert(tag);
            }
        }

        // Второй: None (если есть кадры без тега)
        if (hasNone)
        {
            ComboBox_AddString(data->hComboTag, L"None");
            usedTags.erase(L"None"); // Убираем, чтобы не дублировать при сортировке
        }

        // Остальные: строго по алфавиту
        std::vector<std::wstring> sortedTags(usedTags.begin(), usedTags.end());
        std::sort(sortedTags.begin(), sortedTags.end());

        for (const auto &tag : sortedTags)
        {
            ComboBox_AddString(data->hComboTag, tag.c_str());
        }

        // По умолчанию выбран All
        ComboBox_SetCurSel(data->hComboTag, 0);

        data->currentFilterTag = L"All";

        // Подменяем процедуру для области отрисовки (STATIC)
        SetWindowSubclass(data->hScrollArea, ScrollAreaSubclassProc, 0, reinterpret_cast<DWORD_PTR>(data));

        UpdateScrollRange(data); // Инициализируем скроллбар
        UpdateVisibleIndices(data);
        return TRUE;
    }

    case WM_HSCROLL: {
        if ((HWND)lParam != data->hScrollBar)
            return 0;

        int code = LOWORD(wParam);
        int newPos = data->scrollPos; // Начинаем с текущей позиции

        switch (code)
        {
        case SB_LINELEFT:
            newPos -= 20;
            break;
        case SB_LINERIGHT:
            newPos += 20;
            break;
        case SB_PAGELEFT:
            newPos -= 100;
            break;
        case SB_PAGERIGHT:
            newPos += 100;
            break;
        case SB_THUMBTRACK:
            // Для SB_THUMBTRACK читаем nTrackPos
            {
                SCROLLINFO si = {sizeof(SCROLLINFO)};
                si.fMask = SIF_TRACKPOS;
                GetScrollInfo(data->hScrollBar, SB_CTL, &si);
                newPos = si.nTrackPos;
            }
            break;
        case SB_THUMBPOSITION:
            newPos = HIWORD(wParam);
            break;
        case SB_LEFT:
            newPos = 0;
            break;
        case SB_RIGHT:
            newPos = data->maxScroll;
            break;
        default:
            return 0;
        }

        // Ограничиваем
        newPos = std::max(0, std::min(newPos, data->maxScroll));

        // Обновляем
        if (newPos != data->scrollPos)
        {
            data->scrollPos = newPos;

            // Просто устанавливаем позицию
            SetScrollPos(data->hScrollBar, SB_CTL, newPos, TRUE);

            // Перерисовываем кадры
            InvalidateRect(data->hScrollArea, nullptr, FALSE);
        }
        return TRUE;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == IDC_REORDER_FRAME_COMBO_TAG_FILTER && code == CBN_SELCHANGE)
        {
            int sel = static_cast<int>(SendMessageW(data->hComboTag, CB_GETCURSEL, 0, 0));
            wchar_t buf[64] = {0};
            SendMessageW(data->hComboTag, CB_GETLBTEXT, sel, (LPARAM)buf);
            data->currentFilterTag = buf;
            UpdateVisibleIndices(data);
            return 0;
        }

        if (id == IDOK)
        {
            std::vector<Frame> newFrames;
            newFrames.reserve(data->frameOrder.size());
            for (int idx : data->frameOrder)
                newFrames.push_back(std::move(data->appState->frames[idx]));

            data->appState->frames = std::move(newFrames);

            int total = static_cast<int>(data->appState->frames.size());
            if (data->appState->currentFrameIndex >= total)
                data->appState->currentFrameIndex = std::max(0, total - 1);

            data->appState->isDirty = true;
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (id == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
}

LRESULT CALLBACK ScrollAreaSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    ReorderDlgData *data = reinterpret_cast<ReorderDlgData *>(dwRefData);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        // Создаём буфер в памяти
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        HGDIOBJ oldBmp = SelectObject(memDC, hMemBmp);

        // Рисуем фон в буфер
        FillRect(memDC, &clientRect, GetSysColorBrush(COLOR_3DFACE));

        // Рисуем кадры в буфер
        int currentX = data->padding - data->scrollPos;
        int startY = data->padding;

        for (int i = 0; i < (int)data->visibleIndices.size(); ++i)
        {
            int frameIdx = data->visibleIndices[i];
            if (frameIdx < 0 || frameIdx >= (int)data->appState->frames.size())
                continue;

            const Frame &frame = data->appState->frames[frameIdx];
            RECT thumbRect = {currentX, startY, currentX + data->thumbW, startY + data->thumbH};

            DrawFrameThumbnail(memDC, frame, thumbRect);

            // Рамка
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, thumbRect.left, thumbRect.top, thumbRect.right, thumbRect.bottom);
            SelectObject(memDC, hOldPen);
            DeleteObject(hPen);

            // Индикатор вставки
            if (data->isDragging && data->dropVisIdx == i)
            {
                HPEN hDropPen = CreatePen(PS_SOLID, 3, RGB(255, 100, 50));
                SelectObject(memDC, hDropPen);
                int lineX = thumbRect.left - data->padding / 2;
                MoveToEx(memDC, lineX, thumbRect.top, nullptr);
                LineTo(memDC, lineX, thumbRect.bottom);
                DeleteObject(hDropPen);
            }

            // Текст
            wchar_t infoStr[32];
            swprintf_s(infoStr, L"%d", frameIdx + 1);
            SetBkMode(memDC, TRANSPARENT);
            TextOutW(memDC, thumbRect.left + 4, thumbRect.bottom - 16, infoStr, static_cast<int>(wcslen(infoStr)));

            currentX += data->thumbW + data->padding;
        }

        // Копируем готовый кадр на экран ОДНОЙ операцией
        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);

        // Очистка
        SelectObject(memDC, oldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        // Проверяем, что клик НЕ в зоне скролла (нижние 5 пикселей)
        RECT rc;
        GetClientRect(hWnd, &rc);
        if (y > rc.bottom - 10) // Если кликнули в нижние 10px — игнорируем
            return 0;

        int visIdx = HitTestVisible(data, x);
        if (visIdx != -1)
        {
            data->isDragging = true;
            data->dragVisIdx = visIdx;
            data->dropVisIdx = visIdx;
            SetCapture(hWnd);
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (data->isDragging)
        {
            int x = GET_X_LPARAM(lParam);
            int newDrop = HitTestVisible(data, x);
            if (newDrop != data->dropVisIdx && newDrop != -1 && newDrop != data->dragVisIdx)
            {
                data->dropVisIdx = newDrop;
                InvalidateRect(hWnd, nullptr, FALSE);
            }
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        if (data->isDragging)
        {
            ReleaseCapture();
            if (data->dropVisIdx != -1 && data->dropVisIdx != data->dragVisIdx)
            {
                int srcFrame = data->visibleIndices[data->dragVisIdx];
                int dstFrame = data->visibleIndices[data->dropVisIdx];

                auto itSrc = std::find(data->frameOrder.begin(), data->frameOrder.end(), srcFrame);
                auto itDst = std::find(data->frameOrder.begin(), data->frameOrder.end(), dstFrame);

                if (itSrc != data->frameOrder.end() && itDst != data->frameOrder.end())
                    std::iter_swap(itSrc, itDst);

                UpdateVisibleIndices(data);
            }
            data->isDragging = false;
            data->dragVisIdx = -1;
            data->dropVisIdx = -1;
        }
        return 0;
    }

    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, ScrollAreaSubclassProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void DrawFrameThumbnail(HDC hdc, const Frame &frame, RECT &dstRect)
{
    if (frame.width == 0 || frame.height == 0)
        return;

    int bmpW = dstRect.right - dstRect.left;
    int bmpH = dstRect.bottom - dstRect.top;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, bmpW, bmpH);
    HGDIOBJ oldBmp = SelectObject(memDC, hBmp);

    // Шахматный фон (рисуем относительно (0,0) битмапа)
    int checkerSize = 4;
    HBRUSH hLight = CreateSolidBrush(RGB(240, 240, 240));
    HBRUSH hDark = CreateSolidBrush(RGB(200, 200, 200));
    for (int y = 0; y < bmpH; y += checkerSize)
    {
        for (int x = 0; x < bmpW; x += checkerSize)
        {
            bool isLight = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            RECT cell = {x, y, std::min(x + checkerSize, bmpW), std::min(y + checkerSize, bmpH)};
            FillRect(memDC, &cell, isLight ? hLight : hDark);
        }
    }
    DeleteObject(hLight);
    DeleteObject(hDark);

    // Масштабирование
    float scaleX = (float)bmpW / frame.width;
    float scaleY = (float)bmpH / frame.height;
    float scale = std::min(scaleX, scaleY);

    int offsetX = (int)((bmpW - frame.width * scale) / 2);
    int offsetY = (int)((bmpH - frame.height * scale) / 2);

    // Отрисовка пикселей кадра
    for (int y = 0; y < frame.height; ++y)
    {
        for (int x = 0; x < frame.width; ++x)
        {
            uint32_t pixel = frame.GetPixel(x, y);
            if ((pixel >> 24) & 0xFF)
            {
                COLORREF col = RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);

                // Точные границы destination-пикселя
                int destX = offsetX + (int)(x * scale);
                int destY = offsetY + (int)(y * scale);
                int destW = std::max(1, (int)((x + 1) * scale) - (int)(x * scale));
                int destH = std::max(1, (int)((y + 1) * scale) - (int)(y * scale));

                // Рисуем блок пикселей ровно под рассчитанный размер
                for (int dy = 0; dy < destH; ++dy)
                    for (int dx = 0; dx < destW; ++dx)
                        SetPixelV(memDC, destX + dx, destY + dy, col);
            }
        }
    }

    // Копируем на экран
    BitBlt(hdc, dstRect.left, dstRect.top, bmpW, bmpH, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
}

static void UpdateScrollRange(ReorderDlgData *data)
{
    RECT rc;
    GetClientRect(data->hScrollArea, &rc);
    int clientW = rc.right - rc.left;
    int totalW = (int)data->visibleIndices.size() * (data->thumbW + data->padding);

    data->maxScroll = std::max(0, totalW - clientW);

    SCROLLINFO si = {sizeof(SCROLLINFO)};
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = totalW;   // Общая длина ВСЕХ кадров
    si.nPage = clientW; // Видимая область (ширина контрола)
    si.nPos = 0;

    SetScrollInfo(data->hScrollBar, SB_CTL, &si, TRUE);
}

void UpdateVisibleIndices(ReorderDlgData *data)
{
    data->visibleIndices.clear();
    for (int idx : data->frameOrder)
    {
        if (idx < 0 || idx >= (int)data->appState->frames.size())
            continue;
        const Frame &f = data->appState->frames[idx];
        std::wstring tag = f.tag.empty() ? L"None" : f.tag;
        if (data->currentFilterTag == L"All" || tag == data->currentFilterTag)
            data->visibleIndices.push_back(idx);
    }
    data->scrollPos = 0;
    UpdateScrollRange(data);
    InvalidateRect(data->hScrollArea, nullptr, FALSE);
}

int HitTestVisible(ReorderDlgData *data, int mouseX)
{
    int logicalX = mouseX + data->scrollPos;
    int visIdx = logicalX / (data->thumbW + data->padding);
    return (visIdx >= 0 && visIdx < (int)data->visibleIndices.size()) ? visIdx : -1;
}

} // namespace baresprite