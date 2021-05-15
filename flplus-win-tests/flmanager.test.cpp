#include "pch.h"
#include "../flplus-win/flmanager.h"
#include "../flplus-win/flmanager.cpp"

using namespace FL;

TEST(TestFLManager, FLIsRunning) {
	// ensure FL process is founda
	ASSERT_NE(getFL(), (HWND) NULL);

	// focus windowa
	ASSERT_NE(FocusFL(), 0);
}

TEST(TestFLManager, CreateAutomationClip) {
	FocusFL();

	FL::CreateAutomationClip(getFL());
}

TEST(TestFLManager, ResizePlaylist) {
	FocusFL();

	HWND playlist = FL::getPlaylist();

	SetWindowPos(playlist, HWND_TOP, 0, 0, 600, 600, SWP_NOZORDER);
}