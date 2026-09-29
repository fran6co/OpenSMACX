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
#include "original_seam.h"
#include "infowin.h"
#include "basewin.h"
#include "tutwin.h"
#include "console.h"
#include "council.h"
#include "datalink.h"
#include "statuswin.h"
#include "worldwin.h"
#include "time.h"
#include "font.h"
#include "pushbutton.h"
#include "game.h"
#include "map.h"
#include "faction.h"
#include "maininterface.h"
#include <cstring>

// ORIGINAL: 0x00456300 ?setup_text@InfoWin@@QAEXXZ 0x00456300-0x0045838D
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00456300.cpp", "source:src/infowin.h"], "reason": "InfoWin::timer_change now calls this original 8333-byte panel-content builder. Its existing seed is incomplete; an explicitly unfinished owning-TU definition preserves link continuity without claiming that panel content has been recovered."}
void InfoWin::setup_text() {
    // BODY GOES HERE.
}

// The four animation counters have not yet been given source-owned storage.
// Keep their binding explicit until that owner is migrated; pane state and
// drawing work below use real InfoWin members.
static int *const InfoAnimationCounters = (int *)0x007AF174;

// Shared information-panel extent, immediately preceding WorldMapBounds in
// the original zero-filled layout block. The four stores at 0x00458D8A-DA9
// establish RECT field order, and InfoWin::init consumes the same extent.
// RECOVERY_OBJECT: v2 0x00939EF8 0x00939F08 ?InfoPanelBounds@@3UtagRECT@@A 0x00458D60
RECT InfoPanelBounds;

// The original's `ConsoleGlobal + vbtable[1] + 0x4C8` is the GraphicWin
// virtual base's Buffer DIB height. `pixel_height()` handles its top-down
// sign convention through the real base subobject.
// ORIGINAL: 0x00458D60 ?UNK4@InfoWin@@QAEXXZ 0x00458D60-0x00458DBB
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458D60-0x00458DBB", "git:3a14c197:src/recovered/units/00458d60.cpp", "source:src/console.h:GraphicWin virtual base", "source:src/buffer.h:Dib"], "reason": "The original stores one RECT at 0x00939EF8, sizing it from the screen-width copy and the Console GraphicWin Buffer DIB height; the source name describes the observed behavior, not the catalog placeholder UNK4."}
// symbol    ?configure_panel_bounds@InfoWin@@QAEXXZ
void InfoWin::configure_panel_bounds() {
    const bool wide = WinScreenWidth >= 1000;
    const int panel_width = wide ? 250 : 200;
    const int panel_height = wide ? 212 : 180;
    InfoPanelBounds.left = 0;
    InfoPanelBounds.right = panel_width;
    InfoPanelBounds.top = ConsoleGlobal.pixel_height();
    InfoPanelBounds.bottom = InfoPanelBounds.top + panel_height;
    StatusFlags |= 4;
}

// ORIGINAL: 0x004592A0 ?init@InfoWin@@QAEXXZ 0x004592A0-0x00459414
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004592A0-0x00459414", "git:3a14c197:src/recovered/units/004592a0.cpp", "source:src/infowin.h:owned members", "source:src/maininterface.h:info_panel_frame_"], "reason": "Initialize the typed InfoWin pane states, font, paired pane rectangles, bounds and timer callback in the owner; the original reads MainInterfaceGlobal+0xDA0 as one panel RECT and conditionally initializes the shared panel extent."}
void InfoWin::init() {
    pane_states_[0] = -1;
    pane_states_[1] = -1;
    redraw_phase_ = -1;
    field_B4_ = -1;
    field_AC_ = 0;
    field_94_ = 0;
    field_A8_ = 0;
    field_84_ = 1;
    active_pane_ = 1;

    if ((StatusFlags & 4) == 0) configure_panel_bounds();

    font_.init(const_cast<char *>(DefaultFontFace),
               WinScreenWidth == 800 ? 10 : 12, 0);
    font_.unk_1_ = 4;

    RECT pane = MainInterfaceGlobal.info_panel_frame();
    pane.left += 7;
    pane.top += 7;
    pane.right -= 0x27;
    pane.bottom -= 7;
    if (WinScreenWidth != 800) {
        pane.right = (pane.right - pane.left - 3) / 2 + pane.left;
    }
    pane_rects_[0] = pane;
    if (WinScreenWidth != 800) {
        const int stride = pane.right - pane.left + 3;
        pane.left += stride;
        pane.right += stride;
    }
    pane_rects_[1] = pane;
    UnionRect(&bounds_, &pane_rects_[0], &pane_rects_[1]);
    timer_.start(InfoWin::timer_daemon, 10, 50, 50);
}

