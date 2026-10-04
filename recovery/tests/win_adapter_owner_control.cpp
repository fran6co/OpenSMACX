
#include <stdint.h>

#define __cdecl
typedef uintptr_t HWND;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef intptr_t LONG;
typedef unsigned int UINT;

#define GWL_USERDATA (-21)

typedef void *HDC;
typedef int BOOL;
struct RECT { long left, top, right, bottom; };
struct PAINTSTRUCT { HDC hdc; BOOL fErase; RECT rcPaint; };
HDC BeginPaint(HWND, PAINTSTRUCT *);
BOOL EndPaint(HWND, const PAINTSTRUCT *);

struct WINDOWPOS {
    HWND hwnd;
    HWND hwndInsertAfter;
    int x;
    int y;
    int cx;
    int cy;
    unsigned int flags;
};

// Typed host seam uses the production Win virtual signatures. It is not an
// application class/layout model; native layout/vtable validation is out of
// scope for this host-only control.
class Win {
public:
    virtual LRESULT on_window_pos_changing(WINDOWPOS *pos) = 0;
    virtual LRESULT on_window_pos_changed(WINDOWPOS *pos) = 0;
    virtual void on_move(int x, int y) = 0;
    virtual void on_size(unsigned int flags, int width, int height) = 0;
    virtual void on_size_nc(unsigned int flags, int width, int height) = 0;
    virtual void on_mouse_move(int, int, unsigned int, int) = 0;
    virtual int on_set_cursor(void *, unsigned int, unsigned int) = 0;
    virtual void on_nc_paint(RECT *, int) = 0;
};

extern HWND HandleMain;
LONG GetWindowLongA(HWND hwnd, int index);
LRESULT DefWindowProcA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

// ACTUAL_SOURCE_BODIES

#include <assert.h>
#include <stdio.h>

HWND HandleMain = 10;

static HWND mapped_hwnd = 0;
static Win *mapped_owner = 0;
static int lookup_count = 0;
static int default_count = 0;
static HWND default_hwnd = 0;
static UINT default_message = 0;
static WPARAM default_wparam = 0;
static LPARAM default_lparam = 0;
static LRESULT default_result = 0x5678;
static PAINTSTRUCT *current_paint = 0;
static int paint_phase = 0;
static HDC paint_dc = 0;
static BOOL paint_result = 0;
static HWND paint_hwnd = 0;

HDC BeginPaint(HWND hwnd, PAINTSTRUCT *paint) {
    assert(paint_phase == 0);
    paint_phase = 1;
    current_paint = paint;
    paint_hwnd = hwnd;
    paint->hdc = paint_dc;
    paint->rcPaint.left = -7;
    paint->rcPaint.top = 11;
    paint->rcPaint.right = 37;
    paint->rcPaint.bottom = 59;
    return paint_dc;
}

BOOL EndPaint(HWND hwnd, const PAINTSTRUCT *paint) {
    assert(paint_phase == 2 && hwnd == paint_hwnd && paint == current_paint);
    assert(paint->rcPaint.left == -7 && paint->rcPaint.top == 11);
    assert(paint->rcPaint.right == 37 && paint->rcPaint.bottom == 59);
    paint_phase = 3;
    return paint_result;
}

LONG GetWindowLongA(HWND hwnd, int index) {
    ++lookup_count;
    assert(index == GWL_USERDATA);
    return hwnd == mapped_hwnd ? reinterpret_cast<LONG>(mapped_owner) : 0;
}

LRESULT DefWindowProcA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    ++default_count;
    default_hwnd = hwnd;
    default_message = message;
    default_wparam = wparam;
    default_lparam = lparam;
    return default_result;
}

struct EventSpy : public Win {
    int changing_count;
    int changed_count;
    int move_count;
    int size_count;
    int size_nc_count;
    WINDOWPOS *last_pos;
    int x;
    int y;
    unsigned int flags;
    int width;
    int height;
    LRESULT changing_result;
    LRESULT changed_result;
    unsigned int mouse_keys;
    int mouse_parent;
    int mouse_count;
    void *cursor;
    unsigned int hit;
    unsigned int message;
    int cursor_count;

    EventSpy() : changing_count(0), changed_count(0), move_count(0),
        size_count(0), size_nc_count(0), last_pos(0), x(0), y(0),
        flags(0), width(0), height(0), changing_result(0x1234),
        changed_result(0x2468), mouse_keys(0), mouse_parent(-1), mouse_count(0),
        cursor(0), hit(0), message(0), cursor_count(0) {}

