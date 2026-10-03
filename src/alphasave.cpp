/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "alphasave.h"
#include "console.h"
#include "alpha.h"
#include "font.h"
#include "game.h"
#include "netdaemon.h"
#include "strings.h"
#include "maininterface.h"
#include "palette.h"

extern int TextColourTable[195];
extern BoxSpriteParams InterfaceStandardPopupBoxStyle;

// Image references from AlphaSave::show/save/load store this, hide clears it,
// and filebox_draw tests it. The storage is distinct from ConsoleGlobal.
// RECOVERY_OBJECT: v2 0x006A7524 0x006A7528 ?ActiveAlphaSave@@3PAVAlphaSave@@A 0x00407BE0
AlphaSave *ActiveAlphaSave = 0;

// ORIGINAL: 0x004072D0 ?filebox_draw@AlphaSave@@QAAXPAUGraphicWin@@HHHHH@Z 0x004072D0-0x00407348
// symbol    ?alpha_save_filebox_draw@@YAXPAVGraphicWin@@HHHHH@Z
// The catalogue's member spelling would push a seventh, unused receiver.
// The selected row lives in CurrentListBoxDraw's Dialog virtual base.
// The retired seed corrected Ghidra's missing dereference in this vbtable
// walk; its five default-colour load orderings topped out at 90.7% mnemonic
// similarity because of VC6 instruction scheduling, not a proven layout
// difference. This typed source spelling remains an unclaimed mismatch.
void __cdecl alpha_save_filebox_draw(GraphicWin *window, int item_kind,
                                    int item_id, int x, int y, int height) {
    if (CurrentListBoxDraw == 0 || ActiveAlphaSave == 0) {
        return;
    }
    Buffer *background = CurrentListBoxDraw->Dialog::selected_position_ == item_id
        ? InterfaceBoxStyles[29].mask_ : InterfaceBoxStyles[0].mask_;
    Buffer *canvas = static_cast<Buffer *>(window);
    canvas->tile(background, x, y, x, y, canvas->pixel_width(), height);
    filebox_draw(window, item_kind, item_id, x, y, height);
}

void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_normal();
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);

static void alpha_save_store_rect(RECT &rect, int left, int top,
                                  int right, int bottom) {
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
}