// The timer dispatches a plain C callback. Its argument is forwarded as the
// timer stage, while the process-wide information window owns the state.
// ORIGINAL: 0x00459260 ?timer_daemon@InfoWin@@QAAXH@Z 0x00459260-0x0045927C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00459260-0x0045927C", "git:c732d1a0:src/recovered/00459260.cpp", "source:src/console.cpp:ConsoleInfoWin", "source:src/map.cpp:WorldClimateSkipTerrainClear"], "reason": "The stack-only callback forwards its tick argument to the process-wide InfoWin timer_proc unless climate terrain clearing is active; the old Q-prefixed catalog label is not evidence for an instance receiver."}
// symbol    ?timer_daemon@InfoWin@@SAXH@Z
void __cdecl InfoWin::timer_daemon(int tick) {
    if (WorldClimateSkipTerrainClear == 0) {
        ConsoleInfoWin.timer_proc(tick);
    }
}

// Reveal the active pane until it has reached its timed limit. The final
// pause is suppressed while the adjacent main-interface control is visible.
// That control is flat_button_[11]: MainInterfaceGlobal is at 0x007AE820,
// the array begins at +0xE00, and each FlatButton occupies 0xB4C bytes
// (flatbutton.h:66), placing element 11 at 0x007B7264.
// ORIGINAL: 0x00459150 ?timer_proc@InfoWin@@QAEXH@Z 0x00459150-0x00459260
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00459150.cpp"], "reason": "Home InfoWin timer behavior with typed pane members and MainInterface flat-button slot"}
void InfoWin::timer_proc(int a1) {
    if (field_AC_ != 0) return;
    if (active_interface_slot_occupied()) return;
    if (MainInterfaceGlobal.flat_button_[11].is_visible() != 0) return;

    const int pane = active_pane_;
    const int count = ++visible_characters_[pane];
    const int length = text_lengths_[pane];
    if (count > length + 0x32) {
        int mouse_x;
        int mouse_y;
        MainInterfaceGlobal.get_mouse_pos(&mouse_x, &mouse_y);
        if (in_box(mouse_x, mouse_y, &bounds_) != 0 ||
            field_A8_ != 0 || field_94_ != 0) {
            visible_characters_[pane] = length + 0x28;
            field_94_ = field_94_ == 0 ? 1 : 0;
            draw_text(-1);
            return;
        }
        timer_change(a1);
        return;
    }

    if (count <= length + 1) draw_text(pane);
}

// Advance the selected information pane through its valid display states,
// then repaint the newly selected pane.
// ORIGINAL: 0x00458FF0 ?timer_change@InfoWin@@QAEXH@Z 0x00458FF0-0x00459141
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/00458ff0.cpp", "source:src/infowin.h", "source:src/game.cpp:InterfaceBoxStyles"], "reason": "Home the timed pane transition with the owned InfoWin state, real pane rectangle and source-owned box style; retain the unresolved active-interface and animation-counter storage bindings as explicit dependencies."}
void InfoWin::timer_change(int) {
    if (active_interface_slot_occupied()) return;

    const int old_pane = active_pane_;
    field_94_ = 0;
    active_pane_ = WinScreenWidth == 800 ? 0 : (old_pane - 1) & 1;

    bool first = true;
    for (;;) {
        const int source_state = redraw_phase_ >= 0
            ? redraw_phase_ : pane_states_[first ? old_pane : active_pane_];
        pane_states_[active_pane_] = (source_state + 1) % 8;
        redraw_phase_ = -1;
        first = false;
        if (state_valid(pane_states_[active_pane_])) break;
    }

    for (int index = 0; index != 4; ++index) InfoAnimationCounters[index] = 0;
    visible_characters_[0] = 0;
    visible_characters_[1] = 0;
    field_A8_ = 0;
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    setup_text();
    draw_text(active_pane_);
}

