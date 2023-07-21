#pragma once
#include <Windows.h>

void InitializeMSAA(HINSTANCE hInstance);
void ShutdownMSAA();

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);


LRESULT CALLBACK WindowProc(int nCode, WPARAM wParam, LPARAM lParam);
