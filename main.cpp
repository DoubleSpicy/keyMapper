#include <iostream>
#include <stdio.h>
#include <string>
#include <queue>

#ifndef WINDOWS_H
#include "Windows.h"
#endif

#include "xinput.h"
#include "minhook/include/MinHook.h"
#include <tchar.h>


using namespace std;

static string input_mode = "XInput";
string targetAppName = "Notepad++";
typedef unsigned char BYTE;

static bool deviceStatus[16] = { false }; // DInput support at most 16 devices

// debug messages
static const string keyStrokeInfo = "Raw Input Event: ";

// rawInputDevice
static RAWINPUTDEVICE rawInputDevice[1];
RAWINPUT* event = new RAWINPUT();
UINT eventSize = sizeof(RAWINPUT);
WCHAR* stringBuffer = new WCHAR[256];
UINT strBufSize = sizeof(WCHAR) * 256;

// commands between dll and parent
UINT const WM_HOOK = WM_APP + 1;
struct keyStroke{
	USHORT virtualKeyCode;  // which key is pressed
	USHORT keyPressStatus; // key is up or down
};
deque<keyStroke> approvedRawInputs;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
  	WNDCLASSEX wcex;

  	wcex.cbSize = sizeof(WNDCLASSEX);

  	wcex.style = CS_HREDRAW | CS_VREDRAW;
  	wcex.lpfnWndProc = WndProc;
  	wcex.cbClsExtra = 0;
  	wcex.cbWndExtra = 0;
  	wcex.hInstance = hInstance;
  	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
  	wcex.lpszClassName = TEXT("TEST PROGRAM");

  	return RegisterClassEx(&wcex);
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

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	// helper function
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

bool detectDevice() {
	// helper function to look for device
	bool find = false;
	if (input_mode == "XInput") {
		detectXInputDevice();
		for (int i = 0;i < XUSER_MAX_COUNT; i++) {
			if (deviceStatus[i]) {
				log(string("XInput controller detected: id = " + to_string(i) + '\n'));
				find = true;
			}
		}
	}
	if (!find) {
		log("Fail to find any device!\n");
	}
	return find;
}

HWND findTargetProcess() {
	HWND windowHandle = NULL;
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowHandle));
	log("target process found.\n");
	return windowHandle;
}

int getPID(HWND handle) {
	auto pid = GetWindowThreadProcessId(handle, NULL);
	log(string("thread id of " + targetAppName + " is: " + to_string(pid) + '\n'));
	return pid;
}

bool installDeviceHook(int pid, HWND parentProcess) {
	auto libToInject = LoadLibraryA("keyproc.dll");
	if (libToInject == nullptr) {
		log("Fail to load dll!\n");
	}

	//auto keyProcAddress = reinterpret_cast<HOOKPROC>(GetProcAddress(libToInject, "KeyboardProc"));
	//if (keyProcAddress == nullptr) {
	//	log("Fail to find function!\n");
	//}

	//auto dllVarParentProcess = reinterpret_cast<HWND>(GetProcAddress(libToInject, "parentProcess"));
	//dllVarParentProcess = parentProcess;

	//auto hook = SetWindowsHookEx(WH_KEYBOARD,
	//	keyProcAddress, libToInject, pid);
	//if (hook == nullptr) {
	//	log("SetWindowsHookEx fail, " + GetLastError() + '\n');
	//}
	//else {
	//	MSG Message;
	//	while (GetMessage(&Message, NULL, NULL, NULL)) {
	//		TranslateMessage(&Message);
	//		DispatchMessage(&Message);
	//	}
	//}
	//return hook;
	typedef bool (*GW)(HWND parentProcess);
	GW installHookFunc = (GW)(GetProcAddress(libToInject, "installHook"));
	auto result = installHookFunc(parentProcess);
	log(string("INSTALL ") + (result ? "OK" : "FAIDED") + '\n');
	return result;
}

