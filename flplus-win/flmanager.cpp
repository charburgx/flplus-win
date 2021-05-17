#include "flmanager.h"
#include <string>

BOOL MoveMouse(int dx, int dy) 
{
	POINT p;
	if (!GetCursorPos(&p)) { return FALSE; }
	return SetCursorPos(p.x + dx, p.y + dy);
}

BOOL Click() 
{
	INPUT inputs[2] = { 0 };

	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	return (SendInput(2, inputs, sizeof(INPUT)) == 2);
}

BOOL SendKeystrokes(const WORD* keys, int numKeys) 
{
	BOOL success = true;

	for (int i = 0; i < numKeys; i++) {
		INPUT in;

		in.type = INPUT_KEYBOARD;
		in.ki.wVk = keys[i];
		in.ki.wScan = 0;
		in.ki.time = 0;
		in.ki.dwExtraInfo = 0;
		in.ki.dwFlags = 0;
		success = success & SendInput(1, &in, sizeof(INPUT));

		in.ki.dwFlags = KEYEVENTF_KEYUP;
		success = success & SendInput(1, &in, sizeof(INPUT));
	}

	return (success);
}

BOOL SendKeystrokesSimul(const WORD* keys, int numKeys)
{
	BOOL success = true;

	for (int i = 0; i < numKeys; i++) {
		INPUT in;

		in.type = INPUT_KEYBOARD;
		in.ki.wVk = keys[i];
		in.ki.wScan = 0; in.ki.time = 0; in.ki.dwExtraInfo = 0;
		in.ki.dwFlags = 0;
		success = success & SendInput(1, &in, sizeof(INPUT));
	}

	for (int i = 0; i < numKeys; i++) {
		INPUT in;

		in.type = INPUT_KEYBOARD;
		in.ki.wVk = keys[i];
		in.ki.wScan = 0; in.ki.time = 0; in.ki.dwExtraInfo = 0;
		in.ki.dwFlags = KEYEVENTF_KEYUP;
		success = success & SendInput(1, &in, sizeof(INPUT));
	}

	return success;
}

namespace FL 
{
	HWND g_fl_hwnd = NULL;

	void Init()
	{
		// Load workspace registry
		LoadWorkspacesFromReg();

		// Load Edison reach playback registry
		LoadDisableReachPlaybackReg();
	}

	bool isFL(HWND proc)
	{
		return IsClass(proc, FL_CLASS_MAIN) && (IsWinText(proc, FL_WIN_TEXT) || IsWinText(proc, FL_WIN_TEXT_LEGACY));
	}
	
	bool isFLChild(HWND hwnd) {
		return isFL(GetMainWindow(hwnd));
	}

	HWND getFL()
	{
		return GetWinMatch(isFL);
	}

	BOOL FocusFL() {
		HWND fl = getFL();
		if (fl == NULL) { return FALSE; }

		//BOOL maximize = ShowWindow(fl, SW_MAXIMIZE);
		BOOL focus = SetForegroundWindow(fl);

		return focus;
	}

	// Subwindows //
	/* Type of window is determined by testing class/title text */

	bool isPlaylist(HWND proc) {
		return IsClass(proc, FL_CLASS_PLAYLIST) 
			&& isFL(GetMainWindow(proc))
			&& WinTextStartsWith(proc, FL_WIN_TEXT_PLAYLIST);
	}

	bool isPianoRoll(HWND proc) {
		return IsClass(proc, FL_CLASS_PIANO_ROLL)
			&& isFL(GetMainWindow(proc))
			&& WinTextStartsWith(proc, FL_WIN_TEXT_PIANO_ROLL);
	}

	bool isMixer(HWND proc) {
		return IsClass(proc, FL_CLASS_MIXER)
			&& isFL(GetMainWindow(proc));
	}

	bool isChannelRack(HWND proc) {
		return IsClass(proc, FL_CLASS_CHANNEL_RACK)
			&& isFL(GetMainWindow(proc));
	}

	bool isEdison(HWND proc) {
		return IsClass(proc, FL_CLASS_PLUGIN)
			&& WinTextStartsWith(proc, FL_WIN_TEXT_EDISON)
			&& isFLChild(proc);
	}

	HWND getPlaylist() { return GetWinMatch(isPlaylist); }
	HWND getPianoRoll() { return GetWinMatch(isPianoRoll); }
	HWND getMixer() { return GetWinMatch(isMixer); }
	HWND getChannelRack() { return GetWinMatch(isChannelRack); }