    virtual LRESULT on_window_pos_changing(WINDOWPOS *pos) {
        ++changing_count;
        last_pos = pos;
        return changing_result;
    }
    virtual LRESULT on_window_pos_changed(WINDOWPOS *pos) {
        ++changed_count;
        last_pos = pos;
        return changed_result;
    }
    virtual void on_move(int px, int py) {
        ++move_count;
        x = px;
        y = py;
    }
    virtual void on_size(unsigned int f, int w, int h) {
        ++size_count;
        flags = f;
        width = w;
        height = h;
    }
    virtual void on_size_nc(unsigned int f, int w, int h) {
        ++size_nc_count;
        flags = f;
        width = w;
        height = h;
    }
    virtual void on_mouse_move(int px, int py, unsigned int keys, int parent) {
        ++mouse_count;
        x = px; y = py; mouse_keys = keys; mouse_parent = parent;
    }
    virtual int on_set_cursor(void *c, unsigned int h, unsigned int m) {
        ++cursor_count;
        cursor = c; hit = h; message = m;
        return -37;
    }
    virtual void on_nc_paint(RECT *area, int f) {
        assert(paint_phase == 1 && f == -1);
        assert(area != &current_paint->rcPaint);
        assert(area->left == -7 && area->top == 11 && area->right == 37 && area->bottom == 59);
        area->left = 999;
        paint_phase = 2;
    }
};

static void set_owner(HWND hwnd, Win *owner) {
    mapped_hwnd = hwnd;
    mapped_owner = owner;
}

static void reset_boundary() {
    lookup_count = 0;
    default_count = 0;
}

int main() {
    EventSpy owner;
    WINDOWPOS pos;
    pos.hwnd = 20;
    pos.hwndInsertAfter = 0;
    pos.x = 1;
    pos.y = 2;
    pos.cx = 3;
    pos.cy = 4;
    pos.flags = 5;

    // Proven LRESULT path: owner dispatch returns the exact slot-68 result.
    set_owner(20, &owner);
    reset_boundary();
    assert(win_window_pos_changing(20, reinterpret_cast<LPARAM>(&pos)) == 0x1234);
    assert(owner.changing_count == 1 && owner.last_pos == &pos);
    assert(default_count == 0 && lookup_count == 1);

    // Main-window and absent-owner arms call DefWindowProcA with exact payload.
    set_owner(HandleMain, &owner);
    reset_boundary();
    assert(win_window_pos_changing(HandleMain, reinterpret_cast<LPARAM>(&pos)) == default_result);
    assert(default_count == 1 && default_hwnd == HandleMain);
    assert(default_message == 0x46 && default_wparam == 0 && default_lparam == reinterpret_cast<LPARAM>(&pos));
    set_owner(20, 0);
    reset_boundary();
    assert(win_window_pos_changing(20, reinterpret_cast<LPARAM>(&pos)) == default_result);
    assert(default_count == 1 && default_message == 0x46);
    assert(owner.changing_count == 1);

    // Proven LRESULT path: slot 69 result passes through; null userdata is zero.
    set_owner(20, &owner);
    reset_boundary();
    assert(win_window_pos_changed(20, reinterpret_cast<LPARAM>(&pos)) == 0x2468);
    assert(owner.changed_count == 1 && owner.last_pos == &pos);
    set_owner(20, 0);
    assert(win_window_pos_changed(20, reinterpret_cast<LPARAM>(&pos)) == 0);
    assert(owner.changed_count == 1);

    set_owner(20, &owner);
    win_window_sized(20, 3, 640, 480);
    assert(owner.size_count == 1 && owner.flags == 3 && owner.width == 640 && owner.height == 480);
    win_window_nonclient_sized(20, 4, 800, 600);
    assert(owner.size_nc_count == 1 && owner.flags == 4 && owner.width == 800 && owner.height == 600);
    set_owner(20, 0);
    win_window_sized(20, 9, 1, 2);
    win_window_nonclient_sized(20, 8, 3, 4);
    assert(owner.size_count == 1 && owner.size_nc_count == 1);

    set_owner(20, &owner);
    win_forward_size(20, 7, 320, 240);
    assert(owner.size_count == 2 && owner.flags == 7 && owner.width == 320 && owner.height == 240);
    win_forward_mouse_move(20, -13, 29, 0x8041u);
    assert(owner.mouse_count == 1 && owner.x == -13 && owner.y == 29);
    assert(owner.mouse_keys == 0x8041u && owner.mouse_parent == 0);
    win_window_moved(20, -5, 17);
    assert(owner.move_count == 1 && owner.x == -5 && owner.y == 17);
    assert(win_forward_set_cursor(20, &pos, 13, 0x200) == -37);
    assert(owner.cursor_count == 1 && owner.cursor == &pos && owner.hit == 13 && owner.message == 0x200);
    set_owner(20, 0);
    win_forward_size(20, 0, 0, 0);
    win_forward_mouse_move(20, 0, 0, 0);
    win_window_moved(20, 0, 0);
    assert(win_forward_set_cursor(20, &pos, 1, 2) == 0);
    assert(owner.size_count == 2 && owner.mouse_count == 1 && owner.move_count == 1 && owner.cursor_count == 1);
    paint_phase = 0;
    assert(win_window_nc_paint(20) == 0 && paint_phase == 0);
    set_owner(20, &owner);
    paint_dc = 0;
    assert(win_window_nc_paint(20) == 0 && paint_phase == 1);
    paint_phase = 0;
    paint_dc = &owner;
    paint_result = 37;
    assert(win_window_nc_paint(20) == 37 && paint_phase == 3);
    paint_phase = 0;
    paint_result = -1;
    assert(win_window_nc_paint(20) == -1 && paint_phase == 3);
    puts("Win adapter actual-body host controls: nine typed owner adapters pass.");
    return 0;
}
