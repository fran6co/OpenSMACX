// Actual source bodies are inserted below. Doubles expose callback ordering and
// storage ownership, not SDK integration, real class layout or game acceptance.
#include <assert.h>
#include <string.h>
struct RECT { int left, top, right, bottom; };
struct POINT { int x, y; };
typedef void *HCURSOR;
#define MAKEINTRESOURCE(x) reinterpret_cast<const char *>(x)
#define IDC_ARROW MAKEINTRESOURCE(0x7f00)
const unsigned int WinFlagHitTestIndirect = 0x4000000;
struct Sprite { int iWidth_, iHeight_; };
struct Dib { struct { int biWidth, biHeight; } bmiHeader; };
struct Buffer {
    Dib dib_;
    int init(int, int, int, void *);
    int copy(Buffer *, int, int, int, int, int, int);
};
struct Win {
    unsigned int iFlags_, field_18C_, field_190_;
    Sprite *cursor_sprite_;
    HCURSOR *cursor_handle_;
    int cursor_name_;
    static Win *get_mouse_window(int *, int *);
    static int __cdecl update_cursor(Win *, int);
    static void flip(RECT *);
    int on_nc_hittest(int, int);
};
Win window;
Sprite cursor = {13, 17}, replacement = {23, 29};
Sprite *WinFlipSprite;
Buffer g_WIN_BUFFER, ScreenBuffer;
RECT WinDirtyRect;
RECT last_union;
int WinMouseDirect, WinClipWidth, WinClipHeight, WinDragOffsetX, WinDragOffsetY;
int WinFlipSpriteY, WinFlipSpriteX, WinFlipClipped, WinCursorMoved;
int hit, found, lookup, init_calls, save_calls, restore_calls, unions, flips;
int loads, sets, poll_up, poll_down, rect_helpers, expected_sprite;
int mutate_init, mutate_copy, mutate_flip, mutate_tail, resource;
HCURSOR selected;

int GetCursorPos(POINT *pt) { pt->x = 101; pt->y = 203; return 1; }
Win *Win::get_mouse_window(int *x, int *y) {
    assert(*x == 101 && *y == 203);
    ++lookup; *x = 7; *y = 11;
    return found ? &window : 0;
}
int Win::on_nc_hittest(int x, int y) {
    assert(x == (lookup ? 7 : 101) && y == (lookup ? 11 : 203));
    return hit;
}
int ShowCursor(int show) {
    if (show) { assert(++poll_up <= 3); return poll_up - 2; }
    assert(++poll_down <= 3); return 2 - poll_down;
}
HCURSOR LoadCursorA(void *module, const char *name) {
    assert(!module); ++loads; resource = reinterpret_cast<int>(name);
    return reinterpret_cast<HCURSOR>(0x4567);
}
HCURSOR SetCursor(HCURSOR value) {
    ++sets; selected = value;
    if (mutate_tail == 1) WinMouseDirect = 1;
    if (mutate_tail == 2) window.cursor_sprite_ = &replacement;
    if (mutate_tail == 3) { WinMouseDirect = 0; window.cursor_sprite_ = 0; }
    return reinterpret_cast<HCURSOR>(0x7654);
}
int UnionRect(RECT *dest, const RECT *old, const RECT *added) {
    assert(dest == &WinDirtyRect);
    assert(save_calls ? added == dest && old != dest : old == dest && added != dest);
    ++unions;
    const RECT incoming = *(save_calls ? old : added);
    const RECT previous = *dest;
    assert(incoming.left == WinFlipSpriteY && incoming.top == WinFlipSpriteX);
    assert(incoming.right == incoming.left + WinClipWidth);
    assert(incoming.bottom == incoming.top + WinClipHeight);
    *dest = incoming;
    if (previous.right > previous.left && previous.bottom > previous.top) {
        if (previous.left < dest->left) dest->left = previous.left;
        if (previous.top < dest->top) dest->top = previous.top;
        if (previous.right > dest->right) dest->right = previous.right;
        if (previous.bottom > dest->bottom) dest->bottom = previous.bottom;
    }
    last_union = *dest;
    // Deliberately distinguish this API result from the copy result. The
    // recovered path forwards UnionRect's return, not a synthesized boolean.
    return 37;
}
void make_rect(RECT *rect, int x, int y, int w, int h) {
    ++rect_helpers; rect->left = x; rect->top = y;
    rect->right = x + w; rect->bottom = y + h;
}
int Buffer::init(int w, int h, int tgl, void *draw) {
    assert(this == &g_WIN_BUFFER && w == 13 && h == 17 && !tgl && !draw);
    ++init_calls; dib_.bmiHeader.biWidth = w; dib_.bmiHeader.biHeight = -h;
    if (mutate_init) {
        window.cursor_sprite_ = &replacement;
        WinClipWidth = 23; WinClipHeight = 29;
    }
    return 0;
}
int Buffer::copy(Buffer *dest, int sx, int sy, int dx, int dy, int w, int h) {
    assert(w == WinClipWidth && h == WinClipHeight);
    if (this == &ScreenBuffer) {
        assert(dest == &g_WIN_BUFFER && sx == 98 && sy == 198 && !dx && !dy);
        assert(WinFlipSprite == window.cursor_sprite_);
        ++save_calls;
    } else {
        assert(this == &g_WIN_BUFFER && dest == &ScreenBuffer);
        assert(!sx && !sy && dx == WinFlipSpriteY && dy == WinFlipSpriteX);
        ++restore_calls;
    }
    if (mutate_copy) {
        WinFlipSpriteY = 41; WinFlipSpriteX = 43;
        WinClipWidth = 47; WinClipHeight = 53;
    }
    return 91;
}
void Win::flip(RECT *rect) {
    assert(rect == &WinDirtyRect);
    assert(WinFlipSprite == (expected_sprite ? window.cursor_sprite_ : 0));
    assert(memcmp(rect, &last_union, sizeof(RECT)) == 0);
    if (expected_sprite) assert(WinFlipClipped == 1);
    ++flips; rect->right = 123;
    if (mutate_flip) WinFlipSprite = &replacement;
}