// Primary normal/revealed and secondary normal/revealed foreground/shadow
// pairs read by draw_text. Their initialization policy is still unrecovered.
// RECOVERY_OBJECT: v2 0x008C6BB4 0x008C6BD4 ?InfoTextColors@@3PAY01HA 0x00458420
int InfoTextColors[4][2];
// Rank rows use one palette entry per faction, including native life.
// RECOVERY_OBJECT: v2 0x0068F9F0 0x0068FA10 ?InfoFactionTextColors@@3PAHA 0x00458420
int InfoFactionTextColors[8] = {249, 111, 252, 255, 251, 65, 102, 187};
// RECOVERY_OBJECT: v2 0x0068FA10 0x0068FA30 ?InfoFactionShadowColors@@3PAHA 0x00458420
int InfoFactionShadowColors[8] = {1, 175, 4, 248, 3, 57, 109, 192};

// ORIGINAL: 0x00458420 ?draw_text@InfoWin@@QAEXH@Z 0x00458420-0x004588A9
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458420-0x004588A9"], "reason": "Recovered original behavior through real source classes and owned members; source remains subject to byte and structural verification."}
void InfoWin::draw_text(int pane) {
    Buffer &buffer = MainInterfaceGlobal;
    buffer.set_font(&font_, 0, 0, 0);
    int row = 0;
    int first = pane == -1 ? 0 : pane;
    int end = pane == -1 ? 2 : pane + 1;
    for (int column = first; column < end; ++column) {
        RECT &clip = pane_rects_[column];
        buffer.set_clip(&clip);
        int left = clip.left + 3;
        int right = clip.right - 3;
        int remaining = visible_characters_[column];
        for (; row < row_count_[column] && remaining >= 0; ++row) {
            char *text = row_text_[column][row];
            int length = strlen(text);
            bool ranked = pane_states_[column] == 4 && row >= 1;
            if (length) {
                int color = row_color_[column][row];
                if (ranked) {
                    int faction = FactionRankings[8 - row];
                    buffer.set_text_color(InfoFactionTextColors[faction],
                                          InfoFactionShadowColors[faction], 1, 1);
                } else if (!color) {
                    int primary = field_94_ ? 1 : 0;
                    buffer.set_text_color(InfoTextColors[primary][0],
                                          InfoTextColors[primary][1], 1, 1);
                    buffer.set_text_color2(InfoTextColors[primary + 2][0],
                                           InfoTextColors[primary + 2][1], 1, 1);
                } else {
                    buffer.set_text_color(field_94_ ? row_reveal_color_[column][row]
                                                    : color, -1, 1, 1);
                }
                int y = row_y_[column][row];
                if (length < remaining) {
                    if (ranked) buffer.write_right_l(text, 2, y, right - left - 4, strlen(text));
                    else buffer.write_l(text, row_x_[column][row], y, strlen(text));
                } else {
                    // A zero budget displays no character. In particular, do
                    // not form the seed's row[-1] pointer or use its uninitialized x.
                    if (remaining > 0) {
                        char prefix[80];
                        strcpy(prefix, text);
                        prefix[remaining - 1] = 0;
                        int x = ranked ? right - left - buffer.text_width(text) - 2
                                       : row_x_[column][row];
                        x = buffer.write_l(prefix, x, y, strlen(prefix));
                        strcpy(prefix, text);
                        prefix[remaining] = 0;
                        if (!ranked) {
                            if (!color) {
                                buffer.set_text_color(InfoTextColors[1][0], InfoTextColors[0][1], 1, 1);
                                buffer.set_text_color2(InfoTextColors[3][0], InfoTextColors[3][1], 1, 1);
                            } else {
                                buffer.set_text_color(row_reveal_color_[column][row], -1, 1, 1);
                            }
                        }
                        char *last = prefix + remaining - 1;
                        buffer.write_l(last, x, y, strlen(last));
                    }
                    buffer.reset_text_state();
                }
            }
            remaining -= length;
        }
    }
    buffer.restore_clip();
    MainInterfaceGlobal.update(pane == -1 ? &bounds_ : &pane_rects_[pane], 0);
    TutorialWindow.draw_arrow();
}

