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
// ORIGINAL: 0x00428710 ??_GCouncWin@@UAEPAXI@Z 0x00428710-0x00428731 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00428710-0x00428731", "Pinned class constructor ??0CouncWin@@QAE@XZ at 0x00428620 stores original class/view table VA 0x66aae8 at 0x004286DE (mov dword ptr [esi], 0x66aae8); the current genuine owner vftable ??_7CouncWin@@6BWin@@@ selects generated ??_GCouncWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ECouncWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GCouncWin@@UAEPAXI@Z at 0x00428710 calls the source-mapped ??1CouncWin@@QAE@XZ at 0x00428550, then tests byte ptr [ebp + 8], 1; je 0x42872a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/councwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original CouncWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GCouncWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "councwin.h"
#include "mapwin.h"   // MainInterfaceGlobal - the interface owner on_status drives
#include "subinterface.h"
#include "win.h"
#include "worldwin.h"
#include "menu.h"
#include "alpha.h"       // label_get and the source-owned council label table
#include "base.h"        // GovernorFaction (the elected Planetary Governor)
#include "faction.h"     // Players, PlayersData, and LockEnableMask
#include "game.h"        // IsMultiplayerNet
#include "general.h"     // GenderDefault and PluralityDefault
#include "infowin.h"     // InfoFactionShadowColors
#include "council.h"
#include "technology.h"
#include "time.h"
#include "netdaemon.h"
#include "multiwin.h"
#include "popup.h"
#include "dialogs.h"
#include "xpops.h"

// Source-owned process Council object. The original init receiver establishes
// its identity, not a complete global bound or runtime-initializer proof.
// RECOVERY_OBJECT: v1 0x006FEC80 ?CouncilWindow@@3VCouncWin@@A
CouncWin CouncilWindow;
#include "strings.h"     // StringTemp
#include "console.h"     // ConsoleGlobal's local faction id
#include "netmsg.h"      // NetMsgPrimary

// ORIGINAL: 0x00425050 ?on_left_click@CouncWin@@QAEXHH@Z 0x00425050-0x00425B92;0x00652EC7-0x006530F4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00425050.cpp", "image:terranx_original.exe:0x00425050", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: `Popup popup;` RAII local with the council-vote business logic (Spot::check, treaty/pact gate checks, eligible-faction dialog population loop, popup.exec, network vs single-player branches with parse_says/message_data/NetMsg::pop/ buy_council_vote/diplomacy_ends/vtable slot62) converging on draw_leader+ popup.close(); the `0x4253ca` "reuse a2 as menu selection, skip the dialog" edge case was simplified to a direct jump to draw_leader (goto could not reach the shared merge point without crossing a block's initializations). Diverges at instr #10.
void CouncWin::on_left_click(int x, int y) {
    Popup popup;
    int clicked_faction = 0;
    int selection = 0;
    if (spot_.check(x, y, &clicked_faction, &selection) < 0) {
        popup.close();
        return;
    }

    const int local_faction = ConsoleGlobal.faction_id_;
    const int proposal = static_cast<int>(field_A1C_);
    if (clicked_faction == local_faction || field_A24_ != 0) {
        draw_leader(clicked_faction);
        popup.close();
        return;
    }

    const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
    const uint8_t clicked_bit = static_cast<uint8_t>(1u << clicked_faction);
    if ((clicked_bit & humans) != 0) {
        draw_leader(clicked_faction);
        popup.close();
        return;
    }
    if ((clicked_bit & alive) == 0) {
        // Native rejoins its shared post-selection path with Spot::check's
        // second output as the selected proposal option.
        goto vote_selection_ready;
    }

    if ((PlayersData[clicked_faction].diplo_treaties[local_faction] & 0x30) != 0) {
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[clicked_faction].title_leader, -1, -1);
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[clicked_faction].name_leader, -1, -1);
        X_pop("BUYNOTALK", pop_wait);
        popup.close();
        return;
    }

    if (PlayersData[clicked_faction].unk_29[proposal] != 0 &&
        PlayersData[clicked_faction].unk_30[proposal] == local_faction) {
        popup.close();
        return;
    }

    if (clicked_faction != 0) {
        selection = -1 - X_pop("WHICHVOTE", pop_wait);
        if (selection == 0) {
            draw_leader(clicked_faction);
            popup.close();
            return;
        }
    } else {
        popup.start(PopupStartCaption, "COUNCILVOTEGOV", -1, nullptr,
                    0x200000, nullptr);
        Dialogs &options = popup.dialogs();
        int current_selection = 0;
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            const uint8_t current_humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            if ((current_humans & (1u << faction)) == 0 || !eligible(faction)) {
                continue;
            }
            StringTemp[0] = '\0';
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(StringTemp, Players[faction].title_leader);
            strcat(StringTemp, " ");
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(StringTemp, Players[faction].name_leader);
            options.item(StringTemp, faction);
            if (faction == ConsoleGlobal.faction_id_) {
                current_selection = faction;
            }
        }
        (void)current_selection;
        selection = popup.exec(0, pop_wait);
        if (selection == 0x270f) {
            draw_leader(clicked_faction);
            popup.close();
            return;
        }
    }

