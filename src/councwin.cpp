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
#include "councwin.h"
#include "mapwin.h"   // MainInterfaceGlobal - the interface owner on_status drives
#include "subinterface.h"
#include "temp.h"     // do_all_draws
#include "worldwin.h"
#include "menu.h"
#include "alpha.h"       // label_get and the source-owned council label table
#include "base.h"        // GovernorFaction (the elected Planetary Governor)
#include "faction.h"     // Players, PlayersData, and LockEnableMask
#include "game.h"        // IsMultiplayerNet and string_upper_slot
#include "general.h"     // GenderDefault and PluralityDefault
#include "infowin.h"     // InfoFactionShadowColors
#include "strings.h"     // StringTemp
#include "console.h"     // ConsoleGlobal's local faction id
#include "netmsg.h"      // NetMsgPopup

// The CouncWin methods below used to be measured only in isolated seed
// scaffolds. These bodies keep their receiver as the declared CouncWin and
// express the embedded windows, fonts, Spot, and local faction by owner.

// ORIGINAL: 0x004271C0 ?arrived@CouncWin@@QAEXH@Z 0x004271C0-0x0042722D
// RECOVERY_BINDING: v1 {"evidence": ["src/councwin.h CouncWin::field_A44_; src/temp.h TurnRedrawPending; src/faction.h LockEnableMask; src/graphicwin.h GraphicWin::redraw; src/netmsg.h NetMsgPopup; image data at 0x00683228 identifies the label VOTINGOPEN"], "reason": "Track faction arrival on the CouncWin receiver, redraw through GraphicWin, and signal the voting-open label when all active humans have arrived."}
void CouncWin::arrived(int faction_id) {
    field_A44_ |= (1 << faction_id);
    if (TurnRedrawPending != 0) {
        static_cast<GraphicWin *>(this)->redraw();
        const uint32_t status = LockEnableMask;
        const uint8_t humans = static_cast<uint8_t>(status);
        const uint8_t alive = static_cast<uint8_t>(status >> 8);
        if (((alive & humans) & field_A44_) == humans) {
            NetMsgPopup->pop("VOTINGOPEN", 1000, 1, 0);
        }
    }
}

// ORIGINAL: 0x00427160 ?close@CouncWin@@QAEXXZ 0x00427160-0x004271BE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/councwin.h CouncWin::buttons_[6] and font1_..font5_; src/flatbutton.h FlatButton::close; src/font.h Font::close"], "reason": "Close CouncWin embedded buttons and owned fonts through their typed members."}
void CouncWin::close() {
    for (int i = 0; i < 6; ++i) {
        buttons_[i].close();
    }
    font1_.close();
    font2_.close();
    font3_.close();
    font4_.close();
    font5_.close();
}

// ORIGINAL: 0x00426EE0 ?on_mouse_move@CouncWin@@QAEXHH@Z 0x00426EE0-0x00426F78 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/councwin.h CouncWin::spot_, field_A34_, buttons_[6], draw_leader; src/console.h ConsoleGlobal.faction_id_; git:1b3ed0614446d5283fe4cbd2392cbb64b49db601:src/unrecovered/00426ee0.cpp"], "reason": "Update the selected Council leader from a typed Spot hit test and the six-button/font owner state."}
void CouncWin::on_mouse_move(int x, int y) {
    const int check_result = spot_.check(x, y, &y, 0);
    if (check_result == -1) {
        const int leader = static_cast<int>(field_A34_);
        if (leader != -1) {
            field_A34_ = static_cast<uint32_t>(-1);
            draw_leader(leader);
        }
        return;
    }
    if (buttons_[5].is_visible()) {
        return;
    }
    if (y == ConsoleGlobal.faction_id_) {
        return;
    }
    const int current = static_cast<int>(field_A34_);
    if (current == y) {
        return;
    }
    if (current != -1) {
        field_A34_ = static_cast<uint32_t>(-1);
        draw_leader(current);
    }
    field_A34_ = static_cast<uint32_t>(y);
    draw_leader(y);
}

