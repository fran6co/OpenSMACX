// Test-only collaborators; the handler is extracted from the current product.
#include <assert.h>
#include <string.h>

enum {
    GOV_MANAGES_PRODUCTION = 1, GOV_MANAGES_CITIZENS_SPECS = 0x40,
    GOV_MULTI_PRIORITIES = 0x200000,
    GOV_PRIORITY_EXPLORE = 0x1000000, GOV_PRIORITY_DISCOVER = 0x2000000,
    GOV_PRIORITY_BUILD = 0x4000000, GOV_PRIORITY_CONQUER = 0x8000000,
    GOV_UNK_40000000 = 0x40000000, GOV_ACTIVE = 0x80000000,
    BSTATE_UNK_8000000000 = 0x80000000
};
struct Base {
    short x, y;
    unsigned char faction_id_current;
    unsigned int state, governor, queue_size;
    int resource_sqr_radius, specialist_total, specialist_unk_1;
} bases[2];
Base *BaseCurrent;
int BaseIDCurrentSelected;
unsigned int LockEnableMask;
int lock_result, switch_base, calls[12], count;
void step(int id) { assert(count < 12); calls[count++] = id; }
class NetDaemon {
public:
    int lock_base(int id, int mode, int x, int y) {
        assert(id == 0 && mode == 0 && x == -1 && y == -1);
        step(1); return lock_result;
    }
    void unlock_base(int id) { assert(id == BaseIDCurrentSelected); step(5); }
} NetDaemonNet;
class GraphicWin {
public:
    virtual void redraw() { step(7); }
};
class MainInterface : public GraphicWin {
public:
    virtual void redraw() { assert(false); } // The separate original call is direct.
} MainInterfaceGlobal;
class BaseWin : public GraphicWin {
public:
    void gov_on(int);
};
class Probe : public BaseWin {
public:
    virtual void redraw() { step(6); } // The original receiver call is virtual.
} window;
void base_reset(int id, int force) {
    assert(id == 0 && force == 1);
    assert(BaseCurrent->queue_size == 0 && BaseCurrent->state == 17);
    step(2);
    if (switch_base) { BaseCurrent = &bases[1]; BaseIDCurrentSelected = 1; }
}
void base_compute(int force) {
    assert(force == 1);
    assert(BaseCurrent->resource_sqr_radius == 0);
    assert(BaseCurrent->specialist_total == 0 && BaseCurrent->specialist_unk_1 == 0);
    step(3);
}
void base_doctors() { step(4); }
void draw_radius(int x, int y, int a, int b) {
    assert(x == BaseCurrent->x && y == BaseCurrent->y && a == 2 && b == 2);
    step(8);
}

// RECOVERED_GOVERNOR

void reset(unsigned int governor) {
    memset(bases, 0, sizeof(bases));
    for (int i = 0; i < 2; ++i) {
        bases[i].x = -7; bases[i].y = 12; bases[i].faction_id_current = 2;
        bases[i].governor = governor;
        bases[i].state = 0x80000011; bases[i].queue_size = 9;
        bases[i].resource_sqr_radius = 3;
        bases[i].specialist_total = 4; bases[i].specialist_unk_1 = 5;
    }
    BaseCurrent = &bases[0]; BaseIDCurrentSelected = 0;
    LockEnableMask = 4; count = lock_result = switch_base = 0;
}
void expect(const int *order, int length) {
    assert(count == length);
    for (int i = 0; i < length; ++i) assert(calls[i] == order[i]);
}
int main() {
    const int full[] = {1, 2, 3, 4, 5, 6, 7};
    const int disabled[] = {1, 5, 6, 7, 8};
    const int enabled[] = {1, 5, 6, 7};
    reset(GOV_UNK_40000000); lock_result = 1;
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(count == 1 && calls[0] == 1 && bases[0].governor == GOV_UNK_40000000);

    reset(GOV_UNK_40000000 | GOV_PRIORITY_EXPLORE);
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(bases[0].governor == (GOV_ACTIVE | GOV_PRIORITY_BUILD | 0x41));
    expect(full, 7);

    reset(GOV_MULTI_PRIORITIES | GOV_PRIORITY_EXPLORE);
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(bases[0].governor == (GOV_ACTIVE | GOV_MULTI_PRIORITIES |
                                GOV_PRIORITY_EXPLORE | GOV_PRIORITY_BUILD | 0x41));
    expect(full, 7);
    reset(GOV_ACTIVE | GOV_MULTI_PRIORITIES | GOV_PRIORITY_BUILD);
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(bases[0].governor == GOV_MULTI_PRIORITIES);
    expect(disabled, 5);

    reset(GOV_UNK_40000000);
    window.gov_on(0);
    assert(bases[0].governor == GOV_ACTIVE);
    expect(enabled, 4);
    reset(0); LockEnableMask = 0x400; // Alive byte cannot substitute for human byte.
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(bases[0].governor == (GOV_PRIORITY_BUILD | 0x41));
    assert(bases[0].queue_size == 9 && bases[0].resource_sqr_radius == 3);
    expect(disabled, 5);

    reset(0); switch_base = 1; bases[1].governor = GOV_MANAGES_CITIZENS_SPECS;
    window.gov_on(GOV_PRIORITY_BUILD);
    assert(BaseCurrent == &bases[1] && bases[0].resource_sqr_radius == 3);
    const int changed[] = {1, 2, 3, 4, 5, 6, 7, 8};
    expect(changed, 8); // Reload current-base state after callbacks, not a cached reference.
    return 0;
}
