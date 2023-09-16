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

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
// https://stackoverflow.com/questions/16530871/findwindow-does-not-find-the-a-window
{
	char buffer[128];
	int written = GetWindowTextA(hwnd, buffer, 128);
	if (written && strstr(buffer, "Mozilla Firefox") != NULL) {
		*(HWND*)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}

HHOOK hKeyboardHook = 0;
LRESULT CALLBACK KeyboardCallback(int code, WPARAM wParam, LPARAM lParam) {
	cout << "a key was pressed" << endl;
	//ofstream myfile;
	//myfile.open("hookcheck.txt", ios::ate | ios::app);
	//myfile << "a key was pressed\n";
	//myfile.close();
	return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
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
	string targetAppName = "Steam";
	//log(string("search pid in pattern: " + targetAppName + '\n').c_str());
	HWND windowHandle = FindWindowA(NULL, targetAppName.c_str());
	DWORD threadId = GetWindowThreadProcessId(windowHandle, NULL);
	log(string("thread id is: " + to_string(threadId) + '\n').c_str());
	HHOOK hook = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardCallback, 0, threadId);
	if (!hook) {
	// https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes
		log(string("Hook failed\n").c_str());
		log(string("Error code: " + to_string(GetLastError()) + '\n').c_str());
	}
	else {
		log(string("Hook successed!\n").c_str());
	}

	while (true) {

	}
	/*cin >> block;*/
	CoUninitialize();
	return WM_QUIT;
}
