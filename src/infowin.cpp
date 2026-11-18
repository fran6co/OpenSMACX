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
// ORIGINAL: 0x004562C0 _$E21 0x004562C0-0x004562D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682530 selects initializer0x4562c0..0x4562d6, registering exit0x4562e0..0x4562ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7ad2a0 bound to?ConsoleInfoWin@@3VInfoWin@@A.", "Original startup instruction evidence: [[\"0x4562c0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562c5\", \"call\", \"0x459500\"], [\"0x4562ca\", \"push\", \"0x4562e0\"], [\"0x4562cf\", \"call\", \"0x645398\"], [\"0x4562d4\", \"pop\", \"ecx\"], [\"0x4562d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4562e0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562e5\", \"jmp\", \"0x4594a0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/infowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??0InfoWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??1InfoWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f318; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?ConsoleInfoWin@@3VInfoWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x004562E0 _$E19 0x004562E0-0x004562EA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682530 selects initializer0x4562c0..0x4562d6, registering exit0x4562e0..0x4562ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7ad2a0 bound to?ConsoleInfoWin@@3VInfoWin@@A.", "Original startup instruction evidence: [[\"0x4562c0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562c5\", \"call\", \"0x459500\"], [\"0x4562ca\", \"push\", \"0x4562e0\"], [\"0x4562cf\", \"call\", \"0x645398\"], [\"0x4562d4\", \"pop\", \"ecx\"], [\"0x4562d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4562e0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562e5\", \"jmp\", \"0x4594a0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/infowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??0InfoWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??1InfoWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f318; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?ConsoleInfoWin@@3VInfoWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
#include "stdafx.h"
#include "interface_fonts.h"
#include "interface_palette.h"
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
#include "alpha.h"
#include "pulldown.h"
#include "maininterface.h"
#include "mapwin.h"
#include "terraforming.h"
#include "socialwin.h"
#include "technology.h"
#include <cstring>

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
extern char PercentText[];  // shared "%" (0x00682EA0)

// Process-wide InfoWin lifecycle; original table callback 0x004562C0
// constructs the object at 0x007AD2A0 and registers its teardown.
// Extent: ctor 0x00459500 builds button_ (PushButton, asserted 0xB00) at +0x9D0,
// ending at 0x14D0; 0x7AE770-0x7AE778 before IfaceIdToSlotStorage is unreferenced.
// RECOVERY_OBJECT: v2 0x007AD2A0 0x007AE770 ?ConsoleInfoWin@@3VInfoWin@@A 0x004562C0
InfoWin ConsoleInfoWin;
void __cdecl help_any();

extern char SocialWinColonSpaceText[];

static void infowin_append_number(char *out, int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(out, number);
}

