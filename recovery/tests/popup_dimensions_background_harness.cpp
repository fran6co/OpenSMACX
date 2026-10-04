// Ignored host-only declarations and cases for an extracted C++98 member.
// Keep the declared inheritance/views aligned with src/, without modeling ABI
// offsets: the production member is inserted at the marker below by Python.
#include <stdio.h>
#include <stdlib.h>

#ifndef nullptr
#define nullptr 0
#endif

struct Sprite;
struct GraphicWin;
struct Font {};

struct Win {
    int visible;
    Win() : visible(0) {}
    int is_visible() { return visible; }
};

struct GraphicWin : Win {
    unsigned int iFlags_;
    GraphicWin() : iFlags_(0) {}
};
struct MapWin : public GraphicWin {};

struct Sprite {
    int pixel_width;
    Sprite(int width = 0) : pixel_width(width) {}
    int width() const { return pixel_width; }
};

typedef void (*DrawCallback)(Sprite *, GraphicWin *, int, int, int);
static void host_draw_callback(Sprite *, GraphicWin *, int, int, int) {}

struct Dialog {
    int field_2C_;
    int field_34_;
    int field_40_;
    int field_44_;
    int field_48_;
    int field_5C_;
    int field_B4_;
    Dialog() : field_2C_(0), field_34_(0), field_40_(0), field_44_(0),
               field_48_(0), field_5C_(0), field_B4_(0) {}
};

struct SpriteBox : public virtual GraphicWin, public virtual Dialog {};

struct ListBox : public virtual GraphicWin, public virtual Dialog {};
struct RadioButton : public virtual GraphicWin, public virtual Dialog {};
struct CheckBox : public virtual GraphicWin, public virtual Dialog {};
struct EditGroup : public virtual GraphicWin, public virtual Dialog {};

struct Dialogs : public ListBox, public RadioButton, public CheckBox,
                 public SpriteBox, public EditGroup {
    int kind_;
    int item_count;
    Dialogs() : kind_(0), item_count(0) {}
    int get_num_items() { return item_count; }
};

struct BasePop : public GraphicWin {
    Win *external_window_;
    Sprite *caller_sprite_;
    DrawCallback redraw_callback_;
    unsigned int field_30A4_;
    unsigned int field_30A8_;
    unsigned int field_30AC_;
    unsigned int field_30D0_;
    unsigned int field_30E0_;
    unsigned int field_3104_;
    unsigned int field_3108_;
    unsigned int field_30C0_;
    unsigned int field_30EC_;
    unsigned int field_21B0_;
    unsigned int field_21B4_;
    unsigned int field_21B8_;
    unsigned int field_21BC_;
    unsigned int field_21C0_;
    unsigned int field_21C4_;
    Win *field_322C_;
    int dimension_result;
    int dimension_calls;

    BasePop() : external_window_(0), caller_sprite_(0), redraw_callback_(0),
        field_30A4_(0), field_30A8_(0), field_30AC_(0), field_30D0_(0),
        field_30E0_(0), field_3104_(1), field_3108_(1), field_30C0_(0),
        field_30EC_(0), field_21B0_(0), field_21B4_(0), field_21B8_(0),
        field_21BC_(0), field_21C0_(0), field_21C4_(0), field_322C_(0),
        dimension_result(0), dimension_calls(0) {}
    virtual int calculate_dimensions() {
        ++dimension_calls;
        return dimension_result;
    }
};

struct Popup : public BasePop {
    Dialogs dialogs_;
    int string_color_a_;
    int string_color_b_;
    int string_color_2a_;
    int string_color_2b_;
    int string_color_3a_;
    int string_color_3b_;
    void *owned_text_;
    Popup() : string_color_a_(0), string_color_b_(0), string_color_2a_(0),
        string_color_2b_(0), string_color_3a_(0), string_color_3b_(0),
        owned_text_(0) {}
    virtual int calculate_dimensions();
};

struct MainInterface : public GraphicWin {};
struct DiploPop : public GraphicWin {};
struct Console : public MapWin {
    Font console_font;
};

MainInterface MainInterfaceGlobal;
DiploPop DiplomacyPopup;
Console ConsoleGlobal;
int WinScreenWidth = 1024;
int PopupModalActive = 0;
int PopupModalState = 0;
int TurnRedrawPending = 0;
int TextColourTable[195];
int IfaceIdToSlotStorage[30];
int ActiveIfaceIdStorage = 0;
DrawCallback sprite_draw_func = 0;

// The member under test is extracted from src/popup.cpp and inserted here.
/*__POPUP_MEMBER__*/

static int failures = 0;
static void check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void test_reset() {
    int i;
    MainInterfaceGlobal.visible = 0;
    DiplomacyPopup.visible = 0;
    ConsoleGlobal.visible = 0;
    WinScreenWidth = 1024;
    PopupModalActive = 0;
    PopupModalState = 0;
    TurnRedrawPending = 0;
    ActiveIfaceIdStorage = 0;
    for (i = 0; i < 30; ++i) IfaceIdToSlotStorage[i] = 0;
    for (i = 0; i < 195; ++i) TextColourTable[i] = 1000 + i;
    sprite_draw_func = 0;
}

