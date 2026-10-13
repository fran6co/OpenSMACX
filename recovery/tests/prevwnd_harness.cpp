#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

#define __stdcall
#define CALLBACK
typedef void *HWND;
typedef unsigned int UINT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
// A host-width window-long return keeps fake object pointers representable;
// the game is 32-bit and this fixture does not establish native Windows ABI.
typedef intptr_t LONG;
typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

static const int GWL_USERDATA = -21;
struct Win {
    virtual ~Win() {}
    virtual void on_size(unsigned int, int, int) = 0;
    virtual int on_set_cursor(void *, unsigned int, unsigned int) = 0;
    static LRESULT __stdcall window_proc(HWND, UINT, WPARAM, LPARAM);
};

struct ProbeWin : Win {
    ProbeWin() : size_calls(0), cursor_calls(0), result(73), size_type(0),
                 width(0), height(0), cursor(0), hit(0), mouse_message(0) {}
    void on_size(unsigned int type, int x, int y) {
        ++size_calls; size_type = type; width = x; height = y;
    }
    int on_set_cursor(void *c, unsigned int h, unsigned int m) {
        ++cursor_calls; cursor = c; hit = h; mouse_message = m; return result;
    }
    int size_calls, cursor_calls, result;
    unsigned int size_type;
    int width, height;
    void *cursor;
    unsigned int hit, mouse_message;
};

static ProbeWin *userdata_owner;
static std::vector<int> userdata_reads;
static int fallback_calls;
static HWND fallback_hwnd;
static UINT fallback_message;
static WPARAM fallback_wparam;
static LPARAM fallback_lparam;
static LRESULT fallback_result = 0x3456;

LONG GetWindowLongA(HWND hwnd, int index) {
    assert(index == GWL_USERDATA);
    userdata_reads.push_back(static_cast<int>(reinterpret_cast<intptr_t>(hwnd)));
    return reinterpret_cast<intptr_t>(userdata_owner);
}

static WPARAM target_wparam(uint32_t bits) {
    return static_cast<WPARAM>(bits);
}

static LPARAM target_lparam(uint32_t bits) {
    return static_cast<LPARAM>(static_cast<int32_t>(bits));
}

LRESULT Win::window_proc(HWND, UINT, WPARAM, LPARAM) { return 0; }

LRESULT CallWindowProcA(WNDPROC proc, HWND hwnd, UINT message,
                        WPARAM wparam, LPARAM lparam) {
    assert(proc == &Win::window_proc);
    ++fallback_calls;
    fallback_hwnd = hwnd; fallback_message = message;
    fallback_wparam = wparam; fallback_lparam = lparam;
    return fallback_result;
}

/* ACTUAL_SOURCE */

int main() {
    /* WNDPROC_ASSIGNMENT */
    ProbeWin owner = {};
    HWND hwnd = reinterpret_cast<HWND>(static_cast<intptr_t>(0x1234));

    userdata_owner = &owner;
    userdata_reads.clear();
    assert(PrevWndFunc(hwnd, 5, target_wparam(0xFFFFFFFFu),
                       target_lparam(0xFFFF8000u)) == 0);
    assert(userdata_reads.size() == 1 && userdata_reads[0] == 0x1234);
    assert(owner.size_calls == 1 && owner.cursor_calls == 0);
    assert(owner.size_type == 0xFFFFFFFFu);
    assert(owner.width == -32768 && owner.height == -1);
    assert(fallback_calls == 0);

    userdata_reads.clear();
    const LPARAM packed = target_lparam(0xFFFF8000u);
    assert(PrevWndFunc(hwnd, 5, target_wparam(0x80000001u), packed) == 0);
    assert(userdata_reads.size() == 1);
    assert(owner.size_calls == 2 && owner.size_type == 0x80000001u);
    assert(owner.width == -32768 && owner.height == -1);

    userdata_reads.clear();
    const WPARAM cursor_bits = target_wparam(0x12345678u);
    assert(PrevWndFunc(hwnd, 0x20, cursor_bits,
                       target_lparam(0xABCD8001u)) == owner.result);
    assert(userdata_reads.size() == 1);
    assert(owner.cursor_calls == 1);
    assert(reinterpret_cast<uintptr_t>(owner.cursor) == 0x12345678u);
    assert(owner.hit == 0x8001u && owner.mouse_message == 0xABCDu);
    assert(fallback_calls == 0);

    userdata_reads.clear();
    userdata_owner = 0;
    assert(PrevWndFunc(hwnd, 5, target_wparam(0), target_lparam(0)) == 0);
    assert(userdata_reads.size() == 1 && fallback_calls == 0);
    assert(PrevWndFunc(hwnd, 0x20, target_wparam(0), target_lparam(0)) == 0);
    assert(userdata_reads.size() == 2 && fallback_calls == 0);

    userdata_owner = &owner;
    userdata_reads.clear();
    const LPARAM arbitrary_lparam = target_lparam(0xFEDCBA98u);
    const WPARAM arbitrary_wparam = target_wparam(0x76543210u);
    assert(PrevWndFunc(hwnd, 0x777, arbitrary_wparam,
                       arbitrary_lparam) == fallback_result);
    assert(userdata_reads.empty());
    assert(fallback_calls == 1 && fallback_hwnd == hwnd);
    assert(fallback_message == 0x777u && fallback_wparam == arbitrary_wparam);
    assert(fallback_lparam == arbitrary_lparam);

    std::puts("PrevWndFunc actual-source controls passed");
}