// ORIGINAL: 0x00456300 ?setup_text@InfoWin@@QAEXXZ 0x00456300-0x0045838D
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00456300.cpp; pinned body00456300..0045838D", "Native eight-way jump table004583A8; pane-state normalization is signed remainder modulo8. Font+0C is natural line height, separately from adjusted row pitch.", "Native ranking scores use indices1..7; history reads signed16 samples with incrementing turn stride/divisor. Best-stat ties select later living factions, region sums use indices5..66.", "Rules string fields are real heap pointers produced by text_item_string/Strings::put; Strings::get retains a historical32-bit integer address ABI. Explicit call-boundary conversions do not make these fields ordinal indices.", "recovery/tasks/infowin-ownership.json; complete-owner scope expansion retained; actual-source host damage controls and fresh native compilation required"], "reason": "Recover the complete information-pane text and chart builder through genuine InfoWin rows, Font metrics, faction/rules storage and Buffer graphics. Preserve the existing original identity, seed and historical obligations; this binding is not a matching, runtime or acceptance claim."}
// Transcribed from the image in its block order: the case bodies are laid out
// 0,1,2,3,6,5,4,7 (jump table 0x004583A8), so they are written in that order.
// Every _itoa site has its own 80-byte frame buffer (14 of them), which is the
// helper's local array expanded once per site.
void InfoWin::setup_text() {
    char text[80];
    RECT rect;
    int faction = ConsoleGlobal.faction_id_;
    int row_pitch;
    int mode;
    int pane;
    int x;
    int y;
    int line_height;
    int shifted_x;
    int k;
    // WORKAROUND(type): the image addresses every text row as
    // `this + 0xB8 + (pane*12 + row)*80` with pane*12 carried in a register
    // (0x004571CD..0x004571DA) rather than caching a per-pane row pointer:
    // a flat [24][80] row table indexed by pane*12 + row (0.765 -> 0.851).
    // Retyping row_text_ in infowin.h flips ReportIf::draw_security and
    // draw_intel, so this body indexes the existing [2][12][80] member flat
    // through its first pane instead. Case 5's closing strcpy still does not
    // cross-jump into case 6's as the image's does (0x004571E1).
    int row_base;
    int value;
    int i;
    int j;

    if (font_.unk_1_ >= 0) {
        row_pitch = font_.pixel_height() + font_.unk_1_;
    } else {
        row_pitch = font_.line_height();
    }

    // Native leaves `mode` uninitialized for states 5, 7 and values outside
    // [3,8]; a positive caption scale bypasses its use.
    switch (field_B0_) {
    case 3:
    case 4:
        mode = -12;
        break;
    case 6:
        mode = -9;
        break;
    case 8:
        mode = -6;
        break;
    }

    for (i = 0; i < 2; i++) {
        row_count_[i] = 0;
        x = pane_rects_[i].left + 5;
        y = pane_rects_[i].top + 5;
        for (j = 0; j < 12; j++) {
            row_text_[i][j][0] = 0;
            row_x_[i][j] = x;
            row_y_[i][j] = y;
            y += row_pitch;
            row_color_[i][j] = 0;
            row_reveal_color_[i][j] = 0;
        }
    }

    line_height = zoom_scale(75, mode);
    shifted_x = zoom_scale(100, mode) + x + 2;

    for (pane = active_pane_; pane < active_pane_ + 1; pane++) {
        rect = pane_rects_[pane];
        MainInterfaceGlobal.box_sprite(&rect, &InterfaceBoxStyles[18]);
        rect.left += 3;
        rect.right -= 3;
        rect.top += 3;
        rect.bottom -= 3;
        k = 0;
        row_base = pane * 12;

        switch (pane_states_[pane] % 8) {
        case 0:
            text[0] = 0;
            line_height = 0;
            strcat(text, label_get(0));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            say_year(text);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 0], text);
            energy_compute(faction, 0);
            if (PlayersData[faction].tech_id_researching >= 0
                && (GameDataStateStorage.fields_.game_state_ & 0x80
                    || !(GameDataStateStorage.fields_.game_rules_ & 0x200))) {
                text[0] = 0;
                strcat(text, label_get(0x6A));
                strcat(text, SocialWinColonSpaceText);
                strcat(text, "{");
                strcat(text, table_text(
                    Technology[PlayersData[faction].tech_id_researching].name));
                strcat(text, "}");
                strcpy(row_text_[0][row_base + 1], text);
            }
            text[0] = 0;
            strcat(text, StringTable.get(Energy[0].abbrev));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, (10 - PlayersData[faction].se_alloc_labs
                                         - PlayersData[faction].se_alloc_psych) * 10);
            strcat(text, PercentText);
            strcat(text, BufferJoinSpace);
            strcat(text, BufferJoinSpace);
            strcat(text, SocialWinOpenParenText);
            if (NetIncomeGlobal >= 0) {
                strcat(text, "+");
            }
            infowin_append_number(text, NetIncomeGlobal);
            strcat(text, "/");
            strcat(text, table_text(Labels.strings_ptr[0x30]));
            strcat(text, SocialWinCloseParenText);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 2], text);
            text[0] = 0;
            strcat(text, StringTable.get(Energy[1].abbrev));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, PlayersData[faction].se_alloc_psych * 10);
            strcat(text, PercentText);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 3], text);
            text[0] = 0;
            strcat(text, StringTable.get(Energy[2].abbrev));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, PlayersData[faction].se_alloc_labs * 10);
            strcat(text, PercentText);
            if (BreakthroughTurnsGlobal < 9000) {
                strcat(text, BufferJoinSpace);
                strcat(text, BufferJoinSpace);
                strcat(text, SocialWinOpenParenText);
                infowin_append_number(text, BreakthroughTurnsGlobal);
                strcat(text, BufferJoinSpace);
                strcat(text, label_get(0x31));
                strcat(text, SocialWinCloseParenText);
            }
            strcat(text, "}");
            k = 5;
            strcpy(row_text_[0][row_base + 4], text);
            break;

        case 1:
            energy_compute(faction, 0);
            text[0] = 0;
            line_height = 0;
            strcat(text, label_get(300));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, PlayersData[faction].tech_commerce_bonus + 1);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 0], text);
            text[0] = 0;
            strcat(text, label_get(301));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, PlayersData[faction].turn_commerce_income);
            strcat(text, label_get(304));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 1], text);
            text[0] = 0;
            strcat(text, table_text(Labels.strings_ptr[302]));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, EnergyEconomyTotal);
            strcat(text, label_get(304));
            strcat(text, "}");
            k = 3;
            strcpy(row_text_[0][row_base + 2], text);
            if (Language == 0) {
                text[0] = 0;
                strcat(text, "Total Costs");
                strcat(text, SocialWinColonSpaceText);
                strcat(text, "{");
                infowin_append_number(text, NetIncomeGlobal - EnergyEconomyTotal);
                strcat(text, label_get(304));
                strcat(text, "}");
                k = 4;
                strcpy(row_text_[0][row_base + 3], text);
            }
            text[0] = 0;
            strcat(text, label_get(0x3BF));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, NetIncomeGlobal);
            strcat(text, label_get(304));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + k++], text);
            break;

        case 2: {
            int other = DiploFrictionFactionID;
            line_height = 0;
            text[0] = 0;
            parse_set(Players[other].noun_gender, Players[other].is_noun_plural);
            strcat(text, Players[other].formal_name_faction);
            strcpy(row_text_[0][row_base + 0], text);
            text[0] = 0;
            strcat(text, get_title(other));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(other));
            strcpy(row_text_[0][row_base + 1], text);
            text[0] = 0;
            {
                int mood = get_mood(DiploFriction);
                say_special(text, StringTable.get(Mood[mood]), other);
            }
            k = 3;
            strcpy(row_text_[0][row_base + 2], text);
            text[0] = 0;
            if (PlayersData[faction].diplo_treaties[other] & 1) {
                strcat(text, StringTable.get(Labels.strings_ptr[0xD3]));
            } else if (PlayersData[faction].diplo_treaties[other] & 2) {
                strcat(text, label_get(0xD4));
            } else if (PlayersData[faction].diplo_treaties[other] & 4) {
                strcat(text, label_get(0xD5));
            } else if (PlayersData[faction].diplo_treaties[other] & 0x10) {
                strcat(text, label_get(0xD6));
            }
            // The image tests the buffer address itself (0x00456EE2), not its
            // first character, so the treaty row is always published.
            if (text) {
                k = 4;
                strcpy(row_text_[0][row_base + 3], text);
            }
            break;
        }

        case 3:
            line_height = 0;
            text[0] = 0;
            parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
            strcat(text, Players[faction].formal_name_faction);
            strcpy(row_text_[0][row_base + 0], text);
            text[0] = 0;
            strcat(text, get_title(faction));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(faction));
            strcpy(row_text_[0][row_base + 1], text);
            text[0] = 0;
            strcat(text, label_get(0xDE));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            say_special(text, StringTable.get(
                Repute[range(PlayersData[faction].integrity_blemishes, 0, 7)]), faction);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 2], text);
            text[0] = 0;
            strcat(text, label_get(0xDF));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            say_special(text, StringTable.get(
                Might[7 - range(PlayersData[faction].ranking, 1, 7)].adj), faction);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 3], text);
            text[0] = 0;
            strcat(text, label_get(0x3C0));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            infowin_append_number(text, council_votes(faction));
            strcat(text, "}");
            k = 5;
            strcpy(row_text_[0][row_base + 4], text);
            break;

        case 6:
            line_height = 0;
            value = 0;
            k = ConsoleGlobal.faction_id_;
            i = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (j = 1; j < MaxPlayerNum; j++) {
                if (i & (1 << j) && PlayersData[j].tech_ranking >= value) {
                    value = PlayersData[j].tech_ranking;
                    k = j;
                }
            }
            text[0] = 0;
            strcat(text, label_get(0x41C / 4));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, get_title(k));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(k));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 0], text);

            value = 0;
            k = ConsoleGlobal.faction_id_;
            i = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (j = 1; j < MaxPlayerNum; j++) {
                if (i & (1 << j) && PlayersData[j].mil_strength_1 >= value) {
                    value = PlayersData[j].mil_strength_1;
                    k = j;
                }
            }
            text[0] = 0;
            strcat(text, label_get(0x420 / 4));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, get_title(k));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(k));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 1], text);

            value = 0;
            k = ConsoleGlobal.faction_id_;
            i = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (j = 1; j < MaxPlayerNum; j++) {
                if (i & (1 << j) && PlayersData[j].pop_total >= value) {
                    value = PlayersData[j].pop_total;
                    k = j;
                }
            }
            text[0] = 0;
            strcat(text, label_get(0x424 / 4));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, get_title(k));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(k));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 2], text);

            value = 0;
            k = ConsoleGlobal.faction_id_;
            i = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (j = 1; j < MaxPlayerNum; j++) {
                if (i & (1 << j) && PlayersData[j].energy_reserves >= value) {
                    value = PlayersData[j].energy_reserves;
                    k = j;
                }
            }
            text[0] = 0;
            strcat(text, label_get(0x4F0 / 4));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, get_title(k));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(k));
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 3], text);

            value = 0;
            k = ConsoleGlobal.faction_id_;
            i = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (faction = 1; faction < MaxPlayerNum; faction++) {
                if (i & (1 << faction)) {
                    int sum = 0;
                    for (j = 1; j < 64; j++) {
                        if (!bad_reg(j)) {
                            sum += PlayersData[faction].unk_78[j];
                        }
                    }
                    if (sum >= value) {
                        value = sum;
                        k = faction;
                    }
                }
            }
            text[0] = 0;
            strcat(text, StringTable.get(Labels.strings_ptr[0x97C / 4]));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, get_title(k));
            strcat(text, BufferJoinSpace);
            strcat(text, get_name(k));
            strcat(text, "}");
            k = 5;
            strcpy(row_text_[0][row_base + 4], text);
            // Signed test in the image (test/jle, 0x004577F4).
            if ((int)GameDataStateStorage.fields_.ranking_faction_id_unk1_ > 0) {
                text[0] = 0;
                strcat(text, label_get(0x428 / 4));
                strcat(text, SocialWinColonSpaceText);
                strcat(text, "{");
                strcat(text, get_title(GameDataStateStorage.fields_.faction_rankings_[7]));
                strcat(text, BufferJoinSpace);
                strcat(text, get_name(GameDataStateStorage.fields_.faction_rankings_[7]));
                strcat(text, "}");
                k = 6;
                strcpy(row_text_[0][row_base + 5], text);
            }
            break;

        case 5:
            line_height = 0;
            MainInterfaceGlobal.set_clip(&rect);
            text[0] = 0;
            if ((ScenarioSave.objective_req_victory < 9000
                 || ScenarioSave.objectives_sudden_death_victory < 9000)
                && !ScenarioSave.objective_achieve_points) {
                strcat(text, label_get(899));
                CharUpperA(text);
            } else {
                strcat(text, label_get(0x1B8));
            }
            k = 1;
            strcpy(row_text_[0][row_base + 0], text);
            {
                int plot_height = rect.bottom - rect.top - row_pitch - 4;
                int base = rect.bottom - 2;
                int width;
                rankings(0);
                value = 1;
                for (i = 1; i < MaxPlayerNum; i++) {
                    if (FactionRankingsUnk[i] > value) {
                        value = FactionRankingsUnk[i];
                    }
                }
                width = (rect.right - rect.left) / 7;
                rect.left += (rect.right - rect.left - width * 7) / 2;
                rect.right = rect.left + width;
                for (i = 1; i < MaxPlayerNum; i++) {
                    int height = FactionRankingsUnk[i] * plot_height / value;
                    if (height < 1) {
                        height = 1;
                    }
                    rect.top = base - height;
                    rect.bottom = rect.top + height;
                    MainInterfaceGlobal.box(&rect, InfoFactionShadowColors[i],
                                            InfoFactionShadowColors[i]);
                    rect.left++;
                    rect.right--;
                    rect.top++;
                    rect.bottom--;
                    MainInterfaceGlobal.fill(&rect, InfoFactionTextColors[i]);
                    rect.left--;
                    rect.right++;
                    rect.top--;
                    rect.bottom++;
                    // Only the noun's first letter labels the bar (0x00457AE0).
                    text[0] = 0;
                    strcat(text, get_noun(i));
                    text[1] = 0;
                    MainInterfaceGlobal.set_font(&font_, 0, 0, 0);
                    MainInterfaceGlobal.set_text_color(ChatItemIdTable[i],
                                                       ChatShadowColors[i], 1, 1);
                    MainInterfaceGlobal.write_cent_l(text, &rect);
                    rect.left += width;
                    rect.right += width;
                }
            }
            MainInterfaceGlobal.restore_clip();
            break;

        case 4:
            line_height = 0;
            MainInterfaceGlobal.set_clip(&rect);
            text[0] = 0;
            if (!((ScenarioSave.objective_req_victory < 9000
                 || ScenarioSave.objectives_sudden_death_victory < 9000)
                && !ScenarioSave.objective_achieve_points)) {
                strcat(text, label_get(0x1B8));
            } else {
                strcat(text, label_get(899));
                CharUpperA(text);
            }
            strcpy(row_text_[0][row_base + 0], text);
            {
                int plot_height = rect.bottom - rect.top - row_pitch - 4;
                int step;
                int y_divisor;
                int maximum;
                rankings(0);
                maximum = 1;
                for (i = 1; i < MaxPlayerNum; i++) {
                    if (FactionRankingsUnk[i] > maximum) {
                        maximum = FactionRankingsUnk[i];
                    }
                }
                step = 1;
                while (GameDataStateStorage.fields_.turn_current_num_ / step
                       > rect.right - rect.left) {
                    step++;
                }
                y_divisor = 1;
                while (plot_height * y_divisor < maximum) {
                    y_divisor++;
                }
                for (k = 1; k < MaxPlayerNum; k++) {
                    int last_y = rect.bottom - 1;
                    int last_x;
                    int turn;
                    x = rect.left;
                    last_x = x;
                    for (turn = 1; turn <= GameDataStateStorage.fields_.turn_current_num_;
                         turn += step) {
                        int plot_y = rect.bottom
                            - static_cast<int16_t>(FactionRankingHistory[turn * MaxPlayerNum + k])
                                / y_divisor - 1;
                        if (turn > 1) {
                            // strcmp, not strstr (0x00457D0C): every faction but
                            // the Spartans takes the text colour.
                            MainInterfaceGlobal.line(last_x, last_y, x, plot_y,
                                strcmp(Players[k].search_key, "SPARTANS")
                                    ? InfoFactionTextColors[k]
                                    : InfoFactionShadowColors[k]);
                        }
                        last_y = plot_y;
                        last_x = x;
                        x++;
                    }
                }
            }
            // Seven names, FactionRankings[7] down to [1], go to rows 1..7. The
            // fit test compares against the plot's final x (0x00457E33),
            // not a width.
            k = 8;
            for (i = 7; i > 0; i--) {
                text[0] = 0;
                strcat(text, get_noun(GameDataStateStorage.fields_.faction_rankings_[i]));
                while (strlen(text)
                       && rect.right - rect.left - MainInterfaceGlobal.text_width(text) - 2 < x) {
                    text[strlen(text) - 1] = 0;
                }
                strcpy(row_text_[0][row_base + 8 - i], text);
            }
            MainInterfaceGlobal.restore_clip();
            break;

        case 7:
            text[0] = 0;
            line_height = 0;
            strcat(text, label_get(0x6AC / 4));
            strcpy(row_text_[0][row_base + 0], text);
            text[0] = 0;
            strcat(text, label_get(0x6A8 / 4));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, MapFilePath);
            strcat(text, "}");
            strcpy(row_text_[0][row_base + 1], text);
            text[0] = 0;
            strcat(text, label_get(0x6A4 / 4));
            strcat(text, SocialWinColonSpaceText);
            switch (MapEditorPaintId) {
            case 0:
                strcat(text, label_get(0xEBC / 4));
                break;
            case 1:
                strcat(text, label_get(0xEC0 / 4));
                break;
            case 2:
                strcat(text, label_get(0xEC4 / 4));
                break;
            case 3:
                strcat(text, label_get(0xEC8 / 4));
                break;
            case 4:
                strcat(text, label_get(0xECC / 4));
                break;
            case 5:
                strcat(text, label_get(0xED0 / 4));
                break;
            case 26:
                strcat(text, StringTable.get(Natural[2].name_short));
                break;
            case 27:
                strcat(text, StringTable.get(Natural[6].name_short));
                break;
            case 28:
                strcat(text, StringTable.get(Natural[3].name_short));
                break;
            case 29:
                strcat(text, StringTable.get(Natural[10].name_short));
                break;
            default:
                strcat(text, StringTable.get(Terraforming[MapEditorPaintId - 6].name[0]));
                break;
            }
            strcpy(row_text_[0][row_base + 2], text);
            text[0] = 0;
            strcat(text, label_get(0x684 / 4));
            strcat(text, SocialWinColonSpaceText);
            if (MapEditorBrushId == 1) {
                strcat(text, label_get(0x688 / 4));
            } else {
                infowin_append_number(text, MapEditorBrushId ? MapEditorBrushId - 1 : 0);
            }
            strcpy(row_text_[0][row_base + 3], text);
            text[0] = 0;
            strcat(text, label_get(0xD7C / 4));
            strcat(text, SocialWinColonSpaceText);
            infowin_append_number(text, MapLongitudeBounds / 2);
            strcat(text, "x");
            infowin_append_number(text, MapLatitudeBounds);
            strcat(text, BufferJoinSpace);
            if (MapIsFlat & 1) {
                strcat(text, label_get(0xD80 / 4));
            } else {
                strcat(text, table_text(Labels.strings_ptr[0xD84 / 4]));
            }
            strcpy(row_text_[0][row_base + 4], text);
            text[0] = 0;
            strcat(text, StringTable.get(Labels.strings_ptr[0x84 / 4]));
            strcat(text, SocialWinColonSpaceText);
            strcat(text, "{");
            strcat(text, StringTable.get(Labels.strings_ptr[0x14 + MapCloudCover]));
            strcat(text, "}");
            k = 6;
            strcpy(row_text_[0][row_base + 5], text);
            break;
        }

        row_count_[pane] = k;
        text_lengths_[pane] = 0;
        for (k = 0; k < row_count_[pane]; k++) {
            if (line_height && row_y_[pane][k] < line_height + y + 2) {
                row_x_[pane][k] = shifted_x;
            }
            text_lengths_[pane] += strlen(row_text_[0][row_base + k]);
        }
    }
}