vote_selection_ready:
    if (!IsMultiplayerNet) {
        if ((alive & clicked_bit) == 0) {
            GenderDefault = Players[clicked_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[clicked_faction].title_leader, -1, -1);
            GenderDefault = Players[clicked_faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[clicked_faction].name_leader, -1, -1);
            GenderDefault = Players[clicked_faction].noun_gender;
            PluralityDefault = Players[clicked_faction].is_noun_plural;
            parse_says(2, Players[clicked_faction].noun_faction, -1, -1);
            NetMsgPrimary.pop("ELIMINATED", 0x1388, 0, nullptr);
            popup.close();
            return;
        }
        // Native reloads the current faction after the vote-selection prompt
        // before entering the purchase path (0x4255AE).
        buy_council_vote(ConsoleGlobal.faction_id_, clicked_faction, proposal,
                         selection);
        redraw();
        draw_leader(clicked_faction);
        popup.close();
        return;
    }

    field_A40_ = 0;
    const uint8_t current_status = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    const uint8_t current_alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
    if ((field_A44_ & current_alive & current_status) != current_status) {
        NetMsgPrimary.pop("COUNCILVOTEGOV", 0x1388, 1, nullptr);
        field_A40_ = 1;
        draw_leader(clicked_faction);
        popup.close();
        return;
    }

    const uint8_t live_clicked = static_cast<uint8_t>(1u << clicked_faction);
    if ((live_clicked & current_status) != 0) {
        CommlinkPending = 1;
        CommlinkFaction = clicked_faction;
        // The modal/parser path may have changed the local faction. The image
        // reloads it at 0x4253D7 for this network message.
        message_data(0x1501, clicked_faction, clicked_faction,
                     ConsoleGlobal.faction_id_, 0, 0);
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[clicked_faction].title_leader, -1, -1);
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[clicked_faction].name_leader, -1, -1);
        parse_num(0, clicked_faction);
        NetMsgPrimary.pop("DIPLOCOMM", 0x1388, 0, nullptr);
        field_A40_ = 1;
        draw_leader(clicked_faction);
        popup.close();
        return;
    }
    if ((live_clicked & current_alive) == 0) {
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[clicked_faction].title_leader, -1, -1);
        GenderDefault = Players[clicked_faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[clicked_faction].name_leader, -1, -1);
        GenderDefault = Players[clicked_faction].noun_gender;
        PluralityDefault = Players[clicked_faction].is_noun_plural;
        parse_says(2, Players[clicked_faction].noun_faction, -1, -1);
        NetMsgPrimary.pop("ELIMINATED", 0x1388, 0, nullptr);
        field_A40_ = 1;
        popup.close();
        return;
    }

    if (NetDaemonNet.open_channel(ConsoleGlobal.faction_id_, clicked_faction) == 0) {
        // These are distinct native reloads: open_channel at 0x4255AE,
        // purchase at 0x4256EC, and diplomacy_ends at 0x4256FA. Each call can
        // run callbacks, so the value is intentionally not shared.
        buy_council_vote(ConsoleGlobal.faction_id_, clicked_faction, proposal,
                         selection);
        diplomacy_ends(ConsoleGlobal.faction_id_, clicked_faction);
        redraw();
        field_A40_ = 1;
        draw_leader(clicked_faction);
        popup.close();
        return;
    }

    GenderDefault = Players[clicked_faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[clicked_faction].title_leader, -1, -1);
    GenderDefault = Players[clicked_faction].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[clicked_faction].name_leader, -1, -1);
    NetMsgPrimary.pop("NOCHANNEL", 0x1388, 0, nullptr);
    field_A40_ = 1;
    popup.close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00424720 ?set_vote@CouncWin@@QAEXHH@Z 0x00424720-0x0042486C FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00424720.cpp", "image:terranx_original.exe:0x00424720", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
void CouncWin::set_vote(int faction_id, int vote) {
    if (field_A24_ != 0) {
        return;
    }

    leader_label_kind_[faction_id] = static_cast<uint32_t>(vote);

    const uint32_t faction_status = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
    const uint32_t human_mask = faction_status & 0xff;
    bool calculate_computer_votes = true;
    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        if ((human_mask & (1u << faction)) != 0 &&
            leader_label_kind_[faction] != 0) {
            calculate_computer_votes = false;
        }
    }

    if (calculate_computer_votes) {
        int rounds = 3;
        do {
            uint32_t live_status = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
            for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                const uint32_t bit = 1u << faction;
                if (((live_status >> 8) & bit) != 0 &&
                    (live_status & bit) == 0) {
                    leader_label_kind_[faction] = static_cast<uint32_t>(
                        council_get_vote(faction, static_cast<int>(field_A1C_),
                                         static_cast<int>(field_A20_)));
                    live_status = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
                }
            }
            --rounds;
        } while (rounds != 0);
    }

    static_cast<GraphicWin *>(this)->redraw();
    if (IsMultiplayerNet == 0) {
        make_resolution(1);
    } else if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0 &&
               make_resolution(0) != 0) {
        message_data(0x2603, 0, 0, 0, 0, 0);
    }

    if (IsMultiplayerPBEM != 0 && faction_id == ConsoleGlobal.faction_id_) {
        for (int button = 0; button < 6; ++button) {
            buttons_[button].hide();
        }
        buttons_[5].show(0);
    }
}

// ORIGINAL: 0x00426470 ?draw_info@CouncWin@@QAEXXZ 0x00426470-0x00426A95 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00426470.cpp", "image:terranx_original.exe:0x00426470", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: field-offset `__thiscall` rewrite (two RECT locals reused positionally across the function, verified push/pop order and branch polarity against the raw disassembly rather than trusting Ghidra's dropped thiscall receivers) compiles and follows the retry-loop/goto-restart structure faithfully, but diverges at instruction #3 (stack-frame shape: an 80-byte itoa scratch buffer vs the original's exact 0x7c-byte frame).
void CouncWin::draw_info() {
    Buffer &buffer = MainInterfaceGlobal;
    const int proposal = static_cast<int>(field_A1C_);
    const int target_faction = static_cast<int>(field_A20_);
    int category = 0;
    const int passes = proposal == 0 ? 2 : 1;
    RECT outer;
    for (int pass = 0; pass < passes; ++pass) {
    buffer.set_clip(MainInterfaceGlobal.buffer_working_rect());

    outer.left = static_cast<LONG>(field_BB4_);
    outer.top = static_cast<LONG>(field_BB8_);
    outer.right = static_cast<LONG>(field_BBC_);
    outer.bottom = static_cast<LONG>(field_BC0_);
    RECT inner = {outer.left, outer.top, outer.right, outer.bottom};
    RECT heading = inner;
    heading.bottom = heading.top + 0x14;
    buffer.box_sprite(&heading, &InterfaceBoxStyles[15]);
    buffer.set_font(&font2_, 0, 0, 0);
    buffer.set_text_color(0xa0, -1, 1, 1);
    if (proposal == 1) {
        buffer.set_text_color(ChatItemIdTable[target_faction], -1, 1, 1);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x7c8 / 4));
    CharUpperA(StringTemp);
    if (StringTemp[0] != '\0') {
        buffer.write_cent_l(StringTemp, &heading, static_cast<int>(strlen(StringTemp)));
    }

    inner.top = heading.bottom + 2;
    buffer.box_sprite(&inner, &InterfaceBoxStyles[0]);
    inner.left += 3;
    inner.right -= 3;
    inner.top += 3;
    inner.bottom -= 3;
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(Proposal[proposal].name))));
    CharUpperA(StringTemp);
    int wrapped_height = buffer.wrap_height(StringTemp, inner.right - inner.left);
    buffer.wrap_cent(StringTemp, &inner);
    inner.top += wrapped_height;
    inner.bottom += wrapped_height;

    if (proposal != 0) {
        StringTemp[0] = '\0';
        if (proposal == 1) {
            strcat(StringTemp, label_get(0xe10 / 4));
            strcat(StringTemp, ": ");
            GenderDefault = Players[target_faction].is_leader_female;
            PluralityDefault = 0;
            strcat(StringTemp, Players[target_faction].name_leader);
        } else {
            strcat(StringTemp, StringTable.get(
                static_cast<int>(reinterpret_cast<intptr_t>(Proposal[proposal].description))));
        }
        buffer.wrap_cent(StringTemp, &inner);
    }

    RECT totals = {outer.left + 2, outer.top, outer.right - 2, outer.bottom};
    const int totals_height = font2_.line_advance();
    totals.top = totals.bottom - totals_height;

    if (proposal != 0) {
        int votes_for = 0;
        int votes_against = 0;
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            if ((Players[faction].rule_flags & RFLAG_ALIEN) == 0) {
                const int vote = static_cast<int>(leader_label_kind_[faction]);
                const int weight = proposal < 2 ? leader_tech_[faction] : 1;
                if (vote == -1) votes_for += weight;
                if (vote == -2) votes_against += weight;
            }
        }

        buffer.set_text_color(0xfa, -1, 1, 1);
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x7bc / 4));
        strcat(StringTemp, ": ");
        say_num(votes_for);
        if (StringTemp[0] != '\0') {
            buffer.write_l(StringTemp, &totals, static_cast<int>(strlen(StringTemp)));
        }

        buffer.set_text_color(0xf9, -1, 1, 1);
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x7c0 / 4));
        strcat(StringTemp, ": ");
        say_num(votes_against);
        if (StringTemp[0] != '\0') {
            buffer.write_right_l(StringTemp, &totals, static_cast<int>(strlen(StringTemp)));
        }
    } else {
        if (category < 0x14) {
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if (eligible(faction) == 0) {
                    continue;
                }
                GenderDefault = Players[faction].is_leader_female;
                PluralityDefault = 0;
                StringTemp[0] = '\0';
                strcat(StringTemp, Players[faction].name_leader);
                strcat(StringTemp, ": ");
                int total_votes = 0;
                for (int voter = 1; voter < MaxPlayerNum; ++voter) {
                    if ((Players[voter].rule_flags & RFLAG_ALIEN) == 0 &&
                        static_cast<int>(leader_label_kind_[voter]) == faction) {
                        total_votes += leader_tech_[voter];
                    }
                }
                say_num(total_votes);
                buffer.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
                if (category > 2) {
                    break;
                }
                if (category < 2) {
                    if (category == 0) {
                        buffer.write_l(StringTemp, &totals, static_cast<int>(strlen(StringTemp)));
                    } else {
                        buffer.write_right_l(StringTemp, &totals, static_cast<int>(strlen(StringTemp)));
                    }
                } else {
                    if (WinScreenWidth < 0x400) {
                        break;
                    }
                    buffer.write_cent_l(StringTemp, &totals, static_cast<int>(strlen(StringTemp)));
                }
                ++category;
            }
        }
    }

    if (proposal == 0 && pass == 0) {
        category = 0x14;
    }
    }

    static_cast<GraphicWin &>(MainInterfaceGlobal).soft_update(&outer);
}