// Refresh the active pane's border and contents. The sprite parameters at
// 0x0078D7F8 are InterfaceBoxStyles[18] (game.cpp's table begins at
// 0x0078D528 and each BoxSpriteParams occupies 0x28 bytes). The seed's
// `if (this + 0x88)` tests the address of pane state, so its body is
// unconditional for every valid InfoWin object.
// ORIGINAL: 0x004588B0 ?UNK2@InfoWin@@QAEXXZ 0x004588B0-0x004588FE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004588b0.cpp", "source:src/infowin.h", "source:src/game.cpp:InterfaceBoxStyles", "source:src/buffer.h:BoxSpriteParams"], "reason": "Home the active-pane refresh with its embedded rectangle, shared buffer, and source-owned box style; preserve the measured legacy claim while replacing opaque offsets with InfoWin members."}
void InfoWin::UNK2() {
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    visible_characters_[0] = 0;
    visible_characters_[1] = 0;
    setup_text();
    draw_text(active_pane_);
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004562F0 ?UNK1@InfoWin@@QAEXXZ 0x004562F0-0x004562F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00458910 ?on_redraw@InfoWin@@QAEXXZ 0x00458910-0x00458911 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?on_redraw@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::on_redraw() {
}

// ORIGINAL: 0x00458930 ?on_left_click@InfoWin@@QAEXHH@Z 0x00458930-0x004589C0 BYTE_EXACT
void InfoWin::on_left_click(int, int) {
    TutWinBaseWindow.cancel_zoom_wait();
    field_A8_ = 0;
    ConsoleStatusWin.reset();

    for (int pane = 0; pane < 2; ++pane) {
        if (visible_characters_[pane] < text_lengths_[pane] + 1) {
            visible_characters_[pane] = text_lengths_[pane] + 1;
            draw_text(pane);
            redraw_phase_ = 0;
            return;
        }
    }

    timer_change(1);
    visible_characters_[0] = text_lengths_[0] + 1;
    visible_characters_[1] = text_lengths_[1] + 1;
    draw_text(-1);
}

// State numbers are presentation modes; their display names remain unknown.
// Difficulty and contact tests use the owning faction record, not absolute
// addresses or a manually scaled 0x20CC stride.
// ORIGINAL: 0x00458DC0 ?state_valid@InfoWin@@QAEHH@Z 0x00458DC0-0x00458EDA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458DC0-0x00458EDA; jump table 0x00458EDC-0x00458F0C", "source:src/faction.h:PlayerData.diff_level and diplo_treaties", "git:6455075f:src/unrecovered/00458dc0.cpp"], "reason": "Recover InfoWin mode eligibility using game state and typed faction difficulty/contact fields; switch destinations checked in original image; retired seed is a hint, not product code."}
int InfoWin::state_valid(int state) {
    if (state == 0) {
        return 1;
    }
    if (TurnCurrentNum != 0) {
        switch (state) {
        case 0:
        case 11:
            return 1;
        case 1:
            if (TurnCurrentNum >= 50 &&
                (TurnCurrentNum >= 100 || PlayersData[ConsoleGlobal.faction_id_].diff_level >= DLVL_TALENT)) {
                return 1;
            }
            break;
        case 3:
            if (DiplomacyActive) {
                return 1;
            }
            if (TurnCurrentNum >= 50 || PlayersData[ConsoleGlobal.faction_id_].diff_level >= DLVL_TALENT) {
                for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                    if (faction != ConsoleGlobal.faction_id_ &&
                        (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & DTREATY_COMMLINK)) {
                        return 1;
                    }
                }
            }
            break;
        case 2:
            if (DiplomacyActive) {
                return 1;
            }
            break;
        case 4:
        case 5:
        case 6:
            if (TurnCurrentNum >= 20 || (GameState & STATE_IS_SCENARIO)) {
                return 1;
            }
            break;
        case 7:
            if ((GameState & STATE_OMNISCIENT_VIEW) && (GameState & STATE_SCENARIO_EDITOR)) {
                return 1;
            }
            break;
        }
    }
    return 0;
}