static void test_virtual_base_dispatch() {
    Popup p;
    BasePop *base_view = &p;
    test_reset();
    p.dialogs_.Dialog::field_40_ = 77;
    sprite_draw_func = host_draw_callback;
    check(base_view->calculate_dimensions() == 0,
          "BasePop virtual view dispatches to Popup dimensions override");
    check(p.redraw_callback_ == host_draw_callback
          && p.dialogs_.Dialog::field_40_ == 0 && p.dimension_calls == 1,
          "virtual dispatch runs the Popup body before delegating once to BasePop");
}

static void test_sprite_size_and_clamp() {
    Popup p;
    Sprite sprite(90);
    test_reset();
    p.caller_sprite_ = &sprite;
    p.field_3104_ = 3;
    p.field_3108_ = 2;
    p.field_30E0_ = 7;
    p.dialogs_.Dialog::field_5C_ = 20;
    WinScreenWidth = 800;
    p.calculate_dimensions();
    check(p.dialogs_.Dialog::field_2C_ == 162,
          "800-wide screen computes sprite width over the existing height");

    p.dialogs_.Dialog::field_5C_ = 600;
    p.calculate_dimensions();
    check(p.dialogs_.Dialog::field_2C_ == 494,
          "800-wide screen caps dialog width at 0x1ee");

    test_reset();
    p = Popup();
    p.caller_sprite_ = &sprite;
    p.field_3104_ = 2;
    p.field_3108_ = 1;
    p.dialogs_.Dialog::field_2C_ = 10;
    p.dialogs_.Dialog::field_5C_ = 0;
    sprite.pixel_width = 100;
    p.field_3104_ = 10;
    p.field_3108_ = 1;
    WinScreenWidth = 1024;
    p.calculate_dimensions();
    check(p.dialogs_.Dialog::field_2C_ == 658,
          "zero preferred size falls back to the Dialog width and clamps at screen minus 0x16e");

    test_reset();
    p = Popup();
    p.caller_sprite_ = &sprite;
    p.dialogs_.Dialog::field_5C_ = 30;
    PopupModalActive = 1;
    p.calculate_dimensions();
    check(p.dialogs_.Dialog::field_2C_ == 700,
          "active popup modal state uses the fixed width");
}

static void test_diplomacy_special_cases() {
    Popup p;
    Sprite sprite(100);
    test_reset();
    WinScreenWidth = 800;
    p.external_window_ = static_cast<Win *>(&DiplomacyPopup);
    p.caller_sprite_ = &sprite;
    p.dialogs_.kind_ = 2;
    p.dialogs_.item_count = 5;
    p.calculate_dimensions();
    check(p.caller_sprite_ == &sprite,
          "diplomacy with exactly five items retains its caller sprite");
    check(p.dialogs_.Dialog::field_2C_ == 0,
          "five-item diplomacy special case skips sprite sizing");

    p.dialogs_.item_count = 6;
    p.caller_sprite_ = &sprite;
    p.dialogs_.Dialog::field_2C_ = 17;
    p.calculate_dimensions();
    check(p.caller_sprite_ == 0,
          "diplomacy with more than five items clears caller sprite");
    check(p.dialogs_.Dialog::field_2C_ == 17,
          "cleared diplomacy sprite skips its sizing write");

    p.caller_sprite_ = &sprite;
    p.dialogs_.kind_ = 8;
    p.dialogs_.item_count = 99;
    p.calculate_dimensions();
    check(p.caller_sprite_ == &sprite,
          "kind 8 is excluded from the item-count sprite clearing branch");
}

