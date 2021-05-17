#include "pch.h"
#include "resource.h"
#include "flmanager.h"
#include <stdio.h>
#include <string>

const UINT WM_TRAY = WM_USER + 1;
HINSTANCE g_hInst = NULL;
HICON g_hIcon = NULL;
HWINEVENTHOOK g_hook;
HHOOK g_MouseHook;
HHOOK g_KeyboardHook;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK    FlyoutWndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime);
void CALLBACK HandleWorkspaceSwitch(int i, HWND fl);
void CALLBACK HandleWorkspaceSet(int i, HWND fl);
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

void LoadStringSafe(UINT nStrID, LPTSTR szBuf, UINT nBufLen)
{
    UINT nLen = LoadString(g_hInst, nStrID, szBuf, nBufLen);
    if (nLen >= nBufLen)
        nLen = nBufLen - 1;
    szBuf[nLen] = 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ int nCmdShow) {
    WNDCLASS stWC;
    ZeroMemory(&stWC, sizeof(stWC));
    stWC.lpszClassName = _T("FLPlusDummyWin");

    HWND hHiddenWnd = FindWindow(stWC.lpszClassName, NULL);
    if (hHiddenWnd)
        {   } // don't start app if its already running
    else
    {
        stWC.hInstance = hInstance;
        stWC.lpfnWndProc = WndProc;
    
        ATOM aClass = RegisterClass(&stWC);
        if (aClass)
        {
            g_hInst = hInstance;
            if (hHiddenWnd = CreateWindow((LPCTSTR)aClass, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL))
            {
                // Register hotkeys
                RegisterHotKey(NULL, HOTKEY_AUTOMATION_ID, HOTKEY_AUTOMATION_MODIFIERS, HOTKEY_AUTOMATION_KEY);

                for (int i = 0; i < NUM_WORKSPACES; i++)
                {
                    RegisterHotKey(NULL, HOTKEY_WORKSPACE_SWITCH_ID + i, HOTKEY_WORKSPACE_SWITCH_MODIFIERS, HOTKEY_WORKSPACE_SWITCH_KEYSTART + i);
                    RegisterHotKey(NULL, HOTKEY_WORKSPACE_SET_ID + i, HOTKEY_WORKSPACE_SET_MODIFIERS, HOTKEY_WORKSPACE_SET_KEYSTART + i);
                }

                RegisterHotKey(NULL, HOTKEY_SET_COLOR_ID, HOTKEY_SET_COLOR_MOD, HOTKEY_SET_COLOR);
                RegisterHotKey(NULL, HOTKEY_SET_COLOR_AFTER_ID, HOTKEY_SET_COLOR_AFTER_MOD, HOTKEY_SET_COLOR_AFTER);

                MSG stMsg;
                while (GetMessage(&stMsg, NULL, 0, 0) > 0) 
                {
                    TranslateMessage(&stMsg);

                    // MAIN MESSAGE LOOP //

                    // Hotkeys
                    if (stMsg.message == WM_HOTKEY && FL::isFLChild(GetForegroundWindow()))
                    {
                        HWND win = GetForegroundWindow();
                        HWND fl = GetMainWindow(win);

                        switch (stMsg.wParam)
                        {
                        case HOTKEY_AUTOMATION_ID:
                        {
                            FL::CreateAutomationClip(fl);
                            break;
                        }
                        case HOTKEY_SET_COLOR_ID:
                        {
                            if (FL::isPianoRoll(win))
                            {
                                FL::OpenColorPane(win, false);
                            }
                            break;
                        }
                        case HOTKEY_SET_COLOR_AFTER_ID:
                        {
                            if (FL::isPianoRoll(win))
                            {
                                FL::OpenColorPane(win, true);
                            }
                            break;
                        }
                        default:
                        {
                        for (int i = 0; i < NUM_WORKSPACES; i++)
                        {
                            if (stMsg.wParam == HOTKEY_WORKSPACE_SWITCH_ID + i)
                            {
                                // Switch to workspace i
                                HandleWorkspaceSwitch(i, fl);
                            }
                            else if (stMsg.wParam == HOTKEY_WORKSPACE_SET_ID + i)
                            {
                                // Set workspace i
                                HandleWorkspaceSet(i, fl);
                            }
                        }
                        break;
                        }
                        }
                    }

                    DispatchMessage(&stMsg);
                }

                UnregisterHotKey(NULL, HOTKEY_AUTOMATION_ID);

                if (IsWindow(hHiddenWnd))
                    DestroyWindow(hHiddenWnd);
            }

            UnregisterClass((LPCTSTR)aClass, g_hInst);
        }
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            NOTIFYICONDATA stData;
            ZeroMemory(&stData, sizeof(stData));
            stData.cbSize = sizeof(stData);
            stData.hWnd = hwnd;
            stData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            stData.uCallbackMessage = WM_TRAY;
            stData.hIcon = g_hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
            LoadStringSafe(IDS_TIP, stData.szTip, _countof(stData.szTip));
            if (!Shell_NotifyIcon(NIM_ADD, &stData))
                return -1; // oops

            // Initialize FL Manager
            FL::Init();

            // Create Event Hooks
            g_hook = SetWinEventHook(
                EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
                NULL,                                          
                HandleWinEvent,          
                0, 0,              
                WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

            // Register windows hooks
            g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, g_hInst, 0);
            g_KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, g_hInst, 0);
        }
        return 0;

        case WM_DESTROY:
        {
            NOTIFYICONDATA stData;
            ZeroMemory(&stData, sizeof(stData));
            stData.cbSize = sizeof(stData);
            stData.hWnd = hwnd;
            Shell_NotifyIcon(NIM_DELETE, &stData);

            // Unload Event Hook
            UnhookWinEvent(g_hook);
        }
        return 0;

        case WM_TRAY:
        {
            switch (lParam)
            {
            case WM_RBUTTONUP:
            {
                HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_POPUP));
                if (hMenu)
                {
                    HMENU hSubMenu = GetSubMenu(hMenu, 0);
                    if (hSubMenu)
                    {
                        POINT stPoint;
                        GetCursorPos(&stPoint);

                        CheckMenuItem(hSubMenu, ID_EDISON_REACHPLAYBACK, FL::DoDisableReachPlayback() ? MF_CHECKED : MF_UNCHECKED);

                        TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, stPoint.x, stPoint.y, 0, hwnd, NULL);
                    }

                    DestroyMenu(hMenu);
                }
            }
            break;
            }
            return 0;
        }
        return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case ID_EDISON_REACHPLAYBACK:
                    FL::ToggleDisableReachPlayback();
                    return 0;
                case ID_EXIT:
                    PostQuitMessage(0);
                    return 0;
                }
        break;
    }
    

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime) 
{
    switch (event)
    {
    case EVENT_SYSTEM_FOREGROUND:
        {
        HWND curr = GetForegroundWindow();
        if (FL::isEdison(curr))
        {
            Sleep(540);
            FL::DeactivateReachPlayback(curr);
        }
        }
        break;
    }
}