// Select a valid presentation state for the active pane, reset its display
// budget, and redraw it. The original's final `this + 0x88 != 0` is an
// address check and is always true for a valid object; consequently the
// visible-character counters are always reset regardless of a2.
// TRIED (seed): 0.986 mnemonic similarity, one instruction off (`jns` vs
// `jge`) on the sign test after masking; VC6 emitted a fresh compare rather
// than reusing the AND flags. The seed's other two divergences were fixed by
// emitting the a2==0 ternary arms else-first.
// ORIGINAL: 0x00458F10 ?set_state@InfoWin@@QAEXHH@Z 0x00458F10-0x00458FEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00458f10.cpp", "source:src/infowin.h", "source:src/infowin.cpp:state_valid"], "reason": "Replace the offset-based seed with the InfoWin's active-pane and presentation-state members and its existing eligibility method."}
void InfoWin::set_state(int state, int) {
    if (state < 0) state = 0;
    int &pane_state = pane_states_[active_pane_];
    pane_state = state;
    while (!state_valid(pane_state)) {
        int next = pane_state + 1;
        next &= 0x80000007;
        if (next < 0) next = ((next - 1) | 0xFFFFFFF8) + 1;
        pane_state = next;
    }
    field_94_ = 0;
    visible_characters_[0] = 0;
    visible_characters_[1] = 0;
    field_A8_ = 0;
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    setup_text();
    draw_text(active_pane_);
}



/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00458900 ?change@InfoWin@@QAEXH@Z 0x00458900-0x00458903 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?change@InfoWin@@QAEXH@Z)(InfoWin* this, int)
// callers   10   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::change(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00458920 ?UNK3@InfoWin@@QAEXHH@Z 0x00458920-0x00458923 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@InfoWin@@QAEXHH@Z)(InfoWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK3(int, int) {
}



/*
Purpose: Cancel the pending info-window timer, if one is outstanding.
// ORIGINAL: 0x00459280 ?reset@InfoWin@@QAEXXZ 0x00459280-0x0045929C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?reset@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00459150
Return Value: n/a
Status: Complete
*/
void InfoWin::reset() {
    if (field_A8_) {
        field_A8_ = 0;
        timer_proc(1);
    }
}


/*
// ORIGINAL: 0x00458D40 ?on_right_click@InfoWin@@QAEXHH@Z 0x00458D40-0x00458D54 BYTE_EXACT
// size      20 bytes
// prototype void (__thiscall ?on_right_click@InfoWin@@QAEXHH@Z)(InfoWin* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004589C0
Status: Complete
*/
void InfoWin::on_right_click(int a, int b) {
    right_menu(a, b);
}

func_info_win_right_menu InfoWinOriginalRightMenu =
    original_method<func_info_win_right_menu>(0x004589C0);

