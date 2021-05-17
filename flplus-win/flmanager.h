#pragma once

#include "pch.h"
#include <shlwapi.h>

BOOL MoveMouse(int dx, int dy);
BOOL Click();
BOOL SendKeystrokes(const WORD* keys, int numKeys);
BOOL SendKeystrokesSimul(const WORD* keys, int numKeys);
inline BOOL SendKeystroke(const WORD key) { WORD keys[1] = { key }; return SendKeystrokes(keys, 1); }

bool IsClass(HWND hwnd, LPCTSTR classStr);
bool IsWinText(HWND hwnd, LPCTSTR textStr);
bool WinTextStartsWith(HWND hwnd, LPCTSTR textStr);
HWND GetWinMatch(bool (match)(HWND));

namespace FL {
	struct Workspace
	{
		RECT playlist;
		RECT piano_roll;
		RECT mixer;
		RECT channel_rack;
		int rCode;
	};

	void Init();

	bool isFL(HWND hwnd);
	bool isFLChild(HWND hwnd);

	HWND getFL();
	BOOL FocusFL();

	bool isPlaylist(HWND proc);
	bool isPianoRoll(HWND proc);
	bool isMixer(HWND proc);
	bool isChannelRack(HWND proc);
	bool isEdison(HWND proc);

	HWND getPlaylist();
	HWND getPianoRoll();
	HWND getMixer();
	HWND getChannelRack();

	int CreateAutomationClip(HWND fl);
	
	int DeactivateReachPlayback(HWND edison);
	DWORD DoDisableReachPlayback();
	int ToggleDisableReachPlayback();
	DWORD LoadDisableReachPlaybackReg();
	int SetDisableReachPlaybackReg(DWORD status);

	int SetLocalWorkspace(int i, HWND fl);
	int LoadLocalWorkspace(int i, HWND fl);
	int LoadWorkspacesFromReg();
	int LoadWorkspace(HWND fl, Workspace w);
	Workspace GetWorkspace(HWND fl);
	int SetWorkspaceReg(int i, Workspace w);
	Workspace LoadWorkspaceReg(int i);

	bool OpenColorPane(HWND midi, bool col_set);
	bool CloseColorPane();
}