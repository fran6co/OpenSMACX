// Narrow host owners for actual source-extracted functions, not a game model.
#include <cassert>
#include <climits>
#include <stdint.h>
#include <cstring>
#ifndef __cdecl
#define __cdecl
#define __stdcall
#endif
using std::memcpy;
static int StoredScaleAdjustment;
static uint32_t StatusFlags;
static int desktop_calls;
static int desktop_init(int mode) {
    assert(mode == 1 && StatusFlags == 0);
    ++desktop_calls;
    return 37;
}
struct RECT { int left, top, right, bottom; };
static int bubble_calls;
static char *observed_text;
static RECT observed_rectangle;
struct Win {
    void client_to_screen(int *x, int *y) { assert(bubble_calls == 0); *x += 11; *y -= 7; }
    void set_bubble_text(char *text, RECT *rectangle) {
        observed_text = text; observed_rectangle = *rectangle; ++bubble_calls;
    }
};
struct MainInterface : Win {};
static MainInterface MainInterfaceGlobal;
/* EXTRACTED_OWNED_HELPERS */

static int native_scale(int setting, int value) {
    uint32_t bits = (static_cast<uint32_t>(setting) + 16u) * static_cast<uint32_t>(value);
    // Use a different signed reconstruction to avoid repeating body memcpy.
    int64_t signed_value = bits;
    if (bits > 0x7FFFFFFFu) signed_value -= 0x100000000LL;
    return static_cast<int>(signed_value / 16);
}
int main() {
    assert(sizeof(int) == 4 && sizeof(uint32_t) == 4);
    const int settings[] = {INT_MIN, -100, -1, 0, 1, 16, 57, INT_MAX};
    const int values[] = {INT_MIN, -1001, -17, -1, 0, 1, 17, 1001, INT_MAX};
    for (unsigned s = 0; s < sizeof(settings)/sizeof(settings[0]); ++s) {
        StoredScaleAdjustment = 7;
        const int expected = settings[s] > 0 ? settings[s] : 0;
        assert(set_scale_adjustment(settings[s]) == expected);
        assert(StoredScaleAdjustment == expected);
        for (unsigned v = 0; v < sizeof(values)/sizeof(values[0]); ++v)
            assert(apply_scale_adjustment(values[v]) == native_scale(expected, values[v]));
    }
    StatusFlags = 0xFFFFFFFFu;
    assert(reset_status_and_initialize_desktop() == 37);
    assert(desktop_calls == 1 && StatusFlags == 0);
    char text[] = "Workshop";
    main_interface_bubble_callback_4396b0(text, 20, 30);
    assert(bubble_calls == 1 && observed_text == text);
    assert(observed_rectangle.left == 31 && observed_rectangle.top == 23);
    assert(observed_rectangle.right == 31 && observed_rectangle.bottom == 23);
}