// Modal geometry uses the 18-DWORD FileWinConstruct input; FileWin snapshots
// its 17-DWORD prefix. The dialog borrows the interface fonts and popup style
// while owning the controls initialized below.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00407410 ?init@AlphaSave@@QAEXXZ 0x00407410-0x00407906;0x00650C60-0x00650C75
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00407410.cpp", "recovery/tasks/game-save-load-owner-coverage.json", "pinned image sha256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_alphasave_init_behavior.py", "recovery/tests/test_standard_popup_style_behavior.py"], "reason": "Recover AlphaSave modal initialization through its embedded FileWin, exact admitted FileWinConstruct geometry, current typed fonts, and existing popup-style owners."}
void AlphaSave::init() {
    Buffer scratch;
    const int visible = ConsoleGlobal.is_visible();
    const int compact = visible ? 0 : 1;
    const int inset = visible ? 0x17 : 0;

    const int center_delta = WinScreenWidth - 0x13C;
    const int half_delta = center_delta / 2;
    const int x = half_delta - inset;
    const int y = WinScreenHeight - MainInterfaceGlobal.panel_height()
        - (compact + 0x1A) - 0x177;
    const int width = visible ? 0x18A : 0x14E;
    const int height = visible ? 0x191 : 0x189;
    Win *parent = visible ? static_cast<Win *>(&ConsoleGlobal) : 0;

    GraphicWin::init(x, y, width, height, 0, 0x1001000, parent, 0, 0);
    GraphicWin::fill(9);

    RECT frame = *buffer_working_rect();
    if (visible) {
        frame.left += 0x17;
        frame.right -= 0x37;
        frame.bottom += 0x0A;
        box_sprite(&frame, &InterfaceBoxStyles[9]);
    } else {
        box_sprite(&frame, &InterfaceStandardPopupBoxStyle);
        frame.left += 9;
        frame.top += 9;
        frame.right -= 9;
        frame.bottom -= 9;
    }
    frame.right -= 3;
    frame.left += 3;
    frame.bottom -= 3;
    frame.top += 3;
    box_sprite(&frame, &InterfaceBoxStyles[0]);

    FileWinConstruct layout;
    layout.field_00_ = visible ? 0 : 0x80020;
    if (visible) {
        alpha_save_store_rect(layout.list_box_bounds_, inset + 5, compact + 0x20,
                              inset + 0x136, compact + 0x140);
        alpha_save_store_rect(layout.edit_box_bounds_, inset + 8, compact + 0x142,
                              inset + 0x134, compact + 0x156);
        alpha_save_store_rect(layout.button2_bounds_, inset + 10, compact + 0x15C,
                              inset + 0x99, compact + 0x172);
        alpha_save_store_rect(layout.button1_bounds_, inset + 0xA2,
                              compact + 0x15C, inset + 0x132, compact + 0x172);
    } else {
        alpha_save_store_rect(layout.list_box_bounds_, 0x0E, compact + 0x29,
                              0x13F, compact + 0x149);
        alpha_save_store_rect(layout.edit_box_bounds_, 0x11, compact + 0x14B,
                              0x13D, compact + 0x15F);
        alpha_save_store_rect(layout.button2_bounds_, 0x13, compact + 0x165,
                              0xA2, compact + 0x17B);
        alpha_save_store_rect(layout.button1_bounds_, 0xAB, compact + 0x165,
                              0x13B, compact + 0x17B);
    }
    layout.field_44_ = inset + 8;
    file_win_.init(0, this, &layout);

    Dialog &list_dialog = file_win_.list_box_;
    list_dialog.set_dialog_font(MainInterfaceGlobal.secondary_font(), 0, 0);
    list_dialog.set_dialog_text_color(TextColourTable[0], TextColourTable[1], 1, 1);
    file_win_.list_box_.set_report_text_color(TextColourTable[2]);
    file_win_.list_box_.reset_report_selection();
    file_win_.list_box_.set_draw_callback(alpha_save_filebox_draw);
    list_dialog.field_48_ = 0x16;
    list_dialog.field_4C_ = 1;
    list_dialog.field_B4_ = 0;
    list_dialog.field_20_ &= ~4u;
    list_dialog.on_redraw();

    EditBox &edit = file_win_.edit_box_;
    edit.set_font(MainInterfaceGlobal.secondary_font(), 0, 0, 0);
    edit.field_B1C_ = TextColourTable[10];
    edit.set_text_color(TextColourTable[4], TextColourTable[5], 1, 1);
    edit.field_B20_ = TextColourTable[6];
    edit.field_B24_ = TextColourTable[8];
    edit.field_B28_ = TextColourTable[8];
    edit.field_B2C_ = TextColourTable[8];
    edit.on_dialog_focus(1);
    edit.redraw();

    file_win_.flat_button2_.set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    file_win_.flat_button2_.redraw();
    file_win_.flat_button2_.redraw();
    file_win_.flat_button1_.set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    file_win_.flat_button1_.redraw();
    file_win_.flat_button1_.redraw();
}

// ORIGINAL: 0x00407350 ??0AlphaSave@@QAE@XZ 0x00407350-0x00407410;0x00650BF0-0x00650C60
AlphaSave::AlphaSave() {
}

// ORIGINAL: 0x00408170 ??1AlphaSave@@QAE@XZ 0x00408170-0x00408383;0x00650D10-0x00650E20
// symbol    ??1AlphaSave@@UAE@XZ
// TRIED: plain reinterpret_cast pointer-offset calls into close()/dtors in disasm order; no local subobjects so no SEH frame is generated (diverges at #1, the missing try/finally prologue)
AlphaSave::~AlphaSave() {
    // The owner closes its own window before member destruction begins.
    GraphicWin::close();
}

// ORIGINAL: 0x00407910 ?close@AlphaSave@@QAEXXZ 0x00407910-0x00407915 BYTE_EXACT
// LEVER: thiscall shim - a synthetic GraphicWin class exposes close() as a member so reinterpret_cast<GraphicWin*>(this)->close() folds into the original's bare 5-byte jmp (same shim shape as 0x004210F0)
void AlphaSave::close() {
    GraphicWin::close();
}

// ORIGINAL: 0x00407CF0 ?on_double_clicked@AlphaSave@@QAEXH@Z 0x00407CF0-0x00407D06 BYTE_EXACT
void AlphaSave::on_double_clicked(int button_id) {
    file_win_.on_double_clicked(button_id);
}

// ORIGINAL: 0x00407CD0 ?on_button_clicked@AlphaSave@@QAEXH@Z 0x00407CD0-0x00407CE6 BYTE_EXACT
// symbol    ?forward_button_click@AlphaSave@@QAEXH@Z
// The image symbol is nonvirtual; using that spelling here would override
// Win::on_button_clicked and change AlphaSave's vtable.
void AlphaSave::forward_button_click(int button_id) {
    file_win_.on_button_clicked(button_id);
}

