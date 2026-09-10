// Pytest inserts actual draw bodies and the actual inline text accessor.
// Surrounding UI/list services are scripted doubles: this tests body dataflow,
// not production class layout, complete list semantics or game acceptance.
#include <assert.h>
#include <string.h>
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
void equal(RECT a, int l, int t, int r, int b) {
    assert(a.left == l && a.top == t && a.right == r && a.bottom == b);
}
struct Payload { char *text; uint32_t field_8_; };
struct Entry { Payload *payload; };
struct List {
    Entry *head_, *current_;
    int seeks[4], seek_count, advances;
    bool seek_pos(int position) {
        assert(seek_count < 4);
        seeks[seek_count++] = position;
        return position != -2;
    }
    void next_entry() { ++advances; }
    Payload *current_entry() { return current_->payload; }
    // LIVE_ACCESSOR
};
struct Font { int descent_; };
struct Buffer;
struct Sprite;
static Buffer *expected_control, *expected_parent;
static Sprite *expected_sprite;
static List *active_list;
static Entry *after_sprite;
static char *expected_text;
static bool checkbox, handled, focus, current_row, separator_text;
static int state, copies, sprites, wraps, writes, right_writes, dots, heights, callbacks, ids;
static int expected_top;
struct Buffer {
    Font *font1_;
    int text_line_height() { ++heights; return 7; }
    void copy(Buffer *destination, int sx, int sy, int dx, int dy, int w, int h) {
        assert(this == expected_parent && destination == expected_control);
        assert(sx == 111 && sy == 202 && dx == 11 && dy == 2 && w == 67 && h == 20);
        ++copies;
    }
    void wrap(char *text, RECT *area) {
        assert(this == expected_control && text == expected_text);
        equal(*area, 110, expected_top, 181, 45);
        ++wraps;
    }
    void write_l(char *text, RECT *area, unsigned int length) {
        assert(this == expected_control && text == expected_text);
        assert(length == strlen(text));
        if (separator_text) assert(strcmp(text, "left") == 0);
        equal(*area, 110, expected_top, 181, 45);
        ++writes;
    }
    void write_right_l(char *text, RECT *area, unsigned int length) {
        assert(this == expected_control && separator_text && strcmp(text, "right") == 0);
        assert(length == 5);
        equal(*area, 110, expected_top, 181, 45);
        ++right_writes;
    }
    void dotted_box(RECT *area, uint32_t colour) {
        assert(this == expected_control && state && focus && colour == 77);
        equal(*area, 9, 2, 80, 22);
        ++dots;
    }
};
struct Win {
    Win *win_parent_;
    virtual int vslot_11(int, int, int, int) { assert(false); return 0; }
    int is_dialog_focus() { return focus; }
};
struct GraphicWin : Win, Buffer {};
struct Dialog {
    List item_list_;
    int field_24_, field_28_, field_34_, field_40_, field_44_;
    int field_58_, field_64_, field_68_;
    uint32_t field_F0_, field_B8_;
    int pos_to_id(int position) { assert(position == 0); ++ids; return 123; }
};
static Dialog *active_dialog;
static GraphicWin *active_window;
static Entry *after_parent;
struct Parent : GraphicWin {
    virtual int vslot_11(int receiver, int id, int callback_state, int rectangle) {
        assert(receiver == reinterpret_cast<int>(active_window) && id == 123);
        assert(callback_state == (state ? 2 : current_row ? 1 : 0));
        RECT *area = reinterpret_cast<RECT *>(rectangle);
        equal(*area, 11, 2, 51, 22);
        area->left = 101; area->top = 31; area->right = 181; area->bottom = 45;
        active_dialog->field_64_ = 67;
        active_list->current_ = after_parent;
        ++callbacks;
        return handled;
    }
};
struct Sprite {
    int iHeight_, iTopOffset_, cTransparentIndex_;
    void draw(Buffer *destination, int colour, int x, int y, int numerator, int denominator) {
        assert(this == expected_sprite && destination == expected_control);
        assert(colour == cTransparentIndex_ && x == 11 && y == (state ? -1 : 6));
        assert(numerator == 1 && denominator == 1);
        active_list->current_ = after_sprite;
        ++sprites;
    }
};
struct CheckBox : virtual GraphicWin, virtual Dialog {
    Sprite *checked_sprite_, *unchecked_sprite_;
    void draw_item(int, int, int);
};
struct RadioButton : virtual GraphicWin, virtual Dialog {
    Sprite *checked_sprite_, *unchecked_sprite_;
    void draw_item(int, int);
};
// LIVE_CHECKBOX
// LIVE_RADIO