// ORIGINAL: 0x00424410 ?make_resolution@CouncWin@@QAEHH@Z 0x00424410-0x0042471F FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00424410.cpp", "image:terranx_original.exe:0x00424410", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: full Ghidra-faithful translation compiles and matches control flow (do/while record scan, max-index tie-break, vtable slot 001/002/062 calls); diverges at instr #0 (prologue shape), not yet chased further - large function, landed at MISMATCH.
int CouncWin::make_resolution(int resolve) {
    int vote_total = 0;
    int votes_for = 0;
    int votes_other = 0;
    int governor_objection = 0;
    int option_votes[MaxPlayerNum] = {0};
    const uint8_t live_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
    const int proposal = static_cast<int>(field_A1C_);

    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
            continue;
        }
        const int vote = static_cast<int>(leader_label_kind_[faction]);
        if (vote == 0) {
            return 0;
        }
        if (vote >= -2 && (live_factions & (1u << (faction & 0x1f))) != 0) {
            const int weight = proposal < 2 ? leader_tech_[faction] : 1;
            vote_total += weight;
            if (vote == -1) {
                votes_for += weight;
            } else {
                votes_other += weight;
                if (vote > 0) {
                    option_votes[vote] += weight;
                }
                if (faction == GameDataStateStorage.fields_.governor_faction_ && proposal > 1) {
                    governor_objection = 1;
                }
            }
        }
    }

    if (resolve != 0) {
        if (proposal == 0) {
            int greatest = 0;
            for (int offset = 0; offset < MaxPlayerNum; ++offset) {
                unsigned int candidate =
                    (static_cast<unsigned int>(GameDataStateStorage.fields_.turn_current_num_ + offset) & 0x80000007u);
                if (static_cast<int>(candidate) < 0) {
                    candidate = ((candidate - 1u) | 0xfffffff8u) + 1u;
                }
                if (candidate != 0) {
                    const int count = option_votes[candidate];
                    if (greatest <= count) {
                        if (greatest == count &&
                            static_cast<int>(candidate) != GameDataStateStorage.fields_.governor_faction_) {
                            if (GameDataStateStorage.fields_.governor_faction_ != static_cast<int>(field_A20_)) {
                                field_A20_ = 0;
                            }
                        } else {
                            field_A20_ = candidate;
                            greatest = count;
                        }
                    }
                }
            }
            field_A24_ = field_A20_ != 0 ? 1u : static_cast<uint32_t>(-1);
        } else if (votes_other < votes_for &&
                   (proposal != 1 ||
                    (vote_total * 3 + ((vote_total * 3 >> 31) & 3)) / 4 <= votes_for)) {
            field_A24_ = governor_objection != 0 ? static_cast<uint32_t>(-2) : 1u;
        } else {
            field_A24_ = static_cast<uint32_t>(-1);
        }

        CouncilPbemGate = 0;
        if (IsMultiplayerPBEM) {
            const uint8_t human_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if ((human_factions & (1u << (faction & 0x1f))) != 0 &&
                    faction != ConsoleGlobal.faction_id_) {
                    CouncilNotifyFormSlots[faction - 1] = proposal;
                    CouncilNotifyPluralSlots[faction - 1] = static_cast<int>(field_A24_);
                }
            }
        }

        redraw();
        GameDataStateStorage.fields_.council_last_resolution_turn_[proposal] = GameDataStateStorage.fields_.turn_current_num_;
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            PlayersData[faction].unk_29[proposal] = 0;
            PlayersData[faction].unk_30[proposal] = 0;
        }
        for (int button = 0; button < 6; ++button) {
            buttons_[button].hide();
            buttons_[5].show(0);
        }

        if (field_A24_ == 1) {
            if (proposal == 0) {
                GameDataStateStorage.fields_.governor_faction_ = static_cast<int>(field_A20_);
                redraw();
            }
            if (IsMultiplayerNet) {
                council_action(proposal, static_cast<int>(field_A20_));
            }
        }
    }
    return 1;
}

