// Test-only collaborators around the actual base_compute and soft_zoom bodies.
// No executable data or copied recovery implementations belong in this fixture.
#include <assert.h>

struct Base {
    int resource_sqr_radius;
    int queue_production_id[10];
};
Base Bases[4];
Base *BaseCurrent;
int BaseIDCurrentSelected = -1;
int BaseComputedID = -1;
int IsMultiplayerNet;
int ConsoleControlTurnActive;
int steps[16], step_count, expected_cached_id;
int change_tiles, change_selection, change_focus;
int visible_calls, zoom_calls, zoom_base, zoom_mode;
int lock_calls, unlock_calls, lock_result, locked_base, unlocked_base;

class Win {
public:
    int visible;
    int is_visible() { ++visible_calls; return visible; }
};
class BaseWin : public Win {
public:
    unsigned int focusedBaseID_;
    int displayed_production_id_;
    void soft_zoom(int);
    void zoom(int base, int mode) {
        ++zoom_calls; zoom_base = base; zoom_mode = mode;
    }
};
BaseWin window;
class NetDaemon {
public:
    int lock_base(int base, int mode, int x, int y) {
        assert(mode == 0 && x == -1 && y == -1);
        assert(step_count == 5);  // Synchronization follows all resource passes.
        ++lock_calls; locked_base = base;
        if (change_selection) BaseIDCurrentSelected = 3;
        return lock_result;
    }
    void unlock_base(int base) {
        ++unlock_calls; unlocked_base = base;
    }
} NetDaemonNet;

void set_base(int base) {
    // Collaborator only. The product publisher is independently compiled and
    // checked for a relocation to the real BaseCurrent storage.
    BaseIDCurrentSelected = base;
    BaseCurrent = &Bases[base];
}
void step(int stage) {
    assert(BaseComputedID == expected_cached_id);  // Published before callbacks.
    assert(step_count < 16);
    assert(stage == step_count % 5 + 1);
    steps[step_count++] = stage;
}
void base_support() { step(1); }
void base_yield() {
    step(2);
    if (change_tiles) BaseCurrent->resource_sqr_radius = 42;
    if (change_focus) window.focusedBaseID_ = 2;
}
void base_nutrient() { step(3); }
void base_minerals() { step(4); }
void base_energy() { step(5); }

// RECOVERED_COMPUTE
// RECOVERED_SOFT_ZOOM

void reset() {
    for (int i = 0; i < 4; ++i) {
        Bases[i].resource_sqr_radius = i + 7;
        Bases[i].queue_production_id[0] = -70 - i;
    }
    BaseCurrent = 0;
    BaseComputedID = BaseIDCurrentSelected = -1;
    IsMultiplayerNet = ConsoleControlTurnActive = 0;
    step_count = expected_cached_id = 0;
    change_tiles = change_selection = change_focus = 0;
    visible_calls = zoom_calls = lock_calls = unlock_calls = lock_result = 0;
    locked_base = unlocked_base = zoom_base = zoom_mode = -1;
    window.visible = 0;
    window.focusedBaseID_ = 0;
    window.displayed_production_id_ = 999;
}

int main() {
    reset();
    set_base(1); expected_cached_id = 1;
    base_compute(0);
    assert(step_count == 5 && BaseComputedID == 1);
    base_compute(0);
    assert(step_count == 5);  // Same selected id reuses the cache.
    base_compute(-1);
    assert(step_count == 10);  // Any nonzero argument forces all five passes.
    set_base(2); expected_cached_id = 2;
    base_compute(0);
    assert(step_count == 15 && BaseComputedID == 2);

    reset();
    window.visible = 1;
    window.soft_zoom(1);
    assert(visible_calls == 1 && zoom_calls == 1 && zoom_base == 1 && zoom_mode == 0);
    assert(step_count == 0 && lock_calls == 0 && BaseComputedID == -1);
    assert(window.focusedBaseID_ == 0 && window.displayed_production_id_ == 999);

    for (int network = 0; network <= 1; ++network) {
        for (int control = 0; control <= 1; ++control) {
            for (int tiles = 0; tiles <= 1; ++tiles) {
                for (int failed_lock = 0; failed_lock <= 1; ++failed_lock) {
                    reset();
                    IsMultiplayerNet = network; ConsoleControlTurnActive = control;
                    change_tiles = tiles; lock_result = failed_lock;
                    expected_cached_id = BaseComputedID = 1;
                    window.soft_zoom(1);
                    assert(visible_calls == 1 && zoom_calls == 0 && step_count == 5);
                    assert(BaseCurrent == &Bases[1] && BaseIDCurrentSelected == 1);
                    assert(BaseComputedID == 1 && window.focusedBaseID_ == 1);
                    assert(window.displayed_production_id_ == -71);
                    int should_lock = network && !control && tiles;
                    assert(lock_calls == should_lock);
                    assert(unlock_calls == (should_lock && !failed_lock));
                    if (should_lock) assert(locked_base == 1);
                    if (unlock_calls) assert(unlocked_base == 1);
                }
            }
        }
    }

    reset();
    IsMultiplayerNet = change_tiles = change_selection = 1;
    expected_cached_id = 1;
    window.soft_zoom(1);
    assert(locked_base == 1 && unlocked_base == 3);  // Reload after lock callback.
    assert(window.displayed_production_id_ == -71);  // Original argument, not selected id.

    reset();
    IsMultiplayerNet = change_focus = 1;
    expected_cached_id = 1;
    window.soft_zoom(1);
    assert(lock_calls == 1 && window.focusedBaseID_ == 2);
    assert(window.displayed_production_id_ == -71);  // Original argument, not changed focus.
    return 0;
}
