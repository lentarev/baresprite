// baresprite.cpp : Defines the entry point for the application.
//

#include "baresprite.h"
#include "framework.h"
#include <commctrl.h>
#include <iostream>
#include <shlwapi.h>

#include "AppSettings.h"
#include "AppState.h"
#include "BottomToolbar.h"
#include "Canvas.h"
#include "CanvasScrollView.h"
#include "FramePanel.h"
#include "LeftToolbar.h"
#include "ProjectSettings.h"
#include "RightToolbar.h"
#include "ask_save_dialog.h"
#include "load_project_dialog_proc.h"
#include "new_project_dialog_proc.h"
#include "start_screen_dialog_proc.h"

// Manifesto and Libraries.
#pragma comment(                                                                                                                                               \
    linker,                                                                                                                                                    \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Msimg32.lib")

using namespace baresprite;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

HWND gHWnd = nullptr;

// Глобальные переменные для скролла (или добавь в класс главного окна)
static int g_ScrollX = 0;
static int g_ScrollY = 0;

// All data of project
std::unique_ptr<AppState> gAppState;
std::unique_ptr<LeftToolbar> gLeftToolbar;
std::unique_ptr<BottomToolbar> gBottomToolbar;
std::unique_ptr<RightToolbar> gRightToolbar;
std::unique_ptr<CanvasScrollView> gCanvasScrollView;
std::unique_ptr<ProjectSettings> gProjectSettings;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

#if _DEBUG || defined(_ENABLE_ASAN_CONSOLE)
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char **argv)
{
#if _DEBUG
    // disable buffering
    setvbuf(stdout, nullptr, _IONBF, 0);
#endif

    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWDEFAULT);
}
#else

#pragma comment(linker, "/subsystem:windows")

