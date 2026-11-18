#pragma once

#include <windows.h>

extern "C" LRESULT __stdcall PrevWndFunc(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam);
