// dllmain.cpp : Defines the entry point for the DLL application.
//#include "pch.h"

#include "stdio.h"
#include "windows.h"
#include <iostream>
extern "C" {
    __declspec(dllexport) LRESULT KeyboardProc(
        int nCode, WPARAM wParam, LPARAM lParam) {

        //std::cout << "Key Pressed!" << std::endl;
        
        MessageBoxA(nullptr, "Hello World!",
            nullptr, 0);
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    __declspec(dllexport) LRESULT JoyProc(
        int nCode, WPARAM wParam, LPARAM lParam) {
        std::cout << "Key Pressed!" << std::endl;

        MessageBoxA(nullptr, "Hello World!",
            nullptr, 0);
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason,
    LPVOID lpvReserved) {

    return TRUE;
}