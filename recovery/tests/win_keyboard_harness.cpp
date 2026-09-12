// The live Win keyboard body is inserted by the test, never copied here.
// Window/scroll/OS doubles isolate the global-access and callback branches.
#include <assert.h>
#include <string.h>
#define nullptr 0
typedef unsigned char uint8_t;
struct RECT { int left, top, right, bottom; };
struct ScrollSlots {
    int calls, handled;
    int slot025(int key, int modifiers) {
        assert(key == 'g' && modifiers == 23); ++calls; return handled;
    }
};
struct Win {
    void *guard;
    unsigned int iSomeFlag_;
    ScrollSlots *scroll_vert_, *scroll_horz_;
    int released, shown, updated, flipped, activated, fallback, query;
    void on_char(char, int);
    int is_visible() { return 1; }
    int is_descendant(Win *) { return 0; }
    void show(int value) { assert(value == 0); ++shown; }
    void update_screen(RECT *, int value) { assert(value == 0); ++updated; }
    void flip(RECT *) { ++flipped; }
    void vslot_04() { ++released; }
    void release_modal() { ++released; }
    int vslot_23() { return query; }
    void vslot_07() { ++activated; }
    void key_click_event(int key, int modifiers) {
        assert(key == 'g' && modifiers == 23); ++fallback;
    }
};
Win WinBubbleWindow, other, root1, root2, receiver;
Win *WinBubbleCompanion, *WinPopupWindow, *WinPointerOwner1, *WinPointerOwner2;
Win *WinPointerOwner3, *WinPointerOwner4, *WinFocusWindow, *WinTopDialog;
Win *WinZOrderWindow, *WinDialogList[4], *WinRootWindows[4];
int WinBubbleActive, WinDrawFlags, WinPopupCount, JackalInitFlags;
int WinZOrderCount, WinZOrderFlag, WinRootCount, WinKeyModifiers = 23;
RECT WinBubbleRect;
const char WinMdebugCode[] = "mdebug";
char WinKeyRing[10], *WinKeyRingCursor;
void *HandleMain = &receiver;
int invalidations, traversals, shrink_roots;
void InvalidateRect(void *window, int area, int erase) {
    assert(window == HandleMain && !area && !erase); ++invalidations;
}
void recurse_zorder(Win *window) {
    assert(window == &root1 || window == &root2); ++traversals;
    if (window == &root1) {
        WinZOrderWindow = &root2;
        WinZOrderFlag = 42;
        if (shrink_roots) WinRootCount = 1;
    }
}

// LIVE_KEYBOARD

void arm() {
    memset(WinKeyRing, 0, sizeof(WinKeyRing));
    memcpy(WinKeyRing, "mdebu", 5);
    WinKeyRingCursor = WinKeyRing + 5;
    WinDrawFlags = 1;
    JackalInitFlags = 0;
    WinPointerOwner3 = 0;
    WinPointerOwner4 = 0;
    WinTopDialog = 0;
    receiver.fallback = 0;
}

int main() {
    // This first case safely exposes the old pointee write before testing NULL.
    // A different window's first word resembles the bubble pointer; it must
    // neither compare equal to the bubble nor be overwritten.
    arm(); other.guard = &WinBubbleWindow; WinPointerOwner4 = &other;
    receiver.on_char('g', 0);
    assert(WinPointerOwner4 == &other && other.guard == &WinBubbleWindow);
    assert(receiver.fallback == 1 && WinKeyRingCursor == WinKeyRing + 6);
    assert((WinDrawFlags & 1) == 0);

    arm(); receiver.on_char('g', 0); assert(!WinPointerOwner4);
    arm(); WinPointerOwner4 = &WinBubbleWindow; receiver.on_char('g', 0);
    assert(!WinPointerOwner4);
    arm(); WinPointerOwner3 = &WinBubbleWindow; receiver.on_char('g', 0);
    assert(!WinPointerOwner3 && WinBubbleWindow.released == 1);

    arm(); JackalInitFlags = 1; WinRootCount = 2;
    WinRootWindows[0] = &root1; WinRootWindows[1] = &root2;
    root1.iSomeFlag_ = root2.iSomeFlag_ = 1;
    WinZOrderWindow = &root1; WinZOrderFlag = 41; WinZOrderCount = 17;
    WinTopDialog = &other; receiver.on_char('g', 0);
    assert(traversals == 2 && invalidations == 1 && other.activated == 1);
    assert(WinZOrderFlag == 0 && WinZOrderCount == 0);

    // The traversal callback can change both the selected window and count.
    arm(); JackalInitFlags = 1; WinRootCount = 2; shrink_roots = 1;
    WinZOrderWindow = &root1; traversals = 0;
    receiver.on_char('g', 0);
    assert(traversals == 1 && WinZOrderFlag == 42 && invalidations == 2);
    arm(); JackalInitFlags = 1; WinRootCount = 0;
    receiver.on_char('g', 0); assert(invalidations == 3);

    // Unmatched input still routes vertical, horizontal, then fallback.
    ScrollSlots vertical = {0, 1}, horizontal = {0, 1};
    arm(); WinKeyRing[0] = 'x'; receiver.scroll_vert_ = &vertical;
    receiver.scroll_horz_ = &horizontal; receiver.on_char('g', 0);
    assert(vertical.calls == 1 && horizontal.calls == 0 && receiver.fallback == 0);
    arm(); vertical.handled = 0; receiver.on_char('g', 0);
    assert(vertical.calls == 2 && horizontal.calls == 1 && receiver.fallback == 0);
    arm(); horizontal.handled = 0; receiver.on_char('g', 0);
    assert(receiver.fallback == 1);
    return 0;
}
