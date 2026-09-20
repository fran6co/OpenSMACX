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
    Win *win_parent_, *val_4_, *children_[4], *descendant;
    int child_count_, visible, visibility_queries, descendant_queries;
    ScrollSlots *scroll_vert_, *scroll_horz_;
    int released, shown, updated, flipped, activated, fallback, query;
    void on_char(char, int);
    int is_visible() { ++visibility_queries; return visible; }
    int is_descendant(Win *);
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
struct MultiDebug : Win {};
MultiDebug WinBubbleWindow;
Win other, root1, root2, receiver, parent, child1, child2;
Win *WinBubbleCompanion, *WinPointerOwner1, *WinPointerOwner2;
Win *WinPointerOwner3, *WinPointerOwner4, *WinFocusWindow;
Win *WinZOrderWindow, *WinRootWindows[4];
int WinBubbleActive, JackalInitFlags;
int WinZOrderCount, WinZOrderFlag, WinRootCount, WinKeyModifiers = 23;
RECT WinBubbleRect;
const char WinMdebugCode[] = "mdebug";
char WinKeyRing[10], *WinKeyRingCursor;
void *HandleMain = &receiver;
int invalidations, traversals, shrink_roots, mutate_children;
int Win::is_descendant(Win *window) {
    ++descendant_queries;
    if (this == &child1) {
        if (mutate_children == 1) WinBubbleWindow.child_count_ = 1;
        if (mutate_children == 2) WinBubbleWindow.children_[1] = window;
        if (mutate_children == 3) {
            WinBubbleWindow.children_[1] = window;
            WinBubbleWindow.child_count_ = 2;
        }
    }
    return window == descendant;
}
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
    memset(&WinBubbleWindow, 0, sizeof(WinBubbleWindow));
    memset(&parent, 0, sizeof(parent));
    memset(&child1, 0, sizeof(child1));
    memset(&child2, 0, sizeof(child2));
    WinBubbleWindow.iSomeFlag_ = 0xa5;
    WinPointerOwner1 = WinPointerOwner2 = 0;
    mutate_children = 0;
    JackalInitFlags = 0;
    WinPointerOwner3 = 0;
    WinPointerOwner4 = 0;
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
    assert(WinBubbleWindow.iSomeFlag_ == 0xa4);

    arm(); receiver.on_char('g', 0); assert(!WinPointerOwner4);
    arm(); WinPointerOwner4 = &WinBubbleWindow; receiver.on_char('g', 0);
    assert(!WinPointerOwner4);
    arm(); WinPointerOwner3 = &WinBubbleWindow; receiver.on_char('g', 0);
    assert(!WinPointerOwner3 && WinBubbleWindow.released == 1);

    arm(); JackalInitFlags = 1; WinRootCount = 2;
    WinRootWindows[0] = &root1; WinRootWindows[1] = &root2;
    root1.iSomeFlag_ = root2.iSomeFlag_ = 1;
    WinZOrderWindow = &root1; WinZOrderFlag = 41; WinZOrderCount = 17;
    WinBubbleWindow.val_4_ = &other; receiver.on_char('g', 0);
    assert(traversals == 2 && invalidations == 1 && other.activated == 1);
    assert(WinZOrderFlag == 0 && WinZOrderCount == 0);

    // The traversal callback can change both the selected window and count.
    arm(); JackalInitFlags = 1; WinRootCount = 2; shrink_roots = 1;
    WinZOrderWindow = &root1; traversals = 0;
    receiver.on_char('g', 0);
    assert(traversals == 1 && WinZOrderFlag == 42 && invalidations == 2);
    arm(); JackalInitFlags = 1; WinRootCount = 0;
    receiver.on_char('g', 0); assert(invalidations == 3);

    // The real object's parent controls visibility; no independent popup alias.
    for (int visible = 0; visible != 2; ++visible) {
        arm(); WinBubbleWindow.win_parent_ = &parent; parent.visible = visible;
        receiver.on_char('g', 0);
        assert(parent.visibility_queries == 1);
        assert(WinBubbleWindow.shown == !visible);
        assert(WinBubbleWindow.iSomeFlag_ == (visible ? 0xa4U : 0xa5U));
    }
    arm(); WinBubbleWindow.iSomeFlag_ = 0xa4;
    WinBubbleWindow.win_parent_ = &parent;
    receiver.on_char('g', 0);
    assert(WinBubbleWindow.shown == 1 && !parent.visibility_queries);

    // Child callbacks may shrink/grow the count or replace a later member.
    // A stale count misses growth or visits child2 after shrink; a copied list
    // misses replacement. Owner2 exercises the fallback candidate selection.
    for (int mutation = 0; mutation != 4; ++mutation) {
        arm(); mutate_children = mutation;
        WinBubbleWindow.children_[0] = &child1;
        WinBubbleWindow.children_[1] = &child2;
        WinBubbleWindow.child_count_ = mutation == 3 ? 1 : 2;
        child2.descendant = &other;
        WinPointerOwner2 = &other;
        receiver.on_char('g', 0);
        assert(child1.descendant_queries == 1);
        assert(child2.descendant_queries == (mutation == 0));
        assert(WinPointerOwner2 == (mutation == 1 ? &other : 0));
        assert(!WinPointerOwner1);
    }

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
