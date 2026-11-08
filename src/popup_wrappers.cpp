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
// The popup-wrapper TU (0x0048C060-0x0048D4EA): parse_it and the popp,
// popt, popv and popb wrappers, the vehicle sprite callbacks and veh_pick.
// The boundary is inferred from the image; the original file name is
// unknown.
#include "stdafx.h"
#include "popup.h"
#include "xpops.h"
#include "general.h"
#include "strings.h"
#include "text.h"
#include "game.h"
#include "faction.h"
#include "console.h"
#include <cstring>
#include "wave.h"
#include "fx.h"
#include "alpha.h"
#include "veh.h"
#include "map.h"
#include "mapwin.h"
#include "spritebox.h"
#include "sprite.h"
#include "buffer.h"
#include "netmsg.h"  // NetMsgPrimary - the sprite callbacks pop INJAIL
#include "netdaemon.h"  // NetDaemonNet - the sprite callbacks lock and synch


// Read a script section into the shared substitution buffer. The text
// reader owns TextBufferGetPtr; parse_string supplies the substitutions.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C060 ?parse_it@@YAXPADPBD@Z 0x0048C060-0x0048C099 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned48C060..48C099 clears StringTemp9B86A0, calls X_text_open5BECA0, then text_get5FD570 and parse_string625880(TextBufferGetPtr9B7D00,StringTemp) on success.", "src/recovered/units/0048c060.cpp retains this same original identity, complete57-byte span and historical BYTE_EXACT claim; Quayle redraw4956D0 passes Holobook filename and BOOKTITLE/BOOKINTRO/BOOKLIST section names."], "reason": "Recover the filename/section wrapper using the real shared text and substitution owners; the existing seed retains its historical matching obligation."}
void __cdecl parse_it(char *file_name, const char *section) {
    StringTemp[0] = 0;
    if (X_text_open(file_name, section) == 0) {
        text_get();
        parse_string(TextBufferGetPtr, StringTemp);
    }
}

// ORIGINAL: 0x0048C0A0 ?popp@@YAHPADPBDHPBDP6AHXZ@Z 0x0048C0A0-0x0048C31F;0x006582E0-0x00658337
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0048c0a0.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original popup picture helper belongs to the Popup owner and uses real local Popup/member lifetimes, with integer start status controlling alternate-file retry."}
// TRIED: a real automatic-storage `Popup popup;` local - BasePop already declares a non-trivial `~BasePop()`, so the compiler is forced to emit ITS OWN scope-exit destructor chain the moment such a local exists, which fights the explicit, interleaved close()/dtor/vtable-reset sequence the original shows (SEH funclet table at 0x6582E0, matched only by span - the byte comparison covers 0x48C0A0-0x48C31F only). Landed instead: a raw `unsigned char popupStorage[0x5390]` plus placement `new` to call the real constructor explicitly, then hand-written calls mirroring the disasm's own teardown order (Popup::close, Scroll::close, FlatButton::close, then explicit qualified destructors BaseButton::~BaseButton/~FlatButton/GraphicWin::~GraphicWin/ BasePop::~BasePop) and the two vtable-reset writes per stage (offset 0 and offset 0x444 - a second vtable slot common to every GraphicWin-derived sub-object here). [ebp-0x3278] traced to BasePop's own `sprite_` member via its declared offset (0x2118) plus Sprite's real size (0x2C, not the field list's apparent 13*4). Reaches MISMATCH, 100/181 mnemonics shared (0.53) over the 0x48C0A0-0x48C31F span, first divergence at #3 (no real SEH prologue, expected from the traded-off approach).
// symbol    ?popp@@YAHPBD0H0P6AHXZ@Z
int __cdecl popp(LPCSTR caption, LPCSTR label, int flags, LPCSTR picture,
                 int (__cdecl *callback)()) {
    Popup popup;
    char *alternative = nullptr;
    if (strcmp(caption, ScriptPopupFile) == 0
        || strcmp(caption, "SCRIPT.txt") == 0) {
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            if (ExpansionEnabled) {
                FILE *file = fopen("alienIscript.txt", "rb");
                if (file) {
                    fclose(file);
                    alternative = PopupStartAltFile1;
                }
            }
        } else if (ExpansionEnabled && PopupDialogFactionID > 0
                   && PopupDialogFactionID < 8
                   && (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN)) {
            FILE *file = fopen("alienuscript.txt", "rb");
            if (file) {
                fclose(file);
                alternative = PopupStartAltFile2;
            }
        }
    }
    if (!alternative || popup.start(alternative, label, -1, nullptr,
                                    flags, nullptr) != 0) {
        popup.start(const_cast<char *>(caption), label, -1, nullptr,
                    flags, nullptr);
    }
    if (picture) {
        int width, height;
        if (Buffer::get_pcx_dimensions(picture, &width, &height) == 0
            && popup.owned_picture().init(const_cast<char *>(picture), width,
                                          height) == 0) {
            popup.select_owned_picture();
        }
    }
    const int result = popup.exec(0, callback);
    ConsoleGroupAction = popup.input_poll_state();
    return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C320 ?popt@@YAHPADPBDHPADH@Z 0x0048C320-0x0048C353 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0048c320.cpp", "image:terranx_original.exe:0x0048C320"], "reason": "Recover tutorial popup configuration around the existing X_pop builder, restoring normal configuration after capturing the int result. Original call order establishes ownership and corrects a stale pointer-return declaration."}
