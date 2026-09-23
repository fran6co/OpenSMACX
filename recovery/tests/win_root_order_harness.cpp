// Actual source bodies and the root-lift declaration are injected by the test.
// Registry/callback doubles test ordering and mutation, not real Win layout,
// original registry extent, OS behavior, or full-game acceptance.
#include <assert.h>
#include <string.h>
typedef int int32_t;
struct RECT { int left, top, right, bottom; };
struct Menu { void redraw() {} };
struct List { int count_; void *head_, *current_; };
struct Win {
    unsigned int iFlags_, iSomeFlag_;
    int field_1A0_, val_3_, child_lifts;
    Win *win_parent_;
    RECT outer_rect_, client_rect_;
    Menu *menu_;
    List list_;
    // LIVE_ROOT_DECLARATION
    void show(int visible);
    void bring_child_to_top(Win *window) { assert(window->win_parent_ == this); ++child_lifts; }
    void resize_event(int, int) {}
    void draw_contents() {}
    void vslot_51(int) {}
    int vslot_23() { return 1; }
    void vslot_07() {}
    void client_to_screen(int *, int *) {}
};
Win receiver, root1, root2, root3;
Win *WinRootWindows[4], *WinZOrderWindow, *WinPointerOwner1, *WinPointerOwner2;
int WinRootCount, WinZOrderCount, WinZOrderFlag;
int traversals, mutation, invalidations;
Win *visited[8];
void *HandleMain = &receiver;
void InvalidateRect(void *window, RECT *, int erase) {
    assert(window == HandleMain && erase == 0);
    ++invalidations;
}
void offset_rect(RECT *rect, int x, int y) {
    rect->left += x; rect->right += x;
    rect->top += y; rect->bottom += y;
}
void recurse_zorder(Win *window) {
    assert(traversals < 8);
    visited[traversals++] = window;
    ++WinZOrderCount;
    if (traversals == 1 && mutation) {
        WinRootCount = mutation == 1 ? 1 : 3;
        WinZOrderWindow = &root2;
        WinZOrderFlag = 42;
    }
}

// LIVE_ROOT_LIFT

// LIVE_SHOW

void arm() {
    memset(&receiver, 0, sizeof(receiver));
    memset(&root1, 0, sizeof(root1));
    memset(&root2, 0, sizeof(root2));
    memset(&root3, 0, sizeof(root3));
    memset(visited, 0, sizeof(visited));
    receiver.iSomeFlag_ = 4;
    root1.iSomeFlag_ = root2.iSomeFlag_ = root3.iSomeFlag_ = 1;
    WinRootWindows[0] = &root1;
    WinRootWindows[1] = &root2;
    WinRootWindows[2] = &root3;
    WinRootCount = 3;
    WinZOrderCount = 17; WinZOrderFlag = 41;
    WinZOrderWindow = WinPointerOwner1 = WinPointerOwner2 = 0;
    traversals = mutation = invalidations = 0;
}

int main() {
    // This type must bind the real header declaration without a receiver.
    void (__cdecl *lift)(Win *) = &Win::bring_parent_to_top;
    arm(); lift(0);
    assert(traversals == 0 && WinZOrderCount == 17);
    receiver.iFlags_ = 0x2000000; lift(&receiver);
    assert(traversals == 0 && WinZOrderCount == 17);

    for (int selected = 0; selected != 3; ++selected) {
        arm(); Win *window = WinRootWindows[selected]; lift(window);
        assert(WinRootCount == 3 && traversals == 3);
        assert(WinRootWindows[0] == window && visited[0] == window);
        if (selected == 1) assert(WinRootWindows[1] == &root1 && WinRootWindows[2] == &root3);
        if (selected == 2) assert(WinRootWindows[1] == &root1 && WinRootWindows[2] == &root2);
    }
    arm(); lift(&receiver); // Not found: traverse without inserting it.
    assert(WinRootCount == 3 && traversals == 3 && WinRootWindows[0] == &root1);
    for (int count = -1; count <= 0; ++count) {
        arm(); WinRootCount = count; lift(&receiver);
        assert(WinRootCount == count && traversals == 0 && WinZOrderCount == 0);
    }
    arm(); root2.iSomeFlag_ = 0; lift(&receiver);
    assert(traversals == 2 && visited[0] == &root1 && visited[1] == &root3);
    arm(); mutation = 2; root2.iSomeFlag_ = 0; lift(&receiver);
    assert(traversals == 2 && WinZOrderFlag == 0 && WinZOrderCount == 1);

    for (int change = 1; change <= 2; ++change) {
        arm(); WinRootCount = 2; mutation = change; lift(&receiver);
        assert(traversals == (change == 1 ? 1 : 3));
        assert(WinZOrderFlag == (change == 1 ? 42 : 0));
        assert(WinZOrderCount == (change == 1 ? 1 : 2));

        arm(); WinRootCount = 2; mutation = change;
        receiver.show(7); // Suppress the separate lift; isolate show traversal.
        assert(traversals == (change == 1 ? 1 : 3));
        assert(WinZOrderFlag == (change == 1 ? 42 : 0));
        assert(WinZOrderCount == (change == 1 ? 1 : 2));
        assert(invalidations == 1 && (receiver.iSomeFlag_ & 1));
    }
    arm(); WinRootWindows[2] = &receiver; receiver.show(3);
    assert(WinRootWindows[0] == &receiver && traversals == 6);
    arm(); receiver.win_parent_ = &root3; receiver.show(3);
    assert(root3.child_lifts == 1 && traversals == 3);
    arm(); receiver.iSomeFlag_ |= 1; receiver.show(0);
    assert(traversals == 0 && invalidations == 0 && WinZOrderCount == 17);
    arm(); receiver.iSomeFlag_ = 0; receiver.show(0);
    assert(traversals == 0 && invalidations == 0 && WinZOrderCount == 17);
    return 0;
}
