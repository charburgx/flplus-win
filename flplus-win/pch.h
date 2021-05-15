#pragma once

#include <Windows.h>
#include <tchar.h>
#include <shellapi.h>
#pragma comment (lib, "shell32.lib")

#define UWM_SYS_MSG_FILTER_HOOK_MSG "UWM_SYS_MSG_FILTER-44E531B1_14D3_11d5_A025_006067718D04"
#define UWM_CALL_WND_PROC_HOOK_MSG "UWM_CALL_WND_PROC-AEFD5BE9-B053-4d09-A353-D8B031C224B6"

// Global constants
const LPCTSTR FLPLUS_TITLE = _T("FL Plus");

inline int WARNING_BOX(LPCTSTR message, HWND hwnd) { return MessageBox(hwnd, message, FLPLUS_TITLE, MB_ICONEXCLAMATION); }
inline int INFO_BOX   (LPCTSTR message, HWND hwnd) { return MessageBox(hwnd, message, FLPLUS_TITLE, MB_ICONINFORMATION); }

// Registry
const LPCSTR REG_ROOT = "Software\\charburgx\\FLPlus\\";
const REGSAM REG_FL_SAM = KEY_QUERY_VALUE | KEY_SET_VALUE;

inline LSTATUS CreateFLPlusReg(LPCSTR lpSubKey, HKEY* hKey) { return RegCreateKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, REG_FL_SAM, NULL, hKey, NULL); };
inline LSTATUS OpenFLPlusReg(LPCSTR lpSubKey, HKEY* hKey) { return RegOpenKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, REG_FL_SAM, hKey); }

// Return codes
constexpr int WRC_SUCCESS = 0;
constexpr int WRC_FAILURE = 1;
constexpr int WRC_NOTDETACHED = 2;
constexpr int WRC_REGERROR = 3;
constexpr int WRC_REGDOESNTEXIST = 4;

// Maximum window text length for string comparison
const int MAX_WIN_TEXT_LEN = 100;

// Automation Clips
constexpr int HOTKEY_AUTOMATION_ID = 0;
constexpr UINT HOTKEY_AUTOMATION_MODIFIERS = MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT;
constexpr UINT HOTKEY_AUTOMATION_KEY = 0x41;

// Workspaces
constexpr int NUM_WORKSPACES = 8;

constexpr int HOTKEY_WORKSPACE_SWITCH_ID = 100;
constexpr UINT HOTKEY_WORKSPACE_SWITCH_MODIFIERS = MOD_CONTROL | MOD_NOREPEAT;
constexpr UINT HOTKEY_WORKSPACE_SWITCH_KEYSTART = 0x70;

constexpr int HOTKEY_WORKSPACE_SET_ID = 200;
constexpr int HOTKEY_WORKSPACE_SET_MODIFIERS = MOD_CONTROL | MOD_ALT | MOD_NOREPEAT;
constexpr UINT HOTKEY_WORKSPACE_SET_KEYSTART = HOTKEY_WORKSPACE_SWITCH_KEYSTART;

const LPCSTR REG_ROOT_WORKSPACES = "Workspaces\\";

const LPCSTR REG_WORKSPACE_SUFFIX_LEFT = "_left";
const LPCSTR REG_WORKSPACE_SUFFIX_RIGHT = "_right";
const LPCSTR REG_WORKSPACE_SUFFIX_TOP = "_top";
const LPCSTR REG_WORKSPACE_SUFFIX_BOTTOM = "_bottom";

const LPCSTR REG_WORKSPACE_PREFIX_PLAYLIST = "playlist";
const LPCSTR REG_WORKSPACE_PREFIX_MIDI_ROLL = "piano_roll";
const LPCSTR REG_WORKSPACE_PREFIX_MIXER = "mixer";
const LPCSTR REG_WORKSPACE_PREFIX_CHANNELRACK = "channel_rack";

// Win titles, lengths
const LPCTSTR FL_WIN_TEXT        = _T("FL Studio 20");
const LPCTSTR FL_WIN_TEXT_LEGACY = _T("FL Studio 12");

const LPCTSTR FL_WIN_TEXT_PIANO_ROLL = _T("Piano roll -");
const LPCTSTR FL_WIN_TEXT_PLAYLIST   = _T("Playlist -");
const LPCTSTR FL_WIN_TEXT_EDISON     = _T("Edison");

const LPCTSTR FL_CLASS_MAIN             = _T("TFruityLoopsMainForm");
const LPCTSTR FL_CLASS_CHANNEL_RACK     = _T("TStepSeqForm");
const LPCTSTR FL_CLASS_PLAYLIST         = _T("TEventEditForm");
const LPCTSTR FL_CLASS_PIANO_ROLL       = _T("TEventEditForm");
const LPCTSTR FL_CLASS_MIXER            = _T("TFXForm");
const LPCTSTR FL_CLASS_PLUGIN		    = _T("TPluginForm");

// Edison auto-disable playback
const unsigned int FL_COLOR_EDISON_REACH_PLAYBACK_ACTIVE = 0xc7b7af;
const unsigned int FL_COLOR_EDISON_REACH_PLAYBACK_INACTIVE = 6904146;

const LPCSTR REG_EDISON_REACH_PLAYBACK_LPVAL = "edisonDoDisableReachPlayback";

inline HWND GetMainWindow(HWND hwnd) { return GetAncestor(hwnd, GA_ROOTOWNER); }