int main() {
	if (!detectDevice()) {
		return -1;
	}

	HINSTANCE instance = GetModuleHandle(0);
	std::ios_base::sync_with_stdio(false);
	cout << "Starting...\n";
	FILE* console_output;
	FILE* console_error;
	FreeConsole();
	if (AllocConsole()) {
		freopen_s(&console_output, "CONOUT$", "w", stdout);
		freopen_s(&console_error, "CONERR$", "w", stderr);
	}
	else {
		cout << "Fail to AllocConsole()! Error = " << GetLastError() << "\n";
		return -1;
	}

	// Create message-only window:
	const char* class_name = "SimpleEngine Class";
	const char* class_type = "SimpleEngine";

	// "{}" is necessary, otherwise we have to use ZeroMemory() (which is just memset).
	WNDCLASS window_class = {};
	window_class.lpfnWndProc = WndProc;
	window_class.hInstance = instance;
	window_class.lpszClassName = convertCharArrayToLPCWSTR(class_name);

	if (!RegisterClass(&window_class))
		return -100;

	HWND window = CreateWindow(convertCharArrayToLPCWSTR(class_name), convertCharArrayToLPCWSTR(class_type), 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
	if (window == nullptr)
		return -200;
	// End of creating window.

	// Registering raw input devices
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((unsigned short) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((unsigned short) 0x02)
#endif

// We're configuring just one RAWINPUTDEVICE, the mouse,
// so it's a single-element array (a pointer).
	RAWINPUTDEVICE rid[1];
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x06; // or 0x05?
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = window;
	if (RegisterRawInputDevices(rid, 1, sizeof(rid[0])) == false) {
		return -150;
	}
	// End of resgistering raw input api

	// do keyboard hook
	auto handle = findTargetProcess();
	auto pid = getPID(handle);
	installDeviceHook(pid, window);
	//auto hook = doDeviceHook(pid, handle);
	//if (hook == nullptr) {
	//	log("fail to hook!\n");
	//	return -1;
	//}
	MSG event;
	bool quit = false;

	while (!quit) {
		while (GetMessage(&event, NULL, 0, 0)) {
			if (event.message == WM_QUIT) {
				quit = true;
				break;
			}
			log("checking msg...\n");
			// Does some Windows magic and sends the message to EventHandler()
			// because it's associated with the window we created.
			TranslateMessage(&event);
			DispatchMessage(&event);
		}
		//cout << "checking...\n";

		// Output mouse input to console:
		//std::cout << "Mouse input: (" << input.mouse.x;
		//std::cout << ", " << input.mouse.y;
		//std::cout << ", " << input.mouse.wheel << ")\n";

		// Sleep a bit so that console output can be read...
		Sleep(100);
		// ...before clearing the console:
		//ClearConsole();

		// Reset mouse data in case WM_INPUT isn't called:
		//input.mouse.x = 0;
		//input.mouse.y = 0;
		//input.mouse.wheel = 0;
	}

	fclose(console_output);
	fclose(console_error);
	//UnhookWindowsHookEx(hook);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// callback function to handle input event
	switch (message) {
		case WM_INPUT:
		{
			// input event
			// first get input data structure
			memset(event, 0, eventSize);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, event, &eventSize, sizeof(RAWINPUTHEADER)); // copy data into buffer
		
			// get key pressed
			keyStroke inputButton;
			inputButton.virtualKeyCode = event->data.keyboard.VKey;  // which key is pressed
			inputButton.keyPressStatus = event->data.keyboard.Flags; // key is up or down

			// get device info
			memset(stringBuffer, 0, strBufSize);
			GetRawInputDeviceInfo(rawInputDevice, RIDI_DEVICENAME, stringBuffer, &strBufSize);
			cout << stringBuffer << "\n";
			wstring wDeviceName(stringBuffer);
			log(keyStrokeInfo + string(wDeviceName.begin(), wDeviceName.end()) + " " + to_string(inputButton.virtualKeyCode) + " " + (inputButton.keyPressStatus == 0 ? "DOWN" : "UP") + "\n");

			// if OK:
			approvedRawInputs.push_back(inputButton);
			
		}
		case WM_HOOK:
		{
			// get a struct from keyboard hook dll about a input,
			// find in approvedInputs to match any same button
			log("ask from dll!");
			//PCOPYDATASTRUCT pcds = reinterpret_cast<PCOPYDATASTRUCT>(lParam);
			//keyStroke *target = reinterpret_cast<keyStroke*>(pcds->lpData);
			//
			//for (deque<keyStroke>::iterator iter = approvedRawInputs.begin(); iter != approvedRawInputs.end(); ) {
			//	if (iter->virtualKeyCode == target->virtualKeyCode &&
			//		iter->keyPressStatus == target->keyPressStatus) {
			//		iter = approvedRawInputs.erase(iter);
			//		ReplyMessage(0); // approved!
			//		return 0;
			//	}
			//	else {
			//		iter++;
			//	}
			//}
			ReplyMessage(1); // no msg find
			return 1;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);

}