int main() {
    CheckBox check;
    RadioButton radio;
    Parent parent;
    Font font = {3};
    Sprite checked = {20, 81, 12}, unchecked = {6, 91, 13};
    char old_text[] = "old text", parent_text[] = "parent text", sprite_text[] = "sprite text";
    char parent_pair[] = "left|right", sprite_pair[] = "left|right";
    Payload old_payload = {old_text, 1}, parent_payload = {parent_text, 2}, sprite_payload = {sprite_text, 2};
    Entry old_entry = {&old_payload}, parent_entry = {&parent_payload}, sprite_entry = {&sprite_payload};
    for (int kind = 0; kind != 2; ++kind) {
        checkbox = kind == 0;
        active_window = checkbox ? static_cast<GraphicWin *>(&check) : static_cast<GraphicWin *>(&radio);
        active_dialog = checkbox ? static_cast<Dialog *>(&check) : static_cast<Dialog *>(&radio);
        active_list = &active_dialog->item_list_;
        expected_control = active_window;
        expected_parent = &parent;
        check.checked_sprite_ = radio.checked_sprite_ = &checked;
        check.unchecked_sprite_ = radio.unchecked_sprite_ = &unchecked;
        active_window->font1_ = &font;
        expected_top = checkbox ? 34 : 31;
        // Exercise all state/background/focus combinations, plus one-line
        // separator restoration and the ordinary wrapping path.
        for (int mode = 0; mode != 32; ++mode) {
            state = mode & 1; handled = (mode & 2) != 0; focus = (mode & 4) != 0;
            separator_text = (mode & 8) != 0;
            current_row = (mode & 16) != 0;
            for (int path = 0; path != 3; ++path) {
                copies = sprites = wraps = writes = right_writes = dots = heights = callbacks = ids = 0;
                active_window->win_parent_ = path == 1 ? 0 : &parent;
                active_dialog->field_24_ = 100; active_dialog->field_28_ = 200;
                active_dialog->field_34_ = 9; active_dialog->field_40_ = 11;
                active_dialog->field_44_ = 4; active_dialog->field_58_ = 3;
                active_dialog->field_64_ = 40; active_dialog->field_68_ = 20;
                active_dialog->field_F0_ = current_row ? 0 : 10; active_dialog->field_B8_ = 77;
                active_list->head_ = active_list->current_ = &old_entry;
                active_list->seek_count = active_list->advances = 0;
                parent_payload.text = separator_text ? parent_pair : parent_text;
                sprite_payload.text = separator_text ? sprite_pair : sprite_text;
                parent_payload.field_8_ = sprite_payload.field_8_ = separator_text ? 1 : 2;
                after_parent = &parent_entry; after_sprite = &sprite_entry;
                expected_text = handled ? parent_payload.text : sprite_payload.text;
                expected_sprite = state ? &checked : &unchecked;
                int position = path == 2 ? -2 : 0;
                if (checkbox) check.draw_item(position, state, 99); else radio.draw_item(position, state);
                assert(!active_list->advances);
                if (path != 0) {
                    assert(!copies && !sprites && !wraps && !writes && !right_writes && !dots && !heights && !callbacks && !ids);
                    assert(active_list->seek_count == (path == 1 ? 0 : 2));
                } else {
                    assert(callbacks == 1 && ids == 1 && heights == 1);
                    assert(copies == !handled && sprites == !handled);
                    assert(wraps == !separator_text && writes == separator_text && right_writes == separator_text);
                    assert(dots == (state && focus));
                    assert(strcmp(parent_pair, "left|right") == 0);
                    assert(strcmp(sprite_pair, "left|right") == 0);
                }
                if (path != 1) assert(active_list->seeks[0] == -1 && active_list->seeks[1] == position);
            }
        }
    }
    return 0;
}
