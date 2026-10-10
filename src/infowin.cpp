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
// ORIGINAL: 0x004562C0 _$E21 0x004562C0-0x004562D6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682530 selects initializer0x4562c0..0x4562d6, registering exit0x4562e0..0x4562ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7ad2a0 bound to?ConsoleInfoWin@@3VInfoWin@@A.", "Original startup instruction evidence: [[\"0x4562c0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562c5\", \"call\", \"0x459500\"], [\"0x4562ca\", \"push\", \"0x4562e0\"], [\"0x4562cf\", \"call\", \"0x645398\"], [\"0x4562d4\", \"pop\", \"ecx\"], [\"0x4562d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4562e0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562e5\", \"jmp\", \"0x4594a0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/infowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??0InfoWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??1InfoWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f318; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?ConsoleInfoWin@@3VInfoWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x004562E0 _$E19 0x004562E0-0x004562EA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682530 selects initializer0x4562c0..0x4562d6, registering exit0x4562e0..0x4562ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7ad2a0 bound to?ConsoleInfoWin@@3VInfoWin@@A.", "Original startup instruction evidence: [[\"0x4562c0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562c5\", \"call\", \"0x459500\"], [\"0x4562ca\", \"push\", \"0x4562e0\"], [\"0x4562cf\", \"call\", \"0x645398\"], [\"0x4562d4\", \"pop\", \"ecx\"], [\"0x4562d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4562e0\", \"mov\", \"ecx, 0x7ad2a0\"], [\"0x4562e5\", \"jmp\", \"0x4594a0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/infowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??0InfoWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?ConsoleInfoWin@@3VInfoWin@@A\", \"??1InfoWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f318; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?ConsoleInfoWin@@3VInfoWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
#include "stdafx.h"
#include "interface_fonts.h"
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
#include "alpha.h"
#include "pulldown.h"
#include "maininterface.h"
#include "mapwin.h"
#include "terraforming.h"
#include "socialwin.h"
#include "technology.h"
#include <cstring>

// Process-wide InfoWin lifecycle; original table callback 0x004562C0
// constructs the object at 0x007AD2A0 and registers its teardown.
// RECOVERY_OBJECT: v1 0x007AD2A0 ?ConsoleInfoWin@@3VInfoWin@@A
InfoWin ConsoleInfoWin;
void __cdecl help_any();

static void infowin_append_label(char *out, int index) {
    strcat(out, label_get(index));
}

static void infowin_append_number(char *out, int value) {
    char number[32];
    _itoa(value, number, 10);
    strcat(out, number);
}

static void infowin_write_faction_row(char *text, char *destination,
                                      int label, int selected) {
    text[0] = 0;
    infowin_append_label(text, label);
    strcat(text, ": {");
    GenderDefault = Players[selected].is_leader_female;
    PluralityDefault = 0;
    strcat(text, Players[selected].title_leader);
    strcat(text, " ");
    GenderDefault = Players[selected].is_leader_female;
    PluralityDefault = 0;
    strcat(text, Players[selected].name_leader);
    strcat(text, "}");
    strcpy(destination, text);
}

static int infowin_best_stat_faction(int current_faction, uint8_t alive,
                                     int field) {
    int best_value = 0;
    int best_faction = current_faction;
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if ((alive & (1u << candidate)) == 0) continue;
        int value;
        switch (field) {
        case 0: value = static_cast<int>(PlayersData[candidate].tech_ranking); break;
        case 1: value = PlayersData[candidate].mil_strength_1; break;
        case 2: value = PlayersData[candidate].pop_total; break;
        default: value = PlayersData[candidate].energy_reserves; break;
        }
        // Native signed compare accepts equality, so later factions win ties.
        if (best_value <= value) {
            best_value = value;
            best_faction = candidate;
        }
    }
    return best_faction;
}