// symbol    ?popt@@YAHPADPBDH0H@Z
int __cdecl popt(char *caption, const char *label, int value, char *text, int flags) {
    popups_tutorial();
    const int result = X_pop(caption, label, value, text, flags, nullptr);
    popups_normal();
    return result;
}

// ORIGINAL: 0x0048C360 ?popv@@YAHPADPBDHH@Z 0x0048C360-0x0048C648;0x00658337-0x00658411
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0048c360.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original vehicle popup owns real Popup and Sprite locals; original48C3D0..48C400 selects caller picture when the SpriteBox list is empty but always passes the local Sprite to its embedded SpriteBox."}
// TRIED: Popup popup; Sprite sprite; as real locals (LEVER 1) reproduce the SEH frame/alloca/destructor cascade shape; MISMATCH #14 remains because the ~20 close()/dtor calls after popup.start() are done via raw ebp-relative reinterpret_cast rather than named embedded members (BasePop's dialogs_ region isn't typed granularly enough here to let the calls fall out of automatic C++ destruction).
int __cdecl popv(char *caption, const char *label, int vehicle, int flags) {
    Popup popup;
    Sprite sprite;
    sprite.create_blank(100, 75, WinFillColour);
    popup.start(caption, label, -1, nullptr, flags, nullptr);
    popup.set_caller_picture(popup.picture_enabled() ? nullptr : &sprite);
    popup.set_redraw_callback(veh_sprite_draw);
    popup.dialogs().SpriteBox::sprite(&sprite, nullptr, vehicle);
    const int result = popup.exec(0, nullptr);
    ConsoleGroupAction = popup.input_poll_state();
    return result;
}