	HWND getPlaylistFromFL() 
	{ 
		return GetWinMatch([](HWND hwnd) { return isPlaylist(hwnd) && (GetMainWindow(hwnd) == g_fl_hwnd); }); 
	}

	HWND getPianoRollFromFL()
	{
		return GetWinMatch([](HWND hwnd) { return isPianoRoll(hwnd) && (GetMainWindow(hwnd) == g_fl_hwnd); });
	}

	HWND getMixerFromFL()
	{
		return GetWinMatch([](HWND hwnd) { return isMixer(hwnd) && (GetMainWindow(hwnd) == g_fl_hwnd); });
	}

	HWND getChannelRackFromFL()
	{
		return GetWinMatch([](HWND hwnd) { return isChannelRack(hwnd) && (GetMainWindow(hwnd) == g_fl_hwnd); });
	}

	// Routines //

	int CreateAutomationClip(HWND fl) 
	{
		RECT winPos;
		if (!GetWindowRect(fl, &winPos)) { return WRC_FAILURE; }

		POINT cPos;
		BOOL cur = GetCursorPos(&cPos);

		SetCursorPos(winPos.left, winPos.top);

		MoveMouse(264, 30);
		Click();
		MoveMouse(0, 240);
		Click();
		MoveMouse(300, 110);
		Click();

		if (cur) { SetCursorPos(cPos.x, cPos.y); }

		return WRC_SUCCESS;
	}

	// Edison //

	DWORD DISABLE_REACH_PLAYBACK = true;

	DWORD DoDisableReachPlayback()
	{
		return DISABLE_REACH_PLAYBACK;
	}

	int ToggleDisableReachPlayback()
	{
		DISABLE_REACH_PLAYBACK = !DISABLE_REACH_PLAYBACK;
		return SetDisableReachPlaybackReg(DISABLE_REACH_PLAYBACK);
	}

	int SetDisableReachPlaybackReg(DWORD status)
	{
		LPCSTR lpSubKey = REG_ROOT;
		HKEY hKey;

		DWORD len = sizeof(DWORD);

		if (CreateFLPlusReg(lpSubKey, &hKey) != ERROR_SUCCESS)
		{
			return WRC_REGERROR;
		}
		
		RegSetValueExA(hKey, REG_EDISON_REACH_PLAYBACK_LPVAL, 0, REG_DWORD, (BYTE*)&status, len);

		RegCloseKey(hKey);
		return WRC_SUCCESS;
	}

	DWORD LoadDisableReachPlaybackReg()
	{
		LPCSTR lpSubKey = REG_ROOT;
		HKEY hKey;

		DWORD len = sizeof(DWORD);

		if (CreateFLPlusReg(lpSubKey, &hKey) != ERROR_SUCCESS)
		{
			return true;
		}

		DWORD state = true;
		LSTATUS lstat = RegQueryValueExA(hKey, REG_EDISON_REACH_PLAYBACK_LPVAL, 0, NULL, (BYTE*)&state, &(len));

		if (lstat != ERROR_SUCCESS)
		{
			state = true;
		}

		DISABLE_REACH_PLAYBACK = state;
		
		RegCloseKey(hKey);
		return state;
	}

	// expects HWND to be foreground window
	int DeactivateReachPlayback(HWND edison) 
	{
		if (!DoDisableReachPlayback()) { return WRC_SUCCESS; }

		if (!isEdison(edison)) { return WRC_NOTDETACHED; }

		RECT r;
		if (!GetWindowRect(edison, &r)) { return false; }

		HDC hdc = GetDC(NULL);
		auto arrow_color = GetPixel(hdc, r.right - 185, r.bottom - 17) & 0xFFFFFF;

		bool reach_playback_active = (arrow_color == FL_COLOR_EDISON_REACH_PLAYBACK_ACTIVE);
		bool reach_playback_inactive = (arrow_color == FL_COLOR_EDISON_REACH_PLAYBACK_INACTIVE);

		//OutputDebugStringA((std::to_string(arrow_color) + "\n").c_str());

		if (reach_playback_active)
		{
			return SendKeystroke(0x41) ? WRC_SUCCESS : WRC_FAILURE;
		}

		return WRC_SUCCESS;
	}
	
	// Workspaces //

	Workspace workspaces[NUM_WORKSPACES];

