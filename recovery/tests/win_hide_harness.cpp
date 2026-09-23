// The test inserts the live Win::hide body. These doubles test branch behavior,
// not the real window layout, OS integration, or full-game acceptance.
#include <assert.h>
#include <string.h>
typedef int int32_t;
typedef unsigned char uint8_t;
struct RECT { int left, top, right, bottom; };
struct Win {
    unsigned int iSomeFlag_;
    Win *children_[4], *descendant, *val_4_;
    int child_count_, released, modal_released, activated, query;
    void hide();
    int is_descendant(Win *window) { return window == descendant; }
    void on_lose_mouse_capture() { ++released; }
    void release_modal() { ++modal_released; }
    int vslot_23() { return query; }
    void vslot_07() { ++activated; }
    void update_screen(RECT *, int value) { assert(value == 0); }
    void flip(RECT *) {}
};
Win receiver, other, root1, root2;
Win *WinBubbleCompanion, *WinPointerOwner1, *WinPointerOwner2;
Win *WinPointerOwner3, *WinPointerOwner4, *WinFocusWindow, *WinZOrderWindow;
Win *WinRootWindows[4];
int WinBubbleActive, JackalInitFlags, WinRootCount, WinZOrderCount, WinZOrderFlag;
RECT WinBubbleRect;
void *HandleMain = &receiver;
int invalidations, traversals, shrink_roots;
void InvalidateRect(void *window, int area, int erase) {
    assert(window == HandleMain && area == 0 && erase == 0);
    ++invalidations;
}
void recurse_zorder(Win *window) {
    assert(window == &root1 || window == &root2);
    ++traversals;
    if (window == &root1) {
        WinZOrderWindow = &root2;
        WinZOrderFlag = 42;
        if (shrink_roots) WinRootCount = 1;
    }
}

// LIVE_HIDE

void arm() {
    memset(&receiver, 0, sizeof(receiver));
    memset(&other, 0, sizeof(other));
    receiver.iSomeFlag_ = 1;
    WinBubbleCompanion = WinPointerOwner1 = WinPointerOwner2 = 0;
    WinPointerOwner3 = WinPointerOwner4 = WinFocusWindow = WinZOrderWindow = 0;
    JackalInitFlags = WinRootCount = invalidations = traversals = shrink_roots = 0;
    WinZOrderCount = 17;
    WinZOrderFlag = 41;
}

int main() {
    // Safe negative control: the numeric flag value has low bit zero, while
    // the readable object at that address has low bit one. The old extra
    // indirection fails an assertion here instead of dereferencing address 0.
    static int indirect_flags = 1;
    arm();
    JackalInitFlags = reinterpret_cast<int>(&indirect_flags);
    assert((JackalInitFlags & 1) == 0);
    receiver.hide();
    assert(invalidations == 0 && WinZOrderCount == 17);

    for (int flags = 0; flags != 4; ++flags) {
        arm(); JackalInitFlags = flags;
        receiver.hide();
        assert((receiver.iSomeFlag_ & 1) == 0);
        assert(invalidations == (flags & 1));
    }

    arm(); receiver.iSomeFlag_ = 0; JackalInitFlags = 1;
    receiver.hide();
    assert(invalidations == 0 && WinZOrderCount == 17);

    arm(); receiver.children_[0] = &other; receiver.child_count_ = 1;
    WinPointerOwner1 = &other;
    WinPointerOwner3 = WinPointerOwner4 = WinFocusWindow = &receiver;
    receiver.hide();
    assert(!WinPointerOwner1 && !WinPointerOwner2);
    assert(!WinPointerOwner3 && !WinPointerOwner4);
    assert(receiver.released == 1 && receiver.modal_released == 1);

    for (int shrink = 0; shrink != 2; ++shrink) {
        arm(); JackalInitFlags = 1; WinRootCount = 2; shrink_roots = shrink;
        root1.iSomeFlag_ = root2.iSomeFlag_ = 1;
        WinRootWindows[0] = &root1; WinRootWindows[1] = &root2;
        WinZOrderWindow = &root1;
        receiver.val_4_ = &other;
        receiver.hide();
        assert(traversals == (shrink ? 1 : 2));
        assert(WinZOrderFlag == (shrink ? 42 : 0));
        assert(invalidations == 1 && other.activated == 1);
    }
    return 0;
}
