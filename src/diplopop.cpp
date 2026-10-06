/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "diplopop.h"
#include "basepop.h"
#include "mapwin.h"  // MainInterfaceGlobal - the modal owner at 0x007AE820
#include "datalink.h"
#include "worldwin.h"
#include "base.h"
#include "alpha.h"
#include "strings.h"
#include "sprite.h"
#include "game.h"
#include "console.h"
#include "spying_recovery.h"

void __cdecl report_intel(int selection);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0043F2B0 ?start@DiploPop@@QAEXH@Z 0x0043F2B0-0x0043F5B5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043f2b0.cpp", "image:terranx_original.exe:0x0043F2B0", "source:docs/recovery/subobject-sites.csv:MainInterface22DFC"], "reason": "Recover the genuine diplomacy popup startup using embedded Flic/Time members, borrowed faction Sprite objects and MainInterface sprite11. Native same-faction fast path, FLIC failure/fallback and stored-frame update remain distinct."}
void DiploPop::start(int faction_id) {
    Buffer *canvas = static_cast<Buffer *>(this);
    BasePop::set_def_external_window(this, 0, 0, canvas->pixel_width(), canvas->pixel_height());
    const bool alternate_size = WinScreenWidth != 800;
    diplomacy_check(ConsoleGlobal.faction_id_, faction_id, 0);
    if (counterpart_faction_ != faction_id) {
        counterpart_faction_ = faction_id;
        frame_sprite_ = MainInterfaceGlobal.diplomacy_frame_sprite();
        landscape_sprite_ = &FactionsArt[faction_id].diplomacy_landscape;
        portrait_sprite_ = &FactionsArt[faction_id].diplomacy;
        hq_sprite_ = &FactionsArt[faction_id].diplomacy_logo;
        set_iface_mode();

        StringTemp[0] = '\0';
        strcat(StringTemp, "dip_");
        if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN)) {
            strcat(StringTemp, "a_");
        } else {
            strcat(StringTemp, Players[faction_id].is_leader_female ? "f_" : "m_");
        }
        int mood = static_cast<int32_t>(static_cast<uint32_t>(DiploFriction) * 3) / 20;
        if (mood < 0) mood = 0;
        else if (mood > 2) mood = 2;
        char number[80];
        _itoa(mood, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ".flc");
        Buffer *destination = static_cast<Buffer *>(&MainInterfaceGlobal);
        if (flic2_.init(destination, StringTemp,
                        alternate_size ? 26 : 22, alternate_size ? 90 : 80, 1)) {
            return;
        }
        flic2_.set_animation_flags(9);
        const int x = static_cast<int32_t>(portrait_text_rect_.left + 1);
        const int difference = static_cast<int32_t>(portrait_text_rect_.bottom - portrait_text_rect_.top);
        const int y = static_cast<int32_t>(portrait_text_rect_.top + difference / 4 + 1);
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction_id].filename);
        strcat(StringTemp, ".flc");
        if (flic1_.init(destination, StringTemp, x, y, 0)) {
            if (flic1_.init(destination, const_cast<char *>("faction.flc"), x, y, 0)) {
                return;
            }
        }
        time2_.init(reinterpret_cast<void(__cdecl *)(int, int)>(timer_callback_daemon2),
                    0, reinterpret_cast<int>(this), 2000, 5);
        uint32_t interval = flic2_.header_interval();
        if (interval > 5) {
            if (interval == 0) interval = 100;
            time1_.init(reinterpret_cast<void(__cdecl *)(int, int)>(timer_callback_daemon),
                        0, reinterpret_cast<int>(this), interval, 5);
        }
    } else {
        set_iface_mode();
    }
    field_20B0_ = 0;
    time1_.start();
    time2_.start();
    flic1_.decode_stored_frame();
    MainInterfaceGlobal.soft_update(flic1_.update_rect());
}