static void set_council_rect(RECT &rect, int left, int top, int right,
                             int bottom) {
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
}

// ORIGINAL: 0x00427230 ?init@CouncWin@@QAEXHH@Z 0x00427230-0x00428347;0x006530F4-0x00653109 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00427230.cpp", "image:terranx_original.exe:0x00427230", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: the AI diplomatic-negotiation body (faction relationship scan, tech-trade proposal, vote-threshold check, roughly 0x4281XX-0x4282E2 in the disassembly) was left as a stub comment rather than transcribed - it needed more raw-asm cross-checking than remaining time allowed. VCall::slot001 and slot057 were retyped in place to take the args this call site passes (int; int,void*,void*) per the emitter's own "retype in place" convention. Everything else (field-init prologue, widget/font/spot setup, vote-table build) is a direct transcription of a clean Ghidra decompile.
void CouncWin::init(int proposal, int faction_id) {
    Buffer local_buffer;
    for (int i = 0; i < 6; ++i) {
        buttons_[i].close();
    }
    font1_.close();
    font2_.close();
    font3_.close();
    font4_.close();
    font5_.close();
    spot_.init(8);
    SubInterface::field_4_ = 7;

    field_BC8_ = 7;
    field_BCC_ = 0x2ee;
    field_BD0_ = 0x1b;
    field_BC4_ = 0x1e;
    set_council_rect(leader_rects_[2], 0x1e, 0x35, 0x80, 0x14f);
    set_council_rect(leader_rects_[3], 0x82, 0x2d, 0xe4, 0x147);
    set_council_rect(leader_rects_[4], 0xe6, 0x25, 0x148, 0x13f);
    set_council_rect(leader_rects_[5], 0x14a, 0x1d, 0x1c2, 0x160);
    set_council_rect(leader_rects_[6], 0x1c4, 0x25, 0x226, 0x13f);
    set_council_rect(leader_rects_[7], 0x228, 0x2d, 0x28a, 0x147);
    set_council_rect(leader_rects_[8], 0x28c, 0x35, 0x2ee, 0x14f);
    set_council_rect(leader_rects_[9], 0x20, 0x1d, 0x82, 0x137);
    set_council_rect(leader_rects_[10], 0x87, 0x1d, 0xe9, 0x137);
    set_council_rect(leader_rects_[11], 0xee, 0x1d, 0x150, 0x137);
    set_council_rect(leader_rects_[12], 0x155, 0x1d, 0x1b7, 0x137);
    set_council_rect(leader_rects_[13], 0x1bc, 0x1d, 0x21e, 0x137);
    set_council_rect(leader_rects_[14], 0x223, 0x1d, 0x285, 0x137);
    set_council_rect(leader_rects_[15], 0x28a, 0x1d, 0x2ec, 0x137);

    field_BA4_ = 0xf5;
    field_BA8_ = 0x163;
    field_BAC_ = 0x21a;
    field_BB0_ = 0x17a;
    RECT panel = MainInterfaceGlobal.info_frame_rect();
    panel.left += 4;
    panel.top += 4;
    panel.right -= 4;
    panel.bottom -= 4;
    field_BB4_ = static_cast<uint32_t>(panel.left);
    field_BB8_ = static_cast<uint32_t>(panel.top);
    field_BBC_ = static_cast<uint32_t>(panel.right);
    field_BC0_ = static_cast<uint32_t>(panel.bottom);

    const int inner_bottom = panel.bottom - 3;
    int inner_left = panel.left + 7;
    const int inner_top = panel.bottom - 0x17;
    int inner_right = panel.right - 7;
    const int first_right = ((inner_right - inner_left + 3) / 3) - 3 + inner_left;
    set_council_rect(leader_rects_[16], inner_left, inner_top, first_right, inner_bottom);
    const int gap1 = (first_right - inner_left) + 3;
    inner_left += gap1;
    inner_right = first_right + gap1;
    set_council_rect(leader_rects_[17], inner_left, inner_top, inner_right, inner_bottom);
    const int gap2 = (inner_right - inner_left) + 3;
    inner_left += gap2;
    inner_right += gap2;
    set_council_rect(leader_rects_[18], inner_left, inner_top, inner_right, inner_bottom);
    set_council_rect(leader_rects_[19], panel.left + 3, panel.bottom - 0x17,
                     panel.right - 3, panel.bottom - 3);

    font1_.init(DefaultLargeFontFace, 0x14, 1);
    font2_.init(DefaultLargeFontFace, 0xe, 1);
    font3_.init(DefaultLargeFontFace, 0xe, 1);
    font4_.init(DefaultLargeFontFace, 0xc, 1);
    font3_.unk_1_ = 5;
    font5_.init(DefaultLargeFontFace, 0xc, 1);
    local_buffer.set_font(&font1_, &font2_, &font3_, &font4_);

    field_A1C_ = static_cast<uint32_t>(proposal);
    field_A20_ = static_cast<uint32_t>(faction_id);
    field_A28_ = static_cast<uint32_t>(GameDataStateStorage.fields_.governor_faction_);
    field_A44_ = IsMultiplayerPBEM ? 0xffu :
        ~static_cast<uint32_t>(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_));
    CouncilDialogActive = 1;
    field_A38_ = static_cast<uint32_t>(timeGetTime());
    field_A40_ = 1;
    field_A3C_ = static_cast<uint32_t>(bit_count(
        static_cast<int>(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))) * 60000);

    if (!IsMultiplayerPBEM || CouncilPbemGate == 0) {
        field_A24_ = 0;
        for (int faction = 0; faction < MaxPlayerNum; ++faction) {
            leader_tech_[faction] = council_votes(faction);
        }
        for (int round = 0; round < 3; ++round) {
            for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
                const uint8_t bit = static_cast<uint8_t>(1u << faction);
                if ((humans & bit) == 0 ||
                    (faction != ConsoleGlobal.faction_id_ &&
                     !IsMultiplayerNet && !IsMultiplayerPBEM)) {
                    leader_label_kind_[faction] = static_cast<uint32_t>(
                        council_get_vote(faction, proposal, faction_id));
                } else {
                    leader_label_kind_[faction] = 0;
                }
                if ((alive & bit) == 0) {
                    leader_label_kind_[faction] = static_cast<uint32_t>(-5);
                }
            }
        }
        if (IsMultiplayerPBEM) {
            CouncilPbemGate = ConsoleGlobal.faction_id_;
        }
    }

    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        FactionArt &art = FactionsArt[faction];
        leader_backing_[faction][0] = &art.council_logo[0];
        leader_backing_[faction][1] = &art.council_logo[1];
        leader_portraits_[faction][0] = &art.council_small;
        leader_portraits_[faction][1] = &art.council_big;
    }

    const int remaining_height = WinScreenHeight - MainInterfaceGlobal.field_D7C_;
    int window_height = 0x18b;
    if (remaining_height < window_height) {
        window_height = remaining_height;
    }
    const int window_y = remaining_height - window_height;
    static_cast<GraphicWin *>(this)->init((WinScreenWidth - 0x30c) / 2,
        window_y, 0x32c, window_height, nullptr, 0x1000000,
        static_cast<Win *>(&ConsoleGlobal), nullptr, nullptr);
    static_cast<GraphicWin *>(this)->fill(9);

    RECT &button1 = leader_rects_[16];
    RECT &button2 = leader_rects_[17];
    RECT &button3 = leader_rects_[18];
    RECT &button4 = leader_rects_[19];
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x7bc / 4));
    CharUpperA(StringTemp);
    buttons_[1].init(StringTemp, 1, button1.left, button1.top,
                     button1.right - button1.left, button1.bottom - button1.top,
                     static_cast<Win *>(&MainInterfaceGlobal), 0);
    if (proposal == 0) {
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x7ec / 4));
        CharUpperA(StringTemp);
        buttons_[4].init(StringTemp, 4, button4.left, button4.top,
                         button4.right - button4.left, button4.bottom - button4.top,
                         static_cast<Win *>(&MainInterfaceGlobal), 0);
    } else {
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(GameDataStateStorage.fields_.governor_faction_ == ConsoleGlobal.faction_id_
                                         ? 0x7d0 / 4 : 0x7c0 / 4));
        CharUpperA(StringTemp);
        buttons_[2].init(StringTemp, 2, button2.left, button2.top,
                         button2.right - button2.left, button2.bottom - button2.top,
                         static_cast<Win *>(&MainInterfaceGlobal), 0);
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x7c4 / 4));
        CharUpperA(StringTemp);
        buttons_[3].init(StringTemp, 3, button3.left, button3.top,
                         button3.right - button3.left, button3.bottom - button3.top,
                         static_cast<Win *>(&MainInterfaceGlobal), 0);
    }
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x194 / 4));
    CharUpperA(StringTemp);
    buttons_[5].init(StringTemp, 5, button4.left, button4.top,
                     button4.right - button4.left, button4.bottom - button4.top,
                     static_cast<Win *>(&MainInterfaceGlobal), 0);

    buttons_[5].hide();
    for (int button = 0; button < 6; ++button) {
        buttons_[button].hide();
        static_cast<Buffer &>(buttons_[button]).set_font(&font1_, nullptr, nullptr, nullptr);
        buttons_[button].redraw();
    }
    if (IsMultiplayerNet) {
        message_data(0x2602, 0, faction_id, 0, 0, 0);
    }

    const bool hide_multi = MultiWindow.is_visible() != 0;
    if (hide_multi) {
        MultiWindow.hide();
    }
    const bool hide_alpha = ConsoleGlobal.alpha_menu_.is_visible() != 0;
    if (hide_alpha) {
        ConsoleGlobal.alpha_menu_.hide();
    }
    show(0);

    if (!IsMultiplayerNet && field_A1C_ == 0 &&
        eligible(ConsoleGlobal.faction_id_) == 0) {
        const int local_faction = ConsoleGlobal.faction_id_;
        if (PlayersData[local_faction].diff_level != 0) {
        // Native 0x427DDB..0x427E74 totals live, non-alien voter weights by
        // their selected faction. 0x427E9B..0x427F4A considers only live,
        // eligible candidates whose own tally lies in the native top-two
        // comparisons before calling diplomacy_check(local, candidate, 0).
        int support_by_faction[MaxPlayerNum] = {0};
        const uint8_t alive_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
        for (int tally_candidate = 1; tally_candidate < MaxPlayerNum; ++tally_candidate) {
            if ((Players[tally_candidate].rule_flags & RFLAG_ALIEN) != 0) {
                continue;
            }
            for (int voter = 1; voter < MaxPlayerNum; ++voter) {
                if ((Players[voter].rule_flags & RFLAG_ALIEN) == 0 &&
                    (alive_factions & (1u << voter)) != 0 &&
                    static_cast<int>(leader_label_kind_[voter]) == tally_candidate) {
                    support_by_faction[tally_candidate] += leader_tech_[voter];
                }
            }
        }

        for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
            if ((Players[candidate].rule_flags & RFLAG_ALIEN) != 0 ||
                (alive_factions & (1u << candidate)) == 0 ||
                eligible(candidate) == 0) {
                continue;
            }

            int greatest_other_support = 0;
            for (int other = 1; other < MaxPlayerNum; ++other) {
                if (other != candidate &&
                    (Players[other].rule_flags & RFLAG_ALIEN) == 0 &&
                    greatest_other_support <= support_by_faction[other]) {
                    greatest_other_support = support_by_faction[other];
                }
            }

            const int candidate_support = support_by_faction[candidate];
            const int local_weight = leader_tech_[local_faction];
            if (local_weight + candidate_support <= greatest_other_support ||
                local_weight + greatest_other_support <= candidate_support) {
                continue;
            }

            diplomacy_check(local_faction, candidate, 0);

            if (DiploFriction >= 15 || DiploScratchFA60 != 0 ||
                (PlayersData[1].diplo_treaties[local_faction] & 0x40c31u) != 0) {
                continue;
            }

            // At native +0xE3C the image reads one DWORD beginning at the
            // int16_t proto_id_lost[0xF2] cell: proto_id_lost starts at +0xC58
            // after the two preceding 0x200-byte prototype arrays, and
            // 0xF2 * sizeof(int16_t) reaches +0xE3C. Preserve both halves of
            // that exact read with typed adjacent elements; the member's
            // semantic role in this negotiation gate remains uncertain.
            const uint32_t packed_lost_prototypes =
                static_cast<uint32_t>(static_cast<uint16_t>(
                    PlayersData[candidate].proto_id_lost[0xf2])) |
                (static_cast<uint32_t>(static_cast<uint16_t>(
                    PlayersData[candidate].proto_id_lost[0xf3])) << 16);
            const int32_t packed_lost_value = packed_lost_prototypes <= 0x7fffffffu
                ? static_cast<int32_t>(packed_lost_prototypes)
                : -1 - static_cast<int32_t>(~packed_lost_prototypes);
            const uint32_t candidate_flag_bits = PlayersData[candidate].flags;
            const int32_t candidate_flags = candidate_flag_bits <= 0x7fffffffu
                ? static_cast<int32_t>(candidate_flag_bits)
                : -1 - static_cast<int32_t>(~candidate_flag_bits);
            const uint32_t three_flag_bits =
                static_cast<uint32_t>(candidate_flags) * 3u;
            const int32_t three_flags = three_flag_bits <= 0x7fffffffu
                ? static_cast<int32_t>(three_flag_bits)
                : -1 - static_cast<int32_t>(~three_flag_bits);
            const int32_t flag_quarter = three_flags / 4;
            const uint32_t current_lost_limit_bits =
                static_cast<uint32_t>(GameDataStateStorage.fields_.turn_current_num_) +
                static_cast<uint32_t>(packed_lost_value) * 5u;
            const int32_t current_lost_limit =
                current_lost_limit_bits <= 0x7fffffffu
                    ? static_cast<int32_t>(current_lost_limit_bits)
                    : -1 - static_cast<int32_t>(~current_lost_limit_bits);
            const int32_t lower_limit =
                flag_quarter < current_lost_limit
                    ? flag_quarter : current_lost_limit;
            const int32_t threshold = (lower_limit / 25) * 25;

            // Native 0x427F87..0x427FCF clamps the signed 3/4 flag score
            // against TurnCurrentNum + five times the packed lost-prototype
            // DWORD, then rounds that result down to a multiple of 25. A
            // nonzero threshold not exceeding the candidate's signed flags
            // takes the credit-request route; otherwise it considers tech.
            if (threshold != 0 && candidate_flags >= threshold) {
                parse_num(0, threshold);
                diplomacy_caption(local_faction, candidate);

                Player &local_player = Players[local_faction];
                GenderDefault = local_player.is_leader_female;
                PluralityDefault = 0;
                parse_says(0, local_player.title_leader, -1, -1);
                GenderDefault = local_player.is_leader_female;
                PluralityDefault = 0;
                parse_says(1, local_player.name_leader, -1, -1);
                Player &candidate_player = Players[candidate];
                GenderDefault = candidate_player.is_leader_female;
                PluralityDefault = 0;
                parse_says(2, candidate_player.title_leader, -1, -1);
                GenderDefault = candidate_player.is_leader_female;
                PluralityDefault = 0;
                parse_says(3, candidate_player.name_leader, -1, -1);

                clock_wait(500);
                const int accepted = x_pops_short(
                    "VOTEFORME", 0, FactionDiplomacyPortraits[candidate], nullptr);
                if (accepted != 0) {
                    PlayersData[candidate].energy_reserves -= threshold;
                    PlayersData[local_faction].energy_reserves += threshold;
                    set_vote(local_faction, candidate);
                }
                continue;
            }

            // Native 0x427FD3..0x428151 asks the live technology-analysis
            // producer, builds both parsed technology names, and only grants
            // the scratch IDs still present after the modal callback.
            tech_analysis(local_faction, candidate);
            if (DiploScratchF800 < 0 || DiploScratchF80C < 0) {
                continue;
            }

            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchF800, 0);
            parse_says(4, StringTemp, -1, -1);
            StringTemp[0] = '\0';
            say_tech(StringTemp, DiploScratchF80C, 0);
            parse_says(5, StringTemp, -1, -1);
            diplomacy_caption(local_faction, candidate);

            Player &candidate_player = Players[candidate];
            GenderDefault = candidate_player.is_leader_female;
            PluralityDefault = 0;
            parse_says(0, candidate_player.title_leader, -1, -1);
            GenderDefault = candidate_player.is_leader_female;
            PluralityDefault = 0;
            parse_says(1, candidate_player.name_leader, -1, -1);
            GenderDefault = candidate_player.is_leader_female;
            PluralityDefault = 0;
            parse_says(2, candidate_player.title_leader, -1, -1);
            GenderDefault = candidate_player.is_leader_female;
            PluralityDefault = 0;
            parse_says(3, candidate_player.name_leader, -1, -1);

            clock_wait(500);
            const int accepted = x_pops_short(
                "VOTEFORMETECH", 0, FactionDiplomacyPortraits[candidate], nullptr);
            if (accepted != 0) {
                tech_achieved(local_faction, DiploScratchF800, candidate, 0);
                tech_achieved(local_faction, DiploScratchF80C, candidate, 0);
                set_vote(local_faction, candidate);
            }
        }
    }
    }

    set_modal(0, pop_wait, &MainInterfaceGlobal);
    // The original 0x4282F8 call gives up this CouncWin's active interface
    // mode after the modal proposal loop and before restoring hidden windows.
    SubInterface::release_iface_mode();
    if (hide_alpha) {
        ConsoleGlobal.alpha_menu_.show();
    }
    if (hide_multi) {
        MultiWindow.show();
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// LEVER: tail call, this reinterpreted as Win* to call is_visible() directly
// ORIGINAL: 0x00427150 ?hide@CouncWin@@QAEHXZ 0x00427150-0x00427155 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00427150.cpp", "image:terranx_original.exe:0x00427150", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Original427150 is the Win visibility-query tail jump, not a hide override. Name this ordinary derived visibility query without changing base hide semantics."}
// symbol    ?council_visible@CouncWin@@QAEHXZ
int CouncWin::council_visible() {
    return Win::is_visible();
}


// The CouncWin methods below used to be measured only in isolated seed
// scaffolds. These bodies keep their receiver as the declared CouncWin and
// express the embedded windows, fonts, Spot, and local faction by owner.

// ORIGINAL: 0x004271C0 ?arrived@CouncWin@@QAEXH@Z 0x004271C0-0x0042722D
// RECOVERY_BINDING: v1 {"evidence": ["src/councwin.h CouncWin::field_A44_; src/time.h TurnRedrawPending; src/faction.h LockEnableMask; src/graphicwin.h GraphicWin::redraw; src/netmsg.h NetMsgPrimary; image data at 0x00683228 identifies the label VOTINGOPEN"], "reason": "Track faction arrival on the CouncWin receiver, redraw through GraphicWin, and signal the voting-open label when all active humans have arrived."}
void CouncWin::arrived(int faction_id) {
    field_A44_ |= (1 << faction_id);
    if (TurnRedrawPending != 0) {
        static_cast<GraphicWin *>(this)->redraw();
        const uint32_t status = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
        const uint8_t humans = static_cast<uint8_t>(status);
        const uint8_t alive = static_cast<uint8_t>(status >> 8);
        if (((alive & humans) & field_A44_) == humans) {
            NetMsgPrimary.pop("VOTINGOPEN", 1000, 1, 0);
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
// symbol    ?on_mouse_move@CouncWin@@UAEXHH@Z
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

// on_status calls Popup's source-owned cdecl slide-animation helpers.
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

// The two FlatButton slots on_status drives are Win's own show(int) and
// hide() - FlatButton is BaseButton : GraphicWin : Win, so its vtable opens
// with Win's slots, and `button->show(0)` / `button->hide()` are the image's
// `call dword ptr [edx + 4]` and `[edx + 8]`.

// Slot 12 of the front vtable (0x66AAE8) - the catalogue's
// ?on_redraw@CouncWin@@QAEXXZ, 0x00426AA0. on_status reaches it
// with `mov edx,[ecx-0xa14] / call [edx+0x30]`, a live dispatch through the
// front's own table, so the entry has to be CouncWin's.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00426AA0 ?on_redraw@CouncWin@@QAEXXZ 0x00426AA0-0x00426EA1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00426aa0.cpp", "image:terranx_original.exe:0x00426AA0", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// symbol    ?on_redraw@CouncWin@@UAEXXZ
void CouncWin::on_redraw() {
    Buffer &buffer = static_cast<Buffer &>(*this);
    MainInterfaceGlobal.restore_clip();
    buffer.restore_clip();
    MainInterfaceGlobal.restore_back(1);

    RECT rect = MainInterfaceGlobal.info_frame_rect();
    rect.left += 4;
    rect.top += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    buffer.box_sprite(&rect, &InterfaceBoxStyles[3]);

    rect.left = 0x17;
    rect.top = 0;
    rect.right = static_cast<int>(dib_.bmiHeader.biWidth) - 0x37;
    rect.bottom = 1 - static_cast<int>(dib_.bmiHeader.biHeight);
    buffer.box_sprite(&rect, &InterfaceBoxStyles[9]);
    rect.left += 4;
    rect.top += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    buffer.box_sprite(&rect, &InterfaceBoxStyles[3]);

    draw_info();
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        draw_leader(faction);
    }

    const int result_state = static_cast<int>(field_A24_);
    if (result_state == 0) {
        return;
    }

    RECT message_rect = {
        static_cast<LONG>(field_BA4_), static_cast<LONG>(field_BA8_),
        static_cast<LONG>(field_BAC_), static_cast<LONG>(field_BB0_)
    };
    buffer.box_sprite(&message_rect, &InterfaceBoxStyles[0]);
    message_rect.right -= 3;
    message_rect.left += 3;
    message_rect.top += 4;
    message_rect.bottom -= 2;
    StringTemp[0] = '\0';

    const int proposal = static_cast<int>(field_A1C_);
    if (result_state == -2) {
        buffer.set_text_color(0xf9, -1, 1, 1);
        strcat(StringTemp, label_get(0x7e4 / 4));
    } else if (result_state == -1) {
        buffer.set_text_color(0xf9, -1, 1, 1);
        if (proposal != 0) {
            strcat(StringTemp, label_get(0x7e0 / 4));
            if (proposal == 1) {
                strcat(StringTemp, " ");
                strcat(StringTemp, "(");
                strcat(StringTemp, label_get(0xe6c / 4));
                strcat(StringTemp, ")");
            }
        } else {
            strcat(StringTemp, label_get(0x804 / 4));
        }
    } else if (result_state == 1) {
        const int faction = static_cast<int>(field_A20_);
        Player &player = Players[faction];
        if (proposal == 0) {
            buffer.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            GenderDefault = player.is_leader_female;
            PluralityDefault = 0;
            strcat(StringTemp, player.name_leader);
            strcat(StringTemp, " ");
            const int label_index = GameDataStateStorage.fields_.governor_faction_ == static_cast<int>(field_A28_)
                ? 0x200 : 0x1fd;
            strcat(StringTemp, label_get(label_index));
        } else if (proposal == 1) {
            strcat(StringTemp, label_get(0xe10 / 4));
            strcat(StringTemp, ": ");
            GenderDefault = player.is_leader_female;
            PluralityDefault = 0;
            strcat(StringTemp, player.name_leader);
            strcat(StringTemp, " ");
            buffer.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
        } else {
            buffer.set_text_color(0xfa, -1, 1, 1);
            strcat(StringTemp, label_get(0x7dc / 4));
        }
    }

    buffer.set_font(&font6_, 0, 0, 0);
    CharUpperA(StringTemp);
    buffer.wrap_cent(StringTemp, &message_rect);
}

// Slot 44 of the front vtable - the catalogue's
// ?on_button_clicked@CouncWin@@QAEXH@Z, 0x00424870, 1983 bytes.
// ORIGINAL: 0x00424870 ?on_button_clicked@CouncWin@@QAEXH@Z 0x00424870-0x0042502F;0x00652D40-0x00652EC7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00424870.cpp", "image:terranx_original.exe:0x00424870", "Council interface/window declarations identify embedded fonts/buttons, voting arrays and existing MainInterface/Console owners; original instructions determine call and state order."], "reason": "Recover the original in its genuine UI owner using typed members, real globals and declared operations. Retain seed spans and evidence; no current matching or runtime claim."}
// TRIED: jump table (8 entries at 0x425030) transcribed as a real switch on `a1+2`, matching case targets exactly; the two vtable calls ([this]+0x64 and [this]+0xe8) use original_slot/ORIGINAL rather than a raw __thiscall function-pointer cast (C4234). The ~20-stage widget-teardown cascade (Scroll/2xFlatButton/BasePop/Spot/Dialogs/ Dialog/Sprite closes, twice-inlined in the original for the early-close vs. fallthrough paths) is approximated by a single `popup.close()`; the substantive vote/call-vote/close-decision logic (set_vote, council_get_vote, message_data/NetMsg::pop gating, the eligible-faction Dialogs::item loop, BasePop::exec) is transcribed with real field offsets. Compiles; diverges #10.
// symbol    ?on_button_clicked@CouncWin@@UAEXH@Z
void CouncWin::on_button_clicked(int button_id) {
    Popup popup;
    bool popup_closed = false;

    switch (button_id) {
    case 1:
    case 2:
    case 3:
        if (field_A24_ == 0) {
            if (!IsMultiplayerNet) {
                set_vote(ConsoleGlobal.faction_id_, -button_id);
            } else {
                const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
                if (((field_A44_ & alive) & humans) == humans) {
                    message_data(0x2601, 0, ConsoleGlobal.faction_id_, -button_id, 0, 0);
                } else {
                    NetMsgPrimary.pop("VOTING", 0x1388, 1, nullptr);
                }
            }
        }
        break;

    case 4:
        if (field_A24_ == 0) {
            const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
            if (!IsMultiplayerNet || (((field_A44_ & alive) & humans) == humans)) {
                popup.start(PopupStartCaption, "GOVVOTE", -1, nullptr, 0x200040, nullptr);

                int picture_width = 0;
                int picture_height = 0;
                if (Buffer::get_pcx_dimensions("council_sm.pcx", &picture_width,
                                               &picture_height) == 0 &&
                    popup.owned_picture().init(
                        const_cast<char *>("council_sm.pcx"), picture_width,
                        picture_height) == 0) {
                    popup.select_owned_picture();
                }

                Dialogs &options = popup.dialogs();
                options.item(label_get(0x7c4 / 4), 0);
                for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                    if ((Players[faction].rule_flags & RFLAG_ALIEN) == 0 &&
                        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) &
                         (1u << (faction & 0x1f))) != 0 && eligible(faction) != 0) {
                        StringTemp[0] = '\0';
                        strcat(StringTemp, Players[faction].name_leader);
                        options.item(StringTemp, faction);
                    }
                }

                int result = popup.exec(0, pop_wait);
                if (result >= 0) {
                    if (result == 0) result = -3;
                    if (!IsMultiplayerNet) {
                        set_vote(ConsoleGlobal.faction_id_, result);
                    } else {
                        message_data(0x2601, 0, ConsoleGlobal.faction_id_, result, 0, 0);
                    }
                }
            } else {
                NetMsgPrimary.pop("VOTING", 0x1388, 1, nullptr);
            }
        }
        break;

    case 5:
        on_key_click(0, 0x0d);
        break;

    case -2: {
        bool close_council = field_A24_ != 0;
        bool check_local_vote = false;
        if (!close_council) {
            if ((static_cast<uint8_t>(GameDataStateStorage.fields_.game_more_preferences_) & 0x40) != 0 &&
                !IsMultiplayerNet) {
                if (!IsMultiplayerPBEM) {
                    close_council = true;
                } else {
                    check_local_vote = true;
                }
            } else if (IsMultiplayerPBEM) {
                check_local_vote = true;
            }
            if (check_local_vote && leader_label_kind_[ConsoleGlobal.faction_id_] != 0) {
                close_council = true;
            }
        }

        if (!close_council) {
            const uint8_t human_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            bool human_vote_exists = false;
            for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                if ((human_factions & (1u << (faction & 0x1f))) != 0 &&
                    leader_label_kind_[faction] != 0) {
                    human_vote_exists = true;
                    break;
                }
            }
            if (human_vote_exists) {
                popup.close();
                popup_closed = true;
            } else {
                for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                    const uint8_t current_humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                    if (leader_label_kind_[faction] == 0 &&
                        (current_humans & (1u << (faction & 0x1f))) == 0) {
                        leader_label_kind_[faction] = static_cast<uint32_t>(council_get_vote(
                            faction, static_cast<int>(field_A1C_), static_cast<int>(field_A20_)));
                    }
                }
            }
        } else {
            release_modal();
            TurnRedrawPending = 0;
            CouncilDialogActive = 0;
            for (int button = 0; button < 6; ++button) {
                buttons_[button].hide();
            }
            font1_.close();
            font2_.close();
            font3_.close();
            font4_.close();
            font5_.close();
            do_all_draws();
            if (!IsMultiplayerNet && field_A24_ == 1) {
                council_action(static_cast<int>(field_A1C_), static_cast<int>(field_A20_));
            }
        }
        break;
    }

    default:
        break;
    }

    if (!popup_closed) {
        popup.close();
    }
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