// Shared information-panel extent, immediately preceding WorldMapBounds in
// the original zero-filled layout block. The four stores at 0x00458D8A-DA9
// establish RECT field order, and InfoWin::init consumes the same extent.
// RECOVERY_OBJECT: v2 0x00939EF8 0x00939F08 ?InfoPanelBounds@@3UtagRECT@@A 0x00458D60
RECT InfoPanelBounds;

// The original compares the cached interface width at 0x00939E5C, not the
// live Win screen width, and reads the GraphicWin virtual base's DIB height
// through ConsoleGlobal. `pixel_height()` handles its top-down sign.
// ORIGINAL: 0x00458D60 ?UNK4@InfoWin@@QAEXXZ 0x00458D60-0x00458DBB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458D60-0x00458DBB compares the cached interface-width object at 0x00939E5C", "git:9e39965b:src/recovered/units/00458d60.cpp", "source:src/game.h:IfaceScreenWidth", "source:src/console.h:GraphicWin virtual base", "source:src/buffer.h:Dib"], "reason": "The original stores one RECT at 0x00939EF8, selecting its extent from IfaceScreenWidth and the Console GraphicWin Buffer DIB height; the source name describes the observed behavior, not the catalog placeholder UNK4."}
// symbol    ?configure_panel_bounds@InfoWin@@QAEXXZ
void InfoWin::configure_panel_bounds() {
    int panel_width;
    int panel_height;
    if (IfaceScreenWidth >= 1000) {
        panel_width = 250;
        panel_height = 212;
    } else {
        panel_width = 200;
        panel_height = 180;
    }
    int top = ConsoleGlobal.pixel_height();
    InfoPanelBounds.left = 0;
    InfoPanelBounds.right = panel_width;
    InfoPanelBounds.top = top;
    InfoPanelBounds.bottom = top + panel_height;
    StatusFlags |= 4;
}

