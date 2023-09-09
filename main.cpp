#include <iostream>
#include <stdio.h>
#include <string>
#include "Windows.h"
#include "xinput.h"

using namespace std;

static string input_mode = "XInput";
typedef unsigned char BYTE;

void handleDirectInputGamePlay(BYTE& byte0, BYTE& byte1, BYTE& byte2)
{
	// from jvsemu
	if (input_mode != "DirectInput")
	{
		return;
	}

	JOYINFOEX joy;
	joy.dwSize = sizeof(joy);
	joy.dwFlags = JOY_RETURNALL;
	for (UINT joystickIndex = 0; joystickIndex < 16; ++joystickIndex)
	{
		if (joyGetPosEx(joystickIndex, &joy) == JOYERR_NOERROR)
		{
			if (joy.dwPOV == 0)
			{
				OutputDebugStringA("Up Detected from Joystick");
				byte1 |= static_cast<char>(1 << 5);
			}
			if (joy.dwPOV == 4500)
			{
				OutputDebugStringA("Up Right Detected from Joystick");
				byte1 |= static_cast<char>(1 << 5);
				byte1 |= static_cast<char>(1 << 2);
			}
			if (joy.dwPOV == 9000)
			{
				OutputDebugStringA("Right Detected from Joystick");
				byte1 |= static_cast<char>(1 << 2);
			}
			if (joy.dwPOV == 13500)
			{
				OutputDebugStringA("Right Down Detected from Joystick");
				byte1 |= static_cast<char>(1 << 2);
				byte1 |= static_cast<char>(1 << 4);
			}
			if (joy.dwPOV == 18000)
			{
				OutputDebugStringA("Down Detected from Joystick");
				byte1 |= static_cast<char>(1 << 4);
			}
			if (joy.dwPOV == 22500)
			{
				OutputDebugStringA("Down Left Detected from Joystick");
				byte1 |= static_cast<char>(1 << 4);
				byte1 |= static_cast<char>(1 << 3);
			}
			if (joy.dwPOV == 27000)
			{
				OutputDebugStringA("Left Detected from Joystick");
				byte1 |= static_cast<char>(1 << 3);
			}
			if (joy.dwPOV == 31500)
			{
				OutputDebugStringA("Top Left Detected from Joystick");
				byte1 |= static_cast<char>(1 << 3);
				byte1 |= static_cast<char>(1 << 5);
			}
			//int intJoyDwButtons = (int)joy.dwButtons;
			//if (intJoyDwButtons & key_bind.ArcadeButton1)
			//{
			//	OutputDebugStringA("Button 1 Detected from Joystick");
			//	byte1 |= static_cast<char> (1 << 1);
			//}
			//if (intJoyDwButtons & key_bind.ArcadeButton2)
			//{
			//	OutputDebugStringA("Button 2 Detected from Joystick");
			//	byte1 |= static_cast<char> (1);
			//}
			//if (intJoyDwButtons & key_bind.ArcadeButton3)
			//{
			//	OutputDebugStringA("Button 3 Detected from Joystick");
			//	byte2 |= static_cast<char> (1 << 7);
			//}
			//if (intJoyDwButtons & key_bind.ArcadeButton4)
			//{
			//	OutputDebugStringA("Button 4 Detected from Joystick");
			//	byte2 |= static_cast<char> (1 << 6);
			//}
			//if (intJoyDwButtons & key_bind.ArcadeStartButton)
			//{
			//	OutputDebugStringA("Start Button Detected from Joystick");
			//	byte1 |= static_cast<char>(1 << 7);
			//}
			//if (intJoyDwButtons & key_bind.ArcadeTest)
			//{
			//	OutputDebugStringA("Test Button Detected from Joystick");
			//	byte0 |= static_cast<char>(1 << 7);
			//}
		}
	}
}


void handleXInputGamePlay() {
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS)
		{
			OutputDebugStringA(string("XInput controller detected: id = " + to_string(i) + '\n').c_str());
			// Controller is connected
		}
		else
		{
			// Controller is not connected
		}
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	CoInitialize(0);
	BYTE byte0 = 0;
	BYTE byte1 = 0;
	BYTE byte2 = 0;
	OutputDebugStringA(string("START capturing inputs\nMode = " + input_mode + '\n').c_str());
	//OutputDebugStringA();
	while (true) {
		if (input_mode == "DirectInput") {
			handleDirectInputGamePlay(byte0, byte1, byte2);
		}
		else if (input_mode == "XInput") {
			handleXInputGamePlay();
			break;
		}
	}
	CoUninitialize();

}
