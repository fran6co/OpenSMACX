// Actual recovered handlers with deterministic drawing, timer and OS doubles.
#include <assert.h>
#include <string.h>
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
int redraws, releases, beeps, starts, stops, shift, widths, fills, lines, colors;
int recolor[3][2];
char painted[256];
struct Win;
Win *passed;
struct Win {
    struct List { void *head_, *current_; int count_, tail_; } list_;
    Win *win_parent_;
    virtual void pass_dialog_focus() { passed = this; }
    virtual void on_lose_mouse_capture() { ++releases; }
};
Win *WinPointerOwner3, *WinPointerOwner4;
struct Buffer {
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
    RECT rect2_;
    int color_val_1_;
    int text_width(char *, int count) { assert(++widths < 64); return count * 8; }
    int text_width(char *text) { return text_width(text, strlen(text)); }
    int text_line_height() { return 8; }
    void set_clip(RECT *) {}
    void box(RECT *, int, int) {}
    void vline(int, int, int, int) { ++lines; }
    void write_l(char *text, RECT *, int size) {
        assert(size < 256); memcpy(painted, text, size); painted[size] = 0;
    }
    void change_color(int, int, int, int, int from, int to) {
        assert(colors < 3); recolor[colors][0] = from; recolor[colors++][1] = to;
    }
};
struct GraphicWin : Win, Buffer {
    virtual void redraw() { ++redraws; }
    void fill(int) { ++fills; }
};
struct Time {
    void start() { ++starts; }
    void stop() { ++stops; }
};
enum { VK_SHIFT = 16 };
int GetAsyncKeyState(int key) { assert(key == VK_SHIFT); return shift ? 0x8000 : 0; }
void MessageBeep(int value) { assert(value == 0); ++beeps; }
static void __fastcall stop_edit_timer(Time *timer) { timer->stop(); }
// LIVE_EDITBOX_DECL
EditBox::EditBox() {}
EditBox::~EditBox() {}
// LIVE_FOCUS
// LIVE_DOWN
// LIVE_UP
// LIVE_MOVE
// LIVE_PASS_FOCUS
// LIVE_POSITION
// LIVE_FITS
// LIVE_CHARACTER
// LIVE_KEY
// LIVE_PAINT