	int SetLocalWorkspace(int i, HWND fl)
	{
		Workspace w = GetWorkspace(fl);

		if (w.rCode != WRC_SUCCESS) { return w.rCode; }

		SetWorkspaceReg(i, w);

		workspaces[i] = w;
		return w.rCode;
	}

	int LoadLocalWorkspace(int i, HWND fl)
	{
		Workspace w = workspaces[i];
		if (w.rCode != WRC_SUCCESS) { return w.rCode; }

		return LoadWorkspace(fl, w);
	}

	int LoadWorkspacesFromReg()
	{
		for (int i = 0; i < NUM_WORKSPACES; i++)
		{
			workspaces[i] = LoadWorkspaceReg(i);
		}

		return WRC_SUCCESS;
	}

	int LoadWorkspace(HWND fl, Workspace w)
	{
		g_fl_hwnd = fl;

		HWND playlist = getPlaylistFromFL();
		HWND piano_roll = getPianoRollFromFL();
		HWND mixer = getMixerFromFL();
		HWND channel_rack = getChannelRackFromFL();

		if (playlist == NULL || piano_roll == NULL || mixer == NULL || channel_rack == NULL)
		{
			return WRC_NOTDETACHED;
		}

		auto ResizeWin = [](HWND hwnd, RECT r) { return SetWindowPos(hwnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER); };

		ResizeWin(playlist, w.playlist);
		ResizeWin(piano_roll, w.piano_roll);
		ResizeWin(mixer, w.mixer);
		ResizeWin(channel_rack, w.channel_rack);

		return WRC_SUCCESS;
	}

	Workspace GetWorkspace(HWND fl)
	{
		g_fl_hwnd = fl;

		HWND playlist = getPlaylistFromFL();
		HWND piano_roll = getPianoRollFromFL();
		HWND mixer = getMixerFromFL();
		HWND channel_rack = getChannelRackFromFL();

		Workspace w;

		if (playlist == NULL || piano_roll == NULL || mixer == NULL || channel_rack == NULL)
		{
			w.rCode = WRC_NOTDETACHED;
			return w;
		}

		GetWindowRect(playlist, &w.playlist);
		GetWindowRect(piano_roll, &w.piano_roll);
		GetWindowRect(mixer, &w.mixer);
		GetWindowRect(channel_rack, &w.channel_rack);
		
		w.rCode = WRC_SUCCESS;
		return w;
	}

