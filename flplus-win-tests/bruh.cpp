// QuickCodePage.cpp : Defines the entry point for the application.
//

#include "pch.h"

/*
#include "stdafx.h"
#include "resource.h"

#define _countof(x) (sizeof(x) / sizeof((x)[0]))

const UINT WM_TRAY = WM_USER + 1;
HINSTANCE g_hInstance = NULL;
HICON g_hIcon = NULL;

const TCHAR g_szRegKeyName[] = _T("SOFTWARE\\Misc\\Quick Code-Page Converter");
const TCHAR g_szRegDefCodePage[] = _T("Default Code-Page");
const TCHAR g_szRegPlacement[] = _T("Popup Dlg Placement");

void LoadStringSafe(UINT nStrID, LPTSTR szBuf, UINT nBufLen)
{
	UINT nLen = LoadString(g_hInstance, nStrID, szBuf, nBufLen);
	if (nLen >= nBufLen)
		nLen = nBufLen - 1;
	szBuf[nLen] = 0;
}

HWND g_hCodePages = NULL;
long g_nLastCodePage = 0;

struct CWndSizeData {
	HWND m_hWnd;
	RECT m_stRect;
	int m_nResizeFlags;
};

struct CPopupWndData {
	HWND m_hWnd;
	SIZE m_stInitialSize;

	CWndSizeData* m_pControls; // currently we have 3 child controls
	DWORD m_dwControls;

	static CPopupWndData* s_pThis;
};

CPopupWndData* CPopupWndData::s_pThis = NULL;

BOOL CALLBACK EnumCodePagesProc(LPTSTR szCodePageString)
{
	long nCodePage = 0;
	// convert it to a number (why the hell it is given as a string ???)
	if (szCodePageString)
		for (; *szCodePageString; szCodePageString++)
			nCodePage = nCodePage * 10 + (*szCodePageString - _T('0'));

	CPINFOEX stInfo;
	if (GetCPInfoEx(nCodePage, 0, &stInfo))
	{
		int nItem = SendMessage(g_hCodePages, CB_ADDSTRING, 0, (LPARAM)stInfo.CodePageName);
		if (nItem >= 0)
		{
			SendMessage(g_hCodePages, CB_SETITEMDATA, nItem, nCodePage);
			if (g_nLastCodePage == nCodePage)
				// select it
				SendMessage(g_hCodePages, CB_SETCURSEL, nItem, 0);
		}
	}
	return TRUE;
}

BOOL CALLBACK ConverterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (CPopupWndData::s_pThis)
		{
			CPopupWndData::s_pThis->m_hWnd = hWnd;

			if (g_hIcon)
			{
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			}

			for (HWND hChild = GetWindow(hWnd, GW_CHILD); hChild; hChild = GetWindow(hChild, GW_HWNDNEXT))
				CPopupWndData::s_pThis->m_dwControls++;

			if (CPopupWndData::s_pThis->m_pControls = new CWndSizeData[CPopupWndData::s_pThis->m_dwControls])
			{
				RECT stRect;
				GetClientRect(hWnd, &stRect);
				CPopupWndData::s_pThis->m_stInitialSize.cx = stRect.right;
				CPopupWndData::s_pThis->m_stInitialSize.cy = stRect.bottom;

				DWORD dwControl = 0;
				for (hChild = GetWindow(hWnd, GW_CHILD); hChild; hChild = GetWindow(hChild, GW_HWNDNEXT))
				{
					RECT* pRect = &CPopupWndData::s_pThis->m_pControls[dwControl].m_stRect;
					GetWindowRect(hChild, pRect);
					ScreenToClient(hWnd, (POINT*)pRect);
					ScreenToClient(hWnd, ((POINT*)pRect) + 1);

					CPopupWndData::s_pThis->m_pControls[dwControl].m_hWnd = hChild;

					switch (GetDlgCtrlID(hChild))
					{
					case IDC_CODEPAGE:
						CPopupWndData::s_pThis->m_pControls[dwControl].m_nResizeFlags = 2;
						break;
					case IDC_TEXT:
						CPopupWndData::s_pThis->m_pControls[dwControl].m_nResizeFlags = 10;
						break;
					case IDC_REFRESH:
						CPopupWndData::s_pThis->m_pControls[dwControl].m_nResizeFlags = 3;
						break;
					default:
						CPopupWndData::s_pThis->m_pControls[dwControl].m_nResizeFlags = 0;
					}

					if (dwControl++ >= CPopupWndData::s_pThis->m_dwControls)
						break;
				}

			}
			else
				CPopupWndData::s_pThis->m_dwControls = 0;


			// try to get the default code-page (last saved)
			HKEY hKey = NULL;
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegKeyName, 0, KEY_QUERY_VALUE, &hKey))
			{
				DWORD dwType = 0, dwValue = 0, dwSize = sizeof(DWORD);
				if ((ERROR_SUCCESS == RegQueryValueEx(hKey, g_szRegDefCodePage, NULL, &dwType, (BYTE*)&dwValue, &dwSize)) &&
					(REG_DWORD == dwType) &&
					(sizeof(DWORD) == dwSize))
					g_nLastCodePage = dwValue;

				WINDOWPLACEMENT stWP;
				dwSize = sizeof(stWP) - sizeof(UINT);
				if ((ERROR_SUCCESS == RegQueryValueEx(hKey, g_szRegPlacement, NULL, &dwType, ((BYTE*)&stWP) + sizeof(UINT), &dwSize)) &&
					(REG_BINARY == dwType) &&
					(sizeof(stWP) == dwSize + sizeof(UINT)))
				{
					stWP.length = sizeof(stWP);
					if ((SW_MINIMIZE == stWP.showCmd) || (SW_HIDE == stWP.showCmd) || (SW_SHOWMINIMIZED == stWP.showCmd))
						stWP.showCmd = SW_SHOWNORMAL;
					SetWindowPlacement(hWnd, &stWP);
				}


				RegCloseKey(hKey);
			}

			// fill our combo box with all available code pages
			g_hCodePages = GetDlgItem(hWnd, IDC_CODEPAGE);
			EnumSystemCodePages(EnumCodePagesProc, CP_INSTALLED);

			// display it
			SendMessage(hWnd, WM_COMMAND, MAKELONG(0, CBN_SELENDOK), 0);
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case CBN_SELENDOK:
		{
			int nItem = SendMessage(g_hCodePages, CB_GETCURSEL, 0, 0);
			if (nItem >= 0)
			{
				long nCodePage = SendMessage(g_hCodePages, CB_GETITEMDATA, nItem, 0);
				// save it to the registry
				HKEY hKey = NULL;
				DWORD dwDispos = 0;
				if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, g_szRegKeyName, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, &dwDispos))
				{
					RegSetValueEx(hKey, g_szRegDefCodePage, 0, REG_DWORD, (const BYTE*)&nCodePage, sizeof(DWORD));
					RegCloseKey(hKey);
				}

				HWND hEditCtrl = GetDlgItem(hWnd, IDC_TEXT);
				if (hEditCtrl && OpenClipboard(hWnd))
				{
					LPCSTR szText = (LPCSTR)GetClipboardData(CF_TEXT);
					UINT nLen = szText ? lstrlenA(szText) : 0;
					if (nLen)
					{
						WCHAR* pBuf = new WCHAR[nLen + 1];
						if (pBuf)
						{
							MultiByteToWideChar(nCodePage, 0, szText, nLen, pBuf, nLen);
							pBuf[nLen] = 0;

							SetWindowText(hEditCtrl, pBuf);

							delete[] pBuf;
						}
					}

					CloseClipboard();
				}
			}
		}
		return TRUE;
		}

		switch (LOWORD(wParam))
		{
		case IDC_REFRESH:
			SendMessage(hWnd, WM_COMMAND, MAKELONG(0, CBN_SELENDOK), 0);
			return 0;
		}

		break;

	case WM_SIZE:
		if (SIZE_MINIMIZED == wParam)
			DestroyWindow(hWnd); // no need to hide it
		else
			if (CPopupWndData::s_pThis)
			{
				int nDx = LOWORD(lParam) - CPopupWndData::s_pThis->m_stInitialSize.cx;
				int nDy = HIWORD(lParam) - CPopupWndData::s_pThis->m_stInitialSize.cy;

				// resize our child controls
				for (DWORD dwControl = 0; dwControl < CPopupWndData::s_pThis->m_dwControls; dwControl++)
				{
					RECT stNewRect;
					CopyMemory(&stNewRect, &CPopupWndData::s_pThis->m_pControls[dwControl].m_stRect, sizeof(RECT));

					int nLocks = CPopupWndData::s_pThis->m_pControls[dwControl].m_nResizeFlags;
					if (1 & nLocks)
						stNewRect.left += nDx;
					if (2 & nLocks)
						stNewRect.right += nDx;
					if (4 & nLocks)
						stNewRect.top += nDy;
					if (8 & nLocks)
						stNewRect.bottom += nDy;

					MoveWindow(CPopupWndData::s_pThis->m_pControls[dwControl].m_hWnd, stNewRect.left, stNewRect.top,
						stNewRect.right - stNewRect.left, stNewRect.bottom - stNewRect.top, TRUE);
				}
			}

		return 0;

	case WM_DESTROY:
	{
		WINDOWPLACEMENT stWP;
		stWP.length = sizeof(stWP);
		if (GetWindowPlacement(hWnd, &stWP))
		{
			HKEY hKey = NULL;
			DWORD dwDispos = 0;
			if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, g_szRegKeyName, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, &dwDispos))
			{
				RegSetValueEx(hKey, g_szRegPlacement, 0, REG_BINARY, ((const BYTE*)&stWP) + sizeof(UINT), sizeof(stWP) - sizeof(UINT));
				RegCloseKey(hKey);
			}
		}
	}
	return 0;

	}
	return FALSE;
}

LRESULT CALLBACK HiddenWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		NOTIFYICONDATA stData;
		ZeroMemory(&stData, sizeof(stData));
		stData.cbSize = sizeof(stData);
		stData.hWnd = hWnd;
		stData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		stData.uCallbackMessage = WM_TRAY;
		stData.hIcon = g_hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_TRAYICON));
		LoadStringSafe(IDS_TIP, stData.szTip, _countof(stData.szTip));
		if (!Shell_NotifyIcon(NIM_ADD, &stData))
			return -1; // oops
	}
	return 0;

	case WM_DESTROY:
	{
		NOTIFYICONDATA stData;
		ZeroMemory(&stData, sizeof(stData));
		stData.cbSize = sizeof(stData);
		stData.hWnd = hWnd;
		Shell_NotifyIcon(NIM_DELETE, &stData);
	}
	return 0;

	case WM_TRAY:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			SendMessage(hWnd, WM_COMMAND, ID_SHOW, 0);
			break;

		case WM_RBUTTONDOWN:
		{
			HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUP));
			if (hMenu)
			{
				HMENU hSubMenu = GetSubMenu(hMenu, 0);
				if (hSubMenu)
				{
					POINT stPoint;
					GetCursorPos(&stPoint);

					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, stPoint.x, stPoint.y, 0, hWnd, NULL);
				}

				DestroyMenu(hMenu);
			}
		}
		break;
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOW:
			if (CPopupWndData::s_pThis)
				SetForegroundWindow(CPopupWndData::s_pThis->m_hWnd);
			else
			{
				CPopupWndData stData;
				ZeroMemory(&stData, sizeof(stData));
				CPopupWndData::s_pThis = &stData;

				DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_CONVERTER), NULL, ConverterProc);

				if (stData.m_pControls)
					delete[] stData.m_pControls;
				CPopupWndData::s_pThis = NULL;
			}
			return 0;

		case ID_QUIT:
			PostQuitMessage(0);
			return 0;
		}
		break;

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	TCHAR szTxt[0x100];
	wsprintf(szTxt, _T("hInstance=%08x\n"), hInstance);
	OutputDebugString(szTxt);

	WNDCLASS stWC;
	ZeroMemory(&stWC, sizeof(stWC));
	stWC.lpszClassName = _T("Quick_Translator");

	HWND hHiddenWnd = FindWindow(stWC.lpszClassName, NULL);
	if (hHiddenWnd)
		PostMessage(hHiddenWnd, WM_TRAY, 0, WM_LBUTTONDBLCLK);
	else
	{
		stWC.hInstance = hInstance;
		stWC.lpfnWndProc = HiddenWndProc;

		ATOM aClass = RegisterClass(&stWC);
		if (aClass)
		{
			g_hInstance = hInstance;
			if (hHiddenWnd = CreateWindow((LPCTSTR)aClass, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL))
			{
				MSG stMsg;
				while (GetMessage(&stMsg, NULL, 0, 0) > 0)
				{
					TranslateMessage(&stMsg);
					DispatchMessage(&stMsg);
				}

				if (IsWindow(hHiddenWnd))
					DestroyWindow(hHiddenWnd);
			}
			UnregisterClass((LPCTSTR)aClass, g_hInstance);
		}
	}
	return 0;
}
*/