void reset(EditBox &edit, const char *text, int mark, int cursor) {
    strcpy(edit.text_, text);
    edit.field_B14_ = 255;
    edit.field_B18_ = 0;
    edit.field_B1C_ = 10;
    edit.field_B20_ = edit.field_B24_ = -1;
    edit.field_B28_ = edit.field_B2C_ = edit.field_B30_ = edit.field_B34_ = 0;
    edit.field_B38_ = edit.field_B3C_ = edit.field_B48_ = 0;
    edit.field_B40_ = mark;
    edit.field_B44_ = cursor;
    edit.dib_.bmiHeader.biWidth = 100;
    edit.dib_.bmiHeader.biHeight = -20;
    edit.rect2_.left = edit.rect2_.top = 0;
    edit.rect2_.right = 100; edit.rect2_.bottom = 20;
    edit.color_val_1_ = 20;
    edit.win_parent_ = 0;
    WinPointerOwner3 = WinPointerOwner4 = passed = 0;
    redraws = releases = beeps = starts = stops = shift = widths = fills = lines = colors = 0;
    painted[0] = 0;
}
void selection(EditBox &edit, const char *text, int position) {
    assert(strcmp(edit.text_, text) == 0);
    assert(edit.field_B40_ == static_cast<unsigned>(position));
    assert(edit.field_B44_ == static_cast<unsigned>(position));
}
int main() {
    EditBox edit;
    reset(edit, "abcd", 2, 2);
    assert(edit.vslot_25(1, 0) == 0 && redraws == 0);
    assert(edit.vslot_25('X', 0) == 1);
    selection(edit, "abXcd", 3);
    assert(redraws == 1 && widths == 1);

    reset(edit, "abcd", 1, 3);
    edit.dib_.bmiHeader.biWidth = 0;
    edit.vslot_25('X', 0);
    selection(edit, "aXd", 2);
    assert(widths == 0); // Nonempty selection must skip text_fits entirely.
    reset(edit, "abcd", 3, 1);
    edit.vslot_25('Y', 0);
    selection(edit, "aYd", 2);
    assert(widths == 0);
    reset(edit, "abcd", 2, 2);
    edit.dib_.bmiHeader.biWidth = 16;
    edit.vslot_25('X', 0);
    assert(edit.field_B18_ == 2 && widths == 3);
    reset(edit, "abcd", 4, 4);
    edit.field_B14_ = 4;
    edit.vslot_25('X', 0);
    selection(edit, "abcd", 4); assert(beeps == 1 && !redraws);

    reset(edit, "abcd", 0, 0);
    edit.vslot_26(8); selection(edit, "abcd", 0); assert(beeps == 1);
    reset(edit, "abcd", 2, 2);
    edit.vslot_26(8); selection(edit, "acd", 1);
    reset(edit, "abcd", 1, 3);
    edit.vslot_26(8); selection(edit, "ad", 1);
    reset(edit, "abcd", 3, 1);
    edit.vslot_26(8); selection(edit, "ad", 1);
    reset(edit, "abcd", 2, 2);
    edit.vslot_26(0x2E); selection(edit, "abd", 2);
    reset(edit, "abcd", 1, 3);
    edit.vslot_26(0x2E); selection(edit, "ad", 1);
    reset(edit, "abcd", 3, 1);
    edit.vslot_26(0x2E); selection(edit, "ad", 1);
    reset(edit, "abcd", 2, 2);
    edit.vslot_26(0x10008); selection(edit, "abcd", 2); assert(beeps == 1);
    assert(edit.vslot_26(9) == 0);
    assert(edit.vslot_26(0xFFFF) == 0);

    reset(edit, "abc def", 7, 7);
    edit.vslot_26(0x20025); selection(edit, "abc def", 3);
    reset(edit, "abc def", 0, 0);
    edit.vslot_26(0x20027); selection(edit, "abc def", 4);
    edit.vslot_26(0x10024);
    assert(edit.field_B40_ == 4 && edit.field_B44_ == 0);
    edit.vslot_26(0x23); selection(edit, "abc def", 7);
    edit.vslot_26(0x27); selection(edit, "abc def ", 8);
    reset(edit, "abc", 1, 1);
    edit.vslot_26(0x64); selection(edit, "abc", 0);
    edit.vslot_26(0x66); selection(edit, "abc", 1);
    reset(edit, "abcd", 4, 4);
    edit.field_B14_ = 4;
    edit.vslot_26(0x27);
    assert(edit.field_B44_ == 5 && edit.field_B40_ == 4);
    assert(beeps == 1 && widths == 0 && redraws == 0);
    reset(edit, "abcd", 0, 0);
    edit.vslot_26(0x10025);
    assert(redraws == 0 && edit.field_B44_ == 0);

    reset(edit, "abcd", 4, 4);
    edit.field_B18_ = 1; edit.field_B34_ = 2;
    assert(edit.text_position(1) == 1);
    assert(edit.text_position(2) == 1);
    assert(edit.text_position(10) == 2);
    assert(edit.text_position(100) == 3); // Final comparison is against B44.
    edit.field_B44_ = 2;
    assert(edit.text_position(100) == 4);
    edit.field_B18_ = 4;
    assert(edit.text_position(100) == 0); // Empty suffix returns zero.
    reset(edit, "abcd", 2, 2);
    edit.field_B34_ = 2; edit.dib_.bmiHeader.biWidth = 20;
    assert(!edit.text_fits());
    edit.dib_.bmiHeader.biWidth = 21;
    assert(edit.text_fits());
    edit.field_B3C_ = 2;
    assert(edit.text_fits());

    reset(edit, "abcd", 4, 4);
    edit.vslot_20(9, 0);
    selection(edit, "abcd", 1);
    assert(WinPointerOwner3 == &edit && redraws == 1);
    shift = 1;
    edit.vslot_20(17, 0);
    assert(edit.field_B40_ == 1 && edit.field_B44_ == 3);
    edit.on_mouse_feedback(1, 0);
    assert(edit.field_B44_ == 0);
    edit.vslot_21(9, 0);
    assert(!WinPointerOwner3 && releases == 1 && edit.field_B44_ == 2);
    reset(edit, "abcd", 4, 4);
    edit.on_mouse_feedback(9, 0); assert(edit.field_B44_ == 4 && redraws == 0);
    Win owner;
    WinPointerOwner3 = &owner; WinPointerOwner4 = &edit;
    edit.on_mouse_feedback(9, 0); assert(edit.field_B44_ == 1);
    edit.vslot_21(9, 0); assert(!WinPointerOwner4);
    edit.field_B48_ = 1;
    edit.on_dialog_focus(0); assert(stops == 1 && edit.field_B48_ == 0);
    edit.on_dialog_focus(1); assert(starts == 1);

    reset(edit, "abcd", 1, 3);
    edit.on_redraw();
    assert(strcmp(painted, "abcd") == 0 && fills == 1 && colors == 3);
    assert(recolor[0][0] == 10 && recolor[0][1] == 21);
    assert(recolor[1][0] == 20 && recolor[1][1] == 10);
    assert(recolor[2][0] == 21 && recolor[2][1] == 20);
    reset(edit, "abcd", 3, 1);
    edit.field_B3C_ = 2; edit.field_B48_ = 1;
    edit.on_redraw(); assert(strcmp(painted, "****") == 0 && lines == 1);

    reset(edit, "", 0, 0);
    edit.pass_dialog_focus(); assert(!passed);
    edit.win_parent_ = &owner;
    edit.field_B3C_ = 1;
    owner.list_.count_ = 2; owner.list_.tail_ = 1;
    owner.list_.head_ = &edit; owner.list_.current_ = 0;
    edit.pass_dialog_focus();
    assert(passed == &owner && owner.list_.tail_ == 0 && owner.list_.current_ == &edit);
    passed = 0;
    edit.field_B3C_ = 0;
    edit.pass_dialog_focus(); assert(passed == &edit);
    return 0;
}