static void test_callback_modes_colors_and_layout() {
    Popup p;
    test_reset();
    p.field_30AC_ = 8;
    p.field_30A8_ = 0x10;
    p.dialogs_.kind_ = 1;
    p.owned_text_ = &p;
    sprite_draw_func = host_draw_callback;
    MainInterfaceGlobal.visible = 1;
    ConsoleGlobal.visible = 1;
    IfaceIdToSlotStorage[0] = 3;
    PopupModalState = 0;
    p.calculate_dimensions();
    check(p.redraw_callback_ == sprite_draw_func,
          "null callback receives the source sprite drawing callback");
    check(p.field_30AC_ == 60008,
          "selected modal interface maps to mode 6 and adds mode times 10000");
    check((p.field_30A8_ & 0x200000) != 0,
          "mode 6 sets the modal redraw flag");
    check(p.string_color_a_ == TextColourTable[70]
          && p.string_color_b_ == TextColourTable[71]
          && p.string_color_2a_ == TextColourTable[72]
          && p.string_color_2b_ == TextColourTable[73]
          && p.string_color_3a_ == TextColourTable[74]
          && p.string_color_3b_ == TextColourTable[75],
          "ordinary popup copies its six adjacent text colors");
    check(p.dialogs_.Dialog::field_40_ == 0
          && p.dialogs_.Dialog::field_44_ == 3
          && p.dialogs_.Dialog::field_48_ == 0x13
          && p.dialogs_.Dialog::field_34_ == 0x1B
          && p.dialogs_.Dialog::field_B4_ == 0,
          "Dialog state writes reach the Dialog view of embedded Dialogs");
    check(p.dialogs_.iFlags_ == 0
          && p.field_30A4_ == 0x1000080,
          "mode 6 clears flag 0x40 through GraphicWin and popup state");
    check(p.field_21B0_ == 0x13 && p.field_21B4_ == 0x15
          && p.field_21B8_ == 0x2f && p.field_21BC_ == 0x12
          && p.field_322C_ == 0,
          "non-main-interface mode selects the small visible-state layout");

    test_reset();
    p = Popup();
    ActiveIfaceIdStorage = 0;
    IfaceIdToSlotStorage[0] = 4;
    PopupModalState = 0;
    p.external_window_ = static_cast<Win *>(&DiplomacyPopup);
    p.calculate_dimensions();
    check(p.field_30AC_ == 0,
          "active mode 4 preserves modal mode when an external window exists");

    test_reset();
    p = Popup();
    IfaceIdToSlotStorage[0] = 10;
    p.external_window_ = 0;
    p.calculate_dimensions();
    check(p.field_30AC_ == 0,
          "active mode 10 does not replace the current modal mode");

    test_reset();
    p = Popup();
    PopupModalState = 1000;
    p.dialogs_.Dialog::field_40_ = 77;
    p.dimension_result = 11;
    check(p.calculate_dimensions() == 11 && p.dimension_calls == 1,
          "mode 1000 delegates immediately and propagates BasePop result");
    check(p.dialogs_.Dialog::field_40_ == 77
          && p.field_30AC_ == 10000000,
          "mode 1000 skips layout mutation after publishing its mode tag");

    test_reset();
    p = Popup();
    p.external_window_ = static_cast<Win *>(&DiplomacyPopup);
    p.string_color_a_ = 0;
    p.calculate_dimensions();
    check(p.string_color_a_ == TextColourTable[182]
          && p.string_color_3b_ == TextColourTable[187],
          "DiplomacyPopup uses its own source color table row");
}

static void test_visible_font_and_failure_paths() {
    Popup p;
    test_reset();
    MainInterfaceGlobal.visible = 1;
    p.dialogs_.kind_ = 0;
    p.calculate_dimensions();
    check(p.field_21B0_ == 0x21 && p.field_21B4_ == 0x43
          && p.field_21BC_ == 0x32
          && p.field_322C_ == static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal)),
          "visible main interface selects the wide layout and Console window font owner");
    check(p.dimension_calls == 1,
          "normal path delegates once to BasePop dimension calculation");

    test_reset();
    p = Popup();
    p.redraw_callback_ = sprite_draw_func = host_draw_callback;
    TurnRedrawPending = 1;
    p.calculate_dimensions();
    check((p.field_30A8_ & 0x200000) != 0,
          "pending turn redraw sets the same modal redraw flag without mode 6");
    check(p.redraw_callback_ == sprite_draw_func,
          "existing callback is preserved");

    test_reset();
    p = Popup();
    p.external_window_ = 0;
    p.field_30C0_ = 140;
    p.field_30EC_ = 9;
    MainInterfaceGlobal.visible = 1;
    p.dimension_result = 37;
    check(p.calculate_dimensions() == 37,
          "nonzero BasePop result propagates to caller");
    check(p.field_30C0_ == 140 && p.field_30EC_ == 9,
          "failed BasePop calculation skips minimum-height correction");

    test_reset();
    p = Popup();
    p.external_window_ = 0;
    p.field_30C0_ = 140;
    p.field_30EC_ = 9;
    MainInterfaceGlobal.visible = 1;
    p.calculate_dimensions();
    check(p.field_30C0_ == 300 && p.field_30EC_ == 169,
          "successful visible main popup raises short height and preserves bottom edge");

    test_reset();
    p = Popup();
    p.external_window_ = 0;
    p.field_30C0_ = static_cast<unsigned int>(-1);
    p.field_30EC_ = 9;
    MainInterfaceGlobal.visible = 1;
    p.calculate_dimensions();
    check(p.field_30C0_ == 300 && p.field_30EC_ == 310,
          "minimum-height check interprets the stored height as signed");
}

int main() {
    test_virtual_base_dispatch();
    test_sprite_size_and_clamp();
    test_diplomacy_special_cases();
    test_callback_modes_colors_and_layout();
    test_visible_font_and_failure_paths();
    if (failures != 0) {
        fprintf(stderr, "%d host control assertion(s) failed\n", failures);
        return 1;
    }
    puts("Popup::calculate_dimensions host controls passed");
    return 0;
}
