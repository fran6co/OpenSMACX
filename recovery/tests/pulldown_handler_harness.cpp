#include <cassert>
#include <cstdint>
#include <vector>

struct RECT { long left, top, right, bottom; };
struct POINT { long x, y; };
static int PtInRect(const RECT *r, POINT p) {
    return p.x >= r->left && p.x < r->right &&
           p.y >= r->top && p.y < r->bottom;
}

enum Event { HideEvent, FlushEvent, TasksEvent, MenuEvent, RedrawEvent };
static std::vector<Event> events;
static int menu_x, menu_y, selection_during_menu;
static int selection_during_window_callback;
static class Menu *last_menu;
static class PullDown *callback_owner;
static class Menu *replacement_menu;
static bool replace_menu_during_tasks;

struct Font { int advance; int line_advance() const { return advance; } };
static Font *PullDownDefaultFont;
static int PullDownBorderPadding;

struct Win {
    RECT outer;
    virtual ~Win() {}
    const RECT &outer_rect() const { return outer; }
    virtual void hide() {}
    virtual void redraw() {}
    virtual void on_left_down(int, int) {}
    virtual void on_left_up(int, int) {}
};

struct GraphicWin : Win {
    RECT working;
    RECT *buffer_working_rect() { return &working; }
};

struct PullDownItem {
    char *text;
    char *right_text;
    int id;
    uint32_t flags;
    char *mnemonic;
};

class Menu : public GraphicWin {
public:
    int id = 0;
    void on_left_down(int x, int y) override;
};

class PullDown : public GraphicWin {
public:
    PullDownItem items_[64] = {};
    int item_count_ = 0;
    int selected_ = -1;
    Menu *menu_ = nullptr;
    void hide() override {
        events.push_back(HideEvent);
        selection_during_window_callback = selected_;
    }
    void redraw() override {
        events.push_back(RedrawEvent);
        selection_during_window_callback = selected_;
    }
    void on_left_down(int x, int y) override;
    void on_left_up(int x, int y) override;
};

inline void Menu::on_left_down(int x, int y) {
    events.push_back(MenuEvent);
    last_menu = this;
    menu_x = x;
    menu_y = y;
    selection_during_menu = callback_owner->selected_;
}

static void flush_input() { events.push_back(FlushEvent); }
static void do_all_tasks(Win *) {
    events.push_back(TasksEvent);
    if (replace_menu_during_tasks) {
        callback_owner->menu_ = replacement_menu;
    }
}

// ACTUAL_PULLDOWN_LEFT_DOWN
// ACTUAL_PULLDOWN_LEFT_UP

static char *pointer_with_bits(char (&storage)[64], unsigned mask,
                               unsigned wanted) {
    for (unsigned i = 0; i < sizeof(storage); ++i) {
        char *const candidate = &storage[i];
        if ((reinterpret_cast<uintptr_t>(candidate) & mask) == wanted) {
            return candidate;
        }
    }
    assert(false && "fixture could not select requested pointer object bits");
    return nullptr;
}

static void reset_events() {
    events.clear();
    menu_x = menu_y = selection_during_menu = -999;
    selection_during_window_callback = -999;
    last_menu = nullptr;
    replace_menu_during_tasks = false;
    replacement_menu = nullptr;
}

int main() {
    Font font = {10};
    PullDownDefaultFont = &font;
    PullDownBorderPadding = 0;
    PullDown down;
    down.working = RECT{10, 20, 30, 40};
    down.outer = RECT{100, 200, 400, 500};
    Menu menu;
    menu.outer = RECT{10, 20, 110, 120};
    down.menu_ = &menu;
    menu.id = 1;
    callback_owner = &down;

    // Inside does not close, flush, run tasks, or call the menu.
    reset_events();
    down.selected_ = 7;
    static_cast<Win &>(down).on_left_down(15, 25);
    assert(events.empty() && down.selected_ == 7);

    // Outside retains original order and uses the two outer-rect deltas.
    reset_events();
    down.selected_ = 7;
    static_cast<Win &>(down).on_left_down(3, 4);
    assert((events == std::vector<Event>{HideEvent, FlushEvent, TasksEvent,
                                         MenuEvent}));
    assert(down.selected_ == -1 && selection_during_menu == -1);
    assert(selection_during_window_callback == 7);
    assert(menu_x == 93 && menu_y == 184);

    // The original reads menu_ after the task pass; a task callback mutation
    // therefore selects the later menu rather than a cached earlier value.
    Menu late_menu;
    late_menu.outer = RECT{20, 40, 120, 140};
    reset_events();
    down.menu_ = &menu;
    down.selected_ = 8;
    replacement_menu = &late_menu;
    replace_menu_during_tasks = true;
    static_cast<Win &>(down).on_left_down(3, 4);
    assert(last_menu == &late_menu && menu_x == 83 && menu_y == 164);
    assert(selection_during_menu == -1);

    // No Menu still closes/flushes/runs tasks and clears selection.
    reset_events();
    down.menu_ = nullptr;
    down.selected_ = 3;
    static_cast<Win &>(down).on_left_down(3, 4);
    assert((events == std::vector<Event>{HideEvent, FlushEvent, TasksEvent}));
    assert(down.selected_ == -1);
    down.menu_ = &menu;

    // A release outside the working rect is inert.
    reset_events();
    down.selected_ = 9;
    down.item_count_ = 1;
    static_cast<Win &>(down).on_left_up(3, 4);
    assert(events.empty() && down.selected_ == 9);

    // The original +A24 address is flags (+C) in a row starting at +A18.
    // Give text pointers opposite low bits to reject the old misinterpretation.
    alignas(16) char storage[64] = {};
    down.item_count_ = 1;
    // A requested ordinal beyond the one-row count exhausts the walk and
    // follows the empty-row redraw path rather than selecting row zero.
    down.items_[0].text = pointer_with_bits(storage, 5, 0);
    down.items_[0].flags = 1;
    PullDownBorderPadding = 0;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);  // ordinal 2, count 1
    assert(down.selected_ == -1 &&
           (events == std::vector<Event>{RedrawEvent}));

    PullDownBorderPadding = 20;
    down.items_[0].text = pointer_with_bits(storage, 5, 0);
    down.items_[0].flags = 1;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);  // ordinal 0
    assert(down.selected_ == 0 && (events == std::vector<Event>{HideEvent}));
    assert(selection_during_window_callback == 0);

    down.items_[0].text = pointer_with_bits(storage, 5, 1);
    down.items_[0].flags = 5;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);
    assert(down.selected_ == -1 && (events == std::vector<Event>{RedrawEvent}));
    assert(selection_during_window_callback == -1);

    down.items_[0].text = pointer_with_bits(storage, 5, 1);
    down.items_[0].flags = 0;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);
    assert(down.selected_ == -1 && (events == std::vector<Event>{RedrawEvent}));

    // Preserve the zero/negative-count distinction in the product body.
    down.item_count_ = 0;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);
    assert(down.selected_ == -1 && (events == std::vector<Event>{RedrawEvent}));

    down.item_count_ = -1;
    PullDownBorderPadding = 0;
    down.items_[0].text = pointer_with_bits(storage, 5, 0);
    down.items_[0].flags = 0;
    reset_events();
    static_cast<Win &>(down).on_left_up(15, 25);
    assert(down.selected_ == 0 && (events == std::vector<Event>{HideEvent}));
}
