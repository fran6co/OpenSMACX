// Explicit external component/callee doubles only; no PickTech recovered body.
// Host checks do not establish native layout, EH ABI, physical vtables or game behavior.
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>
#define __cdecl
const int MaxTechnologyNum = 89;
struct RECT { int left, top, right, bottom; };
std::vector<std::string> events;
int next_font = 0, fail_font = 0, pick_result = 0;
bool pick_throws = false;
int expected_faction = 0, expected_mode = 0, expected_count = 0;
char *expected_section = 0;
struct Font {
    int ordinal;
    Font() : ordinal(++next_font) {
        events.push_back("font-ctor" + std::to_string(ordinal));
        if (ordinal == fail_font) throw std::runtime_error("font ctor");
    }
    ~Font() { events.push_back("font-dtor" + std::to_string(ordinal)); }
};
struct GraphicWin {
    GraphicWin() { events.push_back("base-ctor"); }
    virtual ~GraphicWin() { events.push_back("base-dtor"); }
    unsigned close();
    virtual void on_mouse_move(int, int) {}
    virtual int on_key_click(int, int) { return 0; }
    virtual void on_left_click_event(int, int) {}
    virtual void vslot_22(int, int) {}
};
/* PRODUCT_DECLARATION */
/* PRODUCT_LIFETIME_AND_CALLER */
unsigned GraphicWin::close() {
    // This external callee observes the actual close body's four stores.
    PickTech *picker = static_cast<PickTech *>(this);
    assert(picker->column_count_ == 0 && picker->row_count_ == 0);
    assert(picker->selected_index_ == 0 && picker->selection_active_ == 0);
    events.push_back("close");
    return 0;
}
// The modal implementation is outside this caller/lifetime control's scope.
int PickTech::pick(int faction, int mode, int count, char *section) {
    events.push_back("pick");
    assert(faction == expected_faction && mode == expected_mode);
    assert(count == expected_count && section == expected_section);
    column_count_ = 3; row_count_ = 4;
    selected_index_ = -1; selection_active_ = 8;
    if (pick_throws) throw std::runtime_error("pick");
    return pick_result;
}
// Unexercised external callback definitions satisfy the real declaration's
// vtable on the host. Their behavior is not evidence in this caller test.
void PickTech::on_mouse_move(int, int) {}
int PickTech::on_key_click(int, int) { return 0; }
void PickTech::on_left_click_event(int, int) {}
void PickTech::vslot_22(int, int) {}
void reset() {
    events.clear(); next_font = 0; fail_font = 0; pick_throws = false;
    expected_faction = 7; expected_mode = 11; expected_count = 19;
}
int main() {
    char section[] = "STEALTECH";
    const std::vector<std::string> complete = {
        "base-ctor", "font-ctor1", "font-ctor2", "font-ctor3", "font-ctor4",
        "pick", "close", "font-dtor4", "font-dtor3", "font-dtor2",
        "font-dtor1", "base-dtor"
    };
    for (int result : {-1, 42}) {
        for (char *ptr : {section, static_cast<char *>(0)}) {
            reset(); expected_section = ptr; pick_result = result;
            assert(tech_pick(7, 11, 19, ptr) == result);
            assert(events == complete);
        }
    }
    reset(); expected_section = section; pick_throws = true;
    bool caught = false;
    try { tech_pick(7, 11, 19, section); }
    catch (const std::runtime_error &) { caught = true; }
    assert(caught && events == complete);
    for (int ordinal = 1; ordinal <= 4; ++ordinal) {
        reset(); expected_section = section; fail_font = ordinal; caught = false;
        std::vector<std::string> partial = {"base-ctor"};
        for (int i = 1; i <= ordinal; ++i)
            partial.push_back("font-ctor" + std::to_string(i));
        for (int i = ordinal - 1; i >= 1; --i)
            partial.push_back("font-dtor" + std::to_string(i));
        partial.push_back("base-dtor");
        try { tech_pick(7, 11, 19, section); }
        catch (const std::runtime_error &) { caught = true; }
        assert(caught && events == partial);
    }
}