// The complete slide helpers are owned by popup.cpp.




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
    if (GameDataStateStorage.fields_.governor_faction_ == factionID) {
        rank = 4;
    } else if (GameDataStateStorage.fields_.governor_faction_ >= 0 && rank == 4) {
        int governorPreceding = 0;
        for (int governorFaction = 1; governorFaction < GameDataStateStorage.fields_.governor_faction_; ++governorFaction) {
            if ((Players[governorFaction].rule_flags & RFLAG_ALIEN) == 0) {
                ++governorPreceding;
            }
        }
        rank = rankBase + governorPreceding + 1;
    }

    const int layout = GameDataStateStorage.fields_.governor_faction_ < 0 ? 1 : 0;
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

    const int selectedVariant = GameDataStateStorage.fields_.governor_faction_ == factionID ? 1 : 0;
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
            const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF);
            const uint8_t alive = static_cast<uint8_t>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFF);
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
            CharUpperA(StringTemp);
            buffer->set_font(&font4_, 0, 0, 0);
            buffer->write_cent_l(StringTemp, &badgeRect,
                                 static_cast<int>(strlen(StringTemp)));
        }
    }

    if (field_A1C_ <= 1) {
        const int techId = leader_tech_[factionID];
        const uint8_t alive = static_cast<uint8_t>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFF);
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
            CharUpperA(StringTemp);
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
//        The retired adapter spelled `CouncWin::~CouncWin()` explicitly;
//        the current source leaves this destructor implicit so the compiler
//        emits the observed chain without the extra vptr stores. There is no
//        user-written destructor body; this marker records the compiler's.

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

