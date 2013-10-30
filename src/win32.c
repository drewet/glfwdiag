
#ifndef UNICODE
 #define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>
#include <stdio.h>

#include "diag.h"
#include "resource.h"

#define MAIN_WCL_NAME L"GLFWDIAG"

static struct
{
    HINSTANCE instance;
    HWND window;
    HWND edit;
} state;

static void error(void)
{
    exit(EXIT_FAILURE);
}

static WCHAR* utf16_from_utf8(const char* source)
{
    WCHAR* target;
    int length;

    length = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!length)
        return NULL;

    target = malloc(sizeof(WCHAR) * (length + 1));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, length + 1))
    {
        free(target);
        return NULL;
    }

    return target;
}

static char* utf8_from_utf16(const WCHAR* source)
{
    char* target;
    int length;

    length = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!length)
        return NULL;

    target = malloc(length + 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, length + 1, NULL, NULL))
    {
        free(target);
        return NULL;
    }

    return target;
}

static char* get_window_text_utf8(HWND window)
{
    char* text = NULL;
    WCHAR* wideText;
    size_t wideLength;

    wideLength = GetWindowTextLength(window);
    wideText = calloc(wideLength + 1, sizeof(WCHAR));

    if (GetWindowText(window, wideText, wideLength + 1))
        text = utf8_from_utf16(wideText);

    free(wideText);
    return text;
}

static void handle_menu_command(int command)
{
    switch (command)
    {
        case IDM_SAVEAS:
        {
            WCHAR path[MAX_PATH + 1];

            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(ofn));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = state.window;
            ofn.hInstance = state.instance;
            ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0"
                              L"All Files (*.*)\0*.*\0\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = path;
            ofn.nMaxFile = sizeof(path) / sizeof(WCHAR);
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
            ofn.lpstrDefExt = L"txt";

            wcscpy(path, L"GLFWDIAG.txt");

            if (GetSaveFileName(&ofn))
            {
                char* text = get_window_text_utf8(state.edit);
                if (text)
                {
                    FILE* file = _wfopen(path, L"wb");
                    if (file)
                    {
                        fwrite(text, 1, strlen(text), file);
                        fclose(file);
                    }

                    free(text);
                }
            }

            break;
        }

        case IDM_EXIT:
        {
            DestroyWindow(state.window);
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
            handle_menu_command(LOWORD(wParam));
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

static int register_main_class(void)
{
    WNDCLASS wcl;
    ZeroMemory(&wcl, sizeof(wcl));

    wcl.style = CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = main_window_proc;
    wcl.hInstance = state.instance;
    wcl.hIcon = LoadIcon(state.instance, MAKEINTRESOURCE(IDI_GLFWDIAG));
    wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcl.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcl.lpszMenuName = MAKEINTRESOURCE(IDC_MAIN);
    wcl.lpszClassName = MAIN_WCL_NAME;

    return RegisterClass(&wcl);
}

static int create_main_window(int show)
{
    RECT client;
    HFONT font;

    state.window = CreateWindowEx(WS_EX_APPWINDOW,
                                  MAIN_WCL_NAME,
                                  L"GLFW Diagnostics Tool",
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, 0,
                                  CW_USEDEFAULT, 0,
                                  NULL, NULL, state.instance, NULL);
    if (!state.window)
        return FALSE;

    GetClientRect(state.window, &client);

    state.edit = CreateWindowEx(WS_EX_CLIENTEDGE,
                                L"EDIT",
                                L"",
                                WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CHILD |
                                    ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
                                0, 0, client.right, client.bottom,
                                state.window, NULL, state.instance, NULL);
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
    WCHAR* analysis;

    UNREFERENCED_PARAMETER(previous);
    UNREFERENCED_PARAMETER(commandLine);

    ZeroMemory(&state, sizeof(state));
    state.instance = instance;

    if (!register_main_class())
        error();

    if (!create_main_window(show))
        error();

    analysis = utf16_from_utf8(analyze());
    if (!analysis)
        error();

    SetWindowText(state.edit, analysis);
    free(analysis);

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