// ORIGINAL: 0x004592A0 ?init@InfoWin@@QAEXXZ 0x004592A0-0x00459414 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004592A0-0x00459414", "git:9e39965b:src/recovered/units/004592a0.cpp", "source:src/infowin.h:owned members", "source:src/maininterface.h:info_panel_frame_"], "reason": "Initialize the typed InfoWin pane states, font, paired pane rectangles, bounds and timer callback in the owner; the original reads MainInterfaceGlobal+0xDA0 as one panel RECT and conditionally initializes the shared panel extent."}
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

    if (WinScreenWidth == 800) {
        font_.init(const_cast<char *>(DefaultFontFace), 10, 0);
    } else {
        font_.init(const_cast<char *>(DefaultFontFace), 12, 0);
    }
    font_.unk_1_ = 4;

    RECT pane = MainInterfaceGlobal.info_panel_frame();
    // The image narrows by 0x20 first, then insets every edge by 7.
    pane.right -= 0x20;
    pane.left += 7;
    pane.right -= 7;
    pane.top += 7;
    pane.bottom -= 7;
    if (WinScreenWidth != 800) {
        pane.right = (pane.right - pane.left - 3) / 2 + pane.left;
        pane_rects_[0] = pane;
        const int stride = pane.right - pane.left + 3;
        pane.left += stride;
        pane.right += stride;
    } else {
        pane_rects_[0] = pane;
    }
    pane_rects_[1] = pane;
    UnionRect(&bounds_, &pane_rects_[0], &pane_rects_[1]);
    timer_.start(InfoWin::timer_daemon, 10, 50, 50);
}

