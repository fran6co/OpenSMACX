// Private C++98 behavior control for the source-shaped Popup::show body.
// This records decisions/order but does not prove the product ABI/layout.
#include <assert.h>
#include <stddef.h>
#include <vector>

#define __cdecl
static std::vector<int> calls;
enum Event { E_VISIBLE = 1, E_MOVE, E_SHOW, E_PLAY, E_FADE, E_SLIDE, E_SCROLL };

struct RECT { int left, top, right, bottom; };
class Buffer {
public:
    struct Header { int biWidth, biHeight; };
    struct Dib { Header bmiHeader; } dib_;
    // ACTUAL_BUFFER_GETTERS
    void set_size(int w, int h) {
        dib_.bmiHeader.biWidth = w;
        dib_.bmiHeader.biHeight = -h;
    }
};
class Win {
public:
    int visible;
    RECT bounds;
    Win() : visible(0) { bounds.left = 1; bounds.top = 2; bounds.right = 3; bounds.bottom = 4; }
    int is_visible() { calls.push_back(E_VISIBLE); return visible; }
    int move(int x, int y) { calls.push_back(E_MOVE); last_x = x; last_y = y; return 0; }
    virtual void show(int mode) { calls.push_back(E_SHOW); last_mode = mode; }
    int last_x, last_y, last_mode;
};
class GraphicWin : public Win, public Buffer {};
class BasePop : public GraphicWin {
public:
    unsigned int field_30AC_;
    Win *external_window_;
    BasePop() : field_30AC_(0), external_window_(0) { set_size(240, 180); }
};
struct StringList { int entry_count_; };
class Dialog { public: StringList item_list_; };
class ListBox : public virtual Dialog {
    unsigned int field_1C_;
public:
    ListBox() : field_1C_(0) {}
    int visible_item_capacity() const { return static_cast<int>(field_1C_); }
    void set_capacity(int value) { field_1C_ = static_cast<unsigned int>(value); }
};
class Dialogs : public ListBox {
public:
    int kind_;
    Dialogs() : kind_(0) { Dialog::item_list_.entry_count_ = 0; }
};
class Scroll : public GraphicWin {};
class Popup : public BasePop {
public:
    Dialogs dialogs_;
    Scroll scroll_;
    void show(int requested_mode);
};
class MainInterface : public GraphicWin {
public:
    int panel_height() const { return panel; }
    int panel;
    MainInterface() : panel(120) { set_size(1024, 768); }
};
class Wave {
public:
    void play() { calls.push_back(E_PLAY); }
    void fade(unsigned long) { calls.push_back(E_FADE); }
};

MainInterface MainInterfaceGlobal;
Wave g_MENU_UP_WAVE;
int WinScreenWidth = 1024;
int WinScreenHeight = 768;
static int slide_calls, scroll_calls;
void __cdecl slide_show(GraphicWin *, int) { ++slide_calls; calls.push_back(E_SLIDE); }
void __cdecl scroll_show(GraphicWin *, Scroll *) { ++scroll_calls; calls.push_back(E_SCROLL); }

// ACTUAL_SOURCE_BODY

static int first_event(int event) {
    for (std::size_t i = 0; i < calls.size(); ++i)
        if (calls[i] == event) return static_cast<int>(i);
    return -1;
}

int main() {
    Popup popup;

    calls.clear();
    popup.visible = 1;
    popup.show(7);
    assert(calls.size() == 1 && calls[0] == E_VISIBLE);

    calls.clear();
    popup.visible = 0;
    popup.field_30AC_ = 10000000;
    popup.show(9);
    assert(calls.size() == 2 && calls[1] == E_SHOW && popup.last_mode == 9);
    assert(slide_calls == 0 && scroll_calls == 0);

    calls.clear();
    popup.field_30AC_ = 0;
    popup.external_window_ = 0;
    MainInterfaceGlobal.visible = 1;
    popup.dialogs_.kind_ = 2;
    popup.dialogs_.Dialog::item_list_.entry_count_ = 8;
    popup.dialogs_.ListBox::set_capacity(4);
    popup.show(7);
    assert(popup.last_mode == 3);
    assert(slide_calls == 1 && scroll_calls == 1);
    assert(first_event(E_MOVE) < first_event(E_PLAY));
    assert(first_event(E_PLAY) < first_event(E_SLIDE));
    assert(first_event(E_SLIDE) < first_event(E_SCROLL));
    assert(first_event(E_SCROLL) < first_event(E_SHOW));
    assert(first_event(E_SHOW) < first_event(E_FADE));
    assert(popup.last_x == 408 && popup.last_y == 468);

    // Calls through the base dispatch to the recovered Popup slot1 override.
    calls.clear();
    Win *base_view = &popup;
    base_view->show(8);
    assert(popup.last_mode == 3 && slide_calls == 2 && scroll_calls == 2);

    calls.clear();
    popup.external_window_ = &MainInterfaceGlobal;
    popup.show(5);
    assert(scroll_calls == 3 && slide_calls == 2);
    assert(popup.last_mode == 5 && first_event(E_SCROLL) < first_event(E_SHOW));

    calls.clear();
    popup.external_window_ = 0;
    MainInterfaceGlobal.visible = 0;
    popup.dialogs_.Dialog::item_list_.entry_count_ = 8;
    popup.show(6);
    assert(scroll_calls == 3 && popup.last_mode == 6);

    // Main interface visible but non-animation mode also skips scrolling
    // when the second external-window read is still null.
    calls.clear();
    MainInterfaceGlobal.visible = 1;
    popup.field_30AC_ = 10000;
    popup.show(6);
    assert(scroll_calls == 3 && slide_calls == 2 && popup.last_mode == 6);

    // Equality is not overflow; test an external-window path so the capacity
    // predicate actually executes rather than being bypassed by the null gate.
    calls.clear();
    popup.external_window_ = &MainInterfaceGlobal;
    popup.dialogs_.Dialog::item_list_.entry_count_ = 4;
    popup.show(6);
    assert(scroll_calls == 3 && popup.last_mode == 6);

    calls.clear();
    popup.dialogs_.Dialog::item_list_.entry_count_ = 8;
    popup.dialogs_.kind_ = 1;
    popup.show(4);
    assert(scroll_calls == 3 && popup.last_mode == 4);
    calls.clear();
    popup.external_window_ = 0;
    popup.field_30AC_ = 0;
    popup.dialogs_.kind_ = 2;
    popup.dialogs_.Dialog::item_list_.entry_count_ = 4;
    popup.show(4);
    assert(slide_calls == 3 && scroll_calls == 3 && popup.last_mode == 3);
    return 0;
}