// ORIGINAL: 0x00456300 ?setup_text@InfoWin@@QAEXXZ 0x00456300-0x0045838D
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00456300.cpp; pinned body00456300..0045838D", "Native eight-way jump table004583A8; pane-state normalization is signed remainder modulo8. Font+0C is natural line height, separately from adjusted row pitch.", "Native ranking scores use indices1..7; history reads signed16 samples with incrementing turn stride/divisor. Best-stat ties select later living factions, region sums use indices5..66.", "Rules string fields are real heap pointers produced by text_item_string/Strings::put; Strings::get retains a historical32-bit integer address ABI. Explicit call-boundary conversions do not make these fields ordinal indices.", "recovery/tasks/infowin-ownership.json; complete-owner scope expansion retained; actual-source host damage controls and fresh native compilation required"], "reason": "Recover the complete information-pane text and chart builder through genuine InfoWin rows, Font metrics, faction/rules storage and Buffer graphics. Preserve the existing original identity, seed and historical obligations; this binding is not a matching, runtime or acceptance claim."}
// TRIED: frame size does not match 0x504 (many named char[256]/int temps vs the original's smaller register-heavy layout); the 25/19/11-entry jump tables were treated as compiler-generated unwind dispatch (never reproduced) rather than real cases 8-24; CharUpperA and several graphics helpers in the case 4/5 ranking bar-chart panel were stubbed rather than fully modelled.
void InfoWin::setup_text() {
    const int faction = ConsoleGlobal.faction_id_;
    const int font_metric = font_.unk_1_;
    const int natural_line_height = font_.line_height(); // authorized narrow Font accessor
    const int natural_height = font_.pixel_height();
    const int row_pitch = font_metric < 0
        ? natural_line_height : natural_height + font_metric;

    // Native 0x00456333..0x0045635B leaves this local uninitialized for
    // states 5, 7 and values outside [3,8]. Preserve that defect unless a
    // behavior correction is explicitly approved; caller restrictions are
    // not yet established. A positive caption scale bypasses its use.
    int mode;
    switch (static_cast<int>(field_B0_)) {
    case 3:
    case 4: mode = -12; break;
    case 6: mode = -9; break;
    case 8: mode = -6; break;
    }

    for (int clear_pane = 0; clear_pane != 2; ++clear_pane) {
        row_count_[clear_pane] = 0;
        for (int row = 0; row != 12; ++row) {
            row_text_[clear_pane][row][0] = 0;
            row_x_[clear_pane][row] = pane_rects_[clear_pane].left + 5;
            row_y_[clear_pane][row] = pane_rects_[clear_pane].top + 5 + row * row_pitch;
            row_color_[clear_pane][row] = 0;
            row_reveal_color_[clear_pane][row] = 0;
        }
    }

    const int scale = MapWinCaptionScale;
    const int divisor = MapWinCaptionDivisor;
    int line_height;
    int line_height_100;
    if (scale > 0) {
        line_height = scale * 75 / divisor;
        line_height_100 = scale * 100 / divisor;
    } else {
        const int h = (mode * 3 + 0x30) * 0x19;
        line_height = h / 16;
        const int h100 = (mode + 0x10) * 100;
        line_height_100 = h100 / 16;
    }
    // The common tail retains the second pane's final initialization
    // coordinates, not the Font height. Its conditional store is to X.
    const int shifted_row_x = line_height_100 + pane_rects_[1].left + 5 + 2;
    const int final_initial_row_y = pane_rects_[1].top + 5 + 12 * row_pitch;

    // The native tail rereads the selected pane after calls: a callback
    // selecting the second pane while the first is drawn must include it.
    for (int pane = active_pane_; pane <= active_pane_; ++pane) {
        RECT frame = pane_rects_[pane];
        MainInterfaceGlobal.box_sprite(&frame, &InterfaceBoxStyles[18]);
        RECT clip = frame;
        clip.left += 3;
        clip.top += 3;
        clip.right -= 3;
        clip.bottom -= 3;

        char text[256];
        int content_line_height = line_height;
        int displayed_rows = 0;
        Buffer &canvas = MainInterfaceGlobal;

        const int kind = pane_states_[pane] % 8;
        switch (kind) {
        case 0: {
            text[0] = 0;
            content_line_height = 0;
            infowin_append_label(text, 0);
            strcat(text, ": ");
            strcat(text, label_get(0x63));
            say_year(text);
            strcat(text, "{");
            strcpy(row_text_[pane][0], text);

            energy_compute(faction, 0);
            const int current_tech = PlayersData[faction].tech_id_researching;
            if (current_tech >= 0 &&
                ((GameDataStateStorage.fields_.game_state_ & 0x80u) != 0 || (GameDataStateStorage.fields_.game_rules_ & 0x200u) == 0)) {
                text[0] = 0;
                infowin_append_label(text, 0x6a);
                strcat(text, ": ");
                strcat(text, label_get(0x15));
                strcat(text, StringTable.get(Technology[current_tech].name));
                strcat(text, "}");
                strcpy(row_text_[pane][1], text);
                displayed_rows = 2;
            } else {
                displayed_rows = 1;
            }

            text[0] = 0;
            strcat(text, StringTable.get(Energy[0].abbrev));
            strcat(text, ": ");
            strcat(text, "{");
            infowin_append_number(text,
                (10 - PlayersData[faction].se_alloc_labs
                     - PlayersData[faction].se_alloc_psych) * 10);
            strcat(text, "%");
            strcat(text, "  (");
            if (NetIncomeGlobal >= 0) strcat(text, "+");
            infowin_append_number(text, NetIncomeGlobal);
            strcat(text, " ");
            infowin_append_label(text, 0x30);
            strcat(text, ")}");
            strcpy(row_text_[pane][2], text);

            text[0] = 0;
            strcat(text, StringTable.get(Energy[1].abbrev));
            strcat(text, ": ");
            strcat(text, "{");
            infowin_append_number(text, PlayersData[faction].se_alloc_psych * 10);
            strcat(text, "%}");
            strcpy(row_text_[pane][3], text);
            text[0] = 0;
            strcat(text, StringTable.get(Energy[2].abbrev));
            strcat(text, ": {");
            infowin_append_number(text, PlayersData[faction].se_alloc_labs * 10);
            strcat(text, "%");
            if (BreakthroughTurnsGlobal < 9000) {
                strcat(text, "  (");
                infowin_append_number(text, BreakthroughTurnsGlobal);
                strcat(text, " ");
                infowin_append_label(text, 0x31);
                strcat(text, ")");
            }
            strcat(text, "}");
            strcpy(row_text_[pane][4], text);
            displayed_rows = 5;
            break;
        }
        case 1: {
            content_line_height = 0;
            energy_compute(faction, 0);
            text[0] = 0;
            infowin_append_label(text, 300);
            strcat(text, ": ");
            strcat(text, "{");
            infowin_append_number(text,
                PlayersData[faction].tech_commerce_bonus + 1);
            strcat(text, "}");
            strcpy(row_text_[pane][0], text);

            text[0] = 0;
            infowin_append_label(text, 0x12d);
            strcat(text, ": ");
            infowin_append_number(text,
                PlayersData[faction].turn_commerce_income);
            strcat(text, label_get(0x130));
            strcat(text, "}");
            strcpy(row_text_[pane][1], text);

            text[0] = 0;
            infowin_append_label(text, 0x12e);
            strcat(text, ": ");
            infowin_append_number(text, EnergyEconomyTotal);
            strcat(text, label_get(0x130));
            strcat(text, "}");
            strcpy(row_text_[pane][2], text);

            int total_row = 3;
            if (Language == 0) {
                text[0] = 0;
                strcat(text, "Total Costs: ");
                infowin_append_number(text, NetIncomeGlobal - EnergyEconomyTotal);
                strcat(text, label_get(0x130));
                strcat(text, "}");
                strcpy(row_text_[pane][3], text);
                total_row = 4;
            }
            text[0] = 0;
            infowin_append_label(text, 0x3bf);
            strcat(text, ": ");
            infowin_append_number(text, NetIncomeGlobal);
            strcat(text, label_get(0x130));
            strcat(text, "}");
            strcpy(row_text_[pane][total_row], text);
            displayed_rows = total_row + 1;
            break;
        }
        case 2: {
            content_line_height = 0;
            const int other = static_cast<int>(DiploFrictionFactionID);
            text[0] = 0;
            GenderDefault = Players[other].noun_gender;
            PluralityDefault = Players[other].is_noun_plural;
            strcat(text, Players[other].formal_name_faction);
            strcpy(row_text_[pane][0], text);

            text[0] = 0;
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(text, Players[faction].title_leader);
            strcat(text, " ");
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(text, Players[faction].name_leader);
            strcpy(row_text_[pane][1], text);

            text[0] = 0;
            const int mood = static_cast<int>(get_mood(DiploFriction));
            say_special(text, StringTable.get(Mood[mood]), other);
            strcpy(row_text_[pane][2], text);

            text[0] = 0;
            const uint32_t treaty = PlayersData[faction].diplo_treaties[other];
            int treaty_label = -1;
            if (treaty & 1) treaty_label = 0xd3;
            else if (treaty & 2) treaty_label = 0xd4;
            else if (treaty & 4) treaty_label = 0xd5;
            else if (treaty & 0x10) treaty_label = 0xd6;
            if (treaty_label >= 0) infowin_append_label(text, treaty_label);
            strcpy(row_text_[pane][3], text);
            displayed_rows = 4;
            break;
        }
        case 3: {
            content_line_height = 0;
            text[0] = 0;
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            strcat(text, Players[faction].formal_name_faction);
            strcpy(row_text_[pane][0], text);

            text[0] = 0;
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(text, Players[faction].title_leader);
            strcat(text, " ");
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            strcat(text, Players[faction].name_leader);
            strcpy(row_text_[pane][1], text);

            text[0] = 0;
            infowin_append_label(text, 0xde);
            strcat(text, ": ");
            {
                int rank = PlayersData[faction].integrity_blemishes;
                if (rank < 0) rank = 0;
                else if (rank > 7) rank = 7;
                say_special(text,
                            StringTable.get(Repute[rank]), faction);
            }
            strcat(text, "}");
            strcpy(row_text_[pane][2], text);

            text[0] = 0;
            infowin_append_label(text, 0xdf);
            strcat(text, ": ");
            {
                int gap = PlayersData[faction].ranking;
                if (gap < 1) gap = 1;
                else if (gap > 7) gap = 7;
                say_special(text,
                            StringTable.get(Might[7 - gap].adj), faction);
            }
            strcat(text, "}");
            strcpy(row_text_[pane][3], text);

            text[0] = 0;
            infowin_append_label(text, 0x3c0);
            strcat(text, ": ");
            infowin_append_number(text, council_votes(faction));
            strcat(text, "}");
            strcpy(row_text_[pane][4], text);
            displayed_rows = 5;
            break;
        }
        case 4: {
            content_line_height = 0;
            canvas.set_clip(&clip);
            text[0] = 0;
            if ((ScenarioSave.objective_req_victory < 9000 || ScenarioSave.objectives_sudden_death_victory < 9000) &&
                ScenarioSave.objective_achieve_points == 0) {
                infowin_append_label(text, 899);
                CharUpperA(text);
            } else {
                infowin_append_label(text, 0x1b8);
            }
            strcpy(row_text_[pane][0], text);

            rankings(0);

            int maximum = 1;
            for (int rank_index = 1; rank_index < MaxPlayerNum; ++rank_index) {
                const int score = static_cast<int>(FactionRankingsUnk[rank_index]);
                if (score > maximum) maximum = score;
            }
            const int plot_height = clip.bottom - clip.top - natural_line_height - 4;
            int y_divisor = 1;
            int scaled_plot_height = plot_height;
            while (scaled_plot_height < maximum) {
                scaled_plot_height *= 2;
                ++y_divisor;
            }

            int turn_step = 1;
            while (GameDataStateStorage.fields_.turn_current_num_ / turn_step > clip.right - clip.left) ++turn_step;
            for (int faction_index = 1; faction_index < MaxPlayerNum; ++faction_index) {
                int turn = 1;
                int x = clip.left;
                int previous_y = clip.bottom - 1;
                bool have_previous = false;
                while (turn <= GameDataStateStorage.fields_.turn_current_num_) {
                    const int history_index = turn * MaxPlayerNum + faction_index;
                    const int score = static_cast<int16_t>(FactionRankingHistory[history_index]);
                    const int y = clip.bottom - 1 - score / y_divisor;
                    if (turn > 1 && have_previous) {
                        const int color =
                            strstr(Players[faction_index].search_key, "SPARTANS") != 0
                            ? InfoFactionTextColors[faction_index]
                            : InfoFactionShadowColors[faction_index];
                        canvas.line(x - 1, previous_y, x, y, color);
                    }
                    previous_y = y;
                    have_previous = true;
                    turn += turn_step;
                    ++x;
                }
            }

            // Native sets its row-count local to 8 and iterates FactionRankings[7]
            // down through [0], storing names at rows 1..8. Row 8 is consequently
            // stored but not included by the final count pass.
            for (int rank_row = 0; rank_row < MaxPlayerNum; ++rank_row) {
                const int faction_index = static_cast<int>(
                    GameDataStateStorage.fields_.faction_rankings_[MaxPlayerNum - 1 - rank_row]);
                GenderDefault = Players[faction_index].noun_gender;
                PluralityDefault = Players[faction_index].is_noun_plural;
                strcpy(text, Players[faction_index].noun_faction);
                const int maximum_text_width = clip.right - 2 * clip.left - 2;
                int length = static_cast<int>(strlen(text));
                while (length > 0 && canvas.text_width(text) > maximum_text_width) {
                    --length;
                    text[length] = 0;
                }
                strcpy(row_text_[pane][rank_row + 1], text);
            }
            displayed_rows = 8;
            canvas.set_clip(const_cast<RECT *>(&canvas.clip_rect()));
            break;
        }

        // ---- case 5: native 0x4578EB..0x457B6C ----
        case 5: {
            content_line_height = 0;
            canvas.set_clip(&clip);
            text[0] = 0;
            if ((ScenarioSave.objective_req_victory < 9000 || ScenarioSave.objectives_sudden_death_victory < 9000) &&
                ScenarioSave.objective_achieve_points == 0) {
                infowin_append_label(text, 899);
                CharUpperA(text);
            } else {
                infowin_append_label(text, 0x1b8);
            }
            strcpy(row_text_[pane][0], text);
            // Match native's source call/order and score floor.
            rankings(0);
            int maximum = 1;
            for (int rank_index = 1; rank_index < MaxPlayerNum; ++rank_index) {
                const int score = static_cast<int>(FactionRankingsUnk[rank_index]);
                if (score > maximum) maximum = score;
            }

            const int plot_width = clip.right - clip.left - natural_line_height - 4;
            const int bar_width = plot_width / 7;
            const int left = clip.left +
                ((clip.right - clip.left) - bar_width * 7) / 2;
            const int plot_height = clip.bottom - clip.top - natural_line_height - 4;
            const int bottom = clip.bottom - 2;
            for (int faction_index = 1; faction_index < MaxPlayerNum; ++faction_index) {
                int bar_height = static_cast<int>(FactionRankingsUnk[faction_index]) *
                                 plot_height / maximum;
                if (bar_height < 1) bar_height = 1;
                RECT bar;
                bar.left = left + (faction_index - 1) * bar_width;
                bar.right = left + faction_index * bar_width;
                bar.bottom = bottom;
                bar.top = bottom - bar_height;
                canvas.box(&bar, InfoFactionShadowColors[faction_index],
                           InfoFactionShadowColors[faction_index]);
                RECT inner = bar;
                ++inner.left;
                ++inner.top;
                --inner.right;
                --inner.bottom;
                canvas.fill(&inner, InfoFactionTextColors[faction_index]);

                GenderDefault = Players[faction_index].noun_gender;
                PluralityDefault = Players[faction_index].is_noun_plural;
                strcpy(text, Players[faction_index].noun_faction);
                canvas.set_font(&font_, 0, 0, 0);
                canvas.set_text_color(ChatItemIdTable[faction_index],
                                      ChatShadowColors[faction_index], 1, 1);
                canvas.write_cent_l(text, &bar, static_cast<int>(strlen(text)));
            }
            canvas.set_clip(const_cast<RECT *>(&canvas.clip_rect()));
            // Native sets its shared row-count local to 1 at 0x45797E. The common
            // 0x4582BC tail publishes that count and consumes row 0 (the heading).
            displayed_rows = 1;
            break;
        }

        case 6: {
            content_line_height = 0;
            const int live_faction = ConsoleGlobal.faction_id_;
            const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
            infowin_write_faction_row(text, row_text_[pane][0], 0x41c / 4,
                infowin_best_stat_faction(live_faction, alive, 0));
            infowin_write_faction_row(text, row_text_[pane][1], 0x420 / 4,
                infowin_best_stat_faction(live_faction, alive, 1));
            infowin_write_faction_row(text, row_text_[pane][2], 0x424 / 4,
                infowin_best_stat_faction(live_faction, alive, 2));
            infowin_write_faction_row(text, row_text_[pane][3], 0x4f0 / 4,
                infowin_best_stat_faction(live_faction, alive, 3));

            int best_sum = 0;
            int best_sum_faction = live_faction;
            for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
                if ((alive & (1u << candidate)) == 0) continue;
                int sum = 0;
                for (int sample = 1; sample < 63; ++sample)
                    sum += PlayersData[candidate].unk_78[4 + sample];
                if (best_sum <= sum) {
                    best_sum = sum;
                    best_sum_faction = candidate;
                }
            }
            infowin_write_faction_row(text, row_text_[pane][4], 0x97c / 4,
                                      best_sum_faction);
            displayed_rows = 5;

            if (GameDataStateStorage.fields_.ranking_faction_id_unk1_ > 0) {
                infowin_write_faction_row(text, row_text_[pane][5], 0x428 / 4,
                                          static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7]));
                displayed_rows = 6;
            }
            break;
        }

        case 7: {
            content_line_height = 0;

            text[0] = 0;
            infowin_append_label(text, 0x1ab);
            strcpy(row_text_[pane][0], text);

            text[0] = 0;
            infowin_append_label(text, 0x1aa);
            strcat(text, "{");
            strcat(text, MapFilePath);
            strcat(text, "}");
            strcpy(row_text_[pane][1], text);

            text[0] = 0;
            infowin_append_label(text, 0x1a9);
            strcat(text, ": {");
            const char *paint_name;
            switch (MapEditorPaintId) {
            case 0: paint_name = label_get(0xEBC / 4); break;
            case 1: paint_name = label_get(0xEC0 / 4); break;
            case 2: paint_name = label_get(0xEC4 / 4); break;
            case 3: paint_name = label_get(0xEC8 / 4); break;
            case 4: paint_name = label_get(0xECC / 4); break;
            case 5: paint_name = label_get(0xED0 / 4); break;
            case 26:
                paint_name = StringTable.get(Natural[2].name_short);
                break;
            case 27:
                paint_name = StringTable.get(Natural[6].name_short);
                break;
            case 28:
                paint_name = StringTable.get(Natural[3].name_short);
                break;
            case 29:
                paint_name = StringTable.get(Natural[10].name_short);
                break;
            default:
                paint_name = StringTable.get(
                    Terraforming[MapEditorPaintId - 6].name);
                break;
            }
            strcat(text, paint_name);
            strcat(text, "}");
            strcpy(row_text_[pane][2], text);

            text[0] = 0;
            infowin_append_label(text, 0x684 / 4);
            strcat(text, ": ");
            if (MapEditorBrushId == 1) {
                infowin_append_label(text, 0x688 / 4);
            } else {
                infowin_append_number(text,
                    MapEditorBrushId == 0 ? 0 : MapEditorBrushId - 1);
            }
            strcpy(row_text_[pane][3], text);

            text[0] = 0;
            infowin_append_label(text, 0xD7C / 4);
            strcat(text, ": ");
            infowin_append_number(text, MapLongitudeBounds / 2);
            strcat(text, "x");
            infowin_append_number(text, MapLatitudeBounds);
            strcat(text, " ");
            infowin_append_label(text,
                ((MapIsFlat & 1) != 0 ? 0xD80 : 0xD84) / 4);
            strcpy(row_text_[pane][4], text);

            text[0] = 0;
            infowin_append_label(text, 0x84 / 4);
            strcat(text, ": {");
            infowin_append_label(text, 0x14 + static_cast<int>(MapCloudCover));
            strcat(text, "}");
            strcpy(row_text_[pane][5], text);

            displayed_rows = 6;
            break;
        }

        }

        row_count_[pane] = displayed_rows;
        text_lengths_[pane] = 0;
        for (int row = 0; row < displayed_rows; ++row) {
            if (content_line_height != 0 &&
                row_y_[pane][row] < content_line_height + final_initial_row_y + 2) {
                row_x_[pane][row] = shifted_row_x;
            }
            text_lengths_[pane] += static_cast<int>(strlen(row_text_[pane][row]));
        }
    }
}

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
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/00458ff0.cpp", "source:src/infowin.h", "source:src/game.cpp:InterfaceBoxStyles", "pinned:0x004590C7-0x004590DF clears 0x007AF174, 0x007AF17C, 0x007AF178 and 0x007AF180 after successful state_valid", "source:MainInterfaceGlobal at 0x007AE820 has its GraphicWin Buffer subobject at +0x444; those destinations are Buffer +0x510, +0x518, +0x514 and +0x51C", "source:Buffer::reset_text_state clears the existing fields in that original order; InfoWin::draw_text uses the same owner operation"], "reason": "Home the timed pane transition with owned InfoWin state, pane rectangle and box style. The former animation-counter alias actually addressed MainInterfaceGlobal's Buffer text state; reset those members after the successful validity loop, preserving the original store order and callbacks."}
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

    MainInterfaceGlobal.reset_text_state();
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
    // The pin's [EBP-0xC] slot is function-scoped: 0x4586E4/0x458729 assign it,
    // while the zero-budget path skips those stores and reloads it at 0x4587FE.
    // Exact exhaustion lets the next row reuse the prior partial row's x; an
    // entry-zero path with no prior assignment remains uninitialized.
    int x;
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
                    int faction = GameDataStateStorage.fields_.faction_rankings_[8 - row];
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
                    // The original only skips the first, `remaining - 1`
                    // prefix write at zero. It still copies/terminates the
                    // prefix and reaches the final-character path below,
                    // where `prefix + remaining - 1` is invalid. The persistent
                    // x is reused from an earlier partial row if present; it is
                    // uninitialized only when a zero budget is the first use.
                    // Preserve that path rather than normalizing it.
                    char prefix[80];
                    if (remaining > 0) {
                        strcpy(prefix, text);
                        prefix[remaining - 1] = 0;
                        x = ranked ? right - left - buffer.text_width(text) - 2
                                   : row_x_[column][row];
                        x = buffer.write_l(prefix, x, y, strlen(prefix));
                    }
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
// ORIGINAL: 0x00458F10 ?set_state@InfoWin@@QAEXHH@Z 0x00458F10-0x00458FEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00458f10.cpp", "source:src/infowin.h", "source:src/infowin.cpp:state_valid"], "reason": "Replace the offset-based seed with the InfoWin's active-pane and presentation-state members and its existing eligibility method."}
void InfoWin::set_state(int state, int a2) {
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
    visible_characters_[0] = a2 != 0 ? 0 : text_lengths_[0] + 1;
    visible_characters_[1] = a2 != 0 ? 0 : text_lengths_[1] + 1;
    field_A8_ = 0;
    MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                   &InterfaceBoxStyles[18]);
    // The original tests the address of the pane-state array, which is always
    // non-null for a valid object, then clears both values it just assigned.
    visible_characters_[0] = 0;
    visible_characters_[1] = 0;
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
#pragma auto_inline(off)
// ORIGINAL: 0x004589C0 ?right_menu@InfoWin@@QAEXHH@Z 0x004589C0-0x00458D3A;0x00655280-0x00655295
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004589C0-0x00458D3A", "image:0x00655280-0x00655295", "git:e3121de9:src/recovered/units/004589c0.cpp", "source:src/pulldown.h:get_selected_id"], "reason": "Home the information-pane context menu with the real PullDown item-ID API, source-owned label table, pane state and MainInterface renderer; retain the original secondary EH span as a restoration obligation."}
void InfoWin::right_menu(int x, int y) {
    MainInterfaceGlobal.client_to_screen(&x, &y);
    PullDown menu;
    menu.init(0);
    static const int label_slots[8] = {
        0x92C / 4, 0x930 / 4, 0x934 / 4, 0x938 / 4,
        0x948 / 4, 0x94C / 4, 0x950 / 4, 0x954 / 4
    };
    for (int state = 0; state != 8; ++state) {
        if (state_valid(state)) menu.add_item(state, label_get(label_slots[state]));
    }
    menu.add_separator(99);
    menu.add_item(20, label_get(0x8FC / 4));
    menu.show(x, y, pop_expire);

    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_800) != 0) return;
    const int selection = menu.get_selected_id();
    if (selection >= 0 && selection <= 16 && state_valid(selection)) {
        pane_states_[active_pane_] = selection;
        while (!state_valid(pane_states_[active_pane_])) {
            pane_states_[active_pane_] = (pane_states_[active_pane_] + 1) % 8;
        }
        field_94_ = 0;
        visible_characters_[0] = text_lengths_[0] + 1;
        visible_characters_[1] = text_lengths_[1] + 1;
        field_A8_ = 0;
        MainInterfaceGlobal.box_sprite(&pane_rects_[active_pane_],
                                       &InterfaceBoxStyles[18]);
        visible_characters_[0] = 0;
        visible_characters_[1] = 0;
        setup_text();
        draw_text(active_pane_);
    } else if (selection == 20) {
        help_any();
    }
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
