#include <iostream>
#include <stdio.h>
#include <string>

#include "Windows.h"
#include "xinput.h"
#include "minhook/include/MinHook.h"
#include <tchar.h>

using namespace std;

static string input_mode = "XInput";
typedef unsigned char BYTE;

static bool deviceStatus[16] = { false }; // DInput support at most 16 devices

void log(char const * msg) {
#ifdef _DEBUG
	OutputDebugStringA(msg);
#else
	// nothing
#endif
}

void detectXInputDevice() {
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS)
		{
			// Controller is connected
			deviceStatus[i] = true;
		}
	}
}

void handleXInputGamePlay() {
	XINPUT_KEYSTROKE input;
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		if (deviceStatus[i]) {
			if (XInputGetKeystroke(i, 0, &input) == ERROR_SUCCESS && input.Flags != XINPUT_KEYSTROKE_KEYUP) {
				log(string("Device " + to_string(i) + " Pressed " + to_string(input.VirtualKey) + '\n').c_str());
			}
		}
	}
}

LRESULT CALLBACK HookProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	// process event
	//...

		return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	CoInitialize(0);
	BYTE byte0 = 0;
	BYTE byte1 = 0;
	BYTE byte2 = 0;
	log(string("START capturing inputs\nMode = " + input_mode + '\n').c_str());
	
	// detect your stick first
	if (input_mode == "XInput") {
		handleXInputGamePlay();
		detectXInputDevice();
		for (int i = 0;i < XUSER_MAX_COUNT; i++) {
			if (deviceStatus[i]) {
				log(string("XInput controller detected: id = " + to_string(i) + '\n').c_str());
			}
		}
	}
	// find pid of target application
	//https://stackoverflow.com/questions/16530871/findwindow-does-not-find-the-a-window
	HWND windowHandle = FindWindow(NULL, _T("Steam"));
	DWORD threadId = GetWindowThreadProcessId(windowHandle, NULL);
	log(string("thread id is: " + to_string(threadId) + '\n').c_str());
	//HHOOK hook = ::SetWindowsHookEx(WH_CBT, HookCBTProc, hInst, threadId);

	CoUninitialize();

}
