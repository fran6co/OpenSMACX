#include <stdint.h>
#define __cdecl
#define WM_KEYDOWN 0x0100u
#define WM_LBUTTONDOWN 0x0201u
#define WM_LBUTTONDBLCLK 0x0203u
typedef uintptr_t HWND;
typedef uint32_t UINT;
typedef uint32_t WPARAM;
typedef int32_t LPARAM;
typedef int32_t LRESULT;
LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM);
extern HWND MciVideoCallbackWindow;
// ACTUAL_SOURCE_BODIES

#include <assert.h>
#include <stdio.h>

HWND MciVideoCallbackWindow = static_cast<HWND>(0x12345678u);

static int api_calls;
static HWND api_hwnd;
static UINT api_message;
static WPARAM api_wparam;
static LPARAM api_lparam;
static LRESULT api_result;

LRESULT DefWindowProcA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    ++api_calls;
    api_hwnd = hwnd;
    api_message = message;
    api_wparam = wparam;
    api_lparam = lparam;
    return api_result;
}

static void reset_api() {
    api_calls = 0;
    api_hwnd = 0;
    api_message = 0;
    api_wparam = 0;
    api_lparam = 0;
}

int main() {
    api_result = static_cast<LRESULT>(-37);

    // The key helper's false gate exits before the API call with zero EAX.
    reset_api();
    assert(sub_5ffe40(0xdeadbeefu, 0x41u, 0, -3, 32768) == 0);
    assert(api_calls == 0);

    // Key forwarding preserves the target HWND global, WPARAM and low 16 bits
    // of each signed payload dword; it returns DefWindowProcA's exact result.
    reset_api();
    assert(sub_5ffe40(0xdeadbeefu, 0x41u, 1, -3, 32768) == api_result);
    assert(api_calls == 1);
    assert(api_hwnd == MciVideoCallbackWindow);
    assert(api_message == WM_KEYDOWN && api_wparam == 0x41u);
    assert(static_cast<uint32_t>(api_lparam) == 0x8000fffdu);

    // Ordinary and double left-button dispatch share the same signed-coordinate
    // packing and preserve the original key-state WPARAM.
    reset_api();
    assert(sub_5ffe80(0xdeadbeefu, 0, -1, -32768, 0x101u) == api_result);
    assert(api_calls == 1 && api_hwnd == MciVideoCallbackWindow);
    assert(api_message == WM_LBUTTONDOWN && api_wparam == 0x101u);
    assert(static_cast<uint32_t>(api_lparam) == 0x8000ffffu);

    reset_api();
    assert(sub_5ffe80(0, -7, -32768, 32767, 0x202u) == api_result);
    assert(api_calls == 1 && api_message == WM_LBUTTONDBLCLK);
    assert(api_wparam == 0x202u);
    assert(static_cast<uint32_t>(api_lparam) == 0x7fff8000u);

    puts("MCI actual-body host controls: key gate/result and button down/double-click payloads pass.");
    return 0;
}
