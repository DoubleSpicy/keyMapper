// dllmain.cpp : Defines the entry point for the DLL application.
//#include "pch.h"


#include "windows.h"
extern "C" {
    __declspec(dllexport) LRESULT KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam) {

        MessageBoxA(nullptr, "Hello World!",
            nullptr, 0);
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    __declspec(dllexport) void HELLOWORLD() {
        MessageBoxA(nullptr, "Hello World!",
            nullptr, 0);
    }
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason,
    LPVOID lpvReserved) {

    return TRUE;
}