// ORIGINAL: 0x00440700 ?init@DiploPop@@QAEXXZ 0x00440700-0x00440D4B;0x006548F4-0x0065491F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00440700.cpp", "image:terranx_original.exe:0x00440700", "source:src/diplopop.h:DiploPop"], "reason": "Recover the complete DiploPop initializer with temporary Flic, MainInterface geometry, owned fonts/buttons, and native state."}
// TRIED: the SEH __try/__finally frame and its per-object unwind-state byte; the layout arithmetic (0x004407B3-0x00440A21) is transcribed as algebraically-simplified named quantities rather than a literal register-by-register replay, so it matches the VALUES the original computes but not the register shuffling that produces them.
void DiploPop::init() {
    Flic layout_flic;
    time1_.stop();
    time2_.stop();
    hide();
    GraphicWin::close();
    counterpart_faction_ = -1;
    field_20B0_ = 0;
    field_2154_ = 1;

    // The native temporary is a real Flic: init reads decoded_width(), then
    // its ordinary destructor closes the file and destroys Buffer/Palette.
    layout_flic.init(static_cast<Buffer *>(this),
                     const_cast<char *>("faction.flc"), 0, 0, 0);
    int frame_width = layout_flic.decoded_width();
    if (frame_width < 0x28) {
        frame_width = 0x28;
    }
    layout_flic.close();

    const RECT &right_frame = MainInterfaceGlobal.right_frame_rect();
    const RECT &side_rect = MainInterfaceGlobal.rect3();
    const RECT &list_rect = MainInterfaceGlobal.rect2();
    right_panel_button_rect_.left =
        right_frame.right - side_rect.right + right_frame.left;
    right_panel_button_rect_.top = list_rect.top;
    right_panel_button_rect_.right = list_rect.left - 3;
    right_panel_button_rect_.bottom = list_rect.bottom;

    frame_inset_rect_ = right_frame;
    frame_inset_rect_.left += 4;
    frame_inset_rect_.top += 4;
    frame_inset_rect_.right -= 4;
    frame_inset_rect_.bottom = right_panel_button_rect_.top - 6;

    content_rect_ = MainInterfaceGlobal.lower_frame_rect();
    content_rect_.left += 4;
    content_rect_.top += 4;
    content_rect_.right -= 4;
    content_rect_.bottom -= 4;

    rect1_ = MainInterfaceGlobal.main_layout_rect();
    rect1_.left += 4;
    rect1_.top += 4;
    rect1_.right -= 4;
    rect1_.bottom -= 4;
    rect1_.left += 3;
    rect1_.top += 3;
    rect1_.right -= 3;
    rect1_.bottom -= 3;

    const int button_width = rect1_.right - rect1_.left;
    const int split = rect1_.left + (button_width + 3) / 2 - 3;
    const int row_top = rect1_.bottom - 20;
    button_left_rect_.left = rect1_.left;
    button_left_rect_.top = row_top;
    button_left_rect_.right = split;
    button_left_rect_.bottom = rect1_.bottom;
    const int second_delta = split - rect1_.left + 3;
    button_right_rect_.left = rect1_.left + second_delta;
    button_right_rect_.top = row_top;
    button_right_rect_.right = split + second_delta;
    button_right_rect_.bottom = rect1_.bottom;
    // Native reuses the bottom member of rect1_ as scratch at +0x2100
    // after deriving both button rectangles; there is no independent scalar.
    rect1_.bottom = row_top - 3;

    const RECT &main_frame = MainInterfaceGlobal.main_frame_rect();
    portrait_text_rect_.left = main_frame.right - 4 - frame_width - 2;
    portrait_text_rect_.top = main_frame.top + 4;
    portrait_text_rect_.right = main_frame.right - 4;
    portrait_text_rect_.bottom = main_frame.bottom - 4;

    bioscan_title_rect_.left = main_frame.left + 4;
    bioscan_title_rect_.top = main_frame.top + 4;
    bioscan_title_rect_.right = portrait_text_rect_.left - 3;
    bioscan_title_rect_.bottom = bioscan_title_rect_.top + 20;
    bioscan_picture_rect_.left = main_frame.left + 4;
    bioscan_picture_rect_.top = bioscan_title_rect_.bottom + 2;
    bioscan_picture_rect_.right = portrait_text_rect_.left - 3;
    bioscan_picture_rect_.bottom = main_frame.bottom - 4;

    field_4_ = 4;
    font1_.init(DefaultFontFace, 10, 0);
    if (WinScreenWidth == 0x320) {
        font2_.init(DefaultFontFace, 10, 0);
        font3_.init(DefaultFontFace, 12, 1);
    } else {
        font2_.init(DefaultFontFace, 12, 0);
        font3_.init(DefaultFontFace, 14, 1);
    }

    const int window_height = (WinScreenWidth == 0x320)
        ? (0x258 - MainInterfaceGlobal.panel_height()) : 0x1C2;
    GraphicWin::init(0, 0, 0x2BC, window_height, 0,
                     0x05000000, static_cast<Win *>(&ConsoleGlobal),
                     0, 0);
    field_16C_ = 0x1A;
    move((WinScreenWidth - pixel_width() + 0x20) / 2,
         WinScreenHeight - pixel_height() - MainInterfaceGlobal.panel_height());
    Buffer::sync_to_palette(PaletteCurrent);
    GraphicWin::fill(9);

    RECT frame = clip_rect();
    frame.left += 0x17;
    frame.right -= 0x37;
    Buffer::box_sprite(&frame, &InterfaceBoxStyles[9]);

    flatButtons_[0].init(label_get(0xEA8 / 4), 0,
                         button_left_rect_.left, button_left_rect_.top,
                         button_left_rect_.right - button_left_rect_.left,
                         button_left_rect_.bottom - button_left_rect_.top,
                         &MainInterfaceGlobal, 0);
    flatButtons_[1].init(label_get(0xEAC / 4), 2,
                         right_panel_button_rect_.left,
                         right_panel_button_rect_.top,
                         right_panel_button_rect_.right - right_panel_button_rect_.left,
                         right_panel_button_rect_.bottom - right_panel_button_rect_.top,
                         &MainInterfaceGlobal, 0);

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x774 / 4));
    CharUpperA(StringTemp);
    flatButtons_[2].init(StringTemp, 1,
                         button_right_rect_.left, button_right_rect_.top,
                         button_right_rect_.right - button_right_rect_.left,
                         button_right_rect_.bottom - button_right_rect_.top,
                         &MainInterfaceGlobal, 0);

    for (int button_index = 0; button_index < 3; ++button_index) {
        flatButtons_[button_index].set_border_styles(
            &InterfaceBoxStyles[27], &InterfaceBoxStyles[28],
            &InterfaceBoxStyles[29]);
        flatButtons_[button_index].set_text_color(0x41, -1, 1, 1);
        static_cast<Buffer &>(flatButtons_[button_index]).set_font(
            &font3_, 0, 0, 0);
        flatButtons_[button_index].redraw();
        flatButtons_[button_index].hide();
    }
    field_20B0_ = 0;
}

