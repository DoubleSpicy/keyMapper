// dllmain.cpp : Defines the entry point for the DLL application.
//#include "pch.h"

#include "stdio.h"
#include "windows.h"
#include <iostream>
#include <thread>
static HWND parentProcess;
UINT const WM_HOOK = WM_APP + 1;
static HINSTANCE instanceHandle;
static HHOOK hookHandle;
// keyboard hooker to decide pass event to next hooker or not.

// exposed to parent hwnd
static RAWINPUT* raw;
static RAWINPUT keyStrokeData


void msgLoop() {
    while (true) {
        std::cout << "loop...\n";
    }
}

extern "C" {
    BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
        {
        switch (ul_reason_for_call)
	    {
		    case DLL_PROCESS_ATTACH:
 			    instanceHandle = hModule;
			    hookHandle = NULL;
  			    break;
		    case DLL_THREAD_ATTACH:
		    case DLL_THREAD_DETACH:
 		    case DLL_PROCESS_DETACH:
  			    break;
  	    }
  	    return TRUE;
        }

    __declspec(dllexport) LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode != HC_ACTION) {
            // do nothing if not a input event
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }
        OutputDebugStringA("OK");
        // call mother thread to judge related RawInputEvent
        SendMessage(parentProcess, WM_APP, wParam, lParam);
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }


    __declspec(dllexport) LRESULT KeyboardProcHelloWorld(
        int nCode, WPARAM wParam, LPARAM lParam) {

        //std::cout << "Key Pressed!" << std::endl;
        UINT bufferSize;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
        LPBYTE dataBuffer = new BYTE[bufferSize];
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, dataBuffer, &bufferSize, sizeof(RAWINPUTHEADER));
        raw = (RAWINPUT*)dataBuffer;
        OutputDebugStringA("OK");

        // get device info
        
        //GetRawInputDeviceInfo(rawInputDevice, RIDI_DEVICENAME, stringBuffer, &strBufSize);

        //auto wDeviceName = wstring(stringBuffer);
        SendMessage(parentProcess, WM_HOOK, wParam, lParam);
        //MessageBoxA(nullptr, "Hello World from keyProc!!",
        //    nullptr, 0);
        delete dataBuffer;
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    __declspec(dllexport) LRESULT JoyProc(
        int nCode, WPARAM wParam, LPARAM lParam) {
        std::cout << "Key Pressed!" << std::endl;

        MessageBoxA(nullptr, "Hello World!",
            nullptr, 0);
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    bool __declspec(dllexport) __stdcall installHook(HWND parentHWND) {
        if (!parentHWND) return false;
        parentProcess = parentHWND;

        hookHandle = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProcHelloWorld, instanceHandle, 0);
        //std::thread dummy(msgLoop);
        return (hookHandle == NULL ? false : true);
    }


}


//BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason,
//    LPVOID lpvReserved) {
//
//    return TRUE;
//}