// Superseded seed removed; retained findings below are historical evidence.
// ORIGINAL: 0x0048C650 ?popb@@YAHPBDHHPBDPAUSprite@@@Z 0x0048C650-0x0048CEA2;0x00658411-0x006585CB FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:67e9243d4c82aaa8b046653cd25ad7281d0e1bef:src/unrecovered/0048c650.cpp", "pinned:terranx_original:0x48c650", "source:src/popup.cpp"], "reason": "Recover complete popup operation with native early gates, warning/focus/result effects and genuine owned Sprite/Dialogs members, never duplicate locals. Preserve complete spans, original identity and historical obligations; scoped compiler/host observations are not game acceptance."}
// symbol    ?popb@@YAHPBDHH0PAVSprite@@@Z
// TRIED: no Ghidra hypothesis was available for this one (brief carried none). This function registers a real SEH frame (mov fs:[0],esp) around a huge local Popup (which embeds Scroll/2xFlatButton/Sprite/Heap per the scaffold's own already- reverse-engineered layout) plus several more locals (Dialogs embedding Dialog+GraphicWin, a second Sprite, SpriteBox) - the destructor cascades visible in the disasm are the INLINED expansion of each object's own close()/dtor. A plain `Popup popup_;` local compiled but left `??1Popup@@QAE@XZ` as a SECOND external .text symbol (the compiler declined to inline it here, unlike the proven pattern from other landings) - fixed by placement-new into a raw byte buffer with explicit `popup_->close(); popup_->~Popup();` at every exit instead of relying on scope-exit RAII. Also needed `struct Sprite` (not `class`) and a `static` fix on `Buffer::get_pcx_dimensions` to match the catalogued mangled name (PAU not PAV; QAA not QAE). The early-return bailout conditions (3 checks on 9a64c0/9a64e8/939284/93a95c) and the mid-function dialog-population logic are a good-faith reading of the raw disasm, not independently re-verified argument- by-argument the way the other 7 addresses were - lowest-confidence landing in this batch. Compiles; not chased to byte-exact given size (2572 bytes) and time.
int __cdecl popb(LPCSTR text_label, int warning_mask, int sound_id,
                 LPCSTR picture_file, Sprite *caller_picture) {
    Popup popup;
    const int faction = BaseCurrent->faction_id_current;
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction);

    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
        return 0;
    }
    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) != 0 && (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) == 0) {
        return 0;
    }
    if (faction != ConsoleGlobal.faction_id_
        && CurrentFactionEnergyReserves == 0) {
        return 0;
    }

    char popup_text[0x114];
    strcpy(popup_text, text_label);
    parse_says(0, BaseCurrent->name_string, -1, -1);

    if (sound_id >= 0) {
        g_FX.play(sound_id);
    } else if (IsMultiplayerNet != 0) {
        g_FX.play(0x21);
    }

    popup.start(PopupStartCaption, popup_text, -1, 0, 0, 0);

    popup.set_auto_hide_value(0x3EA);
    const bool warning_enabled = warning_mask != 0
        && (GameDataStateStorage.fields_.game_warnings_ & static_cast<uint32_t>(warning_mask)) != 0
        && CurrentFactionEnergyReserves == 0;
    if (!warning_enabled) {
        popup.set_auto_hide_value(2);
    } else {
        bool focus_allowed = faction == ConsoleGlobal.faction_id_
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
        if (!focus_allowed) {
            const Base &selected = Bases[BaseIDCurrentSelected];
            const uint8_t selected_owner = selected.faction_id_current;
            const uint8_t viewer_bit = static_cast<uint8_t>(
                1u << ConsoleGlobal.faction_id_);
            focus_allowed = selected_owner == ConsoleGlobal.faction_id_
                || (selected.visibility & viewer_bit) != 0;
        }
        if (focus_allowed
            && (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP) == 0) {
            ConsoleGlobal.focus(BaseCurrent->x, BaseCurrent->y, faction);
        }
    }

    int picture_width;
    int picture_height;
    if (picture_file != 0
        && Buffer::get_pcx_dimensions(picture_file, &picture_width,
                                      &picture_height) == 0
        && popup.owned_picture().init(const_cast<char *>(picture_file),
                                      picture_width, picture_height) == 0) {
        popup.set_caller_picture(&popup.owned_picture());
    }
    if (BaseProductionZoomRequested == 0
        && faction == ConsoleGlobal.faction_id_
        && IsMultiplayerNet == 0
        && CurrentFactionEnergyReserves == 0) {
        popup.dialogs().item(label_get(0xB0 / 4), 0);
        popup.dialogs().item(label_get(0xB4 / 4), 1);

        if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) != 0 && BaseCurrent->population_size > 1) {
            int facility_offset;
            int facility_mask;
            bitmask(0x23, &facility_offset, &facility_mask);
            const bool has_psi_facility =
                (Bases[BaseIDCurrentSelected].facilities_built[facility_offset]
                 & static_cast<uint8_t>(facility_mask)) != 0;
            const int specialist_flag = static_cast<int>(
                PlayersData[faction].flags) + (has_psi_facility ? 2 : 0);
            if (specialist_flag >= 0) {
                popup.dialogs().item(label_get(0x7AC / 4), 3);
            }
        }
    }

    if (warning_mask != 0) {
        popup.button(label_get(0x6EC / 4));
    }
    popup.set_value(0);
    if (caller_picture != 0) {
        popup.configure_sprite_dialog(caller_picture);
    }

    const int popup_result = popup.exec(0, 0);
    int result = popup_result;
    if (IsMultiplayerNet != 0 || CurrentFactionEnergyReserves != 0) {
        result = 1;
    }

    // Native tests BasePop::field_3100_, independently of exec's return.
    if (popup.input_poll_state() != 0) {
        GameDataStateStorage.fields_.game_warnings_ &= ~static_cast<uint32_t>(warning_mask);
        AlphaIniPrefs.announce = GameDataStateStorage.fields_.game_warnings_;
        prefs_save(0);
    }

    if (sound_id >= 0) {
        g_FX.fade(sound_id);
    }

    if (result == 0 && CurrentFactionEnergyReserves == 0) {
        BaseProductionZoomRequested = 1;
    } else if (result == 2) {
        BaseCurrent->state |= BSTATE_UNK_8000000000 | BSTATE_RESEARCH_DATA_STOLEN;
    } else if (result == 3) {
        action_staple(BaseIDCurrentSelected);
    }
    return result;
}