// LIVE_RESTORE
// LIVE_RECT_HELPERS
// LIVE_CURSOR

void arm() {
    memset(&window, 0, sizeof(window));
    memset(&g_WIN_BUFFER, 0, sizeof(g_WIN_BUFFER));
    memset(&WinDirtyRect, 0, sizeof(WinDirtyRect));
    WinFlipSprite = 0; WinMouseDirect = 0; WinCursorMoved = 0;
    WinClipWidth = 13; WinClipHeight = 17;
    WinDragOffsetX = 3; WinDragOffsetY = 5;
    WinFlipSpriteY = 31; WinFlipSpriteX = 37; WinFlipClipped = 9;
    window.field_18C_ = 3; window.field_190_ = 5;
    hit = 0; found = 1;
    lookup = init_calls = save_calls = restore_calls = unions = flips = 0;
    loads = sets = poll_up = poll_down = rect_helpers = expected_sprite = 0;
    mutate_init = mutate_copy = mutate_flip = mutate_tail = resource = 0; selected = 0;
}
int main() {
    // Sprite storage uses dimensions, saves screen -> backing buffer, retains
    // screen coordinates despite hit lookup, and re-reads callback state.
    for (int tgl = 0; tgl != 2; ++tgl) {
        arm(); window.cursor_sprite_ = &cursor; expected_sprite = 1;
        mutate_init = mutate_copy = 1;
        assert(Win::update_cursor(0, tgl) == (tgl ? 123 : 37));
        assert(init_calls == 1 && save_calls == 1 && !restore_calls);
        assert(poll_down == 3 && WinFlipSprite == &replacement);
        assert(unions == 1 && flips == tgl && WinFlipClipped == 9);
        assert(WinDirtyRect.left == (tgl ? 123 : 41));
        assert(WinDirtyRect.right == (tgl ? 123 : 88));
        assert(WinDirtyRect.bottom == 96);
    }
    // Matching DIB dimensions avoid allocation; a retained same sprite also
    // avoids allocation and restores its old area before taking the new copy.
    for (int same = 0; same != 2; ++same) {
        arm(); window.cursor_sprite_ = &cursor;
        g_WIN_BUFFER.dib_.bmiHeader.biWidth = 13;
        g_WIN_BUFFER.dib_.bmiHeader.biHeight = -17;
        if (same) WinFlipSprite = &cursor;
        assert(Win::update_cursor(&window, 0) == 37);
        assert(!init_calls && save_calls == 1 && restore_calls == same);
    }
    arm(); window.cursor_sprite_ = &cursor;
    g_WIN_BUFFER.dib_.bmiHeader.biWidth = 13;
    g_WIN_BUFFER.dib_.bmiHeader.biHeight = -19;
    assert(Win::update_cursor(&window, 0) == 37 && init_calls == 1);
    arm(); window.cursor_sprite_ = &cursor;
    WinDirtyRect.left = -8; WinDirtyRect.top = -9;
    WinDirtyRect.right = 400; WinDirtyRect.bottom = 500;
    assert(Win::update_cursor(&window, 0) == 37);
    assert(WinDirtyRect.left == -8 && WinDirtyRect.top == -9);
    assert(WinDirtyRect.right == 400 && WinDirtyRect.bottom == 500);
    // Borrowed OS cursor handles bypass resource loading, including restoration
    // of the former sprite. The resource-name path retains all argument bits.
    arm(); HCURSOR handle = reinterpret_cast<HCURSOR>(0x12345678);
    window.cursor_handle_ = &handle; WinFlipSprite = &cursor;
    assert(Win::update_cursor(&window, 0) == 0x7654);
    assert(!loads && sets == 1 && selected == handle && !WinFlipSprite);
    assert(restore_calls == 1 && flips == 1 && poll_up == 3);
    assert(!WinDirtyRect.left && !WinDirtyRect.right);
    arm(); window.cursor_name_ = 0x12345678;
    assert(Win::update_cursor(&window, 0) == 0x7654);
    assert(loads == 1 && resource == 0x12345678);
    arm(); window.cursor_handle_ = &handle; handle = 0;
    assert(Win::update_cursor(&window, 0) == 0x7654);
    assert(loads == 1 && resource == 0x7f00);
    // Independently decoded jump table at 5F2200, index = hit + 2.
    const int cursors[20] = {0,0,0x7f00,0,0x7f00,0x7f00,0x7f00,0x7f00,
        0x7f00,0x7f00,0x7f00,0x7f00,0x7f84,0x7f84,0x7f85,0x7f82,
        0x7f83,0x7f85,0x7f83,0x7f82};
    for (int index = -1; index <= 20; ++index) {
        arm(); window.iFlags_ = WinFlagHitTestIndirect;
        hit = index - 2; WinFlipSprite = &cursor; mutate_flip = 1;
        int chosen = index < 0 || index >= 20 ? 0x7f00 : cursors[index];
        assert(Win::update_cursor(0, 0) == 1);
        assert(poll_up == 3 && restore_calls == 1 && flips == 1);
        assert(loads == (chosen != 0) && resource == chosen);
        assert(WinFlipSprite == (chosen ? 0 : &cursor));
        assert(!WinDirtyRect.left && !WinDirtyRect.right);
        assert(WinCursorMoved == (index == 2 || index == 4 || (index >= 12 && index <= 19)));
        assert(rect_helpers == (index != 2 && index != 4 && index != 12));
    }
    arm(); found = 0; WinFlipSprite = &cursor; mutate_flip = 1;
    assert(Win::update_cursor(0, 0) == 0x7654);
    assert(flips == 1 && !WinFlipSprite && !poll_up && resource == 0x7f00);
    for (int direct = 0; direct != 2; ++direct) {
        for (int index = 0; index != 5; ++index) {
            arm(); window.iFlags_ = WinFlagHitTestIndirect;
            WinMouseDirect = direct; hit = index - 2; WinFlipSprite = &cursor;
            if (!direct) window.cursor_sprite_ = &cursor;
            int changes = index == 2 || index == 4;
            assert(Win::update_cursor(&window, 0) == (changes ? 0x7654 : index));
            assert(!poll_up && flips == changes && restore_calls == changes);
            assert(WinFlipSprite == (changes ? 0 : &cursor));
        }
    }
    // Predicate reads happen after selection callbacks, in both directions.
    for (int mutation = 1; mutation != 4; ++mutation) {
        arm(); window.iFlags_ = WinFlagHitTestIndirect;
        hit = 0; mutate_tail = mutation;
        if (mutation == 3) { WinMouseDirect = 1; window.cursor_sprite_ = &cursor; }
        assert(Win::update_cursor(&window, 0) == (mutation == 3 ? 1 : 0x7654));
        assert(poll_up == (mutation == 3 ? 3 : 0));
    }
    return 0;
}