	int SetWorkspaceReg(int i, Workspace w)
	{
		std::string lpSubKey = std::string() + REG_ROOT + REG_ROOT_WORKSPACES + std::to_string(i);

		HKEY hKey;
		DWORD len = sizeof(DWORD);

		// Create/open registry key
		if (CreateFLPlusReg(lpSubKey.c_str(), &hKey) != ERROR_SUCCESS)
		{
			return WRC_REGERROR;
		}

		// Sets registry values for a rectangle
		auto SetRegProp = [&](std::string lpVal, RECT r) 
		{ 
			RegSetValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_LEFT).c_str(), 0, REG_DWORD, (BYTE*)&r.left, sizeof(DWORD));
			RegSetValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_RIGHT).c_str(), 0, REG_DWORD, (BYTE*)&r.right, sizeof(DWORD));
			RegSetValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_TOP).c_str(), 0, REG_DWORD, (BYTE*)&r.top, sizeof(DWORD));
			RegSetValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_BOTTOM).c_str(), 0, REG_DWORD, (BYTE*)&r.bottom, sizeof(DWORD));
		};

		// Set registry data for each window
		SetRegProp(REG_WORKSPACE_PREFIX_PLAYLIST, w.playlist);
		SetRegProp(REG_WORKSPACE_PREFIX_MIDI_ROLL, w.piano_roll);
		SetRegProp(REG_WORKSPACE_PREFIX_MIXER, w.mixer);
		SetRegProp(REG_WORKSPACE_PREFIX_CHANNELRACK, w.channel_rack);

		RegCloseKey(hKey);

		return WRC_SUCCESS;
	}

	Workspace LoadWorkspaceReg(int i)
	{
		std::string lpSubKey = std::string() + REG_ROOT + REG_ROOT_WORKSPACES + std::to_string(i);

		HKEY hKey;
		DWORD len = sizeof(DWORD);

		Workspace w;

		// Open registry key
		if (OpenFLPlusReg(lpSubKey.c_str(), &hKey) != ERROR_SUCCESS)
		{
			w.rCode = WRC_REGERROR;
			return w;
		}

		struct rpstatus { RECT r; LSTATUS left; LSTATUS right; LSTATUS top; LSTATUS bottom; };

		// Returns rectangle from registry data
		auto GetRegProp = [&](std::string lpVal)
		{
			rpstatus s;
			RECT r;

			s.left = RegQueryValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_LEFT).c_str(), NULL, NULL, (BYTE*)&r.left, &(len));
			s.right = RegQueryValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_RIGHT).c_str(), NULL, NULL, (BYTE*)&r.right, &(len));
			s.top = RegQueryValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_TOP).c_str(), NULL, NULL, (BYTE*)&r.top, &(len));
			s.bottom = RegQueryValueExA(hKey, (lpVal + REG_WORKSPACE_SUFFIX_BOTTOM).c_str(), NULL, NULL, (BYTE*)&r.bottom, &(len));
			s.r = r;

			return s;
		};

		// Assign window rects from reg data
		auto playlist = GetRegProp(REG_WORKSPACE_PREFIX_PLAYLIST);
		auto piano_roll = GetRegProp(REG_WORKSPACE_PREFIX_MIDI_ROLL);
		auto mixer = GetRegProp(REG_WORKSPACE_PREFIX_MIXER);
		auto channel_rack = GetRegProp(REG_WORKSPACE_PREFIX_CHANNELRACK);

		// Handle possible errors
		if (playlist.left == ERROR_FILE_NOT_FOUND || 
			playlist.right == ERROR_FILE_NOT_FOUND || 
			playlist.top == ERROR_FILE_NOT_FOUND || 
			playlist.bottom == ERROR_FILE_NOT_FOUND)
		{
			// TODO: this code seems to never be reached
			w.rCode = WRC_REGDOESNTEXIST;
			return w;
		}
		else if (playlist.left != ERROR_SUCCESS || 
			playlist.right != ERROR_SUCCESS || 
			playlist.top != ERROR_SUCCESS || 
			playlist.bottom != ERROR_SUCCESS)
		{
			w.rCode = WRC_REGERROR;
			return w;
		}

		w = { playlist.r, piano_roll.r, mixer.r, channel_rack.r };
		
		RegCloseKey(hKey);
		w.rCode = WRC_SUCCESS;
		return w;
	}

	// Colors //
	POINT g_col_cPos = { 0, 0 };
	bool  g_col_set  = false;
	bool  g_col      = false;

	bool OpenColorPane(HWND midi, bool col_set) 
	{	
		RECT win;
		if (!GetWindowRect(midi, &win)) { return false; }

		g_col_set = col_set;
		if (!GetCursorPos(&g_col_cPos)) { return false; }

		SetCursorPos(win.left, win.top);
		MoveMouse(30, 60);
		Click();

		g_col = true;
	}

	bool CloseColorPane()
	{
		if (!g_col) return false;

		SetCursorPos(g_col_cPos.x, g_col_cPos.y);

		g_col_cPos = { 0, 0 };
		g_col_set = false;
		g_col = false;

		return true;
	}
}

bool IsClass(HWND hwnd, LPCTSTR classStr) 
{
	TCHAR cName[MAX_CLASS_NAME];
	if (GetClassName(hwnd, cName, MAX_CLASS_NAME) == 0) { return false; }

	return _tcscmp(cName, classStr) == 0;
}

bool IsWinText(HWND hwnd, LPCTSTR textStr) 
{
	const int len = MAX_WIN_TEXT_LEN + 1;

	TCHAR tstr[len];
	GetWindowText(hwnd, tstr, len);
	bool isWin = _tcscmp(tstr, textStr) == 0;
	
	return isWin;
}

bool WinTextStartsWith(HWND hwnd, LPCTSTR textStr) 
{
	const int len = MAX_WIN_TEXT_LEN + 1;

	TCHAR tstr[len];
	GetWindowText(hwnd, tstr, len);
	bool isWin = StrStr(tstr, textStr) == tstr;
	
	return isWin;
}

HWND GetWinMatch(bool (match)(HWND)) 
{
	struct info {
		bool (*match)(HWND);
		HWND hwnd;
	};

	info INFO = { match, NULL };

	EnumWindows([](HWND proc, LPARAM lParam) {
		info *I = (info*) lParam;

		if ((*I).match(proc)) {
			(*I).hwnd = proc;
			return FALSE;
		}

		return TRUE;
	}, (LPARAM)&INFO);

	return INFO.hwnd;
}