// ORIGINAL: 0x00407ED0 ?on_listbox_scroll_delete@AlphaSave@@QAEHPAUScroll@@@Z 0x00407ED0-0x00407EF3 BYTE_EXACT
// symbol    ?on_listbox_scroll_delete@AlphaSave@@QAEHPAVScroll@@@Z
int AlphaSave::on_listbox_scroll_delete(Scroll *scroll) {
    if (Win::is_visible()) {
        scroll_hide(this, scroll);
    }
    return 0;
}

// ORIGINAL: 0x00407D10 ?on_key_click@AlphaSave@@QAEXHH@Z 0x00407D10-0x00407D62 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00407d10.cpp and original symbol", "image:constructor4073F2 installs669EC8; slot25 selects407D10", "image:407D37/407D57 last calls and407D3E/407D5F returns preserve EAX from Win key_down_event/key_click_event"], "reason": "Recover the real AlphaSave key override through typed FileWin subobjects and forward the actual last-call result. The catalogue void spelling is retained as original identity, not the virtual return contract; no arbitrary constant or extra callback slot."}
// symbol    ?on_key_click@AlphaSave@@UAEHHH@Z
int AlphaSave::on_key_click(int key, int flags) {
    switch (flags) {
    case 0x21:
    case 0x22:
    case 0x26:
    case 0x28:
        return file_win_.list_box_.key_down_event(flags);
    default:
        file_win_.edit_box_.key_down_event(flags);
        return file_win_.edit_box_.key_click_event(key, flags);
    }
}

// ORIGINAL: 0x004080E0 ?on_scrolling@AlphaSave@@QAEXHH@Z 0x004080E0-0x0040810C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004080e0.cpp and original symbol"], "reason": "Existing AlphaSave owner forwards to its typed FileWin subobjects; original symbol and seed establish identity and extent"}
void AlphaSave::on_scrolling(int code, int pos) {
    // The embedded ListBox dispatches this notification through its Win base.
    file_win_.list_box_.vslot_48(code, pos);
}

// ORIGINAL: 0x00408110 ?on_scrolled@AlphaSave@@QAEXHH@Z 0x00408110-0x0040813C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00408110.cpp and original symbol"], "reason": "Existing AlphaSave owner forwards to its typed FileWin subobjects; original symbol and seed establish identity and extent"}
void AlphaSave::on_scrolled(int code, int pos) {
    // This is the next Win virtual notification, not ListBox's direct handler.
    file_win_.list_box_.vslot_49(code, pos);
}

// ORIGINAL: 0x00407E80 ?on_listbox_scroll_create@AlphaSave@@QAEHXZ 0x00407E80-0x00407ECA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00407e80.cpp;src/popup.cpp:Popup::on_listbox_scroll_create;src/alphasave.h:scroll_"], "reason": "AlphaSave owns the Scroll at 0x3DD8; original call sequence matches the typed Popup scroll-create pattern and seed evidence"}
int AlphaSave::on_listbox_scroll_create() {
    scroll_normal();
    scroll_.init(0x147, 0x15, 0x18, 0xd8, this, -1, 1);
    scroll_.hide();
    if (Win::is_visible()) {
        scroll_show(this, &scroll_);
    }
    return reinterpret_cast<int>(&scroll_);
}

// ORIGINAL: 0x00407BE0 ?show@AlphaSave@@QAEXH@Z 0x00407BE0-0x00407C57 BYTE_EXACT
// symbol    ?show_save_window@AlphaSave@@QAEXH@Z
// The exact standalone unit was retired after this source body was recorded.
// This routine does not read its argument: the choice is whether the main
// Console is already visible, not the caller's requested display mode.
void AlphaSave::show_save_window(int) {
    if (Win::is_visible()) {
        return;
    }
    if (ConsoleGlobal.is_visible()) {
        slide_show(this, 0);
        if (file_win_.list_box_.has_scroll_flags()) {
            scroll_show(this, &scroll_);
        }
        Win::show(3);
    } else {
        Win::show(2);
    }
    ActiveAlphaSave = this;
}