// ORIGINAL: 0x0043F5F0 ?draw_bioscan@DiploPop@@QAEXXZ 0x0043F5F0-0x0043F784
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043f5f0.cpp", "image:terranx_original.exe:0x0043F5F0", "source:src/diplopop.h:DiploPop"], "reason": "Recover the bioscan renderer through DiploPop rectangles, MainInterface buffer, source-owned styles, and borrowed Sprite."}
void DiploPop::draw_bioscan() {
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT title = bioscan_title_rect_;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);
    title.left += 3;
    title.right -= 3;
    title.top += 3;
    title.bottom -= 3;

    canvas->set_font(&font3_, 0, 0, 0);
    canvas->set_text_color(0xA0, -1, 1, 1);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xE8C / 4));
    CharUpperA(StringTemp);
    canvas->write_cent_l(StringTemp, &title,
                         static_cast<int>(strlen(StringTemp)));

    RECT picture = bioscan_picture_rect_;
    canvas->box_sprite(&picture, &InterfaceBoxStyles[0]);
    picture.left += 3;
    picture.right -= 3;
    picture.top += 3;
    picture.bottom -= 3;

    const int x = bioscan_picture_rect_.right - frame_sprite_->width() * 2;
    const int y = bioscan_title_rect_.bottom +
                  (bioscan_picture_rect_.bottom - frame_sprite_->height() * 2 -
                   bioscan_title_rect_.bottom) / 2;
    frame_sprite_->draw(canvas, frame_sprite_->transparent_index(), x, y, 1, 1);
}

// ORIGINAL: 0x0043F790 ?draw_portrait@DiploPop@@QAEXXZ 0x0043F790-0x0043FA8E
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043f790.cpp", "image:terranx_original.exe:0x0043F790", "source:src/diplopop.h:DiploPop"], "reason": "Recover the portrait renderer through typed DiploPop/Faction state and existing text, mood, and repute services."}
void DiploPop::draw_portrait() {
    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT text = portrait_text_rect_;
    canvas->box_sprite(&text, &InterfaceBoxStyles[0]);

    const int landscape_height = landscape_sprite_->height();
    const int rule_y = text.top + 1 + landscape_height;
    const int rule_color = canvas->get_pixel(text.left, rule_y);
    canvas->hline(text.left + 1, text.right - 1, rule_y, rule_color);

    // Native retains the one-pixel inset and the landscape-height advance
    // in the same local RECT before applying the later text inset.
    ++text.left;
    --text.right;
    text.top = rule_y;
    --text.bottom;

    canvas->set_font(&font3_, 0, 0, 0);
    canvas->set_text_color(0xA0, -1, 1, 1);
    text.left += 4;
    text.right -= 4;
    text.top += 4;
    text.bottom -= 4;

    StringTemp[0] = '\0';
    strcat(StringTemp, Players[counterpart_faction_].name_leader);
    CharUpperA(StringTemp);
    canvas->write_l(StringTemp, text.left, text.top,
                    static_cast<int>(strlen(StringTemp)));
    text.top += font3_.line_advance();

    canvas->set_font(&font2_, 0, 0, 0);
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xE90 / 4));
    strcat(StringTemp, ": ");
    int repute = PlayersData[counterpart_faction_].integrity_blemishes;
    if (repute < 0) {
        repute = 0;
    } else if (repute > 7) {
        repute = 7;
    }
    say_special(StringTemp,
                StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
                    Repute[repute]))),
                counterpart_faction_);
    canvas->write_l(StringTemp, text.left, text.top,
                    static_cast<int>(strlen(StringTemp)));
    text.top += font2_.line_advance();

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0xE94 / 4));
    strcat(StringTemp, ": ");
    const int local_faction = ConsoleGlobal.faction_id_;
    const int mood = static_cast<int>(get_mood(
        PlayersData[counterpart_faction_].diplo_friction[local_faction]));
    char *mood_text = StringTable.get(static_cast<int>(
        reinterpret_cast<intptr_t>(Mood[mood])));
    say_special(StringTemp, mood_text, counterpart_faction_);
    canvas->write_l(StringTemp, text.left, text.top,
                    static_cast<int>(strlen(StringTemp)));
}

// CRT entry 0x0043EFC0 constructs this object and registers 0x0043EFE0,
// which destroys the same receiver. The CRT table at 0x00682080 contains
// that initializer. Its complete constructed member footprint is 0x433C;
// the original storage is zero-initialized before those constructors run.
// RECOVERY_OBJECT: v2 0x00733990 0x00737CCC ?DiplomacyPopup@@3VDiploPop@@A 0x0043EFF0
// ORIGINAL: 0x0043EFC0 ??__EDiplomacyPopup@@YAXXZ 0x0043EFC0-0x0043EFD6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original relocated CRT slot682080 names43EFC0, which constructs733990 with43EFF0 then registers43EFE0 through atexit645398. Native compiler emits the constructor/registration helpers as an initializer chain; that split remains a measurement difference, not a source workaround."], "reason": "Bind the natural C++ process-lifetime initializer for the diplomacy popup, without a handwritten startup thunk."}
// symbol    _$E21
// ORIGINAL: 0x0043EFE0 ??__FDiplomacyPopup@@YAXXZ 0x0043EFE0-0x0043EFEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original43EFE0 loads733990 and tail-calls the complete destructor440D50. This is the entry registered by43EFC0, not the scalar deleting destructor and not an independently allocated popup."], "reason": "Bind the compiler-generated teardown registered for the same real diplomacy popup object."}
// symbol    _$E19
DiploPop DiplomacyPopup;