// Two of on_status's callees are still original bodies the tree has not homed
// (0x0045D440 / 0x0045D970, both archive copies in src/recovered/units, which
// the build does not compile). They are cdecl free functions the image reaches
// with a plain `call rel32`, so they are declared here and defined as
// forwarders at the foot of this file.
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

// The two FlatButton slots on_status drives are Win's own show(int) and
// hide() - FlatButton is BaseButton : GraphicWin : Win, so its vtable opens
// with Win's slots, and `button->show(0)` / `button->hide()` are the image's
// `call dword ptr [edx + 4]` and `[edx + 8]`.

// Slot 12 of the front vtable (0x66AAE8) - the catalogue's
// ?on_redraw@CouncWin@@QAEXXZ, 0x00426AA0, unrecovered. on_status reaches it
// with `mov edx,[ecx-0xa14] / call [edx+0x30]`, a live dispatch through the
// front's own table, so the entry has to be CouncWin's.
void CouncWin::on_redraw() {
    typedef void(__fastcall *pending)(CouncWin *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00426AA0))(
        this, nullptr);
}

// Slot 44 of the front vtable - the catalogue's
// ?on_button_clicked@CouncWin@@QAEXH@Z, 0x00424870, 1983 bytes, unrecovered.
void CouncWin::on_button_clicked(int button_id) {
    typedef void(__fastcall *pending)(CouncWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00424870))(
        this, nullptr, button_id);
}

/*
// ORIGINAL: 0x004270F0 ?on_iface_button_clicked@CouncWin@@QAEXH@Z 0x004270F0-0x0042710D BYTE_EXACT
// symbol    ?on_iface_button_clicked@CouncWin@@UAEXH@Z
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@CouncWin@@QAEXH@Z)(CouncWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00427103
Return Value: n/a
Status: Complete
*/
// LEVER: the receiver is the ENTRY (the image's `add ecx,-0xa14` is VC6's own
//   adjustment to the front) and the dispatch is the front's slot 44, so the
//   whole body is one unqualified virtual call once the front is named: the
//   `Win *front = this` conversion is where the -0xa14 comes from, and
//   `on_button_clicked(a1)` is the image's `call dword ptr [eax + 0xb0]`.
void CouncWin::on_iface_button_clicked(int button_id) {
    on_button_clicked(button_id);
}

/*
// ORIGINAL: 0x00426FA0 ?on_status@CouncWin@@QAEXH@Z 0x00426FA0-0x00427054 BYTE_EXACT
// symbol    ?on_status@CouncWin@@UAEXH@Z
// size      180 bytes
// prototype void (__thiscall ?on_status@CouncWin@@QAEXH@Z)(CouncWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D56B0 0x005FCBB0 0x0045D440 0x005ED9D0 0x0045D380 0x0045D970 0x005EDCD0
// indirect  0x00426FBD 0x00426FD3 0x0042701D
Return Value: n/a
Status: Complete
*/
// LEVER: the receiver is the ENTRY and every front reach goes through VC6's
//   own -0xA14, so `this` spelled at each use is the whole receiver story -
//   there is NO `front` local: naming one costs a spill slot and the image
//   recomputes `lea reg,[edi-0xa14]` at every site. The two button loops are
//   Win::show(0) / Win::hide() on the council buttons - FlatButton is
//   BaseButton : GraphicWin : Win, so slots 1 and 2 of its vtable ARE show and
//   hide, and the plain virtual call is the image's `call [edx+4]` /
//   `call [edx+8]`. The front's own show and hide are called DIRECTLY
//   (`call 0x5ed9d0` / `call 0x5edcd0`), so both are qualified to suppress
//   the virtual dispatch.
void CouncWin::on_status(int mode) {
    if (mode != 0) {
        static_cast<GraphicWin *>(this)->on_redraw();
        for (int i = 0; i < 6; i++) {
            if (i != 5) {
                buttons_[i].show(0);
            }
        }
        MainInterfaceGlobal.update(0);
        do_all_draws();
        slide_show(this, 0);
        this->Win::show(3);
    } else {
        FlatButton *button = buttons_;
        int count = 6;
        do {
            button->hide();
            ++button;
            --count;
        } while (count != 0);
        SubInterface::release_iface_mode();
        slide_hide(this, 0);
        this->Win::hide();
        do_all_draws();
    }
}