// ORIGINAL: 0x00407C60 ?hide@AlphaSave@@QAEXXZ 0x00407C60-0x00407CC4 BYTE_EXACT
// symbol    ?hide_save_window@AlphaSave@@QAEXXZ
// The exact standalone unit was retired after this typed path was recorded.
void AlphaSave::hide_save_window() {
    if (!Win::is_visible()) {
        return;
    }
    if (ConsoleGlobal.is_visible()) {
        if (file_win_.list_box_.has_scroll_flags()) {
            scroll_hide(this, &scroll_);
        }
        slide_hide(this, 0);
    }
    ActiveAlphaSave = 0;
    Win::hide();
}

// ORIGINAL: 0x00407D90 ?on_selected@AlphaSave@@QAEXH@Z 0x00407D90-0x00407E7B
// The callback argument is unused in the image. The selected position is
// held by the embedded ListBox's Dialog virtual base, whose StringList
// supplies the filename. The retired seed reached 96.4% mnemonic similarity
// with raw offsets but could not make VC6 assign the original ebx=this,
// esi=list-base, edi=count register trio. The typed body is not yet exact;
// restore its historical extent through the source identity above.
void AlphaSave::on_selected(int) {
    Dialog &dialog = file_win_.list_box_;
    dialog.item_list_.seek_pos(dialog.selected_position_);
    file_win_.edit_box_.set_text(dialog.item_list_.current_text());
    file_win_.edit_box_.select_all_text();
    file_win_.edit_box_.redraw();
    file_win_.edit_box_.redraw();
}

// ORIGINAL: 0x00407920 ?save@AlphaSave@@QAEXXZ 0x00407920-0x00407A7E
// The caption is label slot 674; the paired load dialog uses 673. The
// original temporarily disables the two Net voice hotkeys while modal.
// The retired standalone transcription reached the correct owner calls but
// diverged early in VC6 register scheduling; this typed body is not exact.
void AlphaSave::save() {
    RECT caption = *buffer_working_rect();
    if (ConsoleGlobal.is_visible()) {
        caption.left += 0x17;
        caption.right -= 0x37;
    } else {
        caption.left += 9;
        caption.top += 9;
        caption.right -= 9;
    }
    caption.left += 6;
    caption.top += 6;
    caption.right -= 6;
    caption.bottom = caption.top + 0x14;

    Buffer *canvas = this;
    canvas->box_sprite(&caption, &InterfaceBoxStyles[6]);
    canvas->set_text_color(0xA0, -1, 1, 1);
    canvas->set_font(find_font(0xE, 1), 0, 0, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(674));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &caption, strlen(StringTemp));

    ActiveAlphaSave = this;
    NetDaemonNet.voice_hotkey_48_ = 0;
    NetDaemonNet.voice_hotkey_4c_ = 0;
    file_win_.save();
    NetDaemonNet.voice_hotkey_48_ = 0xDC;
    NetDaemonNet.voice_hotkey_4c_ = 0x100DC;
}

// ORIGINAL: 0x00407A80 ?load@AlphaSave@@QAEXXZ 0x00407A80-0x00407BDE
// symbol    ?load@AlphaSave@@QAEPADXZ
// The retired standalone spelling could not reproduce VC6's two-register
// frame, but its call order and owner identities are retained here.
char *AlphaSave::load() {
    RECT caption = *buffer_working_rect();
    if (ConsoleGlobal.is_visible()) {
        caption.left += 0x17;
        caption.right -= 0x37;
    } else {
        caption.left += 9;
        caption.top += 9;
        caption.right -= 9;
    }
    caption.left += 6;
    caption.top += 6;
    caption.right -= 6;
    caption.bottom = caption.top + 0x14;

    Buffer *canvas = this;
    canvas->box_sprite(&caption, &InterfaceBoxStyles[6]);
    canvas->set_text_color(0xA0, -1, 1, 1);
    canvas->set_font(find_font(0xE, 1), 0, 0, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(673));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &caption, strlen(StringTemp));

    ActiveAlphaSave = this;
    NetDaemonNet.voice_hotkey_48_ = 0;
    NetDaemonNet.voice_hotkey_4c_ = 0;
    char *const selected = file_win_.load();
    NetDaemonNet.voice_hotkey_48_ = 0xDC;
    NetDaemonNet.voice_hotkey_4c_ = 0x100DC;
    // Original407BBF..407BDD forwards EAX unchanged through these stores;
    // load_map tests that pointer, including null with nonempty defaults.
    return selected;
}

int AlphaSave::add_default_extension(char *extension) {
    return file_win_.add_def_ext(extension);
}

int AlphaSave::set_default_filename(char *name) {
    return file_win_.set_def_name(name);
}