// Original 0x4405B0/0x4405D8 set this distinct state word around the
// report/datalink calls, then clear it at 0x4405C9/0x4405F5. Its broader
// meaning is unresolved; do not merge it with the popup's interface state.
// Its initial zero is loader storage after the bounded popup, not an
// initializer backed by file bytes. Only the original start is bound.
// RECOVERY_OBJECT: v1 0x00737CD0 ?DiploCallbackState737CD0@@3HA
int DiploCallbackState737CD0;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0043EFF0 ??0DiploPop@@QAE@XZ 0x0043EFF0-0x0043F0EA;0x00654880-0x006548F4
// RECOVERY_BINDING: v1 {"evidence": ["Original43EFF0..43F0EA and unwind654880..6548F4 construct GraphicWin, the secondary interface base, three Fonts, two Times atA98/AC0, Flics atAE8/15CC and three FlatButtons at2158 with strideB4C. All compiler-generated unwind targets belong to those real lifetimes.", "43F0C8/43F0D2 set counterpart factionA94=-1 and field2154=1 only after all members.43F30D publishes the faction argument intoA94. The old field20AC was inside the second Flic, not an additional scalar. Original CRT43EFC0 constructs733990 and registers its destructor wrapper43EFE0."], "reason": "Recover the complete diplomacy-popup construction through real C++ bases and embedded member lifetimes."}
DiploPop::DiploPop() {
    // Original 0x0043F0C8/0x0043F0D2 publish these only after the three
    // Fonts, two Times, two Flics and three FlatButtons have been built.
    counterpart_faction_ = -1;
    field_2154_ = 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00440D50 ??1DiploPop@@QAE@XZ 0x00440D50-0x00440EB8;0x006547B0-0x00654871
// RECOVERY_BINDING: v1 {"evidence": ["Original440D50..440EB8 and unwind6547B0..654871 restore the class tables, perform the same two Time stops, virtual hide and direct GraphicWin close as close(), then resetA94/20B0/2154. Normal C++ destructor dispatch supplies those table transitions; no table stores are handwritten.", "440DEB onward destroys FlatButton[3] in reverse, then Flic2/Flic1 including their Palette/Buffer members, Time2/Time1, Font3/Font2/Font1, and GraphicWin. The complete unwind path preserves the same partially live subobjects. Existing downstream lifecycle and renderer limitations are not waived."], "reason": "Recover complete diplomacy-popup destruction through close followed by automatic reverse member and base destruction."}
// symbol    ??1DiploPop@@UAE@XZ
DiploPop::~DiploPop() {
    // The original inlines close before reverse member/base teardown.
    // Normal C++ destruction also owns partial-construction unwinding.
    close();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404C0 ?on_iface_left_click@DiploPop@@QAEXHH@Z 0x004404C0-0x004404C3 BYTE_EXACT
// symbol    ?on_iface_left_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404D0 ?on_iface_right_click@DiploPop@@QAEXHH@Z 0x004404D0-0x004404D3 BYTE_EXACT
// symbol    ?on_iface_right_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404E0 ?on_iface_left_down@DiploPop@@QAEXHH@Z 0x004404E0-0x004404E3 BYTE_EXACT
// symbol    ?on_iface_left_down@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404F0 ?on_iface_right_down@DiploPop@@QAEXHH@Z 0x004404F0-0x004404F3 BYTE_EXACT
// symbol    ?on_iface_right_down@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440500 ?on_iface_mouse_move@DiploPop@@QAEXHH@Z 0x00440500-0x00440503 BYTE_EXACT
// symbol    ?on_iface_mouse_move@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440510 ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z 0x00440510-0x00440513 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440520 ?on_iface_selected@DiploPop@@QAEXHH@Z 0x00440520-0x00440523 BYTE_EXACT
// symbol    ?on_iface_selected@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440530 ?on_iface_left_double_click@DiploPop@@QAEXHH@Z 0x00440530-0x00440533 BYTE_EXACT
// symbol    ?on_iface_left_double_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440540 ?on_iface_right_double_click@DiploPop@@QAEXHH@Z 0x00440540-0x00440543 BYTE_EXACT
// symbol    ?on_iface_right_double_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440610 ?on_iface_button_toggled@DiploPop@@QAEXHH@Z 0x00440610-0x00440613 BYTE_EXACT
// symbol    ?on_iface_button_toggled@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_button_toggled(int, int) {
}

// ---- the interface table's own slots still carrying an original body ------

// PENDING 0x00440380 ?on_status@DiploPop@@QAEXH@Z 0x00440380-0x004404B4
// symbol    ?on_status@DiploPop@@UAEXH@Z
// size      308 bytes
// prototype void (__thiscall ?on_status@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// ORIGINAL: 0x00440380 ?on_status@DiploPop@@QAEXH@Z 0x00440380-0x004404B4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00440380.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440380 complete control/member accesses and concrete interface table", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00440380.cpp"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_status@DiploPop@@UAEXH@Z
void DiploPop::on_status(int mode) {
    GraphicWin *window = static_cast<GraphicWin *>(this);
    if (mode) {
        MainInterfaceGlobal.on_redraw();
        for (int i = 0; i < 3; ++i) flatButtons_[i].show(0);
        slide_show(window, 2);
        show(3);
        time1_.start();
        time2_.start();
        MainInterfaceGlobal.flat_button(6).show(0);
        MainInterfaceGlobal.flat_button(7).show(0);
        MainInterfaceGlobal.button_group(1).set(-1, 1);
        flatButtons_[2].set(1);
        MainInterfaceGlobal.redraw();
    } else {
        time1_.stop();
        time2_.stop();
        flic2_.rewind();
        slide_hide(window, 2);
        for (int i = 0; i < 3; ++i) flatButtons_[i].hide();
        MainInterfaceGlobal.flat_button(6).hide();
        MainInterfaceGlobal.flat_button(7).hide();
        hide();
    }
}

// PENDING 0x00440550 ?on_iface_button_clicked@DiploPop@@QAEXH@Z 0x00440550-0x00440604
// Historical emitted spelling was ?vslot_13@DiploPop@@UAEXH@Z.
// size      180 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// indirect  0x00440582
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00440550 ?on_iface_button_clicked@DiploPop@@QAEXH@Z 0x00440550-0x00440604 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00440550.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440550 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_iface_button_clicked@DiploPop@@UAEXH@Z
void DiploPop::on_iface_button_clicked(int button_id) {
    switch (button_id) {
    case 0:
        DiploCallbackState737CD0 = 1;
        DatalinkMain.exec(0xF, counterpart_faction_);
        DiploCallbackState737CD0 = 0;
        break;
    case 1:
        DiploCallbackState737CD0 = 1;
        report_intel(counterpart_faction_);
        DiploCallbackState737CD0 = 0;
        return;
    case 2:
        WorldClimateWorldWin.hide();
        MapWinTable[1]->hide();
        MainInterfaceGlobal.button_group(1).set(-1, 1);
        flatButtons_[2].set(1);
        draw_hq();
        break;
    }
}

// PENDING 0x00440620 ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z 0x00440620-0x004406CB
// symbol    ?on_iface_group_clicked@DiploPop@@UAEXHHH@Z
// size      171 bytes
// prototype void (__thiscall ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z)(DiploPop* this, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// indirect  0x00440643
// ORIGINAL: 0x00440620 ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z 0x00440620-0x004406CB FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00440620.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440620 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// TRIED: MISMATCH #7 sub/add, virtual-base offset arithmetic form differs
// symbol    ?on_iface_group_clicked@DiploPop@@UAEXHHH@Z
void DiploPop::on_iface_group_clicked(int group, int x_coord, int y_coord) {
    int selection = y_coord;
    if (selection > 1000) selection -= 1000;
    selection -= 6;

    if (selection == 0) {
        WorldClimateWorldWin.show(0);
        MapWinTable[1]->show(0);
        group = 0x3EE;
    } else if (selection == 1) {
        WorldClimateWorldWin.hide();
        MapWinTable[1]->hide();
        group = 0x3EF;
    } else {
        on_iface_button_clicked(2);
        draw_hq();
        return;
    }

    MainInterfaceGlobal.button_group(1).set(group, 0);
    flatButtons_[2].set(0);
    draw_hq();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00440080 ?draw_hq@DiploPop@@QAEXXZ 0x00440080-0x00440291 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00440080.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440080 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
void DiploPop::draw_hq() {
    Buffer *const canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT area = rect1_;

    if (flatButtons_[2].selected_state() == 0) {
        canvas->set_clip(&area);
        MainInterfaceGlobal.restore_back(0);
        canvas->set_clip(MainInterfaceGlobal.buffer_working_rect());
        MainInterfaceGlobal.soft_update(&rect1_);
        return;
    }

    area.bottom -= 4;
    canvas->box_sprite(&area, &InterfaceBoxStyles[0]);

    const int left = area.left + 1;
    const int top = area.top + 1;
    const int right = area.right - 1;
    area.bottom -= 1;
    const int text_top = top + 0x15;
    area.left = left;
    area.top = top;
    area.right = right;

    int base_id = 0;
    StringTemp[0] = 0;
    while (base_id < GameDataStateStorage.fields_.base_current_count_) {
        if (Bases[base_id].faction_id_current == counterpart_faction_
            && has_fac_built_call(FAC_HEADQUARTERS, base_id)) break;
        ++base_id;
    }

    const char *heading = base_id == GameDataStateStorage.fields_.base_current_count_
        ? label_get(0x3A9) : Bases[base_id].name_string;
    strcat(StringTemp, heading);
    CharUpperA(StringTemp);

    canvas->set_font(&font3_, 0, 0, 0);
    canvas->set_text_color(0xA0, -1, 1, 1);
    if (StringTemp[0]) {
        canvas->write_cent_l(StringTemp, &area,
                             static_cast<int>(strlen(StringTemp)));
    }

    area.top = text_top + 1;
    const int x = (area.right - hq_sprite_->width() - area.left) / 2 + area.left;
    hq_sprite_->draw(canvas, hq_sprite_->transparent_index(), x, area.top, 1, 1);
    MainInterfaceGlobal.soft_update(&rect1_);
}


/*
// ORIGINAL: 0x0043F170 ?hide@DiploPop@@QAEXXZ 0x0043F170-0x0043F185 BYTE_EXACT
// symbol    ?hide@DiploPop@@UAEXXZ
// size      21 bytes
// prototype void (__thiscall ?hide@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x005F7E90
Status: Complete
*/
void DiploPop::hide() {
    Win *self = reinterpret_cast<Win *>(this);
    if (self->is_visible()) {
        // QUALIFIED: the image emits a direct `call 0x5edcd0`, not a vtable
        // dispatch. `Win::hide` is virtual now, so an unqualified call would
        // go through slot 2 - which is what made this claim regress.
        self->Win::hide();
    }
}

// ORIGINAL: 0x0043F0F0 ?close@DiploPop@@QAEXXZ 0x0043F0F0-0x0043F137 FILE
// size      71 bytes
// prototype void (__thiscall ?close@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00616730
// indirect  0x0043F10D
// Historical source-form experiments below are retained as seed evidence.
// TRIED: 2026-09-03 homing pass, 2 spellings. MISMATCH 2/16 is the
//        `Time::stop` ceiling again - the image CALLS 0x00616730 here and
//        time.h keeps `stop` in-class MEASURED so `close`/`~Time` inline it,
//        and VC6 has no noinline. Splitting `stop` is a time.h edit.
// The complete constructor/destructor establish the typed members in
// diplopop.h. Slot 2 is ordinary virtual hide; the following close is direct.
void DiploPop::close() {
    time1_.stop();
    time2_.stop();
    hide();
    GraphicWin::close();
    counterpart_faction_ = -1;
    field_20B0_ = 0;
    field_2154_ = 1;
}

// ORIGINAL: 0x0043F140 ?show@DiploPop@@QAEXH@Z 0x0043F140-0x0043F162 BYTE_EXACT
// symbol    ?show@DiploPop@@UAEXH@Z
// LEVER: the tail call must be QUALIFIED (`win->Win::show(0)`), otherwise the
//        override dispatches through the vtable where the image has
//        `call rel32`.
// size      34 bytes
// prototype void (__thiscall ?show@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED9D0 0x005F7E90
// 0x0043F140  ?show@DiploPop@@QAEXH@Z  ->  ?show@DiploPop@@QAEXH@Z
//
// Historical seed evidence is retained alongside this product definition.


void DiploPop::show(int) {
    if (is_visible() == 0 && counterpart_faction_ != -1) {
        Win::show(0);
    }
}

// ORIGINAL: 0x0043F5C0 ?stop@DiploPop@@QAEXXZ 0x0043F5C0-0x0043F5F0 BYTE_EXACT
// size      48 bytes
// prototype void (__thiscall ?stop@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x00629DF0
// 0x0043F5C0  ?stop@DiploPop@@QAEXXZ  ->  ?stop@DiploPop@@QAEXXZ
//
// Historical seed evidence is retained alongside this product definition.


void DiploPop::stop() {
    release_iface_mode();
    flic2_.close();
    flic1_.close();
    counterpart_faction_ = -1;
}

// ORIGINAL: 0x0043F290 ?on_button_clicked@DiploPop@@QAEXH@Z 0x0043F290-0x0043F2AD FILE BYTE_EXACT
// symbol    ?on_button_clicked@DiploPop@@UAEXH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      29 bytes
// prototype void (__thiscall ?on_button_clicked@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043F2A3
// Forward to the currently modal Win through its actual button-click slot.


void DiploPop::on_button_clicked(int button_id) {
    Win *target = WinModalFocus;
    if (target) {
        target->on_button_clicked(button_id);
    }
}


/*
// ORIGINAL: 0x004406D0 ?set_modal@DiploPop@@QAEHHP6AHXZPAUWin@@@Z 0x004406D0-0x004406EC FILE BYTE_EXACT
// symbol    ?set_modal@DiploPop@@UAEHHP6AHXZPAVWin@@@Z
// LEVER: the modal owner is not the caller's `this` but the fixed
// MainInterface at 0x007AE820, now named MainInterfaceGlobal (mapwin.cpp)
// instead of a carried g_ binding.
*/
int DiploPop::set_modal(int flag, int (__cdecl *poll)(), Win *owner) {
    return reinterpret_cast<Win *>(this)->Win::set_modal(
        flag | 1, poll, &MainInterfaceGlobal);
}

/*
// ORIGINAL: 0x004406F0 ?release_modal@DiploPop@@QAEXXZ 0x004406F0-0x00440707 FILE
// symbol    ?release_modal@DiploPop@@UAEXXZ
// TRIED: every spelling of a non-virtual override. The image's release_modal
//        is `QAEXX`, this tree emits `UAEXX` because win.h declares
//        `Win::release_modal` virtual at slot 58. Same wall as BaseWin::hide.
// TRIED: 2026-09-03 homing pass. The image's own bytes at 0x004406F0 are
//        `E9 8B DB 1A 00` (jmp Win::release_modal) followed by eleven 0x90
//        nops to 0x004406FF; 0x00440700 starts a DIFFERENT function
//        (`55 8B EC 6A FF`). The span on the ORIGINAL line,
//        0x004406F0-0x00440707, therefore swallows the next function's
//        prologue, and no body can reproduce 23 bytes that are only 5 bytes
//        of code plus padding. The marker line is not editable, so the claim
//        is demoted rather than mismatches forever. The body below is the
//        faithful 5-byte tail call.
*/
void DiploPop::release_modal() {
    reinterpret_cast<Win *>(this)->Win::release_modal();
}

// ORIGINAL: 0x0043F190 ?portrait_anim@DiploPop@@QAEXXZ 0x0043F190-0x0043F1BF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original decodes the Flic at DiploPop +0xAE8 using its stored x/y at Flic +0x5AC/+0x5B0, then passes Flic +0x58C to the main interface's soft_update.", "DiploPop::flic1_ is the constructed Flic at +0xAE8, and MainInterfaceGlobal is the existing GraphicWin owner at 0x007AE820."], "reason": "Home the portrait animation using its embedded Flic, typed update rectangle, and process-owned update window."}
void DiploPop::portrait_anim() {
    flic1_.decode_stored_frame();
    MainInterfaceGlobal.soft_update(flic1_.update_rect());
}

// ORIGINAL: 0x0043F1C0 ?bioscan_anim@DiploPop@@QAEXXZ 0x0043F1C0-0x0043F1FC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original increments DiploPop +0x20B0, decodes the Flic at +0x15CC using its stored x/y at Flic +0x5AC/+0x5B0, and updates Flic +0x58C.", "DiploPop::flic2_ is the constructed Flic at +0x15CC; its footprint ends at +0x20B0, where the existing animation counter begins."], "reason": "Home the bioscan animation through its typed Flic member, update rectangle, and owner counter."}
void DiploPop::bioscan_anim() {
    ++field_20B0_;
    flic2_.decode_stored_frame();
    MainInterfaceGlobal.soft_update(flic2_.update_rect());
}

// ORIGINAL: 0x0043F200 ?timer_callback_daemon@DiploPop@@QAAXHH@Z 0x0043F200-0x0043F245 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Disassembly reads the popup pointer from the second cdecl stack argument ([ebp+0xC]); it does not use ECX as a hidden DiploPop receiver.", "The callback's Flic2 decode, animation counter increment, and update rectangle are exactly the work owned by DiploPop::bioscan_anim.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0043f200.cpp"], "reason": "Keep the timer entry a real two-argument cdecl callback and route its explicit popup parameter into the typed owner method. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?timer_callback_daemon@@YAXHPAVDiploPop@@@Z
void __cdecl timer_callback_daemon(int, DiploPop *popup) {
    popup->bioscan_anim();
}

// ORIGINAL: 0x0043F250 ?timer_callback_daemon2@DiploPop@@QAAXHH@Z 0x0043F250-0x0043F288 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Disassembly reads the popup pointer from the second cdecl stack argument ([ebp+0xC]); this is a free callback, not a DiploPop member entry despite the historical catalog symbol spelling.", "The callback's Flic1 decode and update rectangle are the work owned by DiploPop::portrait_anim."], "reason": "Preserve the timer callback's actual cdecl stack ABI and use the typed portrait animation implementation."}
// symbol    ?timer_callback_daemon2@@YAXHPAVDiploPop@@@Z
void __cdecl timer_callback_daemon2(int, DiploPop *popup) {
    popup->portrait_anim();
}

// Original interface table 0x0066B424 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_scrolled(int, int) {}
// Original interface table 0x0066B424 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_scrolling(int, int) {}
// Original interface table 0x0066B424 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_left_up(int, int) {}

// Original interface table 0x0066B424 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@DiploPop@@UAEHH@Z 0x00440380
int DiploPop::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066B424 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@DiploPop@@UAEHPAVGraphicWin@@@Z 0x00440380
int DiploPop::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066B424 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@DiploPop@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x00440380
int DiploPop::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}

// ORIGINAL: 0x0043FA90 ?draw_info@DiploPop@@QAEXXZ 0x0043FA90-0x0044007F
// RECOVERY_BINDING: v1 {"evidence": ["Pinned43FA90..44007F; screenwidth800 portrait branch; Buffer wrap field at receiver454; MainInterface Buffer receiver7AEC64; font2/3 and faction portrait typed layouts. Ranking reads96C9E4 PlayerData+4, not misleading94C9E4 Mood. Repeated spying calls and live local-faction loads preserved; host controls verify drawing phases/treaty priority/call ordering."], "reason": "Recover complete two-panel diplomacy display as a genuine nonvirtual DiploPop member, using actual Buffer bases, embedded fonts/portrait assets and source-owned faction state rather than raw self offsets."}
// TRIED: transcription mixing Ghidra's control-flow structure with a raw-asm re-derivation of the RECT field-to-stack-slot mapping for the box_sprite/draw_mono/draw calls, since Ghidra's own local_XX names there did not correspond to their literal ebp offsets (proven by cross-checking the Y-formula against the confirmed slot layout: RECT starts at ebp-0x10, not ebp-0x14 as the naming implied) - trusted raw asm for that part, and Ghidra's formulas/call order for the later (mechanical) string-building/wrap/write_l portion. The `Sprite* portrait = g_0078ee74 + leaderIdx*0x65c` stride and its iWidth_/iHeight_/cTransparentIndex_ field reads were confirmed against Sprite's catalogued layout. Reaches MISMATCH #4 ('push' vs 'mov'); did not chase the exact prologue/register-allocation shape further given the function's size.
void DiploPop::draw_info() {
    Buffer *const canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    const RECT source = rect1_;
    int left_edge = source.left;
    const int top_edge = source.top;
    const int bottom_edge = source.bottom;
    int right_edge = ((source.right - left_edge) + 3) / 2 - 3 + left_edge;

    static_cast<Buffer *>(this)->set_wrap_width(15);
    for (int panel = 0; panel < 2; ++panel) {
        RECT frame = {left_edge, top_edge, right_edge, bottom_edge};
        canvas->box_sprite(&frame, &InterfaceBoxStyles[0]);

        int left = left_edge + 3;
        int right = right_edge - 3;
        int top = top_edge + 3;
        int bottom = bottom_edge - 3;
        const int faction = panel == 0 ? counterpart_faction_
                                       : ConsoleGlobal.faction_id_;
        Sprite *const portrait = &FactionsArt[faction].diplomacy_logo;

        if (WinScreenWidth == 800) {
            const int portrait_y = ((bottom - portrait->height()) - top) / 2 + top;
            const int portrait_x = ((right - portrait->width()) - left) / 2 + left;
            portrait->draw_mono(canvas, portrait->transparent_index(),
                                portrait_x, portrait_y, 0x41, 1, 1);
        } else {
            portrait->draw(canvas, portrait->transparent_index(), left,
                           font3_.line_advance() + top + 3, 1, 1);
        }

        canvas->set_text_color(ChatItemIdTable[faction], -1, 1, 1);
        canvas->set_font(&font3_, nullptr, nullptr, nullptr);
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction].name_leader);
        CharUpperA(StringTemp);

        RECT name_area = {left, top + 2, right,
                          font3_.line_advance() + top};
        int text_y = canvas->wrap_cent(StringTemp, &name_area);
        if (WinScreenWidth != 800) {
            left += 2 + portrait->width();
        }

        canvas->set_text_color(0xA0, -1, 1, 1);
        right -= 3;
        top += 3;
        bottom -= 3;
        const int text_x = left + 3;
        canvas->set_font(&font2_, nullptr, nullptr, nullptr);
        text_y += 3;

        if (panel == 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0xE98 / 4));
            strcat(StringTemp, ": ");

            const uint32_t relation =
                PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction];
            int relation_label;
            if ((relation & DTREATY_PACT) != 0) {
                relation_label = 0xA14 / 4;
            } else if ((relation & DTREATY_TREATY) != 0) {
                relation_label = 0xA10 / 4;
            } else if ((relation & DTREATY_TRUCE) != 0) {
                relation_label = 0xA0C / 4;
            } else if ((relation & DTREATY_VENDETTA) != 0) {
                relation_label = 0xA1C / 4;
            } else {
                relation_label = 0x64 / 4;
            }
            strcat(StringTemp, label_get(relation_label));
            CharUpperA(StringTemp);
            text_y = canvas->wrap(StringTemp, text_x, text_y, right - text_x);
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0xE9C / 4));
        strcat(StringTemp, ": ");
        const int espionage = spying(faction);
        strcat(StringTemp, label_get(espionage == 0 ? 0xEA0 / 4 : 0x614 / 4));
        text_y = canvas->wrap(StringTemp, text_x, text_y, right - text_x);

        if (faction == ConsoleGlobal.faction_id_ || spying(faction) != 0) {
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0x90 / 4));
            strcat(StringTemp, ": ");
            char amount[80];
            _itoa(PlayersData[faction].energy_reserves -
                      static_cast<int>(PlayersData[faction].hurry_cost_total),
                  amount, 10);
            strcat(StringTemp, amount);
            text_y = canvas->write_l(StringTemp, text_x, text_y,
                                     static_cast<int>(strlen(StringTemp)));
            text_y += font2_.line_advance();
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x37C / 4));
        strcat(StringTemp, ": ");
        const int next_y = canvas->write_l(StringTemp, text_x, text_y,
                                           static_cast<int>(strlen(StringTemp)));

        StringTemp[0] = '\0';
        int might_index = 7 - static_cast<int>(PlayersData[faction].ranking);
        if (might_index < 0) might_index = 0;
        else if (might_index > 6) might_index = 6;
        char *const might = StringTable.get(static_cast<int>(
            reinterpret_cast<intptr_t>(Might[might_index].adj_start)));
        say_special(StringTemp, might, faction);
        CharUpperA(StringTemp);
        canvas->write_l(StringTemp, next_y, text_y,
                        static_cast<int>(strlen(StringTemp)));

        const int step = (right_edge - left_edge) + 3;
        left_edge += step;
        right_edge += step;
    }
}

// ORIGINAL: 0x00539970 sub_539970 0x00539970-0x00539998 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/00539970.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x539970 complete original body; independent source-owner review.", "source:src/diplopop.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_539970@@YAXXZ
void __cdecl sub_539970() {
    DiploPop &popup = DiplomacyPopup;
    if (static_cast<Win &>(popup).is_visible()) {
        popup.draw_info();
        static_cast<GraphicWin &>(popup).soft_update(&popup.rect1_);
    }
}