void CALLBACK HandleWorkspaceSwitch(int i, HWND fl)
{
    int rCode = FL::LoadLocalWorkspace(i, fl);
    
    switch (rCode)
    {
    case WRC_NOTDETACHED:
        WARNING_BOX(_T("Windows are not detached!"), NULL);
        break;
    case WRC_REGDOESNTEXIST:
        WARNING_BOX(_T("Workspace has not been set."), NULL);
        break;
    case WRC_REGERROR:
        WARNING_BOX(_T("Workspace has not been set."), NULL);
        break;
    case WRC_FAILURE:
        WARNING_BOX(_T("Unexpected error"), NULL);
        break;
    default:
        break;
    }
}

void CALLBACK HandleWorkspaceSet(int i, HWND fl)
{
    int rCode = FL::SetLocalWorkspace(i, fl);

    switch (rCode)
    {
    case WRC_SUCCESS:
        INFO_BOX(_T("Workspace successfully set!"), NULL);
        break;
    case WRC_NOTDETACHED:
        WARNING_BOX(_T("Windows are not detached!"), NULL);
        break;
    default:
        WARNING_BOX(_T("Unsuccessful. Please report this to a developer."), NULL);
        break;
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)(lParam);
    POINT p;
    
    if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP)
    {
        FL::CloseColorPane();
    }

    return 0;
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)(lParam);

    if (wParam == WM_KEYDOWN )//&& k->vkCode != HOTKEY_SET_COLOR && k->vkCode != VK_LSHIFT && k->vkCode != VK_RSHIFT)
    {
        FL::CloseColorPane();
    }

    return 0;
}