// The two slide helpers, still original bodies. See the declarations at the
// top of the file.
void __cdecl slide_show(GraphicWin *window, int mode) {
    typedef void(__cdecl *pending)(GraphicWin *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045D440))(window,
                                                                     mode);
}

void __cdecl slide_hide(GraphicWin *window, int mode) {
    typedef void(__cdecl *pending)(GraphicWin *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0045D970))(window,
                                                                     mode);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00426EB0 ?on_group_clicked@CouncWin@@QAEXHH@Z 0x00426EB0-0x00426EB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_group_clicked@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_group_clicked(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00426F80 ?on_button_passover@CouncWin@@QAEXHH@Z 0x00426F80-0x00426F83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_passover@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_button_passover(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427060 ?on_iface_left_click@CouncWin@@QAEXHH@Z 0x00427060-0x00427063 BYTE_EXACT
// symbol    ?on_iface_left_click@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427070 ?on_iface_right_click@CouncWin@@QAEXHH@Z 0x00427070-0x00427073 BYTE_EXACT
// symbol    ?on_iface_right_click@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427080 ?on_iface_left_down@CouncWin@@QAEXHH@Z 0x00427080-0x00427083 BYTE_EXACT
// symbol    ?on_iface_left_down@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427090 ?on_iface_right_down@CouncWin@@QAEXHH@Z 0x00427090-0x00427093 BYTE_EXACT
// symbol    ?on_iface_right_down@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270A0 ?on_iface_mouse_move@CouncWin@@QAEXHH@Z 0x004270A0-0x004270A3 BYTE_EXACT
// symbol    ?on_iface_mouse_move@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270B0 ?on_iface_mouse_leave@CouncWin@@QAEXHH@Z 0x004270B0-0x004270B3 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270C0 ?on_iface_selected@CouncWin@@QAEXHH@Z 0x004270C0-0x004270C3 BYTE_EXACT
// symbol    ?on_iface_selected@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270D0 ?on_iface_left_double_click@CouncWin@@QAEXHH@Z 0x004270D0-0x004270D3 BYTE_EXACT
// symbol    ?on_iface_left_double_click@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270E0 ?on_iface_right_double_click@CouncWin@@QAEXHH@Z 0x004270E0-0x004270E3 BYTE_EXACT
// symbol    ?on_iface_right_double_click@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427110 ?on_iface_button_toggled@CouncWin@@QAEXHH@Z 0x00427110-0x00427113 BYTE_EXACT
// symbol    ?on_iface_button_toggled@CouncWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427120 ?on_iface_group_clicked@CouncWin@@QAEXHHH@Z 0x00427120-0x00427123 BYTE_EXACT
// symbol    ?on_iface_group_clicked@CouncWin@@UAEXHHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@CouncWin@@QAEXHHH@Z)(CouncWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_group_clicked(int, int, int) {
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00426F90 ?on_button_text_draw@CouncWin@@QAEHPAUGraphicWin@@PADH@Z 0x00426F90-0x00426F95 BYTE_EXACT
// symbol    ?on_button_text_draw@CouncWin@@QAEHPAVGraphicWin@@PADH@Z
// size      5 bytes
// prototype int (__thiscall ?on_button_text_draw@CouncWin@@QAEHPAUGraphicWin@@PADH@Z)(CouncWin* this, GraphicWin*, int8*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int CouncWin::on_button_text_draw(::GraphicWin *, char *, int) {
    return 0;
}


/*
// ORIGINAL: 0x00426EC0 ?on_mouse_leave@CouncWin@@QAEXHH@Z 0x00426EC0-0x00426EDE BYTE_EXACT
// symbol    ?on_mouse_leave@CouncWin@@UAEXHH@Z
// size      30 bytes
// prototype void (__thiscall ?on_mouse_leave@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00425DB0
Status: Complete
*/
void CouncWin::on_mouse_leave(int a1, int a2) {
    int v = field_A34_;
    if (v != -1) {
        field_A34_ = -1;
        draw_leader(v);
    }
}

/*
// ORIGINAL: 0x00425D90 ?draw_all_leaders@CouncWin@@QAEXXZ 0x00425D90-0x00425DAA BYTE_EXACT
// size      26 bytes
// prototype void (__thiscall ?draw_all_leaders@CouncWin@@QAEXXZ)(CouncWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00425DB0
Status: Complete
*/
void CouncWin::draw_all_leaders() {
    for (int i = 1; i < 8; i++) {
        draw_leader(i);
    }
}

/*
// ORIGINAL: 0x00427130 ?show@CouncWin@@QAEHH@Z 0x00427130-0x0042714B BYTE_EXACT
// symbol    ?show@CouncWin@@UAEXH@Z
// size      27 bytes
// prototype int (__thiscall ?show@CouncWin@@QAEHH@Z)(CouncWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void CouncWin::show(int visible) {
    // THE RETURN WAS A TRANSCRIPTION ARTIFACT. This body is byte-identical
    // to BaseWin::show at 0x0041E890 - the same nine instructions - and the
    // image sets no return value: eax merely holds what the last call left.
    // The catalogue's `?show@CouncWin@@QAEHH@Z` guessed `H` from that, and
    // declaring Win::show virtual is what made the two spellings collide.
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::set_iface_mode();
    }
}

// Recover the complete leader rendering path through CouncWin's paired
// sprite/rectangle members. The retained 0x00425DB0 seed supplies the
// case map and call evidence; source ownership and exactness are checked
// separately.
// ORIGINAL: 0x00425DB0 ?draw_leader@CouncWin@@QAEXH@Z 0x00425DB0-0x00426459
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00425db0.cpp"], "reason": "Home leader rendering in CouncWin with typed faction, portrait and label members"}
void CouncWin::draw_leader(int factionID) {
    if (factionID == 0 || (Players[factionID].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }

    int activeFactions = 0;
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if ((Players[faction].rule_flags & RFLAG_ALIEN) == 0) {
            ++activeFactions;
        }
    }
    const int rankBase = (7 - activeFactions) / 2;

    int precedingFactions = 0;
    for (int precedingFaction = 1; precedingFaction < factionID; ++precedingFaction) {
        if ((Players[precedingFaction].rule_flags & RFLAG_ALIEN) == 0) {
            ++precedingFactions;
        }
    }

    int rank = rankBase + precedingFactions + 1;
    if (GovernorFaction == factionID) {
        rank = 4;
    } else if (GovernorFaction >= 0 && rank == 4) {
        int governorPreceding = 0;
        for (int governorFaction = 1; governorFaction < GovernorFaction; ++governorFaction) {
            if ((Players[governorFaction].rule_flags & RFLAG_ALIEN) == 0) {
                ++governorPreceding;
            }
        }
        rank = rankBase + governorPreceding + 1;
    }

    const int layout = GovernorFaction < 0 ? 1 : 0;
    RECT rect = leader_rects_[layout + rank * 2];
    spot_.kill_type(rank);
    spot_.add(factionID, rank, rect.left, rect.top,
              rect.right - rect.left, rect.bottom - rect.top);

    // GraphicWin contains Buffer as its second base, so this cast expresses
    // the actual subobject rather than the seed's `this + 0x444` arithmetic.
    Buffer *buffer = &static_cast<Buffer &>(*this);
    buffer->box_sprite(&rect, &InterfaceBoxStyles[0]);
    ++rect.left;
    --rect.right;
    ++rect.top;
    --rect.bottom;

    const int selectedVariant = GovernorFaction == factionID ? 1 : 0;
    Sprite *backing = leader_backing_[factionID][selectedVariant];
    backing->draw(buffer, backing->transparent_index(), rect.left, rect.top, 1, 1);

    const int centerX = (rect.left + rect.right) / 2;
    int textY = backing->height() + rect.top + 4;
    buffer->set_text_color(0xA0, -1, 1, 1);
    buffer->set_font(&font3_, 0, 0, 0);

    GenderDefault = Players[factionID].is_leader_female;
    PluralityDefault = 0;
    StringTemp[0] = '\0';
    strcat(StringTemp, Players[factionID].title_leader);
    buffer->write_cent_l(StringTemp, centerX, textY, static_cast<int>(strlen(StringTemp)));

    textY += font3_.line_advance();
    StringTemp[0] = '\0';
    GenderDefault = Players[factionID].is_leader_female;
    PluralityDefault = 0;
    strcat(StringTemp, Players[factionID].name_leader);
    buffer->write_cent_l(StringTemp, centerX, textY, static_cast<int>(strlen(StringTemp)));

    RECT badgeRect = rect;
    const int portraitY = textY + font3_.line_advance() + 10;
    badgeRect.top = portraitY;
    const int portraitVariant = field_A34_ != factionID ? 1 : 0;
    Sprite *portrait = leader_portraits_[factionID][portraitVariant];
    Sprite *portraitSize = leader_portraits_[factionID][0];
    const int portraitX = rect.left + (rect.right - portraitSize->width() - rect.left) / 2;
    portrait->draw(buffer, portrait->transparent_index(), portraitX, portraitY, 1, 1);

    const int labelKind = static_cast<int>(leader_label_kind_[factionID]);
    if (labelKind != 0) {
        bool showLabel = true;
        if (IsMultiplayerNet != 0) {
            const uint8_t humans = static_cast<uint8_t>(LockEnableMask & 0xFF);
            const uint8_t alive = static_cast<uint8_t>((LockEnableMask >> 8) & 0xFF);
            showLabel = (field_A44_ & alive & humans) == humans;
        }
        if (showLabel) {
            buffer->set_text_color(0x41, -1, 1, 1);
            StringTemp[0] = '\0';
            switch (labelKind) {
            case -5:
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[31]);
                strcat(StringTemp, label_get(0x7BC / 4));
                break;
            case -4:
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[30]);
                strcat(StringTemp, label_get(0x7C0 / 4));
                break;
            case -3:
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[32]);
                strcat(StringTemp, label_get(0x7C4 / 4));
                break;
            case -2:
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[32]);
                strcat(StringTemp, label_get(0x7D4 / 4));
                break;
            case -1:
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[32]);
                strcat(StringTemp, label_get(0x7D8 / 4));
                break;
            default: {
                buffer->box_sprite(&badgeRect, &InterfaceBoxStyles[0]);
                const int textColor = labelKind < 0
                    ? InfoFactionShadowColors[labelKind + 8]
                    : ChatItemIdTable[labelKind];
                buffer->set_text_color(textColor, -1, 1, 1);
                GenderDefault = Players[factionID].is_leader_female;
                PluralityDefault = 0;
                strcat(StringTemp, Players[factionID].name_leader);
                break;
            }
            }
            string_upper_slot()(StringTemp);
            buffer->set_font(&font4_, 0, 0, 0);
            buffer->write_cent_l(StringTemp, &badgeRect,
                                 static_cast<int>(strlen(StringTemp)));
        }
    }

    if (field_A1C_ <= 1) {
        const int techId = leader_tech_[factionID];
        const uint8_t alive = static_cast<uint8_t>((LockEnableMask >> 8) & 0xFF);
        if (techId != 0 && (alive & (1 << (factionID & 0x1F))) != 0 &&
            PlayersData[factionID].major_atrocities == 0) {
            RECT techRect = badgeRect;
            techRect.left -= 0x14;
            techRect.top -= 0x14;
            char number[32];
            StringTemp[0] = '\0';
            _itoa(techId, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get((techId == 1 ? 0x808 : 0x80C) / 4));
            string_upper_slot()(StringTemp);
            buffer->set_text_color(0xA0, -1, 1, 1);
            buffer->write_cent_l(StringTemp, &techRect,
                                 static_cast<int>(strlen(StringTemp)));
        }
    }

    soft_update(&rect);
}

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
// ORIGINAL: 0x00428550 ??1CouncWin@@QAE@XZ 0x00428550-0x00428614;0x00652C40-0x00652CB4 BYTE_EXACT
// symbol    ??1CouncWin@@UAE@XZ
// size      312 bytes
// prototype void (__thiscall ??1CouncWin@@QAE@XZ)(CouncWin* this)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00618EE0 0x006456E4
The catalogued QAE is a reconstruction over an unnamed image body
(sub_428550); Win's own vtable slot 0 is ??_GWin, a virtual destructor's
emission, so the inherited chain - GraphicWin, CouncWin - is virtual too,
and this build emits the U spelling.
*/
// The image's destructor is the compiler's own member-teardown chain: `??_M`
// over `buttons_[6]` (0xB4C elements, `??1FlatButton@@QAE@XZ` at 0x00406880
// as the element deleter), the six Fonts in reverse, `~Spot`, then the
// GraphicWin base - the implicit epilogue VC6 emits for the members
// councwin.h now declares, with the SEH frame (the funclet span at
// 0x00652C40) the non-trivial destructors pull in. The empty body below
// reproduces ALL of that - except two instructions the tree adds and the
// image does not have:
// RESOLVED 2026-09-06, and the open question above was the wrong one: the
//        inheritance edge is fine. The vtable stores were the USER-WRITTEN
//        destructor's. Probing cl 12.00.8168 (/O2 /Gy /GR- /GX) with a
//        polymorphic base, a member with a non-trivial destructor and an
//        ARRAY member: a user-written `~D() {}` emits the vptr store, the
//        IMPLICITLY declared destructor emits none and is otherwise the same
//        chain (`add ecx,N / jmp ~Member` for BattleWin; the full ??_M walk
//        here). Dropping the declaration from councwin.h is the whole fix -
//        deleting_thunks.cpp's explicit `CouncWin::~CouncWin()` still names
//        it, because an implicitly declared destructor may be called
//        explicitly. There is no body to write: the definition below is gone
//        and this marker records the compiler's.

/*
// ORIGINAL: 0x00428620 ??0CouncWin@@QAE@XZ 0x00428620-0x00428706;0x00652CC0-0x00652D34 BYTE_EXACT
// size      231 bytes
// prototype void (__thiscall ??0CouncWin@@QAE@XZ)(CouncWin* this)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
Return Value: n/a
Status: Complete
*/
// THE WHOLE BODY IS THE COMPILER'S. Every line the archive transcription
// needed by hand - `new (self + 0xa48) Spot()`, the six Fonts, the
// `??_M` array walk over buttons_[6], the three vftable stores at 0, 0x444
// and 0xA14 - is what VC6 emits for an empty constructor over the members
// councwin.h declares: bases and members in declaration order, the interface
// base's all-pure table (0x66A6E4) first and CouncWin's own (0x66AA94) last.
CouncWin::CouncWin() {
}

// Original interface table 0x0066AA94 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@CouncWin@@UAEXHH@Z 0x00426FA0
void CouncWin::on_iface_scrolled(int, int) {}
// Original interface table 0x0066AA94 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@CouncWin@@UAEXHH@Z 0x00426FA0
void CouncWin::on_iface_scrolling(int, int) {}
// Original interface table 0x0066AA94 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@CouncWin@@UAEXHH@Z 0x00426FA0
void CouncWin::on_iface_left_up(int, int) {}

// Original interface table 0x0066AA94 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@CouncWin@@UAEHH@Z 0x00426FA0
int CouncWin::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066AA94 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@CouncWin@@UAEHPAVGraphicWin@@@Z 0x00426FA0
int CouncWin::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066AA94 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@CouncWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x00426FA0
int CouncWin::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}