// ORIGINAL: 0x0048CEB0 ?veh_sprite_draw@@YAHPAUSprite@@PAUGraphicWin@@HHH@Z 0x0048CEB0-0x0048CEF2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0048ceb0.cpp: canonical original identity, complete spans and retained historical obligations", "source:src/basepop.h: real owned Dialogs, Sprite and draw callback contract", "source:src/popup.h: Popup lifetime and start interface"], "reason": "The original vehicle draw callback is installed by popv in BasePop draw callback storage3168; real Sprite and GraphicWin pointer formals, GraphicWin Buffer base and source-owned scale globals express its actual owner relationships."}
// symbol    ?veh_sprite_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl veh_sprite_draw(Sprite *, GraphicWin *owner, int vehicle,
                           int x, int y) {
    const int scale = (SpriteScaleNumerator << 4) / SpriteScaleDenominator - 16;
    veh_draw(owner ? static_cast<Buffer *>(owner) : nullptr,
                    vehicle, x, y, scale, 0, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048CF00 sub_48cf00 0x0048CF00-0x0048CF4A BYTE_EXACT
// symbol    ?sub_48cf00@@YAHPAVBasePop@@H@Z

// 0x0048CF00 is a cdecl operation with an explicit BasePop receiver, not a
// member call. Its old exact seed and complete span remain in the mapping.
// Retained seed facts: 74 bytes at 0x0048CF00..0x0048CF4A, zero mapped
// callers, one direct target 0x006108E0; SpriteBox is the real +0x2240
// subobject and the redraw callback slot is BasePop +0x3168.
int __cdecl sub_48cf00(BasePop *owner, int vehicle) {
    Sprite *const first_vehicle_sprite = &g_VEH_SPRITES[0];
    if (!owner->picture_enabled()) {
        owner->set_caller_picture(first_vehicle_sprite);
    } else {
        owner->set_caller_picture(nullptr);
    }
    const int result = owner->dialogs().SpriteBox::sprite(
        first_vehicle_sprite, nullptr, vehicle);
    owner->set_redraw_callback(veh_sprite_draw);
    return result;
}

/*
Purpose: The single-click handler for a fleet sprite: in a net game a
         selectable, idle, unbusy unit is simply taken; a jailed unit pops
         INJAIL; otherwise the click runs the net lock round-trip (lock,
         wake, synch, unlock) before returning.

// ORIGINAL: 0x0048CF50 ?veh_sprite_click@@YAHH@Z 0x0048CF50-0x0048D006 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["veh.h already declares the exact cdecl callback signature and console.cpp installs this function as the sprite click callback.","The body uses the existing Veh array, NetMsgPrimary, NetDaemonNet, and declared selection/wake/sync helpers; lock, wake, sync, await, and unlock order is preserved.","The literal at pinned 0x00686B5C is exactly `49 4e 4a 41 49 4c 00` (`INJAIL\\0`); replacing the fixed pointer alias with the source literal preserves the label bytes without an executable-address dependency.","The callback remains the existing cdecl function, not a guessed member or generated adapter; native symbol/reference measurement remains separate."],"reason":"Move the existing registered click callback into veh.cpp, replace only its verified INJAIL text pointer with the equivalent source literal, and preserve every return/call path."}
// size      182 bytes
// prototype int (__cdecl ?veh_sprite_click@@YAHH@Z)(int a1)
// callers   ?   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020 0x005310F0 0x00531480 0x00593220 0x005C1A20 0x005C1C40 0x005C1D70
Return Value: 1 when the click only selects the unit, else 0
Status: Coverage transcription - claim pending re-measurement
*/
int __cdecl veh_sprite_click(int veh_id) {
    if (IsMultiplayerNet != 0 && veh_selectable(veh_id) != 0
        && Vehs[veh_id].order == ORDER_NONE
        && (Vehs[veh_id].state & 0x8004200) == 0) {
        return 1;
    }
    if (veh_jail(veh_id) != 0) {
        NetMsgPrimary.pop("INJAIL", 5000, 0, 0);
        return 0;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) == 0) {
        veh_wake(veh_id);
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
    }
    return 0;
}

/*
Purpose: The double-click twin: the same net-select shortcut and INJAIL
         pop, but the lock failure path returns early and a completed
         lock round-trip reports the unit taken.

// ORIGINAL: 0x0048D010 ?veh_sprite_double_click@@YAHH@Z 0x0048D010-0x0048D0CD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["veh.h already declares the exact cdecl callback signature and console.cpp installs this function as the sprite double-click callback.","The body uses the existing Veh array, NetMsgPrimary, NetDaemonNet, and declared selection/wake/sync helpers; lock failure still returns before the wake/sync path.","The literal at pinned 0x00686B64 is exactly `49 4e 4a 41 49 4c 00` (`INJAIL\\0`); replacing the fixed pointer alias with the source literal preserves the label bytes without an executable-address dependency.","The callback remains the existing cdecl function, not a guessed member or generated adapter; native symbol/reference measurement remains separate."],"reason":"Move the existing registered double-click callback into veh.cpp, replace only its verified INJAIL text pointer with the equivalent source literal, and preserve its distinct lock-failure return."}
// size      189 bytes
// prototype int (__cdecl ?veh_sprite_double_click@@YAHH@Z)(int a1)
// callers   ?   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020 0x005310F0 0x00531480 0x00593220 0x005C1A20 0x005C1C40 0x005C1D70
Return Value: 1 when the double-click takes the unit, else 0
Status: Coverage transcription - claim pending re-measurement
*/
int __cdecl veh_sprite_double_click(int veh_id) {
    if (IsMultiplayerNet != 0 && veh_selectable(veh_id) != 0
        && Vehs[veh_id].order == ORDER_NONE
        && (Vehs[veh_id].state & 0x8004200) == 0) {
        return 1;
    }
    if (veh_jail(veh_id) != 0) {
        NetMsgPrimary.pop("INJAIL", 5000, 0, 0);
        return 0;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return 0;
    }
    veh_wake(veh_id);
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
    return 1;
}

// ORIGINAL: 0x0048D0D0 ?veh_pick@@YAHHPBDH@Z 0x0048D0D0-0x0048D4EA;0x006585CB-0x006586B3
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x48d0d0 hot and cold spans; private full-body native-order review and actual-source callback/lifetime controls.", "git:decce96da7981b60ac91563a8323362cdab2e250:src/unrecovered/0048d0d0.cpp", "source:src/basepop.h:typed SpriteBox click callback at BasePop+3160; native CodeView and producer/consumer evidence."], "reason": "Recover the complete picker/supply operation through genuine Popup, Sprite, Veh, Base and network owners; preserve full original spans and seed findings without a new matching claim."}
// TRIED: declaring BOTH `Popup();` (matched: this disassembly calls Popup's own ctor at 0x4048a0, not BasePop+Scroll separately) AND `~Popup();` (declaration only, no body) on the Popup class lets `Popup pop;` be a real local without the compiler synthesising-and-EMITTING its own ~Popup() - the destructor declaration alone suppresses synthesis, so scope-exit just calls the external symbol once. This compiles clean (unlike 0x00405140, which hit the one-external-symbol error from an implicit ~Popup()). The extensive widget teardown after the `if (a3 != 0)` branch (FlatButton/BaseButton/Dialogs/Dialog/ Spot/Heap) is called through small raw-byte-buffer casts rather than real locals, since those types have no declared constructors in this scaffold and a real local would risk the same implicit-synthesis problem. Reaches MISMATCH #11 ('push' vs 'lea'); did not chase the exact field-store sequence (callback pointers, vtable resets) further.
int __cdecl veh_pick(int veh_id, const char *label, int flags) {
    // Native constructs Popup first at 0x48D0FB, then Sprite at 0x48D108.
    Popup popup;
    Sprite sprite;
    sprite.create_blank(0x64, 0x4B, WinFillColour);
    const int popup_flags = (flags != 0 ? 0x40 : 0) | 0x800;
    popup.start(PopupStartCaption, label, -1, 0, popup_flags, 0);

    int listed_count = 0;
    int listed_vehicle = veh_id;
    if (listed_vehicle >= 0) {
        int previous_vehicle = Vehs[listed_vehicle].prev_veh_id_stack;
        while (previous_vehicle >= 0) {
            listed_vehicle = previous_vehicle;
            previous_vehicle = Vehs[listed_vehicle].prev_veh_id_stack;
        }

        if (listed_vehicle >= 0) {
            do {
                say_morale(listed_vehicle, 0);
                StringTemp[0] = '\0';
                ::strcat(StringTemp, " ");
                const int prototype = Vehs[listed_vehicle].proto_id;
                ::strcat(StringTemp, VehPrototypes[prototype].veh_name);
                popup.dialogs().SpriteBox::sprite(
                    &sprite, StringTemp, listed_vehicle);

                // Native fetches the next stack link after the sprite call.
                const int next_vehicle = Vehs[listed_vehicle].next_veh_id_stack;
                ++listed_count;
                listed_vehicle = next_vehicle;
            } while (listed_vehicle >= 0);
        }
    }

    if (listed_count == 0) {
        // Native empty path closes the independent Sprite before Popup.
        sprite.close();
        popup.close();
        return -1;
    }

    popup.set_redraw_callback(veh_sprite_draw);
    popup.dialogs().SpriteBox::set_redraw_callback(veh_sprite_draw);
    popup.set_sprite_click_callback(veh_sprite_click);
    popup.set_sprite_double_click_callback(veh_sprite_double_click);

    PopupModalState = 6;
    const int result = popup.exec(0, 0);
    PopupModalState = 0;

    // Native successful path closes Popup before the local Sprite.
    popup.close();
    sprite.close();
    return result;
}