// The timer dispatches a plain C callback. Its argument is forwarded as the
// timer stage, while the process-wide information window owns the state.
// ORIGINAL: 0x00459260 ?timer_daemon@InfoWin@@QAAXH@Z 0x00459260-0x0045927C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00459260-0x0045927C", "git:8e72de62:src/recovered/00459260.cpp", "source:src/console.cpp:ConsoleInfoWin", "source:src/map.cpp:WorldClimateSkipTerrainClear"], "reason": "The stack-only callback forwards its tick argument to the process-wide InfoWin timer_proc unless climate terrain clearing is active; the old Q-prefixed catalog label is not evidence for an instance receiver."}
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
// ORIGINAL: 0x00459150 ?timer_proc@InfoWin@@QAEXH@Z 0x00459150-0x00459260 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00459150.cpp"], "reason": "Home InfoWin timer behavior with typed pane members and MainInterface flat-button slot"}
void InfoWin::timer_proc(int a1) {
    if (field_AC_ != 0) return;
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0) return;
    if (MainInterfaceGlobal.flat_button_[11].is_visible() != 0) return;

    ++visible_characters_[active_pane_];
    const int pane = active_pane_;
    const int length = text_lengths_[pane];
    const int count = visible_characters_[pane];
    if (count > length + 0x32) {
        int mouse_x;
        int mouse_y;
        MainInterfaceGlobal.get_mouse_pos(&mouse_x, &mouse_y);
        // A named flag: the image tests all three conditions before laying
        // out the hold arm as the fall-through.
        int hold = in_box(mouse_x, mouse_y, &bounds_) != 0 ||
                   field_A8_ != 0 || field_94_ != 0;
        if (hold) {
            const int current_pane = active_pane_;
            visible_characters_[current_pane] =
                text_lengths_[current_pane] + 0x28;
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
// ORIGINAL: 0x00458FF0 ?timer_change@InfoWin@@QAEXH@Z 0x00458FF0-0x00459141 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c4c11a8a:src/recovered/units/00458ff0.cpp", "source:src/infowin.h", "source:src/game.cpp:InterfaceBoxStyles", "pinned:0x004590C7-0x004590DF clears 0x007AF174, 0x007AF17C, 0x007AF178 and 0x007AF180 after successful state_valid", "source:MainInterfaceGlobal at 0x007AE820 has its GraphicWin Buffer subobject at +0x444; those destinations are Buffer +0x510, +0x518, +0x514 and +0x51C", "source:Buffer::reset_text_state clears the existing fields in that original order; InfoWin::draw_text uses the same owner operation"], "reason": "Home the timed pane transition with owned InfoWin state, pane rectangle and box style. The former animation-counter alias actually addressed MainInterfaceGlobal's Buffer text state; reset those members after the successful validity loop, preserving the original store order and callbacks."}
void InfoWin::timer_change(int) {
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0) return;

    const int old_pane = active_pane_;
    field_94_ = 0;
    if (WinScreenWidth == 800) {
        active_pane_ = 0;
    } else {
        active_pane_ = (old_pane - 1) & 1;
    }

    int first = 1;
    for (;;) {
        if (redraw_phase_ >= 0) {
            pane_states_[active_pane_] = (redraw_phase_ + 1) % 8;
        } else {
            pane_states_[active_pane_] =
                (pane_states_[first ? old_pane : active_pane_] + 1) % 8;
        }
        redraw_phase_ = -1;
        first = 0;
        if (state_valid(pane_states_[active_pane_])) break;
    }

    MainInterfaceGlobal.reset_text_state();
    visible_characters_[0] = 0;
    visible_characters_[1] = 0;
    field_A8_ = 0;
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    if (pane_states_ != 0) {
        visible_characters_[0] = 0;
        visible_characters_[1] = 0;
    }
    setup_text();
    draw_text(active_pane_);
}

// Rank rows use one palette entry per faction, including native life.
// RECOVERY_OBJECT: v2 0x0068F9F0 0x0068FA10 ?InfoFactionTextColors@@3PAHA 0x00458420
int InfoFactionTextColors[8] = {249, 111, 252, 255, 251, 65, 102, 187};
// RECOVERY_OBJECT: v2 0x0068FA10 0x0068FA30 ?InfoFactionShadowColors@@3PAHA 0x00458420
int InfoFactionShadowColors[8] = {1, 175, 4, 248, 3, 57, 109, 192};

// ORIGINAL: 0x00458420 ?draw_text@InfoWin@@QAEXH@Z 0x00458420-0x004588A9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458420-0x004588A9"], "reason": "Recovered original behavior through real source classes and owned members; source remains subject to byte and structural verification."}
// The ranked rows right-align through Buffer's guarded write_right; the image
// tests the text for null before strlen exactly as the header's guarded
// write_l does.
void InfoWin::draw_text(int pane) {
    MainInterfaceGlobal.set_font(&font_, 0, 0, 0);
    int row = 0;
    // The pin's [EBP-0xC] slot is function-scoped: 0x4586E4/0x458729 assign it,
    // while the zero-budget path skips those stores and reloads it at 0x4587FE.
    // Exact exhaustion lets the next row reuse the prior partial row's x; an
    // entry-zero path with no prior assignment remains uninitialized.
    int x;
    int column;
    int end;
    if (pane == -1) {
        column = 0;
        end = 2;
    } else {
        column = pane;
        end = pane + 1;
    }
    for (; column < end; ++column) {
        MainInterfaceGlobal.set_clip(&pane_rects_[column]);
        RECT area = pane_rects_[column];
        area.left += 3;
        area.right -= 3;
        int remaining = visible_characters_[column];
        for (; row < row_count_[column] && remaining >= 0; ++row) {
            char *text = row_text_[column][row];
            int length = strlen(text);
            int ranked = pane_states_[column] == 4 && row > 0;
            if (length != 0) {
                if (ranked) {
                    int faction = GameDataStateStorage.fields_.faction_rankings_[8 - row];
                    MainInterfaceGlobal.set_text_color(InfoFactionTextColors[faction],
                                                       InfoFactionShadowColors[faction], 1, 1);
                } else {
                    int color = row_color_[column][row];
                    if (color != 0) {
                        if (field_94_ != 0) {
                            MainInterfaceGlobal.set_text_color(row_reveal_color_[column][row], -1, 1, 1);
                        } else {
                            MainInterfaceGlobal.set_text_color(color, -1, 1, 1);
                        }
                    } else if (field_94_ != 0) {
                        MainInterfaceGlobal.set_text_color(TextColourTable[24], TextColourTable[25], 1, 1);
                        MainInterfaceGlobal.set_text_color2(TextColourTable[28], TextColourTable[29], 1, 1);
                    } else {
                        MainInterfaceGlobal.set_text_color(TextColourTable[22], TextColourTable[23], 1, 1);
                        MainInterfaceGlobal.set_text_color2(TextColourTable[26], TextColourTable[27], 1, 1);
                    }
                }
                if (remaining > length) {
                    if (ranked) {
                        MainInterfaceGlobal.write_right(text, 2, row_y_[column][row],
                                                        area.right - area.left - 4);
                    } else {
                        MainInterfaceGlobal.write_l(text, row_x_[column][row],
                                                    row_y_[column][row]);
                    }
                } else {
                    // The original only skips the first, `remaining - 1`
                    // prefix write at zero. It still copies/terminates the
                    // prefix and reaches the final-character path below,
                    // where `prefix + remaining - 1` is invalid. The persistent
                    // x is reused from an earlier partial row if present; it is
                    // uninitialized only when a zero budget is the first use.
                    // Preserve that path rather than normalizing it.
                    char prefix[80];
                    if (remaining != 0) {
                        strcpy(prefix, text);
                        prefix[remaining - 1] = 0;
                        if (ranked) {
                            x = area.right - area.left -
                                MainInterfaceGlobal.text_width(text) - 2;
                            x = MainInterfaceGlobal.write_l(prefix, x, row_y_[column][row]);
                        } else {
                            x = MainInterfaceGlobal.write_l(prefix, row_x_[column][row],
                                                            row_y_[column][row]);
                        }
                    }
                    strcpy(prefix, text);
                    prefix[remaining] = 0;
                    if (!ranked) {
                        if (row_color_[column][row] != 0) {
                            MainInterfaceGlobal.set_text_color(row_reveal_color_[column][row], -1, 1, 1);
                        } else {
                            MainInterfaceGlobal.set_text_color(TextColourTable[24], TextColourTable[23], 1, 1);
                            MainInterfaceGlobal.set_text_color2(TextColourTable[28], TextColourTable[29], 1, 1);
                        }
                    }
                    MainInterfaceGlobal.write_l(&prefix[remaining - 1], x,
                                                row_y_[column][row]);
                    MainInterfaceGlobal.reset_text_state();
                }
            }
            remaining -= length;
        }
    }
    MainInterfaceGlobal.restore_clip();
    if (pane == -1) {
        MainInterfaceGlobal.update(&bounds_, 0);
    } else {
        MainInterfaceGlobal.update(&pane_rects_[pane], 0);
    }
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
    if (pane_states_ != 0) {
        visible_characters_[0] = 0;
        visible_characters_[1] = 0;
    }
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
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00458DC0 0x00458EDC 0x00458F0C 0x00458DE6
// ORIGINAL: 0x00458DC0 ?state_valid@InfoWin@@QAEHH@Z 0x00458DC0-0x00458EDA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00458DC0-0x00458EDA; jump table 0x00458EDC-0x00458F0C", "source:src/faction.h:PlayerData.diff_level and diplo_treaties", "git:b4928ace:src/unrecovered/00458dc0.cpp"], "reason": "Recover InfoWin mode eligibility using game state and typed faction difficulty/contact fields; switch destinations checked in original image; retired seed is a hint, not product code."}
int InfoWin::state_valid(int state) {
    if (state == 0) {
        return 1;
    }
    if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
        switch (state) {
        case 0:
        case 11:
            return 1;
        case 1:
            if (GameDataStateStorage.fields_.turn_current_num_ >= 50 &&
                (GameDataStateStorage.fields_.turn_current_num_ >= 100 || PlayersData[ConsoleGlobal.faction_id_].diff_level >= DLVL_TALENT)) {
                return 1;
            }
            break;
        case 3:
            if (DiplomacyActive) {
                return 1;
            }
            if (GameDataStateStorage.fields_.turn_current_num_ >= 50 || PlayersData[ConsoleGlobal.faction_id_].diff_level >= DLVL_TALENT) {
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
            if (GameDataStateStorage.fields_.turn_current_num_ >= 20 || (GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO)) {
                return 1;
            }
            break;
        case 7:
            if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) && (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)) {
                return 1;
            }
            break;
        }
    }
    return 0;
}

