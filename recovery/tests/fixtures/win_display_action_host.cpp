#include <assert.h>
#include <stdint.h>
#include <limits.h>

#define __cdecl
using uint32_t = unsigned int;
using int32_t = int;
static const int SW_MAXIMIZE = 3;
static const int SW_MINIMIZE = 6;

static int show_result;
static int show_mode;
static int zoom_result;
static int zoom_calls;
static int capture_lost;
static int slot52_calls;
static int hook_calls;
static int hook_result;
static int query_calls;
static int play_calls;
static int playing_result;
static int play_result;
static int event_count;
static int events[8];
static void event(int value) { events[event_count++] = value; }

struct Sound {
    int is_playing() { ++query_calls; event(3); return playing_result; }
    int play() { ++play_calls; event(4); return play_result; }
};
typedef int (__cdecl *WinDisplayActionHookFn)();
struct AutoSound { Sound *val_35_ = 0; };
struct Win : AutoSound {
    WinDisplayActionHookFn field_438_ = 0;
    void on_lose_mouse_capture() { ++capture_lost; event(1); }
    void vslot_52() { ++slot52_calls; event(2); }
    static int __cdecl display_action(int action);
};

static Win *WinCallbackWindow;
static Win *WinPointerOwner1;
static Win *WinPointerOwner2;
static Win *WinPointerOwner3;
static Win *WinPointerOwner4;
static void *HandleMain = reinterpret_cast<void *>(1);
static int IsZoomed(void *) { ++zoom_calls; return zoom_result; }
static int ShowWindow(void *, int mode) {
    show_mode = mode;
    event(5);
    return show_result;
}

// ACTUAL_METHOD_INSERTION

static int display_hook() { ++hook_calls; event(6); return hook_result; }

static void reset() {
    show_result = show_mode = -1;
    zoom_result = zoom_calls = capture_lost = slot52_calls = 0;
    hook_calls = query_calls = play_calls = event_count = 0;
    hook_result = playing_result = play_result = 0;
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner3 = WinPointerOwner4 = 0;
}

int main() {
    Win win;
    WinCallbackWindow = &win;

    reset();
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner3 = WinPointerOwner4 = &win;
    zoom_result = 1;
    show_result = 0x1234;
    assert(Win::display_action(0) == 0x1234);
    assert(zoom_calls == 1 && show_mode == SW_MAXIMIZE);
    assert(capture_lost == 1 && WinPointerOwner1 == 0 && WinPointerOwner2 == 0 &&
           WinPointerOwner3 == 0 && WinPointerOwner4 == 0);

    reset();
    show_result = -7;
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner3 = WinPointerOwner4 = &win;
    assert(Win::display_action(1) == -7);
    assert(zoom_calls == 0 && show_mode == SW_MINIMIZE && capture_lost == 1);

    reset();
    Sound sound;
    win.val_35_ = &sound;
    win.field_438_ = &display_hook;
    hook_result = 37;
    playing_result = 0x2468;
    assert(Win::display_action(2) == 0x2468);
    assert(slot52_calls == 1 && hook_calls == 1 && query_calls == 1 && play_calls == 0);
    assert(events[0] == 2 && events[1] == 6 && events[2] == 3);

    reset();
    hook_result = 37;
    playing_result = 0;
    play_result = -3;
    assert(Win::display_action(2) == -3);
    assert(slot52_calls == 1 && hook_calls == 1 && query_calls == 1 && play_calls == 1);

    reset();
    win.val_35_ = 0;
    hook_result = 41;
    assert(Win::display_action(2) == 41);

    reset();
    win.field_438_ = 0;
    assert(Win::display_action(2) == 0);

    reset();
    assert(Win::display_action(9) == 7);
    assert(Win::display_action(INT_MIN) == 0x7ffffffe);
    return 0;
}
