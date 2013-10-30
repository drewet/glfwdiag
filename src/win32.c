
#ifndef UNICODE
 #define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>

#include "diag.h"
#include "resource.h"

#define MAIN_WCL_NAME L"GLFWDIAG"

static struct
{
    HWND window;
    HWND edit;
} state;

static void error(void)
{
    exit(EXIT_FAILURE);
}

static void handle_menu_command(HWND window, int command)
{
    switch (command)
    {
        case IDM_EXIT:
        {
            DestroyWindow(window);
            break;
        }
    }
}

static LRESULT CALLBACK main_window_proc(HWND window,
                                         UINT message,
                                         WPARAM wParam,
                                         LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            handle_menu_command(window, LOWORD(wParam));
            return 0;
        }

        case WM_SIZE:
        {
            MoveWindow(state.edit, 0, 0, LOWORD(lParam), HIWORD(lParam), FALSE);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(window, message, wParam, lParam);
}

static int register_main_class(HINSTANCE instance)
{
    WNDCLASS wcl;
    ZeroMemory(&wcl, sizeof(wcl));

    wcl.style = CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = main_window_proc;
    wcl.hInstance = instance;
    wcl.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_GLFWDIAG));
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcl.lpszMenuName = MAKEINTRESOURCE(IDC_MAIN);
    wcl.lpszClassName = MAIN_WCL_NAME;

    return RegisterClass(&wcl);
}

static int create_main_window(HINSTANCE instance, int show)
{
    RECT client;
    HFONT font;

    state.window = CreateWindow(MAIN_WCL_NAME,
                                L"GLFW Diagnostics Tool",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, 0,
                                CW_USEDEFAULT, 0,
                                NULL, NULL, instance, NULL);
    if (!state.window)
        return FALSE;

    GetClientRect(state.window, &client);

    state.edit = CreateWindow(L"EDIT",
                              L"",
                              WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CHILD | ES_AUTOVSCROLL | ES_MULTILINE,
                              0, 0, client.right, client.bottom,
                              state.window, NULL, instance, NULL);
    if (!state.edit)
        return FALSE;

    font = CreateFont(0, 0, 0, 0, FW_NORMAL,
                      FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET,
                      OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS,
                      CLEARTYPE_QUALITY,
                      DEFAULT_PITCH,
                      L"Courier New");
    if (!font)
        return FALSE;

    SetWindowFont(state.edit, font, FALSE);
    Edit_LimitText(state.edit, 0);

    ShowWindow(state.window, show);
    UpdateWindow(state.window);

    return TRUE;
}

int APIENTRY WinMain(HINSTANCE instance,
                     HINSTANCE previous,
                     LPSTR commandLine,
                     int show)
{
    MSG msg;

    UNREFERENCED_PARAMETER(previous);
    UNREFERENCED_PARAMETER(commandLine);

    if (!register_main_class(instance))
        error();

    if (!create_main_window(instance, show))
        error();

    SetWindowTextA(state.edit, analyze());

    for (;;)
    {
        const BOOL result = GetMessage(&msg, NULL, 0, 0);

        if (result == 0)
            break;

        if (result == -1)
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    exit(EXIT_SUCCESS);
}