// Select a valid presentation state for the active pane, reset its display
// budget, and redraw it. The original's final `this + 0x88 != 0` is an
// address check and is always true for a valid object. Argument a2 selects
// transient counter values before box_sprite; the post-call address check
// then resets both counters regardless of a2.
// TRIED (seed): 0.986 mnemonic similarity, one instruction off (`jns` vs
// `jge`) on the sign test after masking; VC6 emitted a fresh compare rather
// than reusing the AND flags. The seed's other two divergences were fixed by
// emitting the a2==0 ternary arms else-first.
// ORIGINAL: 0x00458F10 ?set_state@InfoWin@@QAEXHH@Z 0x00458F10-0x00458FEA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00458f10.cpp", "source:src/infowin.h", "source:src/infowin.cpp:state_valid"], "reason": "Replace the offset-based seed with the InfoWin's active-pane and presentation-state members and its existing eligibility method."}
void InfoWin::set_state(int state, int a2) {
    const int pane = active_pane_;
    if (state < 0) state = 0;
    pane_states_[pane] = state;
    while (!state_valid(pane_states_[pane])) {
        pane_states_[pane] = (pane_states_[pane] + 1) % 8;
    }
    field_94_ = 0;
    visible_characters_[0] = a2 != 0 ? 0 : text_lengths_[0] + 1;
    visible_characters_[1] = a2 != 0 ? 0 : text_lengths_[1] + 1;
    field_A8_ = 0;
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    // The original tests the pane-state array address, not its contents.
    if (pane_states_ != 0) {
        visible_characters_[0] = 0;
        visible_characters_[1] = 0;
    }
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

// The original pulls each menu caption from the label table, then applies
// the chosen state to the active pane. The menu's stored item ID is not the
// same as its selected slot; PullDown owns that translation.
// ORIGINAL: 0x004589C0 ?right_menu@InfoWin@@QAEXHH@Z 0x004589C0-0x00458D3A;0x00655280-0x00655295 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004589C0-0x00458D3A", "image:0x00655280-0x00655295", "git:bfa66ccf:src/recovered/units/004589c0.cpp", "source:src/pulldown.h:get_selected_id"], "reason": "Home the information-pane context menu with the real PullDown item-ID API, source-owned label table, pane state and MainInterface renderer; retain the original secondary EH span as a restoration obligation."}
void InfoWin::right_menu(int x, int y) {
    PullDown menu;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    menu.init(0);
    if (state_valid(0)) menu.add_item(0, StringTable.get(Labels.strings_ptr[0x92C / 4]));
    if (state_valid(1)) menu.add_item(1, StringTable.get(Labels.strings_ptr[0x930 / 4]));
    if (state_valid(2)) menu.add_item(2, StringTable.get(Labels.strings_ptr[0x934 / 4]));
    if (state_valid(3)) menu.add_item(3, StringTable.get(Labels.strings_ptr[0x938 / 4]));
    if (state_valid(4)) menu.add_item(4, StringTable.get(Labels.strings_ptr[0x948 / 4]));
    if (state_valid(5)) menu.add_item(5, StringTable.get(Labels.strings_ptr[0x94C / 4]));
    if (state_valid(6)) menu.add_item(6, StringTable.get(Labels.strings_ptr[0x950 / 4]));
    if (state_valid(7)) menu.add_item(7, StringTable.get(Labels.strings_ptr[0x954 / 4]));
    menu.add_separator(99);
    menu.add_item(20, StringTable.get(Labels.strings_ptr[0x8FC / 4]));
    menu.show(x, y, pop_expire);

    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_800) == 0) {
        const int selected_index = menu.get_selected();
        // The pinned caller checks only the signed upper bound before indexing.
        if (selected_index < 64) {
            const int selection = menu.item_id(selected_index);
            if (selection >= 0 && selection <= 16 && state_valid(selection)) {
                const int pane = active_pane_;
                pane_states_[pane] = selection;
                while (!state_valid(pane_states_[pane])) {
                    pane_states_[pane] = (pane_states_[pane] + 1) % 8;
                }
                field_94_ = 0;
                visible_characters_[0] = text_lengths_[0] + 1;
                visible_characters_[1] = text_lengths_[1] + 1;
                field_A8_ = 0;
                MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                               &InterfaceBoxStyles[18]);
                // The original tests the pane-state array address, not contents.
                if (pane_states_ != 0) {
                    visible_characters_[0] = 0;
                    visible_characters_[1] = 0;
                }
                setup_text();
                draw_text(active_pane_);
                return;
            } else if (selection == 20) {
                help_any();
            }
        }
    }
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

// ORIGINAL: 0x00459420 ?UNK5@InfoWin@@QAEXHHHHH@Z 0x00459420-0x0045949B;0x00655295-0x006552BA FILE BYTE_EXACT
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
// The image gives the third Font its own unwind state (three cleanup
// funclets, and the state is reset to 1 before ~font_c) although nothing
// visible runs while it lives: VC6 only does that when some potentially
// throwing call sat there and was folded away after state numbering. A
// constant-false call on font_c reproduces it byte for byte; nesting scopes
// or reordering the locals does not.
void InfoWin::UNK5(int, int, int, int, int) {
    Font font_a;
    Font font_b;
    Font font_c;
    // WORKAROUND(inline): stand-in for folded-away code between font_c's
    // construction and destruction (image keeps EH state 2 for it).
    if (0) {
        font_c.init(0, 0, 0);
    }
}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00456300 0x00458390 0x004583A8 0x0045633B
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00456300 0x004583A8 0x004583C8 0x004564EF
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00456300 0x004583C8 0x004583F4 0x00457F9E
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00456300 0x004583F4 0x00458412 0x00457F98