// right_menu's own body has NOT homed yet: it calls help_any (0x0044CB30),
// which this TU now defines, but it also needs GraphicWin::update(RECT *,
// GraphicWin *), which is neither declared nor defined anywhere in the tree -
// see the HANDOFF in the report. So the out-of-line forwarder stays until that
// declaration lands. The auto_inline(off) is load-bearing and being defined
// after the caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the
// end of the translation unit and would fold the forwarder back into
// on_right_click, losing the 20-byte BYTE_EXACT match.
#pragma auto_inline(off)
void InfoWin::right_menu(int a1, int a2) {
    (ORIGINAL(this)->*InfoWinOriginalRightMenu)(a1, a2);
}
#pragma auto_inline(on)


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00459500 ??0InfoWin@@QAE@XZ 0x00459500-0x00459556;0x00655260-0x00655280 BYTE_EXACT
// size      118 bytes
// prototype void (__thiscall ??0InfoWin@@QAE@XZ)(InfoWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006161D0 0x00618EA0 0x0062BF20
*/

// Time, Font and PushButton are owned members. Their adjusted receivers
// (+0x30, +0x58, +0x9D0) are explicit in the original construction sequence.
// Ordinary member initialization supplies construction and unwind cleanup.
InfoWin::InfoWin() {
}

/*
Purpose: Tear down an InfoWin: destroy the PushButton, Font and Time
         subobjects in reverse construction order.
// ORIGINAL: 0x004594A0 ??1InfoWin@@QAE@XZ 0x004594A0-0x004594FB;0x00655240-0x00655260 BYTE_EXACT
// size      123 bytes
// prototype void (__thiscall ??1InfoWin@@QAE@XZ)(InfoWin* this)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616200 0x00618EE0 0x0062C010
Return Value: n/a
Status: Complete
*/
InfoWin::~InfoWin() {
}

// ORIGINAL: 0x00459420 ?UNK5@InfoWin@@QAEXHHHHH@Z 0x00459420-0x0045949B;0x00655295-0x006552BA FILE
// working copy - scaffold materialised by --work
// size      160 bytes
// prototype void (__thiscall ?UNK5@InfoWin@@QAEXHHHHH@Z)(InfoWin* this, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00618EA0 0x00618EE0
// UNK5 IS EVIDENCE-FREE AS A NAME. Searched 2026-09-05: the catalogue records
// callers 0 and call targets 0 for 0x00459420, so nothing names it from a call
// site; the mangled name is `QAE`, so it is not virtual and no vtable slot
// names it either; docs/recovery/behaviour-member-names.csv has no InfoWin
// entry; and the body is a stub - it builds three `Font`s, tears them down in
// reverse order, and returns through `ret 0x14` with all five arguments
// unread. A name invented to clear the census would be a false statement that
// outlives whoever wrote it.
// The five arguments stay unnamed on purpose: the body never reads one, so
// there is no evidence for any of them, and an unnamed parameter is exactly
// what the tree already writes for the same situation (BaseWin::UNK5(int),
// BaseWin::on_button_toggled(int, int)).
// MISMATCH 17 of 30: the image emits one more EH state store
// (`mov byte ptr [ebp - 4], 1` twice, not once) around the third Font than
// VC6 will produce for three flat locals in one scope. Not closed; the shape
// - three constructed then destroyed in reverse order - is the image's.
void InfoWin::UNK5(int, int, int, int, int) {
    Font font_a;
    Font font_b;
    Font font_c;
}

// ORIGINAL: 0x0044CB30 ?help_any@@YAXXZ 0x0044CB30-0x0044CB3F BYTE_EXACT
// size      15 bytes
// prototype 
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00429180
// 0x0044CB30  ?help_any@@YAXXZ  ->  ?help_any@@YAXXZ
//
// HOMING NOTE: the tree names the singleton, so the body spells it the way
// datalink.h already declares it - `DatalinkMain`, the object at 0x00703EA0 -
// rather than binding the address again under an anonymous name.

void __cdecl help_any() {
    DatalinkMain.exec(0, 0);
}