#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    //  Initialization of modern controls
    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BARESPRITE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    gAppState = std::make_unique<AppState>();
    gProjectSettings = std::make_unique<ProjectSettings>(*gAppState);

    // Initializing the project data change flag
    gAppState->isDirty = false;

    auto appSettings = std::make_unique<AppSettings>(*gAppState);

    if (appSettings->Load())
    {
        gAppState->isExistAppConfig = true;
    }
    else
    {
        gAppState->isExistAppConfig = false;
    }

    // === Startup Loop ===
    bool launchEditor = false;

    while (!launchEditor)
    {
        // Show start screen dialog
        INT_PTR startResult = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_START_SCREEN), nullptr, StartScreenDialogProc);

        // If you closed the start screen (cross / Esc / Cancel), exit the application
        if (startResult == IDCANCEL || startResult == 0)
        {
            return 0;
        }

        if (startResult == IDOK) // New Project
        {
            // Show new project dialog
            INT_PTR newProjectResult =
                DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_NEW_PROJECT), nullptr, NewProjectDialogProc, reinterpret_cast<LPARAM>(gAppState.get()));

            if (newProjectResult == IDOK)
            {
                if (appSettings->IsProjectExist(gAppState->projectPath))
                {
                    MessageBox(nullptr, L"The project you are trying to create already exists.", L"Error", MB_OK | MB_ICONEXCLAMATION);
                }
                else
                {
                    // Saves information in config.ini about the new project.
                    appSettings->Save();

                    // Создаем конфиг для нового проека
                    gProjectSettings->Save();

                    // Initialization
                    gAppState->frames.emplace_back(gAppState->imageSize, gAppState->imageSize);
                    gAppState->availableTags = {L"None", L"Idle", L"Walk", L"Run", L"Jump", L"Die", L"Attack"};

                    launchEditor = true;

                    gAppState->isProjectLoadedFromConfig = false;
                }
            }
        }
        else if (startResult == IDC_BUTTON_LOAD_PROJECT)
        {
            // Show Load Project dialog
            INT_PTR loadProjectResult =
                DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_LOAD_PROJECT), nullptr, LoadProjectDialogProc, reinterpret_cast<LPARAM>(gAppState.get()));

            if (loadProjectResult == IDOK)
            {
                // Check is project exists
                if (appSettings->IsProjectExist(gAppState->projectPath))
                {
                    // Load project
                    if (gProjectSettings->Load())
                    {
                        // Saves information in config.ini about the loaded project.
                        appSettings->Save();

                        // Initialization
                        if (gAppState->currentFrameIndex < 0 || gAppState->currentFrameIndex >= static_cast<int>(gAppState->frames.size()))
                        {
                            gAppState->currentFrameIndex = 0;
                        }

                        launchEditor = true;

                        gAppState->isProjectLoadedFromConfig = true;
                    }
                }
                else
                {
                    MessageBox(nullptr, L"The project you are trying to load is missing configuration data.", L"Error", MB_OK | MB_ICONEXCLAMATION);
                }
            }
        }
    }

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BARESPRITE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(gHWnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_BARESPRITE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_BARESPRITE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_BARESPRITE));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    gHWnd = hWnd;

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Create left toolbar child window
    gLeftToolbar = std::make_unique<LeftToolbar>(hWnd, hInstance, *gAppState);

    // Create right toolbar child window
    gRightToolbar = std::make_unique<RightToolbar>(hWnd, hInstance);

    // Create canvas scroll view container
    gCanvasScrollView = std::make_unique<CanvasScrollView>(hWnd, hInstance, *gAppState);

    // Create frame toolbar child window
    gBottomToolbar = std::make_unique<BottomToolbar>(hWnd, hInstance, *gAppState);

    // Если проект был загружен из файла конфигурации
    if (gAppState->isProjectLoadedFromConfig)
    {
        if (gAppState->canvas && !gAppState->frames.empty())
        {
            gAppState->canvas->LoadFrame(gAppState->frames[gAppState->currentFrameIndex]);
        }
    }

    RECT rc;

    GetClientRect(hWnd, &rc); // Get the current size of the client area

    if (gLeftToolbar)
    {
        gLeftToolbar->OnSize(rc.right, rc.bottom);
    }

    if (gBottomToolbar)
    {
        gBottomToolbar->OnSize(rc.right, rc.bottom);
    }

    if (gRightToolbar)
    {
        gRightToolbar->OnSize(rc.right, rc.bottom);
    }

    if (gCanvasScrollView)
    {
        gCanvasScrollView->OnSize(rc.right, rc.bottom);
    }

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam); // notification code (BN_CLICKED or BN_DBLCLK)

        // CUT
        if (wmId == ID_EDIT_CUT)
        {
            if (gAppState->canvas)
            {
                gAppState->canvas->OnCut();
            }
            return 0;
        }

        // COPY
        if (wmId == ID_EDIT_COPY)
        {
            if (gAppState->canvas)
            {
                gAppState->canvas->OnCopy();
            }
            return 0;
        }

        // PASTE
        if (wmId == ID_EDIT_PASTE)
        {
            if (gAppState->canvas)
            {
                gAppState->canvas->OnPaste();
            }
            return 0;
        }

        // UNDO
        if (wmId == ID_EDIT_UNDO || wmId == ID_UNDO)
        {
            if (gAppState->canvas)
            {
                gAppState->canvas->OnUndo();
            }
            return 0;
        }

        // REDO
        if (wmId == ID_EDIT_REDO || wmId == ID_REDO)
        {
            if (gAppState->canvas)
            {
                gAppState->canvas->OnRedo();
            }
            return 0;
        }

        if (wmId == ID_PREV_FRAME)
        {
            if (gBottomToolbar)
            {
                gBottomToolbar->GetFramePanel()->OnButtonPrev();
            }
            return 0;
        }

        // ID_NEXT_FRAME
        if (wmId == ID_NEXT_FRAME)
        {
            if (gBottomToolbar)
            {
                gBottomToolbar->GetFramePanel()->OnButtonNext();
            }
            return 0;
        }

        if (wmId == ID_CURSOR_IN)
        {
            if (gCanvasScrollView)
            {
                gCanvasScrollView->GetCanvas()->IncreaseBrushSize();
            }
            return 0;
        }

        if (wmId == ID_CURSOR_OUT)
        {
            if (gCanvasScrollView)
            {
                gCanvasScrollView->GetCanvas()->DecreaseBrushSize();
            }
            return 0;
        }

        if (wmId == ID_ZOOM_IN || wmId == ID_VIEW_ZOOMIN)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            if (gCanvasScrollView)
            {
                if (gCanvasScrollView->GetCanvas()->ZoomIn())
                {
                    gCanvasScrollView->RecalculateCanvasCentering();
                }
            }

            return 0;
        }

        if (wmId == ID_ZOOM_OUT || wmId == ID_VIEW_ZOOMOUT)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            if (gCanvasScrollView)
            {
                if (gCanvasScrollView->GetCanvas()->ZoomOut())
                {
                    gCanvasScrollView->RecalculateCanvasCentering();
                }
            }

            return 0;
        }

        if (wmId == ID_TOOL_BRUSH)
        {

            if (gLeftToolbar)
            {
                gLeftToolbar->SelectTool(0);
            }

            return 0;
        }

        if (wmId == ID_TOOL_ERASER)
        {

            if (gLeftToolbar)
            {
                gLeftToolbar->SelectTool(1);
            }

            return 0;
        }

        if (wmId == ID_TOOL_SELECT)
        {

            if (gLeftToolbar)
            {
                gLeftToolbar->SelectTool(2);
            }

            return 0;
        }

        if (wmId == ID_TOOL_FILL)
        {

            if (gLeftToolbar)
            {
                gLeftToolbar->SelectTool(3);
            }

            return 0;
        }

        // LeftToolbar
        if (gLeftToolbar && gLeftToolbar->OnCommand(wmId, wmEvent))
        {
            return 0; // Command processed.
        }

        // BottomToolbar
        if (gBottomToolbar && gBottomToolbar->OnCommand(wmId, wmEvent))
        {
            return 0; // Command processed.
        }

        // RightToolbar
        if (gRightToolbar && gRightToolbar->OnCommand(wmId, wmEvent))
        {
            return 0; // Command processed.
        }

        switch (wmId)
        {
        case IDM_ABOUT:

            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            if (gAppState && gProjectSettings && AskSaveDialog(hWnd, *gAppState, *gProjectSettings))
            {
                DestroyWindow(hWnd);
            }

            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

        SetFocus(hWnd);
        break;

    case WM_HSCROLL: {
        int csrollCode = LOWORD(wParam);
        HWND hSlider = (HWND)lParam;

        // BottomToolbar
        if (gBottomToolbar && gBottomToolbar->OnHScroll(csrollCode, hSlider))
        {
            return 0; // HScroll processed.
        }
    }

    break;

    case WM_SIZE:

        if (wParam != SIZE_MINIMIZED)
        {
            if (gLeftToolbar)
            {
                gLeftToolbar->OnSize(LOWORD(lParam), HIWORD(lParam));
            }

            if (gBottomToolbar)
            {
                gBottomToolbar->OnSize(LOWORD(lParam), HIWORD(lParam));
            }

            if (gRightToolbar)
            {
                gRightToolbar->OnSize(LOWORD(lParam), HIWORD(lParam));
            }

            if (gCanvasScrollView)
            {
                gCanvasScrollView->OnSize(LOWORD(lParam), HIWORD(lParam));
            }
        }

        return 0;

    case WM_SETCURSOR: {
        if (LOWORD(lParam) != HTCLIENT)
        {
            if (gCanvasScrollView && gCanvasScrollView->GetCanvas())
            {
                gCanvasScrollView->GetCanvas()->SetCustomCursor(false);
            }
            // Системный курсор восстановится через DefWindowProc
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        // Курсор над клиентской областью — проверяем, над канвасом ли
        POINT screenPos;
        GetCursorPos(&screenPos);
        HWND hwndUnderCursor = WindowFromPoint(screenPos);

        if (gCanvasScrollView && gCanvasScrollView->GetCanvas() && hwndUnderCursor == gCanvasScrollView->GetCanvas()->GetHWndCanvas())
        {

            // Конвертируем в координаты канваса
            POINT clientPos = screenPos;
            ScreenToClient(gCanvasScrollView->GetCanvas()->GetHWndCanvas(), &clientPos);

            // Логические координаты
            int logX = clientPos.x / gAppState->checkerSize;
            int logY = clientPos.y / gAppState->checkerSize;

            // Проверка границ холста
            bool isInside = false;
            if (!gAppState->frames.empty())
            {

                isInside = (logX >= 0 && logX < gAppState->imageSize && logY >= 0 && logY < gAppState->imageSize);
            }

            if (isInside)
            {
                // Внутри холста: показываем кастомный курсор
                gCanvasScrollView->GetCanvas()->SetCustomCursor(true);
                SetCursor(nullptr); // Скрыть системный
            }
            else
            {
                // На серой области: системный курсор
                gCanvasScrollView->GetCanvas()->SetCustomCursor(false);
                SetCursor(LoadCursor(nullptr, IDC_ARROW));
            }
            return TRUE;
        }
        else
        {
            // Курсор над тулбаром или другой панелью — системный курсор
            if (gCanvasScrollView && gCanvasScrollView->GetCanvas())
            {
                gCanvasScrollView->GetCanvas()->SetCustomCursor(false);
            }
            // Системный курсор восстановится автоматически
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = reinterpret_cast<MINMAXINFO *>(lParam);

        // Минимальный размер клиентской области (без рамок)
        constexpr int MIN_CLIENT_W = 900;
        constexpr int MIN_CLIENT_H = 650;

        RECT rc = {0, 0, MIN_CLIENT_W, MIN_CLIENT_H};

        AdjustWindowRect(&rc, (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE), FALSE);

        mmi->ptMinTrackSize.x = rc.right - rc.left;
        mmi->ptMinTrackSize.y = rc.bottom - rc.top;

        return 0;
    }

    case WM_CLOSE: {
        if (gAppState && gProjectSettings && AskSaveDialog(hWnd, *gAppState, *gProjectSettings))
        {
            DestroyWindow(hWnd);
        }

        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
