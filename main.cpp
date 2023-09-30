#include <iostream>
#include <stdio.h>
#include <string>

#ifndef WINDOWS_H
#include "Windows.h"
#endif

#include "xinput.h"
#include "minhook/include/MinHook.h"
#include <tchar.h>


using namespace std;

static string input_mode = "XInput";
static DWORD threadId = -1;
string targetAppName = "GUNDAM EXTREME";
typedef unsigned char BYTE;

static bool deviceStatus[16] = { false }; // DInput support at most 16 devices

void log(string msg) {
	cout << msg;
#ifdef _DEBUG
	OutputDebugStringA(msg.c_str());
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

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
// https://stackoverflow.com/questions/16530871/findwindow-does-not-find-the-a-window
{
	char buffer[128];
	int written = GetWindowTextA(hwnd, buffer, 128);
	if (written && strstr(buffer, targetAppName.c_str()) != NULL) {
		*(HWND*)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	CoInitialize(0);
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	log(string("START capturing inputs\nMode = " + input_mode + '\n'));
	// detect your stick first
	if (input_mode == "XInput") {
		detectXInputDevice();
		for (int i = 0;i < XUSER_MAX_COUNT; i++) {
			if (deviceStatus[i]) {
				log(string("XInput controller detected: id = " + to_string(i) + '\n'));
			}
		}
	}

	HWND windowHandle = NULL;
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowHandle));
	threadId = GetWindowThreadProcessId(windowHandle, NULL);
	log(string("thread id of " + targetAppName + " is: " + to_string(threadId) + '\n'));

	auto libToInject = LoadLibraryA("keyproc.dll");
	if (libToInject == nullptr) {
		log("Fail to load dll!\n");
	}

	auto keyProcAddress = reinterpret_cast<HOOKPROC>(GetProcAddress(libToInject, "JoyProc"));
	if (keyProcAddress == nullptr) {
		log("Fail to find function!\n");
	}

	auto hook = SetWindowsHookEx(WH_KEYBOARD,
		keyProcAddress, libToInject, threadId);
	if (hook == nullptr) {
		log("SetWindowsHookEx fail, " + GetLastError() + '\n');
	}
	else {
		MSG Message;
		while (GetMessage(&Message, NULL, NULL, NULL)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}
	// press any key to continue
	system("pause");
	UnhookWindowsHookEx(hook);
	CoUninitialize();
	return WM_QUIT;
}