// The native front-table slot 25 selects the Council keyboard handler.
// Keep it an override of Win's existing event, not an extra virtual slot.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00425BA0 ?on_key_click@CouncWin@@QAEHHH@Z 0x00425BA0-0x00425D8D
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00425ba0.cpp", "src/win.h", "recovery/tests/test_key_event_hierarchy.py"], "reason": "Recover the complete Council keyboard operation through its existing Win slot25 override, genuine buttons/fonts and Council state. Pinned fronttable66AAE8 slot25 targets425BA0; nonvirtual declaration made the earlier source unreachable by keyboard dispatch. Preserve native vote completion, modal release and redraw/proposal effects."}
// symbol    ?on_key_click@CouncWin@@UAEHHH@Z
int CouncWin::on_key_click(int, int key) {
    if (key != 0x1b && key != 0x0d) return 1;

    const int local_faction = ConsoleGlobal.faction_id_;
    const uint8_t human_factions = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    const bool can_close =
        (human_factions & (1u << (local_faction & 0x1f))) == 0 ||
        ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40) != 0 &&
         (GameDataStateStorage.fields_.game_more_preferences_ & 0x1000) != 0 && !IsMultiplayerPBEM) ||
        field_A24_ != 0 ||
        (IsMultiplayerPBEM && CouncilPbemGate != 0 &&
         leader_label_kind_[local_faction] != 0);
    if (!can_close) {
        for (int faction = 0; faction < MaxPlayerNum; ++faction) {
            const uint8_t current_humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            if (leader_label_kind_[faction] == 0 &&
                (current_humans & (1u << (faction & 0x1f))) == 0) {
                leader_label_kind_[faction] = static_cast<uint32_t>(council_get_vote(
                    faction, static_cast<int>(field_A1C_), static_cast<int>(field_A20_)));
            }
        }
        return 1;
    }
    release_modal();
    TurnRedrawPending = 0;
    CouncilDialogActive = 0;
    for (int button = 0; button < 6; ++button) buttons_[button].hide();
    font1_.close();
    font2_.close();
    font3_.close();
    font4_.close();
    font5_.close();
    do_all_draws();
    if (!IsMultiplayerNet && field_A24_ == 1)
        council_action(static_cast<int>(field_A1C_), static_cast<int>(field_A20_));
    return 1;
}
