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
// ORIGINAL: 0x004AD840 ??_GReportWin@@UAEPAXI@Z 0x004AD840-0x004AD861 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004AD840-0x004AD861", "Pinned class constructor ??0ReportWin@@QAE@XZ at 0x004AD6B0 stores original class/view table VA 0x66d754 at 0x004AD81A (mov dword ptr [esi], 0x66d754); the current genuine owner vftable ??_7ReportWin@@6BWin@@@ selects generated ??_GReportWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EReportWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GReportWin@@UAEPAXI@Z at 0x004AD840 calls the source-mapped ??1ReportWin@@QAE@XZ at 0x004AD3B0, then tests byte ptr [ebp + 8], 1; je 0x4ad85a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/reportwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original ReportWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GReportWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include <math.h>
#include "random.h"
#include "basewin.h"
#include "infowin.h"
#include "popup.h"
#include "temp.h"
#include "reportwin.h"
#include "reportif.h"  // ReportIfGlobal - the singleton on_key_click re-reports
#include "menu.h"
#include "win.h"
#include "sprite.h"  // g_IFACE_TECH_TREE_ARROW_SPRITES - labs_line's arrow
#include "datalink.h"  // DatalinkMain - the research screen's host window
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "tutwin.h"  // TutWinBaseWindow - the report's base-map zoom target
#include "vector_teardown.h"
#include "alpha.h"
#include "socialwin.h"
#include "console.h"
#include "technology.h"
#include "game.h"
#include "base.h"
#include "netdaemon.h"
#include "pulldown.h"
#include "faction.h"
#include "log.h"
#include "veh.h"
#include "worldwin.h"
#include "map.h"
#include "general.h"
#include "council.h"
#include "text.h"

// Shared dependencies identified by the original animation calls. The daemon
// owners already live in leaf_recoveries.cpp; action_sat_attack and the colour
// table require coordinator-owned publication before a complete link.
void __cdecl sat_anim_daemon(ReportWin *win);
void __cdecl sat_expl_daemon(ReportWin *win);
void __cdecl bl_daemon(ReportIf *report);
void __cdecl action_sat_attack(int attacker, int target, int type, int base_id);
extern Sprite g_TECH_ICON_SPRITES[89];
// Original init passes these two independently addressed empty captions at
// 496A80/496B64. Their initial NUL is proven; complete original object extents
// are not, so these start-only bindings do not claim four-byte storage.
// RECOVERY_OBJECT: v1 0x008A6244 ?ReportWindowCaptionSmall@@3PADA
char ReportWindowCaptionSmall[] = "";
// RECOVERY_OBJECT: v1 0x008A6248 ?ReportWindowCaptionLarge@@3PADA
char ReportWindowCaptionLarge[] = "";
void __cdecl sat_attack(int faction_id, int target_id, int type);
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl draw_radius(int x, int y, int mode, int draw_type);
void __cdecl sat_edit(int faction_id, int type, int flags);
int __cdecl break_treaty(int faction_id, int target_faction_id, int treaty);
int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
// Original 0x00590910 is the real source-owned facility text formatter;
// its void-pointer parameter is part of the emitted PAX symbol.
void __cdecl say_facility(void *text, int facility_id, int with_cost);

// Report page dispatch at 0x00496D35 uses the original table at 0x00496DE4:
// 0 intel; 1 no drawer; 2 labs; 3 energy; 4 ops; 5 projects; 6 satellites;
// 7 security; 8 score. The retired explicit-receiver helper shifted these
// categories and forwarded to original addresses. This is Win's genuine
// slot-12 override, with the owned Buffer and ReportIf's category state.
/*
Purpose: Repaint the report window's selected page inside the topic panel.
// ORIGINAL: 0x00496D00 ?on_redraw@ReportWin@@QAEXXZ 0x00496D00-0x00496DE4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original496D00..496DE4; retired seed git:117282ee:src/unrecovered/00496d00.cpp records the same identity and spans with its switch fold TRIED note kept below.", "The receiver is reportwin.h's own window: reset() is declared there (0x00496BB0), the +0x444 Buffer (set_clip is buffer.h's own) carries the topic rectangle at +0x5974, and the page selector is 0x00885F40.", "The eight page drawers are the seed's measured call targets 0x00498B30/0x0049BCD0/0x0049D540/0x0049EB10/0x0049F350/0x004A08E0/0x004A1140/0x004A2210, unrecovered members reached at their original addresses until declarations land."], "reason": "Coverage transcription of the report repaint with the page dispatch kept at the measured switch shape."}
// symbol    ?on_redraw@ReportWin@@UAEXXZ
// TRIED (seed): a switch on the 9 cases reproduces the disassembly's eight
//         separate trailing set_clip copies; VC6 does not tail-merge across
//         switch jump-table targets.
Status: Coverage transcription - unclaimed body
*/
void ReportWin::on_redraw() {
    reset();
    field_59AC_ = -1;
    Buffer &canvas = *this;
    canvas.set_clip(&energy_panel_rect_);
    switch (ReportIfGlobal.field_8_) {
    case 0: draw_intel(); break;
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    }
    canvas.set_clip(buffer_working_rect());
}

static bool reportwin_input_blocked() {
    return ConsoleControlTurnActive != 0
        || (NetDaemonNet.lock_state_[0].trailing_words_[7]
            & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0
        || NetDaemonNet.lock_state_[2].payload_.flags != 0
        || NetDaemonNet.lock_state_[0].leading_words_[3] != 0;
}

static bool reportwin_security_filter(int prototype_id, int filter) {
    if (filter >= 4) return true;
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    if (filter < 3) return Chassis[prototype.chassis_id].triad == filter
        && Weapon[prototype.weapon_id].offense_rating != 0;
    return Weapon[prototype.weapon_id].offense_rating == 0;
}

static void reportwin_append_number(char *text, int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(text, number);
}

// The tree's four upper-tier buttons are fed by at most four matching
// prerequisite slots. The image's loop notices a fifth match and stops
// (0x4996CA..0x49970B), then clamps the displayed count to four
// (0x499AE2..0x499AF3). Keep that native stop condition without writing the
// fifth result past the four-button local destination.
static int reportwin_collect_lab_children(int technology, int *children) {
    int count = 0;
    // 0x4996CA exits when the number of matches is greater than four. The
    // fifth match is a stop signal only; only four child IDs have storage.
    for (int slot = 0; slot < MaxTechnologyNum * 2 && count <= 4; ++slot) {
        const int candidate = slot / 2;
        const int prerequisite = (slot & 1)
            ? Technology[candidate].preq_tech_2
            : Technology[candidate].preq_tech_1;
        if (prerequisite == technology) {
            if (count < 4) children[count] = candidate;
            ++count;
        }
    }
    return count;
}

// 0x004997A1..7CB, 0x004998D7..9995, 0x004999A0..A9A, and the
// repeated child-button setup 0x00499B80..C0F all use BaseButton::init with
// the same font, earned-tech emphasis and three border-style records.
static void reportwin_init_lab_button(Buffer &canvas, FlatButton &button,
                                     int technology,
                                     int button_id, int x, int y,
                                     int width, int height, Win *parent,
                                     int faction) {
    StringTemp[0] = 0;
    say_tech(StringTemp, technology, 0);
    button.init(StringTemp, button_id, x, y, width, height, parent, 0);
    button.set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    if (has_tech(technology, faction)) {
        button.set_text_color(0xFF, -1, 1, 1);
    }
    button.set_border_styles(&InterfaceBoxStyles[27], &InterfaceBoxStyles[28],
                             &InterfaceBoxStyles[29]);
    button.redraw();
    RECT frame = {x - 3, y - 3, x + width + 3, y + height + 3};
    canvas.box_sprite(&frame, &InterfaceBoxStyles[3]);
}

// 0x499FE3..0x49A389 performs a measure pass, then a centered draw pass, for
// the selected technology's two status lines. Their string pieces are the
// native bracket literals, `say_tech`, the category label at StringTable
// offset 0x209+category, and the prerequisite-depth number.
static void reportwin_draw_lab_status_rows(Buffer &canvas, int technology,
                                           int faction, int x,
                                           int vertical_sum,
                                           uint32_t &frame_width,
                                           int &cursor_y,
                                           int &measured_height,
                                           bool draw_pass,
                                           bool child_panel,
                                           Font *first_row_font,
                                           Font *second_row_font) {
        StringTemp[0] = 0;
        canvas.set_font(first_row_font, first_row_font, first_row_font, 0);
        canvas.set_text_color(has_tech(technology, faction) ? 0xFF : 0xE1,
                              -1, 1, 1);
        strcat(StringTemp, "[");
        say_tech(StringTemp, technology, 0);
        strcat(StringTemp, "]");
        CharUpperA(StringTemp);
        if (draw_pass && child_panel) {
            const int center_y = (vertical_sum - measured_height) / 2;
            cursor_y = center_y;
            const int current_frame_width = static_cast<int>(frame_width);
            RECT outer = {x - 6, center_y - 9, x + current_frame_width,
                          measured_height + center_y + 6};
            canvas.box_sprite(&outer, &InterfaceBoxStyles[3]);
            RECT inner = {outer.left + 3, outer.top + 3,
                          outer.right - 3, outer.bottom - 3};
            canvas.box_sprite(&inner, &InterfaceBoxStyles[0]);
        }
        if (!draw_pass) {
            measured_height += canvas.wrap_height(StringTemp,
                static_cast<int>(frame_width) - 6);
        } else {
            cursor_y = canvas.wrap_cent(StringTemp, x, cursor_y,
                static_cast<int>(frame_width) - 6);
        }

        StringTemp[0] = 0;
        canvas.set_font(second_row_font, second_row_font, second_row_font, 0);
        strcat(StringTemp, "[");
        const int category = tech_category(technology);
        strcat(StringTemp, label_get(0x209 + category));
        strcat(StringTemp, " ");
        reportwin_append_number(StringTemp, tech_recurse(technology, 0));
        strcat(StringTemp, "]");
        canvas.set_text_color(mandate_color(category), -1, 1, 1);
        if (!draw_pass) {
            measured_height += canvas.wrap_height(StringTemp,
                static_cast<int>(frame_width) - 6);
        } else {
            cursor_y = canvas.wrap_cent(StringTemp, x, cursor_y,
                static_cast<int>(frame_width) - 6);
            if (child_panel) {
                Sprite &icon = g_TECH_ICON_SPRITES[technology];
                icon.draw_mono(&canvas, icon.transparent_index(),
                    x + (static_cast<int>(frame_width) - 0x34) / 2,
                    cursor_y + 3, mandate_color(category), 0x32,
                    icon.width());
            }
            cursor_y += 0x3A;
        }
        if (!draw_pass) measured_height += 0x3A;
}

static void reportwin_append_lab_entry(bool datalink, int &entry_count,
                                       const char *section, LPSTR name,
                                       bool name_in_string_table,
                                       int category, int index) {
    if (entry_count == 0) {
        strcat(StringTemp, section);
        strcat(StringTemp, ": ");
    } else {
        strcat(StringTemp, ", ");
    }
    LPSTR resolved_name = name_in_string_table
        ? StringTable.get(reinterpret_cast<int>(name)) : name;
    if (datalink && category != 0) {
        char id[32];
        DatalinkMain.construct_id(id, category, index);
        help_create_link(StringTemp, resolved_name, id);
    } else {
        strcat(StringTemp, resolved_name);
    }
    ++entry_count;
}

static void reportwin_finish_lab_section(Buffer &canvas, bool draw_pass,
                                         int x, int &y, uint32_t *frame_width,
                                         int &measured_height) {
    if (StringTemp[0] == 0) return;
    if (draw_pass) {
        y = canvas.wrap(StringTemp, x, y,
            static_cast<int>(*frame_width) - 6);
    } else {
        measured_height += canvas.wrap_height(StringTemp,
            static_cast<int>(*frame_width) - 6);
    }
    StringTemp[0] = 0;
}

// 0x49A5F2..0x49A654 formats RulesChassis bytes +0x48/+0x4A.
static void reportwin_append_chassis_lab_detail(int speed, int range) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (speed < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (range >= 3) {
        strcat(StringTemp, label_get(0x200 / 4));
    } else {
        reportwin_append_number(StringTemp, speed);
        strcat(StringTemp, label_get(0x200 / 4));
    }
    strcat(StringTemp, ")");
}

// 0x49A7C3..0x49A866 formats RulesWeapon signed +8 and mode byte +0xA.
static void reportwin_append_weapon_lab_detail(int offense, int mode) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (offense < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else if (mode >= 3) {
        strcat(StringTemp, label_get(0x200 / 4));
    } else {
        reportwin_append_number(StringTemp, offense);
        strcat(StringTemp, label_get(0x200 / 4));
    }
    strcat(StringTemp, ")");
}

// 0x49A9D7..0x49AA66 formats RulesArmor signed defense byte +8.
static void reportwin_append_armor_lab_detail(int defense) {
    strcat(StringTemp, " ");
    strcat(StringTemp, "(");
    if (defense < 0) {
        strcat(StringTemp, label_get(0x310 / 4));
    } else {
        reportwin_append_number(StringTemp, defense);
    }
    strcat(StringTemp, ")");
}

// Native section scans and their inclusive/exclusive bounds are visible at
// 0x49A389..0x49B162. They are intentionally not replaced by Max* counts:
// several original ranges stop before the end of their source arrays.
static void reportwin_draw_lab_unlock_sections(Buffer &canvas, int technology,
                                               bool datalink, bool draw_pass,
                                               int x, int &y,
                                               uint32_t *frame_width,
                                               int &measured_height) {
    int entries = 0;
    StringTemp[0] = 0;
    for (int i_reactor = 0; i_reactor < 4; ++i_reactor) {
        if (Reactor[i_reactor].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x234 / 4),
                                   Reactor[i_reactor].name, true,
                                   5, i_reactor + 1);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_chassis = 0; i_chassis < 9; ++i_chassis) {
        if (Chassis[i_chassis].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x120 / 4),
                                   Chassis[i_chassis].offsv_1_name, true,
                                   4, i_chassis);
        reportwin_append_chassis_lab_detail(static_cast<signed char>(Chassis[i_chassis].speed),
                                            Chassis[i_chassis].range);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_weapon = 0; i_weapon < 26; ++i_weapon) {
        if (Weapon[i_weapon].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x124 / 4),
                                   Weapon[i_weapon].name, true,
                                   6, i_weapon);
        reportwin_append_weapon_lab_detail(Weapon[i_weapon].offense_rating,
                                           Weapon[i_weapon].mode);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_armor = 0; i_armor < 14; ++i_armor) {
        if (Armor[i_armor].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x128 / 4),
                                   Armor[i_armor].name, true,
                                   7, i_armor);
        reportwin_append_armor_lab_detail(Armor[i_armor].defense_rating);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_ability = 0; i_ability < 29; ++i_ability) {
        if (Ability[i_ability].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x12C / 4),
                                   Ability[i_ability].name, true,
                                   8, i_ability);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    // 0x49AC42..0x49AD90 scans the 29 built-in vehicle prototypes after
    // abilities. The native loop initializes EBX to zero at 0x49AC42 and
    // increments it unconditionally at 0x49AD2B, so Datalink item IDs use
    // the prototype table index even when earlier records do not match.
    entries = 0;
    for (int i_prototype = 0; i_prototype < 29; ++i_prototype) {
        VehPrototype &prototype = VehPrototypes[i_prototype];
        if ((prototype.flags & 1) == 0
            || prototype.preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries,
                                   label_get(0x318 / 4),
                                   prototype.veh_name, false,
                                   3, i_prototype);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_facility = 1; i_facility < 134; ++i_facility) {
        if (Facility[i_facility].preq_tech != technology
            && Facility[i_facility].free_tech != technology) continue;
        const int category = i_facility >= 70 ? 0xB : 0xA;
        reportwin_append_lab_entry(datalink, entries, label_get(0x110 / 4),
                                   Facility[i_facility].name, true,
                                   category, i_facility);
        if (Facility[i_facility].free_tech == technology) {
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x4DC / 4));
            strcat(StringTemp, ")");
        }
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);

    entries = 0;
    for (int i_citizen = 0; i_citizen < 7; ++i_citizen) {
        if (Citizen[i_citizen].preq_tech != technology) continue;
        reportwin_append_lab_entry(datalink, entries, label_get(0x4CC / 4),
                                   Citizen[i_citizen].singular_name, true,
                                   0, i_citizen);
    }
    reportwin_finish_lab_section(canvas, draw_pass, x, y, frame_width, measured_height);
}

// The three-per-category records are existing InterfaceBoxStyles entries:
// native jump tables at 0x499199 and 0x499224 select 38+3*category and
// 36+3*category respectively.
static BoxSpriteParams *reportwin_lab_category_style(int category,
                                                      int first_style) {
    if (category < 0 || category > 3) return 0;
    return &InterfaceBoxStyles[first_style + category * 3];
}

// ORIGINAL: 0x00498B30 ?draw_labs@ReportWin@@QAEXXZ 0x00498B30-0x0049B1DC
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00498B30-0x0049B1DC", "git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00498b30.cpp", "source:src/recovered/units/00430b30.cpp"], "reason": "The ReportWin laboratory page is shared by the ordinary report and Datalink entry renderer. Recover its real Buffer hosts, FlatButton members, technology Sprite array and ReportIf fonts; original chart geometry, status rows and unlock-list scans identify these owners. Retain the historical seed for further matching and runtime verification."}
// TRIED: full pixel-geometry reconstruction of the lab-tier bar chart (labs_line/FUN_0049b450/vtable-slot draw calls with exact coordinate math across up to 15 rows and 4 switch-selected sprite tables) - this pass preserves top-level branch shape and call order, not the per-row layout arithmetic.
void ReportWin::draw_labs() {
    const int faction = ReportIfGlobal.report_faction_id_;
    const bool datalink = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    Buffer &canvas = datalink
        ? *static_cast<Buffer *>(&DatalinkMain)
        : *static_cast<Buffer *>(this);
    if (datalink) canvas.set_text_color(0xFE, -1, 1, 1);
    // Native 0x498B90 reads the four Datalink entry-clip edges. The ordinary
    // report instead reads rect1_ as left/top/width/height at 0x498C5A.
    RECT host_area;
    if (datalink) {
        host_area = DatalinkMain.entry_clip_rect();
    } else {
        host_area.left = rect1_.left;
        host_area.top = rect1_.top;
        host_area.right = rect1_.left + rect1_.right;
        host_area.bottom = rect1_.top + rect1_.bottom;
    }
    const int left = host_area.left + 3;
    const int top = host_area.top + 3;
    const int right = host_area.right - 3;
    const int bottom = host_area.bottom - 3;
    const int width = right - left;
    const int height = bottom - top;
    // These are the button-cell dimensions published before chart setup;
    // they differ when the ordinary renderer is embedded by Datalink.
    field_59A0_ = datalink ? 0x96 : 0xBE;
    field_59A4_ = 0x28;
    field_59A8_ = datalink ? 0xAA : 0xBE;
    // Chart anchors are prepared before its controls and connector graph.
    // Native 0x498B90..C55 uses the Datalink entry clip; 0x498C5A..D1A
    // uses ReportIf's established wide-screen state and ReportWin::rect1_.
    const bool wide_layout = !datalink
        && ReportIfGlobal.field_1E230_ != 0;
    if (datalink) {
        field_A2C_ = top + 0x1E;
        field_A28_ = host_area.left + 3
            + ((host_area.right - host_area.left - 0xB0) / 2);
        field_A30_ = field_A28_ + 0xAA;
        field_A34_ = field_A2C_ + 0xF0;
    } else {
        field_A2C_ = host_area.top + 0x63
            + (wide_layout ? 0x62 : 0);
        field_A28_ = host_area.left
            + ((host_area.right - host_area.left - 0xBE) / 2);
        field_A30_ = field_A28_ + 0xBE;
        field_A34_ = field_A2C_ + 0xAE;
    }
    // Native 0x498CC3 snapshots the wide-layout flag and publishes these
    // anchor members before calling energy_compute at 0x498D20. That call is
    // allowed to update global accounting, but cannot retroactively change
    // this frame's already-selected chart layout.
    if (!datalink) {
        energy_compute(faction, 0);
    }
    // Native 0x498D26 samples the current player's research income only after
    // energy_compute has run. Keep this read here: that call updates the live
    // accounting used by the research header below.
    const int current_tech_rate = datalink ? 0 : tech_rate(faction);
    const bool hide_research = !datalink && (GameDataStateStorage.fields_.game_state_ & 0x80) == 0
        && (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) != 0;
    int research_header_end_x = host_area.left + 0x49;
    int research = -1;
    if (!datalink) {
        RECT research_header = {rect1_.left, rect1_.top,
            rect1_.left + rect1_.right, rect1_.top + 0x46};
        canvas.set_text_color(0xE1, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        canvas.box_sprite(&research_header, &InterfaceBoxStyles[3]);

        // Native 0x498DA0..0x498DF7 suppresses current-tech detail only for
        // blind research when the game-state reveal bit is clear.
        if (hide_research) {
            RECT icon_frame = {host_area.left + 3, host_area.top + 3,
                host_area.right - 3, 0x2F};
            canvas.box_sprite(&icon_frame, &InterfaceBoxStyles[1]);
            strcat(StringTemp, label_get(0xAC4 / 4));
        } else {
            research = PlayersData[faction].tech_id_researching;
            if (research >= 0) {
                const int research_color = mandate_color(tech_category(research));
                Sprite &research_icon = g_TECH_ICON_SPRITES[research];
                research_icon.draw_mono(&canvas,
                    research_icon.transparent_index(), host_area.left + 3,
                    host_area.top + 3, research_color, 0x40,
                    research_icon.width());
            }

            // Original 0x498E61 draws style 1 over the inset icon area. Its
            // bottom edge is the literal 0x2f local retained by the image.
            RECT icon_frame = {host_area.left + 3, host_area.top + 3,
                host_area.right - 3, 0x2F};
            canvas.box_sprite(&icon_frame, &InterfaceBoxStyles[1]);

            // 0x498E71..0x498EF1 emits the research label first; write_l's
            // returned pen x is then reused by the adjacent tech name.
            strcat(StringTemp, label_get(0x254 / 4));
            strcat(StringTemp, "  ");
            strcat(StringTemp, "]");
            const int text_x = host_area.left + 0x49;
            const int text_y = host_area.top + 8;
            research_header_end_x = canvas.write_l(StringTemp, text_x,
                text_y, strlen(StringTemp));
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            if (research >= 0) {
                say_tech(StringTemp, research, 0);
                strcat(StringTemp, "]");
                CharUpperA(StringTemp);
                if (research == MaxTechnologyNum - 1) {
                    StringTemp[strlen(StringTemp) - 1] = 0;
                    strcat(StringTemp, " ");
                    reportwin_append_number(StringTemp,
                        PlayersData[faction].tech_accumulated + 1);
                } else {
                    canvas.write_l(StringTemp, research_header_end_x,
                        text_y, strlen(StringTemp));
                }
            }
        }
    }
    // Native 0x49907C..0x4990AA rebuilds this ordinary-report hit region
    // after the header text/technology icon phase and before chart setup.
    if (!datalink) {
        spot_.kill_type(0);
        spot_.add(0, 0x29A, left, top, width, height);
        if (hide_research) {
            strcat(StringTemp, "DISCOV");
            canvas.write_cent_l(StringTemp, research_header_end_x,
                host_area.top + 8,
                host_area.right - 3 - (host_area.left + 0x46),
                strlen(StringTemp));
        } else {
            strcat(StringTemp, "]");
            canvas.write_l(StringTemp, research_header_end_x,
                host_area.top + 8, strlen(StringTemp));
        }

        const int chart_technology = PlayersData[faction].tech_id_researching;
        if (chart_technology >= 0 && chart_technology < MaxTechnologyNum) {
            RECT category_bar = {left, 0x31, right, host_area.top + 0x43};
            const int original_left = category_bar.left;
            const int original_right = category_bar.right;
            int category = tech_category(
                PlayersData[faction].tech_id_researching);
            BoxSpriteParams *style = reportwin_lab_category_style(category, 38);
            if (style != 0) canvas.box_sprite(&category_bar, style);

            int fill_right = original_left
                + ((original_right - original_left)
                    * PlayersData[faction].tech_accumulated)
                    / current_tech_rate;
            if (fill_right < original_left || original_right < original_left)
                fill_right = original_left;
            else if (fill_right > original_right)
                fill_right = original_right;
            category_bar.right = fill_right;
            canvas.set_clip(&category_bar);
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 36);
            if (style != 0) canvas.box_sprite(&category_bar, style);
            canvas.set_clip(buffer_working_rect());

            const int next_fill_right = fill_right
                + (((original_right - original_left) * EnergyLabsTotal / 100)
                    / current_tech_rate);
            const int bounded_next_fill_right = next_fill_right < fill_right
                || original_right < fill_right ? fill_right
                : next_fill_right > original_right ? original_right
                : next_fill_right;
            RECT next_turn = {fill_right, category_bar.top,
                bounded_next_fill_right, category_bar.bottom};
            canvas.set_clip(&next_turn);

            next_turn.left -= 7;
            next_turn.top -= 7;
            next_turn.right += 7;
            next_turn.bottom += 7;
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 36);
            if (style != 0) canvas.box_sprite(&next_turn, style);

            next_turn.left += 7;
            next_turn.top += 7;
            next_turn.right -= 7;
            next_turn.bottom -= 7;
            canvas.set_clip(buffer_working_rect());
            RECT inner_clip = {next_turn.left + 1, next_turn.top + 1,
                next_turn.right - 1, next_turn.bottom - 1};
            canvas.set_clip(&inner_clip);
            RECT outer_edge = {inner_clip.left - 7, inner_clip.top - 7,
                inner_clip.right + 7, inner_clip.bottom + 7};
            category = tech_category(PlayersData[faction].tech_id_researching);
            style = reportwin_lab_category_style(category, 38);
            if (style != 0) canvas.box_sprite(&outer_edge, style);
            canvas.set_clip(buffer_working_rect());
        }
    }
    // Native 0x499433..0x499521 emits the discovery and estimated-next-
    // discovery captions after the category fill. Its x starts at the
    // header-text column (0x49 from the host left); the y is the same native
    // host-top + field_E268_ + 0x0b cursor established at 0x499172..189.
    if (!datalink) {
        const int progress_x = host_area.left + 0x49;
        const int progress_y = host_area.top + field_E268_ + 0x0B;
        canvas.set_text_color(0xE1, -1, 1, 1);
        parse_num(0, BreakthroughTurnsGlobal);
        parse_it(ScriptPopupFile, "DISCOV");
        const int next_caption_x = canvas.write_l(StringTemp, progress_x,
            progress_y, static_cast<int>(strlen(StringTemp))) + 4;

        int remaining = current_tech_rate
            - PlayersData[faction].tech_accumulated;
        if (remaining < 0) remaining = 0;
        else if (remaining > 0x5F5E0FF) remaining = 0x5F5E0FF;
        int turns_to_discovery = 0x270F;
        if (EnergyLabsTotal != 0) {
            turns_to_discovery = (remaining * 100 + EnergyLabsTotal - 1)
                / EnergyLabsTotal;
        }
        parse_num(0, static_cast<int>(game_year(
            GameDataStateStorage.fields_.turn_current_num_ + turns_to_discovery)));
        parse_it(ScriptPopupFile, "NEXTDISCOV");
        canvas.write_l(StringTemp, next_caption_x, progress_y,
            static_cast<int>(strlen(StringTemp)));
    }
    // Native 0x499524..0x499660 sizes the chart from the report frame and
    // lays out its three horizontal tiers around the selected-tech column.
    const int chart_left = datalink ? left : host_area.left;
    const int chart_width = datalink ? width : rect1_.right;
    const int chart_y = field_E268_
        + (datalink ? host_area.top + 3 + 0x14 : host_area.top + 0x46) + 0x0F;
    const int chart_height = static_cast<int>(energy_panel_rect_.bottom)
        - chart_y - 0x1E;
    int tier_height = chart_height;
    if (!datalink && WinScreenWidth != 0x320) tier_height -= 0xC8;
    rect1_.right = chart_width;
    rect1_.bottom = chart_height;
    field_594C_ = chart_left + 5;
    field_5950_ = chart_left + 5
        + (chart_width - static_cast<int>(field_59A0_)) / 2 - 5;
    field_5954_ = chart_left + 5 + chart_width
        - static_cast<int>(field_59A0_) - 10;
    field_5950_ = field_A28_;
    field_5958_ = chart_y
        + (tier_height - static_cast<int>(field_59A4_)) / 2;
    field_595C_ = field_5958_ - static_cast<int>(field_59A4_) - 0x10;
    field_5960_ = field_5958_ + static_cast<int>(field_59A4_) + 0x10;
    const int tier_pitch = static_cast<int>(field_59A4_) + 0x10;
    field_5964_ = field_5958_ - tier_pitch - tier_pitch / 2;
    field_5968_ = field_5958_ - tier_pitch / 2;
    field_596C_ = field_5958_ + tier_pitch - tier_pitch / 2;
    field_5970_ = field_5958_ + tier_pitch * 2 - tier_pitch / 2;

    // The native two-prerequisite route creates the left/right prerequisite
    // controls (0x499867..0x499A99); the one-prerequisite route creates the
    // center control (0x499722..0x499866). The common one/four-child routes
    // use the four owned FlatButtons at indices 3..6 (0x499AE2..0x499C7E).
    int status_child_count = 0;
    if (field_FAB8_ >= 0 && field_FAB8_ < MaxTechnologyNum) {
        const int prerequisite1 = Technology[field_FAB8_].preq_tech_1;
        const int prerequisite2 = Technology[field_FAB8_].preq_tech_2;
        Win *const parent = datalink ? static_cast<Win *>(&DatalinkMain)
                                     : static_cast<Win *>(this);
        if (prerequisite1 >= 0 && prerequisite2 >= 0) {
            labs_line(field_A28_, field_5958_, field_594C_, field_5968_,
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[1], prerequisite1,
                0x379, field_594C_, field_5968_, field_59A0_, field_59A4_,
                parent, faction);
            labs_line(field_A28_, field_5958_, field_594C_, field_596C_,
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[2], prerequisite2,
                0x37A, field_594C_, field_596C_, field_59A0_, field_59A4_,
                parent, faction);
        } else if (prerequisite1 >= 0) {
            labs_line(field_A28_, field_5958_, field_594C_, field_5958_,
                      field_59A0_, field_59A4_, 0);
            reportwin_init_lab_button(canvas, flatButtons1_[1], prerequisite1,
                0x379, field_594C_, field_5958_, field_59A0_, field_59A4_,
                parent, faction);
        }

        int children[4] = {-1, -1, -1, -1};
        int child_count = reportwin_collect_lab_children(field_FAB8_, children);
        status_child_count = child_count;
        if (child_count > 4) child_count = 4;
        if (child_count == 1 || child_count == 2
            || child_count == 3 || child_count == 4) {
            int x = field_5954_;
            int y = child_count == 4 ? field_5964_
                : child_count == 3 ? field_595C_
                : child_count == 2 ? field_5968_ : field_5958_;
            for (int child = 0; child < child_count; ++child) {
                labs_line(x, y, field_5950_, field_5958_,
                          field_59A0_, field_59A4_, 0);
                reportwin_init_lab_button(canvas, flatButtons1_[child + 3],
                    children[child], 0x37B + child, x, y,
                    field_59A0_, field_59A4_, parent, faction);
                if (child_count == 4) {
                    if (child == 0) y = field_5968_;
                    else if (child == 1) y = field_596C_;
                    else if (child == 2) y = field_5970_;
                } else if (child_count == 2) {
                    if (child == 0) y = field_596C_;
                } else if (child_count == 3) {
                    y = child == 0 ? field_5958_
                        : child == 1 ? field_5960_ : y;
                }
            }
        }
    }

    int measured_height = 0;
    int status_y = 0;
    const int section_x = left + (datalink ? 3 : 0);
    field_A24_ = 0;
    for (int pass = 0; pass < 2; ++pass) {
        const bool draw_pass = pass != 0;
        reportwin_draw_lab_status_rows(canvas, field_FAB8_, faction,
            field_A28_ + 3, field_A34_ + field_A2C_,
            field_59A8_, status_y, measured_height, draw_pass,
            status_child_count == 3,
            ReportIfGlobal.field_1E19C_, ReportIfGlobal.field_1E190_);

        if (field_FAB8_ != -1) {
            // The native host configuration runs in each outer pass. HOST
            // chooses the font and colors; PASS chooses measure versus draw.
            if (datalink) {
                canvas.set_wrap_width(10);
                canvas.set_font(DatalinkMain.entry_text_font(), 0, 0,
                                DatalinkMain.entry_link_font());
                canvas.set_text_color(0x99, -1, 1, 1);
                canvas.set_text_color_hyper(0x96, -1, 1, 1);
            } else {
                canvas.set_text_color(0xE1, -1, 1, 1);
                canvas.set_text_color2(0xE1, -1, 1, 1);
            }
            int section_y = status_y;
            reportwin_draw_lab_unlock_sections(canvas, field_FAB8_, datalink,
                draw_pass, section_x, section_y, &field_59A8_, measured_height);
        }
        if (!draw_pass) field_A24_ = measured_height;
    }

    // The six native list controls have completed their status rows. The
    // host's wrap width is cleared before mouse feedback at 0x49B186.
    canvas.set_wrap_width(0);

    // 0x49B178..0x49B1B5 refreshes mouse feedback on the six technology
    // controls after their geometry has settled. The original uses the
    // inherited Win::get_mouse_pos and virtual on_mouse_move methods
    // (slot 17); these are real FlatButton subobjects, not an address table.
    int mouse_x = 0;
    int mouse_y = 0;
    for (int button = 1; button < 7; ++button) {
        flatButtons1_[button].get_mouse_pos(&mouse_x, &mouse_y);
        flatButtons1_[button].on_mouse_move(mouse_x, mouse_y);
    }

    // The native host clears Buffer::field_10_ at GraphicWin+0x454, refreshes
    // the six buttons above, then restores the host's working clip at
    // 0x49B1BA..1D5. This typed host path supplies that real Buffer subobject.
    canvas.set_clip(datalink ? DatalinkMain.buffer_working_rect()
                            : buffer_working_rect());
    // 0x49B1C3..1CF publishes the final chart cursor in the owned RECT.
    rect1_.left = mouse_x;
    rect1_.top = mouse_y;
    // Native 0x498D25..0x499523 is covered above: the icon uses the sprite's
    // native width as its draw_mono denominator, the final technology formats
    // accumulated research instead of a normal technology name, and the four
    // category-bar boxes retain their native style dispatch and clip sequence.
}

// The process-wide report window, 0x00876478 - see reportwin.h. The image's
// dynamic initializer at 0x00496930 constructs it through the real recovered
// constructor below. The fixed list/timer receivers used by ReportIf are
// members of this object, not separate global storage. Complete original
// allocation bounds remain unproved by this start-only binding.
// RECOVERY_OBJECT: v1 0x00876478 ?ReportWinGlobal@@3VReportWin@@A
ReportWin ReportWinGlobal;  // 0x00876478

// MainInterface owns the interface-slot array and active interface ID.

// ORIGINAL: 0x0049B220 ?on_button_passover@ReportWin@@QAEXHH@Z 0x0049B220-0x0049B446
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0049B220-0x0049B446 and 0x0065C0C2-0x0065C20E", "git:a538e0a3:src/recovered/units/0049b220.cpp retains the recovered seed and original claim", "source:src/reportwin.h", "source:src/reportif.h:ReportIfGlobal.field_8_"], "reason": "Recover the category-button passover drawing through the seven owned FlatButtons, shared work RECT, real host window and existing labs_line. The original seed and historical claim remain restoration obligations; current byte correspondence is measured separately."}
void ReportWin::on_button_passover(int button_id, int state) {
    GraphicWin *host = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8
        ? static_cast<GraphicWin *>(&DatalinkMain)
        : static_cast<GraphicWin *>(this);
    if (ReportIfGlobal.field_8_ != 2 || button_id < 0x378) return;

    const int index = button_id - 0x378;
    // Original 0x0049B26C indexes without an upper-bound check. Preserve that
    // path rather than silently adding a behavior-changing safety guard.
    report_work_rect_ = flatButtons1_[index].outer_rect();

    int first_x = static_cast<int>(field_5950_);
    int first_y = static_cast<int>(field_5958_);
    int last_x = report_work_rect_.left;
    int last_y = report_work_rect_.top;
    if (index >= 3) {
        first_x = report_work_rect_.left;
        first_y = report_work_rect_.top;
        last_x = static_cast<int>(field_5950_);
        last_y = static_cast<int>(field_5958_);
    }

    report_work_rect_.left -= 3;
    report_work_rect_.top -= 3;
    report_work_rect_.right += 3;
    report_work_rect_.bottom += 3;

    const int center_y = (static_cast<int>(field_A34_)
        + static_cast<int>(field_A2C_) - static_cast<int>(field_A24_)) / 2;
    RECT marker = {
        static_cast<int>(field_A28_) - 3,
        center_y - 9,
        static_cast<int>(field_59A8_ + field_A28_) + 3,
        static_cast<int>(field_A24_) + center_y + 6
    };
    Buffer *canvas = static_cast<Buffer *>(host);
    BoxSpriteParams *style = state == 0
        ? &InterfaceBoxStyles[3] : &InterfaceBoxStyles[5];
    canvas->box_sprite(&marker, style);
    marker.left += 3;
    marker.top += 3;
    marker.right -= 3;
    marker.bottom -= 3;
    canvas->box_sprite(&report_work_rect_, style);
    labs_line(first_x, first_y, last_x, last_y,
              static_cast<int>(field_59A0_), static_cast<int>(field_59A4_),
              state != 0);
    host->soft_update();
}

// Fixed890208/890D5C receivers are ReportIfGlobal.listBox1_/listBox2_.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

/*
Purpose: Construct every sub-object in image order - a FlatButton[7] run, a
         Spot, a ListBox, nine individually-constructed FlatButtons split
         by two ButtonGroups, a Scroll, three Times, two Buffers and a Flic -
         then install ReportWin's own vftables.
// ORIGINAL: 0x004AD6B0 ??0ReportWin@@QAE@XZ 0x004AD6B0-0x004AD838;0x006592F0-0x00659419 BYTE_EXACT
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 14 -> 10
//   of 87. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (149 compiled against 87). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 9/87 instructions agree.
// LEVER: flatButtons1_ and spot_ (the whole prefix before the irreducible
//        listBox_) converted to real declared members, built implicitly.
//        9/87 -> 14/87 agreeing instructions - still MISMATCH, real
//        improvement. Everything from listBox_ onward stays raw/explicit:
//        listBox_ has no declared one-argument constructor and Flic's own
//        recovery is out of this batch's scope, and real members declared
//        after either would be hoisted into the same implicit preamble
//        ahead of them, which the image does not do.
// size      689 bytes
// prototype void (__thiscall ??0ReportWin@@QAE@XZ)(ReportWin* this)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005D7210 0x005FA860 0x006051D0 0x00607CF0 0x00609DB0 0x006161D0 0x00629D60 0x0062B7C0 0x006456E4 0x006457C2
*/
ReportWin::ReportWin() {
    // The base and members construct once, in declaration order. The extra
    // argument at the original ListBox call is its most-derived ABI flag.
}

// ORIGINAL: 0x004AD3B0 ??1ReportWin@@QAE@XZ 0x004AD3B0-0x004AD6A9;0x00659140-0x006592EE
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004ad3b0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ??1ReportWin@@UAE@XZ
ReportWin::~ReportWin() {
    // Original 4AD3B0 contains only compiler-generated reverse member/base
    // teardown. In particular, Flic and Time own their close operations.
}

// ORIGINAL: 0x00496960 ?init@ReportWin@@QAEXXZ 0x00496960-0x00496BA5
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/00496960.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::init() {
    for (int i = 0; i < 16; ++i) selected_bases_[i] = 0;
    for (int j = 0; j < 4; ++j) selected_prototypes_[j] = 0;
    rect1_.right = 650;
    rect1_.bottom = 353;
    spot_.init(256);
    const bool large = ReportIfGlobal.field_1E230_ != 0;
    rect1_.bottom += large ? 200 : 20;
    rect1_.left = ((large ? WinScreenWidth : 800) - rect1_.right) / 2;
    rect1_.top = large ? WinScreenHeight - rect1_.bottom - 212
                      : 388 - rect1_.bottom;
    report_panel_rect_.left = 23;
    report_panel_rect_.top = 0;
    report_panel_rect_.right = rect1_.right + 23;
    report_panel_rect_.bottom = rect1_.bottom;
    energy_panel_rect_.left = 26;
    energy_panel_rect_.top = 3;
    energy_panel_rect_.right = rect1_.right - 6 + 26;
    energy_panel_rect_.bottom = rect1_.bottom - 56 + 3;
    GraphicWin::init(rect1_.left - 23, rect1_.top, rect1_.right + 78,
        rect1_.bottom, large ? ReportWindowCaptionLarge : ReportWindowCaptionSmall,
        0x1000000, static_cast<Win *>(&ConsoleGlobal), 0, 0);
    field_E268_ = ReportIfGlobal.field_1E184_->line_advance();
}

/*
Purpose: Reset the seven report buttons, initialize the report text buffer,
         copy the current report bounds, draw both panel frames, and restore
         the inset frame rectangle after its temporary expansion.
// ORIGINAL: 0x00496BB0 ?reset@ReportWin@@QAEXXZ 0x00496BB0-0x00496CFC
// RECOVERY_BINDING: v1 {"evidence":["Independent IDA/Ghidra function records agree on the original extent 0x00496BB0-0x00496CFC and classify it as game code.","git:caf4963a:src/recovered/units/00496bb0.cpp records the button-close loop, Buffer font/color setup, rectangle copies and box_sprite calls in this order.","docs/recovery/member-accesses.csv records reset's writes through the report geometry and embedded button/panel members; reportwin.h provides the corresponding source-owned member layout."],"reason":"Recover the report reset in its existing ReportWin owner using the declared button array, buffer storage, rectangles and shared interface box styles."}
// prototype void (__thiscall ?reset@ReportWin@@QAEXXZ)(ReportWin* this)
// kind      game
// calls     0x005D5890 0x005E2DD3
// Homed from src/recovered/units/00496bb0.cpp.
*/
void ReportWin::reset() {
    for (int button = 0; button != 7; ++button) {
        flatButtons1_[button].close();
    }

    Buffer &buffer = buffer1_;
    buffer.set_font(ReportIfGlobal.field_1E184_, ReportIfGlobal.field_1E184_,
                    ReportIfGlobal.field_1E190_, nullptr);
    buffer.set_text_color(0x99, -1, 1, 1);
    buffer.set_text_color2(0xF2, -1, 1, 1);
    buffer.set_text_color3(0xE1, -1, 1, 1);

    rect1_.left = energy_panel_rect_.left;
    rect1_.top = energy_panel_rect_.top;
    rect1_.right = energy_panel_rect_.right - energy_panel_rect_.left;
    rect1_.bottom = energy_panel_rect_.bottom - energy_panel_rect_.top;

    report_work_rect_ = energy_panel_rect_;

    buffer.box_sprite(&report_panel_rect_, &InterfaceBoxStyles[9]);

    report_frame_rect_.left -= 3;
    report_frame_rect_.top -= 3;
    report_frame_rect_.right += 3;
    report_frame_rect_.bottom += 3;
    buffer.box_sprite(&report_frame_rect_, &InterfaceBoxStyles[3]);
    report_frame_rect_.left += 3;
    report_frame_rect_.top += 3;
    report_frame_rect_.right -= 3;
    report_frame_rect_.bottom -= 3;
}

// ORIGINAL: 0x004A5F00 ?show@ReportWin@@QAEXH@Z 0x004A5F00-0x004A5F16 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Home the existing ReportWin show identity in its class owner, retaining its measured byte-exact claim.","evidence":["src/recovered/004a5f00.cpp records the original 0x004A5F00-0x004A5F16 body and BYTE_EXACT claim.","ReportWin derives from Win and ReportIfGlobal owns the interface-mode transitions used by the seed."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/004a5f00.cpp
// symbol    ?show@ReportWin@@UAEXH@Z
/*
Purpose: Enter or leave the report interface mode only when its window changes
         visibility. The visibility check belongs to the inherited Win base;
         mode ownership belongs to the process-wide ReportIf subinterface.
*/
void ReportWin::show(int) {
    if (!is_visible()) {
        ReportIfGlobal.set_iface_mode();
    }
}

// ORIGINAL: 0x004A5F20 ?hide@ReportWin@@QAEXXZ 0x004A5F20-0x004A5F34 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Home the existing ReportWin hide identity in its class owner, retaining its measured byte-exact claim.","evidence":["src/recovered/units/004a5f20.cpp records the original 0x004A5F20-0x004A5F34 body and BYTE_EXACT claim.","ReportWin derives from Win and ReportIfGlobal owns the interface-mode transitions used by the seed."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/004a5f20.cpp
// symbol    ?hide@ReportWin@@UAEXXZ
void ReportWin::hide() {
    if (is_visible()) {
        ReportIfGlobal.release_iface_mode();
    }
}

// ORIGINAL: 0x00497A60 ?on_left_double_click@ReportWin@@QAEXHH@Z 0x00497A60-0x00497AB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Home ReportWin's existing double-click handler against its declared Spot member and the verified base-window object.","evidence":["src/recovered/units/00497a60.cpp records the original 0x00497A60-0x00497AB2 method and BYTE_EXACT claim.","src/reportwin.h places spot_ at 0x5994; src/tutwin.h binds TutWinBaseWindow at the seed's 0x006A7628 zoom receiver."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00497a60.cpp
// symbol    ?on_left_double_click@ReportWin@@UAEXHH@Z
/*
Purpose: Double-click the currently hit report spot to zoom to a base when
         the active report page is the base report (page 4).
*/
void ReportWin::on_left_double_click(int x, int y) {
    int spot_position;
    int spot_type;
    int result = spot_.check(x, y, &spot_position, &spot_type);
    if (result >= 0 && ReportIfGlobal.field_8_ == 4 &&
        spot_position >= 0x400) {
        int base_id = spot_position - 0x400;
        if (base_id < 0x400) {
            TutWinBaseWindow.zoom(base_id, 0);
        }
    }
}

// ORIGINAL: 0x00498180 ?on_scrolled@ReportWin@@QAEXHH@Z 0x00498180-0x004981D8
// RECOVERY_BINDING: v1 {"reason":"Home the ReportWin scrollbar callback using its declared position member and inherited redraw virtual.","evidence":["src/recovered/units/00498180.cpp records the original 0x00498180-0x004981D8 handler and page dispatch.","src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498180.cpp
/*
Purpose: Track the report scrollbar's committed position and repaint the
         project page directly; recognized remaining pages use the window's
         redraw virtual, while unlisted pages do nothing.
*/
void ReportWin::on_scrolled(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 5:
        reset();
        draw_projects();
        soft_update();
        return;
    case 0:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
        redraw();
        return;
    }
}

// ORIGINAL: 0x00498200 ?on_scrolling@ReportWin@@QAEXHH@Z 0x00498200-0x00498258
// RECOVERY_BINDING: v1 {"reason":"Home the ReportWin scroll-in-progress callback using the same source-owned position and redraw behavior.","evidence":["src/recovered/units/00498200.cpp records the original 0x00498200-0x00498258 handler and page dispatch.","src/reportwin.h places field_C0FC_ at 0xC0FC; src/graphicwin.h identifies redraw as Win slot 62."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498200.cpp
void ReportWin::on_scrolling(int, int pos) {
    field_C0FC_ = pos;
    switch (ReportIfGlobal.field_8_) {
    case 5:
        reset();
        draw_projects();
        soft_update();
        return;
    case 0:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
        redraw();
        return;
    }
}

// ORIGINAL: 0x00498110 ?UNK1@ReportWin@@QAEXHHHH@Z 0x00498110-0x00498174
// RECOVERY_BINDING: v1 {"reason":"Home ReportWin's range callback and type its embedded Scroll storage while preserving the constructor's explicit placement order.","evidence":["src/recovered/units/00498110.cpp records the original 0x00498110-0x00498174 method and its position/range/paging updates.","src/reportwin.h places Scroll at 0xC10C; src/scroll.h places paging_ at Scroll+0xA40, which is the seed's 0xCB4C write."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/recovered/units/00498110.cpp
/*
Purpose: Store the active position and range, update the embedded Scroll's
         page size and position, then clear the ReportWin page-transition flag.
*/
void ReportWin::UNK1(int position, int minimum, int maximum, int page_size) {
    field_C0FC_ = position;
    field_C100_ = minimum;
    field_C104_ = maximum;
    field_C108_ = page_size;
    Scroll &scroll = scroll_;
    scroll.set_range(minimum, maximum);
    scroll.paging_ = page_size;
    scroll.set_pos(field_C0FC_);
    field_E270_ = 0;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004980F0 ?on_mouse_move@ReportWin@@QAEXHH@Z 0x004980F0-0x004980F3 BYTE_EXACT
// symbol    ?on_mouse_move@ReportWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00498100 ?on_mouse_leave@ReportWin@@QAEXHH@Z 0x00498100-0x00498103 BYTE_EXACT
// symbol    ?on_mouse_leave@ReportWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_leave(int, int) {
}


/*
Purpose: Step the receiver back to the subobject ??_GReportWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AD870 ??_GReportWin@@WEEE@AEPAXI@Z 0x004AD870-0x004AD87B BYTE_EXACT
// symbol    ??_EReportWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3ReportWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AD840` into
//   ??_GReportWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ORIGINAL: 0x0049EB10 ?draw_projects@ReportWin@@QAEXXZ 0x0049EB10-0x0049F343
// RECOVERY_BINDING: v1 {"reason":"Recover the projects page through owned ReportWin rectangles, spots and scroll state, preserving queued/completed rendering and the destroyed-owner byte alias through its real PlayerData member.","evidence":["image:terranx_original.exe:0x0049EB10-0x0049F343", "src/unrecovered/0049eb10.cpp:retained seed and trial history", "Original97CDDC=PlayersData96C9E0+7*20CC+goals19FC+70*10+base_idC; the native unsigned-byte read determines the destroyed-project faction without dereferencing Bases[-2]."]}
// Deleted seed provenance: git:c2e28f62c941b2012636504516275bc26988ee80:src/unrecovered/0049eb10.cpp
// TRIED: full transcription (call-argument order verified against raw disasm for add/box_sprite/draw_mono/write_l/write_cent_l/ set_text_color/Strings::get, all matched Ghidra) reaches MISMATCH #3 'mov' vs 'push' - a prologue-level register/stack allocation divergence, not a logic error; not chased further given function size (2099 bytes, nested loops with gotos).
void ReportWin::draw_projects() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id_;
    RECT frame = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, energy_panel_rect_.bottom - 3};
    const int pitch = static_cast<int>(field_E268_) + 9;
    const int row_pitch = 54;
    const int content_top = energy_panel_rect_.top + pitch;
    const int rows = (energy_panel_rect_.bottom - content_top - 5) / row_pitch;
    const int panel_width = frame.right - frame.left;
    // Native width is (panel.right - panel.left - 6) / 2 - 1. `frame`
    // already has the three-pixel inset on each side, so do not inset again.
    field_59A0_ = panel_width / 2 - 1;
    field_59A4_ = row_pitch;
    spot_.kill_type(0);
    // The original follows kill_type with an inlined add_count_ = 0 store
    // at ReportWin+0x599C before any drawing or scroll recursion.
    spot_.reset_count();
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&frame, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    // The original obtains this title through Strings::get using the entry
    // at StringTable + 0xAD0, rather than through a numeric label key.
    strcat(StringTemp, label_get(0xAD0 / 4));
    strcat(StringTemp, "]");
    RECT heading = {frame.left, frame.top - 1, frame.right, content_top - 1};
    canvas.write_cent_l(StringTemp, &heading, strlen(StringTemp));

    // The image lays the entries out in three passes: projects currently in
    // production by the viewed faction, production by other factions, and
    // projects already present in the completed-project owner table.
    int entry_count = -1;
    for (int project = 0; project < MaxSecretProjectNum; ++project) {
        if (base_project(project) != SP_Unbuilt)
            ++entry_count;
    }
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id)
        if (Bases[base_id].queue_production_id[0] <= -FacilitySPStart)
            ++entry_count;
    if (!field_C0F8_) {
        if (entry_count >= rows * 2) field_E26C_ = 1;
    } else if (entry_count < rows * 2) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_) {
        field_C108_ = rows - 1;
        field_C0FC_ = field_C100_ = 0;
        field_C104_ = entry_count / 2;
        scroll_.set_range(0, field_C104_);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
        draw_projects();
    }
    int logical_row = 0;
    for (int mode = 0; mode < 3; ++mode) {
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            const int facility = FacilitySPStart + project;
            const int owner_base = base_project(project);
            if (mode < 2 && owner_base != SP_Unbuilt) continue;
            if (mode == 2 && owner_base == SP_Unbuilt) continue;

            int base_id = owner_base;
            int faction = -1;
            if (mode < 2) {
                for (int candidate = 0; candidate < GameDataStateStorage.fields_.base_current_count_; ++candidate) {
                    const Base &base = Bases[candidate];
                    const bool same_faction = base.faction_id_current == viewer;
                    if ((mode == 0) != same_faction) continue;
                    if (base.queue_production_id[0] != -facility) continue;
                    base_id = candidate;
                    faction = base.faction_id_current;

                    const int ordinal = logical_row++;
                    const int visible_row = ordinal / 2
                        - static_cast<int>(field_C0FC_);
                    if (visible_row < 0 || visible_row >= rows) continue;
                    const int column = ordinal & 1;
                    const int card_left = frame.left
                        + column * (static_cast<int>(field_59A0_) + 2);
                    const int card_top = content_top + visible_row * (row_pitch + 2) + 2;
                    RECT item = {card_left, card_top,
                                 card_left + static_cast<int>(field_59A0_),
                                 card_top + row_pitch};
                    rect1_.left = item.left;
                    rect1_.top = item.top;
                    report_work_rect_ = item;
                    const int selection = base_id + 0x400;
                    spot_.add(selection, 0, &report_work_rect_);
                    const bool selected = field_59AC_ == selection;
                    canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[selected ? 2 : 1]);
                    if (selected) canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);

                    Sprite &project_icon = g_SECRET_PROJECT_ICON_SPRITES[project];
                    project_icon.draw_mono(&canvas, project_icon.transparent_index(),
                        rect1_.left + 2, rect1_.top + 1, mandate_color(1),
                        row_pitch - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
                    StringTemp[0] = 0;
                    strcat(StringTemp, StringTable.get(
                        reinterpret_cast<int>(Facility[facility].name)));
                    canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
                    canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
                    rect1_.top = item.top + 5;
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                                   strlen(StringTemp));
                    rect1_.top += field_E268_;
                    StringTemp[0] = 0;
                    if (mode == 1) strcat(StringTemp, "[");
                    if (mode == 0 || spying(faction)) {
                        say_base(StringTemp, base_id);
                    } else {
                        GenderDefault = Players[faction].noun_gender;
                        PluralityDefault = Players[faction].is_noun_plural;
                        strcat(StringTemp, Players[faction].noun_faction);
                    }
                    if (mode == 1) strcat(StringTemp, "]");
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                        strlen(StringTemp));
                    rect1_.top += field_E268_;
                    StringTemp[0] = 0;
                    if (mode == 1) strcat(StringTemp, "[");
                    strcat(StringTemp, label_get(0x6BC / 4));
                    if (mode == 1) strcat(StringTemp, "]");
                    canvas.set_text_color(0xF9, -1, 1, 1);
                    canvas.set_text_color3(0xF9, -1, 1, 1);
                    canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                        strlen(StringTemp));
                }
            } else {
                if (base_id == SP_Destroyed) {
                    // The original byte load at 0x0097CDDC aliases
                    // PlayersData[7].goals[70].base_id. Preserve its low-byte
                    // faction value and the subsequent native table indexing.
                    faction = static_cast<uint8_t>(PlayersData[7].goals[70].base_id);
                } else {
                    faction = Bases[base_id].faction_id_current;
                }
            }
            if (mode < 2) continue;

            const int ordinal = logical_row++;
            const int scroll_row = ordinal / 2
                - static_cast<int>(field_C0FC_);
            if (scroll_row < 0 || scroll_row >= rows) continue;
            const int card_left = frame.left
                + (ordinal & 1) * (static_cast<int>(field_59A0_) + 2);
            const int card_top = content_top + scroll_row * (row_pitch + 2) + 2;
            RECT item = {card_left, card_top,
                         card_left + static_cast<int>(field_59A0_),
                         card_top + row_pitch};
            rect1_.left = item.left;
            rect1_.top = item.top;
            report_work_rect_ = item;
            const int selection = mode == 2 ? project : base_id + 0x400;
            spot_.add(selection, 0, &report_work_rect_);
            const bool selected = field_59AC_ == selection;
            canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[selected ? 2 : 1]);
            if (selected) canvas.box_sprite(&report_work_rect_, &InterfaceBoxStyles[5]);

            g_SECRET_PROJECT_ICON_SPRITES[project].draw_mono(
                &canvas, g_SECRET_PROJECT_ICON_SPRITES[project].transparent_index(),
                rect1_.left + 2, rect1_.top + 1, mandate_color(1),
                row_pitch - 2, g_SECRET_PROJECT_ICON_SPRITES[0].height());
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[facility].name)));
            strcat(StringTemp, "]");
            canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
            rect1_.top = item.top + 5;
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top,
                           strlen(StringTemp));
            rect1_.top += field_E268_;
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            say_base(StringTemp, base_id);
            strcat(StringTemp, "]");
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top, strlen(StringTemp));
            rect1_.top += field_E268_;
            StringTemp[0] = 0;
            strcat(StringTemp, "[");
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            strcat(StringTemp, Players[faction].noun_faction);
            strcat(StringTemp, "]");
            canvas.set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            canvas.set_text_color3(ChatItemIdTable[faction], -1, 1, 1);
            canvas.write_l(StringTemp, rect1_.left + 46, rect1_.top, strlen(StringTemp));
        }
    }
}

/*
Purpose: Draw the energy page's five-panel frame and calculate the faction's
         underlying accounts. Text, row breakdowns and selection are pending.
// ORIGINAL: 0x0049BCD0 ?draw_energy@ReportWin@@QAEXXZ 0x0049BCD0-0x0049D481 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0049bcd0.cpp", "src/reportwin.h", "src/socialwin.cpp"], "reason": "Recover the energy-page frame and shared accounting through owned ReportWin geometry and the typed energy computation; preserve the remaining row and selection work explicitly."}
// TRIED: byte-exact prologue - the six local RECTs the original packs into overlapping/aliased ebp slots (shared fields between the six box_sprite calls) are instead six independent RECT locals here, so the frame layout differs from the first instruction on; 765/1663 mnemonics matched (80% rebuilt/original count), all string-building "report line" blocks, the diplomacy/loan loop, both base-relations branches and the ListBox/Dialog vbtable-relative subobject calls are transcribed field-for- field and call-for-call from the disassembly (cross-checked, not just taken from Ghidra). The two `mov ecx,[x]; mov edx, [ecx+8]; ...` virtual-base lookups are reproduced as raw pointer arithmetic rather than through ListBox's own `dialog_` member, since that field's offset is unverified.
// size      6065 bytes
// prototype void (__thiscall ?draw_energy@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00445130 0x0048C060 0x004E3A00 0x004F6510 0x0050BA00 0x00590910 0x005DACB0 0x005DACE0 0x005DAD10 0x005DCEA0 0x005DD130 0x005DD300 0x005DDF60 0x005DE1D0 0x005E2DD3 0x00609470 0x00609C60 0x00609C90 0x00609CC0 0x00609CF0 0x00609F20 0x00609FA0 0x0060A670 0x0060C920 0x006169A0 0x00625E30 0x00625EC0 0x006453E0 0x00645470 0x0064FC88
Facility mode scans owned bases for one selected facility; base mode scans
facilities 0..64 in one selected base. Those indices are not faction IDs.
*/
void ReportWin::draw_energy() {
    Buffer *canvas = static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id();
    const int line_height = static_cast<int>(field_E268_);
    const int middle = (energy_panel_rect_.left + energy_panel_rect_.right) / 2;
    const int first_break = energy_panel_rect_.top + line_height + 9;
    const int second_break = first_break + line_height + 9;

    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3, first_break};
    RECT reserves = {energy_panel_rect_.left + 3, first_break + 2,
                     middle - 1, second_break};
    RECT income = {middle + 2, first_break + 2,
                   energy_panel_rect_.right - 3, second_break};
    RECT expenses = {energy_panel_rect_.left + 3, second_break + 2,
                     middle - 1, energy_panel_rect_.bottom - 3};
    RECT balance = {middle + 2, second_break + 2,
                    energy_panel_rect_.right - 3,
                    energy_panel_rect_.bottom - 3};
    canvas->box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[3]);
    canvas->box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&reserves, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&income, &InterfaceBoxStyles[1]);
    canvas->box_sprite(&expenses, &InterfaceBoxStyles[0]);
    canvas->box_sprite(&balance, &InterfaceBoxStyles[0]);
    canvas->set_text_color(0x99, -1, 1, 1);
    canvas->set_text_color2(0xf2, -1, 1, 1);
    canvas->set_text_color3(0xe1, -1, 1, 1);

    energy_compute(viewer, 0);
    rect1_.left = middle + 8;
    rect1_.top = second_break + 8;
    rect1_.right = -15;

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].adj_name_faction);
    CharUpperA(StringTemp);
    GenderDefault = Players[viewer].noun_gender;
    PluralityDefault = Players[viewer].is_noun_plural;
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xA98 / 4));
    strcat(StringTemp, "]");
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xB9C / 4));
    strcat(StringTemp, "]");
    canvas->write_cent_l(StringTemp, &reserves, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0xE4 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, reserves.left + 8, reserves.top + line_height + 5,
                    strlen(StringTemp));
    strcat(StringTemp, "}");
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, PlayersData[viewer].energy_reserves);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, reserves.left + 8,
                          reserves.top + line_height + 5,
                          reserves.right - reserves.left - 16,
                          strlen(StringTemp));

    const int value_x = rect1_.left;
    // The original passes -15 as the right-text width, not the remaining
    // panel width (0x0049BCD0 stores it in ReportWin+0xA1C).
    const int value_width = static_cast<int>(rect1_.right);
    const int value_pitch = line_height;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F4 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, EnergyEconomyTotal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top, value_width,
                          strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5F8 / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top + value_pitch,
                    strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{-");
    reportwin_append_number(StringTemp, EnergyMaintenanceTotal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top + value_pitch * 2,
                          value_width, strlen(StringTemp));

    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x5FC / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, rect1_.top + value_pitch * 3,
                    strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    if (EnergyCommerceDelta > 0) strcat(StringTemp, "+");
    reportwin_append_number(StringTemp, EnergyCommerceDelta);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, rect1_.top + value_pitch * 3,
                          value_width, strlen(StringTemp));

    int relationship_y = rect1_.top + value_pitch * 4;
    int have_relationship_heading = 0;
    if ((Players[viewer].rule_flags & 0x4000) != 0) {
        have_relationship_heading = 1;
        relationship_y += line_height;
        const int interest = Players[viewer].rule_interest;
        parse_num(0, interest);
        char section[32];
        strcpy(section, "FACT");
        if (interest < 0) {
            strcat(section, "PEN");
        } else {
            strcat(section, "BON");
            if (interest == 0) strcat(section, "2");
        }
        parse_it(ScriptPopupFile, section);
        canvas->wrap_flying(StringTemp, rect1_.left, relationship_y,
                            rect1_.right);
        canvas->wrap_flying_flush();
        relationship_y += line_height * 2;
    }

    // The native table iterates the other seven factions, using the live
    // participant mask and PlayerData's typed bilateral treaty/loan arrays.
    // It keeps the original asymmetric balance/payment choices for outgoing
    // versus incoming loans.
    const unsigned char live_factions = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
    for (int other = 1; other < MaxPlayerNum; ++other) {
        if (other == viewer) continue;
        if (((live_factions >> (other & 31)) & 1) == 0
            || ((live_factions >> (viewer & 31)) & 1) == 0) continue;

        const int outgoing_balance = PlayersData[viewer].loan_balance[other];
        const int incoming_balance = PlayersData[other].loan_balance[viewer];
        const int incoming_sanction = PlayersData[other].sanction_turns;
        if (outgoing_balance == 0 && incoming_balance == 0) continue;

        const bool loan_payment =
            (PlayersData[viewer].diplo_treaties[other] & 0x10) != 0;
        if (!have_relationship_heading) {
            have_relationship_heading = 1;
            relationship_y += 10;
        }
        relationship_y += line_height * 2;

        char relationship_prefix[256];
        relationship_prefix[0] = 0;
        if (!loan_payment) {
            if ((outgoing_balance != 0 && incoming_sanction != 0)
                || (incoming_balance != 0
                    && PlayersData[viewer].energy_reserves != 0)) {
                strcat(relationship_prefix, "*");
                strcat(relationship_prefix, label_get(0x708 / 4));
                strcat(relationship_prefix, " ");
            }
        } else {
            strcat(relationship_prefix, "2");
            strcat(relationship_prefix, label_get(0x604 / 4));
            strcat(relationship_prefix, " ");
        }

        parse_says(3, relationship_prefix, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = 0;
        parse_says(0, Players[other].title_leader, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = 0;
        parse_says(1, Players[other].name_leader, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = Players[other].is_noun_plural;
        parse_says(2, Players[other].noun_faction, -1, -1);

        const int principal = outgoing_balance != 0
            ? outgoing_balance : incoming_balance;
        parse_num(1, principal);
        const int payment = outgoing_balance != 0
            ? PlayersData[viewer].loan_payment[other]
            : PlayersData[other].loan_payment[viewer];
        parse_num(0, payment);
        parse_it(ScriptPopupFile, loan_payment ? "LOANPAYMENT" : "LOANINCOME");
        canvas->wrap_flying(StringTemp, rect1_.left, relationship_y,
                            rect1_.right);
        canvas->wrap_flying_flush();
    }

    relationship_y += line_height * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xEFC / 4));
    strcat(StringTemp, ": ");
    canvas->write_l(StringTemp, value_x, relationship_y, strlen(StringTemp));
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    reportwin_append_number(StringTemp, NetIncomeGlobal);
    strcat(StringTemp, "}");
    canvas->write_right_l(StringTemp, value_x, relationship_y, value_width,
                          strlen(StringTemp));

    if (field_59AC_ == -1) {
        field_59B0_ = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == viewer) {
                field_59B0_ = 1;
                field_59AC_ = base_id;
                break;
            }
        }
    }
    if (field_59AC_ == -1) return;

    rect1_.left = middle + 8;
    rect1_.top = second_break + 8;
    rect1_.right = energy_panel_rect_.right - 3 - (middle + 2) - 12;
    rect1_.bottom = energy_panel_rect_.bottom - 6;
    listBox_.close();
    Dialog &dialog = static_cast<Dialog &>(listBox_);
    RECT list_rect = {middle + 4, second_break + 5,
                      energy_panel_rect_.right - 6,
                      energy_panel_rect_.bottom - 6};
    dialog.init(&list_rect, 0);
    dialog.field_48_ = line_height + 3;
    dialog.set_dialog_font(ReportIfGlobal.field_1E184_,
                           ReportIfGlobal.field_1E184_,
                           ReportIfGlobal.field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox_.set_report_text_color(0xF2);

    StringTemp[0] = 0;
    if (field_59B0_ == 0) {
        const int facility = field_59AC_;
        strcat(StringTemp, "[");
        say_facility(StringTemp, facility, 0);
        CharUpperA(StringTemp);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x160 / 4));
        strcat(StringTemp, "]");
        RECT heading = {middle + 2, second_break + 2,
                        energy_panel_rect_.right - 6, second_break + 2};
        canvas->write_cent_l(StringTemp, &heading, strlen(StringTemp));

        int maintenance_total = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (facility >= 0x41
                || Bases[base_id].faction_id_current != viewer) continue;
            int byte_offset;
            int bit;
            bitmask(facility, &byte_offset, &bit);
            if ((Bases[base_id].facilities_built[byte_offset] & bit) == 0)
                continue;

            StringTemp[0] = 0;
            say_base(StringTemp, base_id);
            strcat(StringTemp, "|");
            const int maintenance = fac_maint(facility, viewer);
            const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
            const bool halved = project_base >= 0
                && Bases[project_base].faction_id_current == viewer;
            maintenance_total += maintenance;
            strcat(StringTemp, "{");
            reportwin_append_number(StringTemp,
                halved ? maintenance / 2 : maintenance);
            if (halved && (maintenance & 1) != 0) strcat(StringTemp, ".5");
            strcat(StringTemp, "}");
            listBox_.item(StringTemp, base_id + 0x400);
        }
        listBox_.item(" | ", 0x05F5E0FE);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "|");
        const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
        const bool halved = project_base >= 0
            && Bases[project_base].faction_id_current == viewer;
        if (halved) {
            reportwin_append_number(StringTemp, maintenance_total / 2);
            if ((maintenance_total & 1) != 0) strcat(StringTemp, ".5");
        } else {
            reportwin_append_number(StringTemp, maintenance_total);
        }
        strcat(StringTemp, "}");
        listBox_.item(StringTemp, 99999999);
    } else {
        const int base_id = field_59AC_;
        strcat(StringTemp, "[");
        say_base(StringTemp, base_id);
        CharUpperA(StringTemp);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xA98 / 4));
        strcat(StringTemp, "]");
        RECT heading = {middle + 2, second_break + 2,
                        energy_panel_rect_.right - 6, second_break + 2};
        canvas->write_cent_l(StringTemp, &heading, strlen(StringTemp));

        int maintenance_total = 0;
        for (int facility = 0; facility < 0x41; ++facility) {
            if (Bases[base_id].faction_id_current != viewer) continue;
            int byte_offset;
            int bit;
            bitmask(facility, &byte_offset, &bit);
            if ((Bases[base_id].facilities_built[byte_offset] & bit) == 0)
                continue;

            StringTemp[0] = 0;
            say_facility(StringTemp, facility, 0);
            strcat(StringTemp, "|");
            const int maintenance = fac_maint(facility, viewer);
            const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
            const bool halved = project_base >= 0
                && Bases[project_base].faction_id_current == viewer;
            maintenance_total += maintenance;
            strcat(StringTemp, "{");
            reportwin_append_number(StringTemp,
                halved ? maintenance / 2 : maintenance);
            if (halved && (maintenance & 1) != 0) strcat(StringTemp, ".5");
            strcat(StringTemp, "}");
            listBox_.item(StringTemp, facility);
        }
        listBox_.item(" | ", 0x05F5E0FE);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp, label_get(0xA9C / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "|");
        const int project_base = GameDataStateStorage.fields_.secret_project_.planetary_energy_grid();
        const bool halved = project_base >= 0
            && Bases[project_base].faction_id_current == viewer;
        if (halved) {
            reportwin_append_number(StringTemp, maintenance_total / 2);
            if ((maintenance_total & 1) != 0) strcat(StringTemp, ".5");
        } else {
            reportwin_append_number(StringTemp, maintenance_total);
        }
        strcat(StringTemp, "}");
        listBox_.item(StringTemp, 99999999);
    }

    listBox_.reset_report_selection();
    dialog.field_B4_ = 0;
    listBox_.attach(this, dialog.field_24_, dialog.field_28_, 0x81020);
    listBox_.set_selected_id(99999999);
    static_cast<GraphicWin &>(listBox_).show(0);
}

/*
Purpose: The topic-panel selection callback. The image:

               cmp edi, 0x400 / jge else        - the selection id
               [esi+0x59AC] = id; [esi+0x59B0] = 0
               ListBox(0x00890208).set_selected_pos(-1, 1)
               ListBox(0x00890D5C).set_selected_id(id)
               ... else:
               cmp edi, 0x5F5E0FE / jl skip     - 0x5F5E0FD = 99999999
               ListBox(self+0x5A0C).set_selected_id(99999999) and return
               [esi+0x59AC] = id - 0x400; [esi+0x59B0] = 1
               ListBox(0x00890208).set_selected_id(id)
               ListBox(0x00890D5C).set_selected_pos(-1, 1)
               then reset(); draw_energy(); GraphicWin::soft_update()

         0x59AC/0x59B0 remember (id, kind): the field pair this class
         declares past flatButtons1_.
// ORIGINAL: 0x0049D490 ?on_selected@ReportWin@@QAEXHH@Z 0x0049D490-0x0049D535 FILE
// size      165 bytes
// prototype void (__thiscall ?on_selected@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00496BB0 0x0049BCD0 0x005D5890 0x00609FA0 0x0060A020
// Homed from src/unrecovered/0049d490.cpp.
*/
void ReportWin::on_selected(int selection_id, int) {
    if (selection_id < 0x400) {
        field_59AC_ = selection_id;
        field_59B0_ = 0;
        ReportIfGlobal.listBox1_.set_selected_pos(-1, 1);
        ReportIfGlobal.listBox2_.set_selected_id(selection_id);
    } else {
        if (selection_id > 0x5F5E0FD) {
            listBox_.set_selected_id(99999999);
            return;
        }
        field_59AC_ = selection_id - 0x400;
        field_59B0_ = 1;
        ReportIfGlobal.listBox1_.set_selected_id(selection_id);
        ReportIfGlobal.listBox2_.set_selected_pos(-1, 1);
    }
    reset();
    draw_energy();
    soft_update();
}

/*
Purpose: The report window's keyboard handler. Always answers 1.

             Enter (0x0D)                      - release the modal loop and
                                                 the active interface mode
             F1 (0x70)                         - on_button_clicked(1), slot 44
             F2-F8 (0x71-0x77)                 - switch to report 2..8 unless
                                                 that report is already up;
                                                 then re-report it on the
                                                 ReportIf singleton when the
                                                 active interface is the
                                                 world map's slot 5, or
                                                 set_modal(0, pop_wait,
                                                 MainInterfaceGlobal) - Win
                                                 slot 57 - over this window
             0x25/0x27                         - category +/- 1 with the
                                                 wrap stored twice, exactly
                                                 as the image emits it
             0x21/0x22/0x26/0x28               - page/line scroll of the
                                                 mirrored range state at
                                                 0xC0F8..0xC108, clamped and
                                                 pushed into scroll_ before
                                                 the slot-62 redraw()
             0x23/0x24                         - jump to the range maximum /
                                                 minimum
             0x30-0x5A or Escape               - release the modal loop and
                                                 the active interface mode
// ORIGINAL: 0x00498280 ?on_key_click@ReportWin@@QAEHHH@Z 0x00498280-0x00498691
// symbol    ?on_key_click@ReportWin@@UAEHHH@Z
// size      1041 bytes
// prototype int (__thiscall ?on_key_click@ReportWin@@QAEHHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D380 0x004A3950 0x005EE280 0x00605D20
// indirect  0x0049829C 0x00498418 0x00498652
// Homed from src/unrecovered/00498280.cpp. pop_wait (0x005398E0) rides the
// set_modal slot as the poll callback, not as a call target.
*/
int ReportWin::on_key_click(int, int code) {
    if (code == 0x0D) {
        ReportWinGlobal.Win::release_modal();
        ReportIfGlobal.SubInterface::release_iface_mode();
        return 1;
    }
    if (code == 0x70) {
        on_button_clicked(1);
        return 1;
    }
    if (code > 0x70 && code < 0x78) {
        if (ReportIfGlobal.field_8_ == code - 0x6F) {
            ReportWinGlobal.Win::release_modal();
            ReportIfGlobal.SubInterface::release_iface_mode();
            return 1;
        }
        int report_code;
        if (code == 0x71) {
            ReportIfGlobal.field_8_ = 2;
            report_code = 2;
        } else if (code == 0x72) {
            ReportIfGlobal.field_8_ = 3;
            report_code = 3;
        } else if (code == 0x73) {
            ReportIfGlobal.field_8_ = 4;
            report_code = 4;
        } else if (code == 0x74) {
            ReportIfGlobal.field_8_ = 5;
            report_code = 5;
        } else if (code == 0x75) {
            ReportIfGlobal.field_8_ = 6;
            report_code = 6;
        } else if (code == 0x76) {
            ReportIfGlobal.field_8_ = 7;
            report_code = 7;
        } else {
            ReportIfGlobal.field_8_ = 8;
            report_code = 8;
        }

        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
            ReportIfGlobal.report(report_code,
                                  ReportIfGlobal.report_faction_id(), 0);
            return 1;
        }
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return 1;
    }

    if (code < 0x21 || code > 0x28) {
        if ((code < 0x30 || code > 0x5A) && code != 0x1B) {
            return 1;
        }
        ReportWinGlobal.Win::release_modal();
        ReportIfGlobal.SubInterface::release_iface_mode();
        return 1;
    }

    if (code == 0x25 || code == 0x27) {
        int moved = (code == 0x25) ? (ReportIfGlobal.field_8_ + 4)
                                   : (ReportIfGlobal.field_8_ + 6);
        int remainder = moved % 7;
        ReportIfGlobal.field_8_ = remainder + 2;

        int report_code;
        switch (remainder) {
        case 0:
            ReportIfGlobal.field_8_ = 2;
            report_code = 2;
            break;
        case 1:
            ReportIfGlobal.field_8_ = 3;
            report_code = 3;
            break;
        case 2:
            ReportIfGlobal.field_8_ = 4;
            report_code = 4;
            break;
        case 3:
            ReportIfGlobal.field_8_ = 5;
            report_code = 5;
            break;
        case 4:
            ReportIfGlobal.field_8_ = 6;
            report_code = 6;
            break;
        case 5:
            ReportIfGlobal.field_8_ = 7;
            report_code = 7;
            break;
        default:
            ReportIfGlobal.field_8_ = 8;
            report_code = 8;
            break;
        }

        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5) {
            ReportIfGlobal.report(report_code,
                                  ReportIfGlobal.report_faction_id(), 0);
            return 1;
        }
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return 1;
    }

    switch (code) {
    case 0x21: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) -
                     static_cast<int>(field_C108_);
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x22: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C108_) +
                     static_cast<int>(field_C0FC_);
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x23:
        if (field_C0F8_ == 0) {
            return 1;
        }
        field_C0FC_ = field_C104_;
        break;
    case 0x24:
        if (field_C0F8_ == 0) {
            return 1;
        }
        field_C0FC_ = field_C100_;
        break;
    case 0x26: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) - 1;
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    case 0x28: {
        if (field_C0F8_ == 0) {
            return 1;
        }
        int range_maximum = static_cast<int>(field_C104_);
        int range_minimum = static_cast<int>(field_C100_);
        int target = static_cast<int>(field_C0FC_) + 1;
        if (range_minimum <= target && range_minimum <= range_maximum) {
            range_minimum = target;
            if (range_maximum < target) {
                range_minimum = range_maximum;
            }
        }
        field_C0FC_ = static_cast<uint32_t>(range_minimum);
        break;
    }
    default:
        break;
    }

    if (field_C0F8_ != 0) {
        scroll_.set_pos(field_C0FC_);
        redraw();
    }
    return 1;
}

/*
Purpose: Draw one lab-connection line of the research tree, plus its arrow.

The drawing surface is this window's Buffer unless the active interface is
slot 8, in which case it is the Datalink's (the research screen). The line
is a box_sprite from the shared InterfaceBoxStyles pair - element 3 when
style_select is zero, element 5 otherwise - clipped against the working
RECT at field_E258_, and the arrow sprite is the matching entry of the
tech-tree arrow table.

             host = IfaceIdToSlot[ActiveIfaceId] != 8 ? this : &DatalinkMain
             buffer = host + 0x444 (the GraphicWin Buffer base)
             arrow = g_IFACE_TECH_TREE_ARROW_SPRITES[style_flag]
             style = style_select == 0 ? styles[3] : styles[5]

The first branch (first_id == fields[0x1654]) draws the tag-to-line
connector and returns; the three middle branches draw the line's right
half for each matched lab pair member; the common tail draws the line box
and the arrow head.
// ORIGINAL: 0x0049B450 ?labs_line@ReportWin@@QAEXHHHHHHH@Z 0x0049B450-0x0049BCC7 FILE
// TRIED: Ghidra-structure translation fixed to real Sprite::draw()/ Buffer::set_clip()/box_sprite() member calls (Ghidra had flattened the thiscall receiver away on all 6 draw() sites and both callee methods); compiles, MISMATCH #6 (early - likely the local8/table[idx]!=8 selection prologue).
// size      2167 bytes
// prototype void (__thiscall ?labs_line@ReportWin@@QAEXHHHHHHH@Z)(ReportWin* this, int, int, int, int, int, int, int)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D8000 0x005E2DD3 0x005E4B9A
// Homed from src/unrecovered/0049b450.cpp. The seed's 0x78D5A0/0x78D5F0
// styles are shared InterfaceBoxStyles elements 3/5 (base 0x78D528: socialwin
// already re-homed element 3/6 the same way), and its 0x75AFD0 sprite is
// g_IFACE_TECH_TREE_ARROW_SPRITES (stride 0x2C, so stB*4 == 44*style_flag).
*/
void ReportWin::labs_line(int first_id, int second_id, int, int branch_id,
                          int x_offset, int line_height, int style_select) {
    // The declared member block as dwords: fields[0x1653..0x165C] are
    // field_594C_..field_5970_, fields[0x166A] is field_59A8_, and
    // fields[0x3896..0x3899] is the work RECT at field_E258_.
    int *fields = reinterpret_cast<int *>(this);

    // The research screen draws on the Datalink's own surface; every other
    // interface draws on this window's. Both hosts are GraphicWin-derived,
    // so the Buffer base sits at +0x444 either way.
    Buffer *buffer;
    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 8) {
        buffer = static_cast<Buffer *>(this);
    } else {
        buffer = static_cast<Buffer *>(&DatalinkMain);
    }
    // The arrow draw's guarded receiver: the image passes the buffer only
    // after re-testing the host, so the seed kept the null-guard form.
    Buffer *draw_buffer = (buffer != 0) ? buffer : static_cast<Buffer *>(0);

    int style_flag = (style_select != 0) ? 2 : 0;
    Sprite *arrow = &g_IFACE_TECH_TREE_ARROW_SPRITES[style_flag];
    unsigned char sprite_byte = arrow->transparent_index();
    BoxSpriteParams *box_style = (style_select == 0)
                                     ? &InterfaceBoxStyles[3]
                                     : &InterfaceBoxStyles[5];

    RECT *work_rect = &report_work_rect_;

    int line_id = fields[0x1654];
    int half_height = line_height / 2;
    int centre_y = fields[0x1656] + half_height;

    int tail_bottom, tail_right;

    if (first_id == line_id) {
        int line_left = fields[0x1653];
        int centre_x = (line_left + line_id + x_offset) / 2;
        if (branch_id == fields[0x165A] || branch_id == fields[0x165B]) {
            fields[0x3896] = line_left + 3 + x_offset;
            fields[0x3897] = fields[0x165A] + half_height;
            fields[0x3898] = centre_x - 2;
            fields[0x3899] = fields[0x165B] + half_height;
            if (branch_id == fields[0x165A]) {
                fields[0x3899] = centre_y;
            } else {
                fields[0x3897] = centre_y + 1;
            }
            buffer->set_clip(work_rect);
            fields[0x3899] = fields[0x165B] + half_height;
            fields[0x3896] = fields[0x1653] - 4 + x_offset;
            fields[0x3897] = fields[0x165A] + half_height;
            fields[0x3898] = centre_x - 2;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        } else {
            int left_edge = line_left - 4 + x_offset;
            fields[0x3898] = centre_x + 5;
            fields[0x3897] = centre_y;
            fields[0x3896] = left_edge;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] = left_edge + 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
            box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                            : &InterfaceBoxStyles[5];
            buffer->box_sprite(work_rect, box_style);
        }
        buffer->restore_clip();
        fields[0x3896] = centre_x - 9;
        fields[0x3897] = centre_y;
        fields[0x3898] = line_id + 4;
        fields[0x3899] = centre_y + 0x14;
        fields[0x3896] += 7;
        fields[0x3898] -= 7;
        fields[0x3899] -= 7;
        buffer->set_clip(work_rect);
        fields[0x3896] -= 7;
        fields[0x3898] += 7;
        fields[0x3899] += 7;
        box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                        : &InterfaceBoxStyles[5];
        buffer->box_sprite(work_rect, box_style);
        buffer->restore_clip();
        arrow->draw(draw_buffer, sprite_byte, line_id - 10,
                    fields[0x1656] - 6 + half_height);
        return;
    }

    int line_right = fields[0x1655];
    int mid_x = (fields[0x166A] + line_right + line_id) / 2;
    int mid_x2 = mid_x - 2;

    if (second_id == fields[0x1657] || second_id == fields[0x1658]) {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x1657] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x1658] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x1657]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x1657] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x1658] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x1658] + half_height;
        fields[0x3897] = fields[0x1657] + half_height;
        fields[0x3896] = mid_x2;
    } else if (second_id == fields[0x1659] || second_id == fields[0x165C]) {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x1659] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x165C] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x1659]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x1659] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x165C] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x165C] + half_height;
        fields[0x3897] = fields[0x1659] + half_height;
        fields[0x3896] = mid_x2;
    } else if (second_id != fields[0x165A] && second_id != fields[0x165B]) {
        arrow->draw(draw_buffer, sprite_byte, line_right - 10,
                    centre_y - 6);
        fields[0x3898] = line_right + 4;
        fields[0x3897] = centre_y;
        fields[0x3896] = mid_x - 9;
        fields[0x3899] = centre_y + 0x14;
        fields[0x3896] += 7;
        fields[0x3898] -= 7;
        fields[0x3899] -= 7;
        buffer->set_clip(work_rect);
        fields[0x3896] -= 7;
        fields[0x3898] += 7;
        fields[0x3899] += 7;
        goto common_tail;
    } else {
        fields[0x3896] = mid_x2;
        fields[0x3897] = fields[0x165A] + half_height;
        fields[0x3898] = line_right - 3;
        fields[0x3899] = fields[0x165B] + half_height;
        int draw_x, draw_y;
        if (second_id == fields[0x165A]) {
            fields[0x3899] = centre_y;
            draw_x = line_right;
            draw_y = fields[0x165A] - 6;
        } else {
            fields[0x3897] = centre_y + 1;
            draw_x = line_right;
            draw_y = fields[0x165B] - 7;
        }
        arrow->draw(draw_buffer, sprite_byte, draw_x - 10,
                    draw_y + half_height);
        buffer->set_clip(work_rect);
        tail_right = line_right + 4;
        tail_bottom = fields[0x165B] + half_height;
        fields[0x3897] = fields[0x165A] + half_height;
        fields[0x3896] = mid_x2;
    }
    fields[0x3898] = tail_right;
    fields[0x3899] = tail_bottom;
common_tail:
    box_style = (style_select == 0) ? &InterfaceBoxStyles[3]
                                    : &InterfaceBoxStyles[5];
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
    {
        int left_edge = fields[0x166A] - 4 + fields[0x1654];
        fields[0x3896] = left_edge;
        fields[0x3897] = centre_y;
        if (style_select == 0) {
            fields[0x3898] = mid_x + 5;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] += 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[3];
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
        } else {
            fields[0x3898] = mid_x + 5;
            fields[0x3899] = centre_y + 0x14;
            fields[0x3896] = left_edge + 7;
            fields[0x3898] -= 7;
            fields[0x3899] -= 7;
            buffer->set_clip(work_rect);
            box_style = &InterfaceBoxStyles[5];
            fields[0x3896] -= 7;
            fields[0x3898] += 7;
            fields[0x3899] += 7;
        }
    }
    buffer->box_sprite(work_rect, box_style);
    buffer->restore_clip();
}


/*
Purpose: Advance the satellite orbital overlay one tick: clear the orbit
         rectangle, integrate the sixteen satellites' positions from their
         velocities and the ReportWin's own step scalar at 0xEE30, decode
         the next Flic frame under them, composite it through the second
         surface buffer, then fill one 2x2 marker per live satellite and
         publish the orbit rectangle.

// ORIGINAL: 0x0049FE40 ?sat_anim@ReportWin@@QAEXXZ 0x0049FE40-0x0049FFB7 FILE
// size      375 bytes
// prototype void (__thiscall ?sat_anim@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5350 0x005D5440 0x005D5930 0x005D8370 0x0062A070 0x006458C8
// Homed from src/recovered/units/0049fe40.cpp. The four float arrays live
// in field_F938_'s storage (0xF938..0xFAB8): position pairs at 0xF938/
// 0xF978, scratch results at 0xF9B8/0xF9F8, velocities at 0xFA38, the
// live-satellite flags at 0xFA78; the loop cursor sits just before
// field_E2B0_ at 0xE29C. The 0xEE30 scalar is field_EE30_ (see the layout
// note above: it is NOT the Flic, which starts at 0xEE34), and the frame
// counters the decode reads are the Flic's own at 0xF3E0/0xF3E4.
Status: Coverage transcription - unclaimed body
*/
void ReportWin::sat_anim() {
    fill(&satellite_rect_, 0x41);
    field_E29C_ = 0;
    do {
        int i = field_E29C_;
        satellite_dx_[i] -= satellite_acceleration_[i] * satellite_x_[i];
        i = field_E29C_;
        satellite_dy_[i] -= satellite_acceleration_[i] * satellite_y_[i];
        i = field_E29C_;
        satellite_x_[i] += satellite_dx_[i] * field_EE30_;
        i = field_E29C_;
        satellite_y_[i] += satellite_dy_[i] * field_EE30_;
        ++field_E29C_;
    } while (field_E29C_ < 16);
    flic_.decode_stored_frame();
    buffer2_.draw(static_cast<Buffer *>(this), 9, field_E2A0_, field_E2A4_);
    field_E29C_ = 0;
    do {
        if (satellite_live_[field_E29C_] != 0) {
            fill(static_cast<int>(satellite_x_[field_E29C_]) + field_E2A8_,
                 static_cast<int>(satellite_y_[field_E29C_]) + field_E2AC_,
                 2, 2, 0xFF);
        }
        ++field_E29C_;
    } while (field_E29C_ < 16);
    soft_update(&satellite_rect_);
}

// ORIGINAL: 0x0049FFD0 ?start_attack@ReportWin@@QAEXHHHH@Z 0x0049FFD0-0x004A00F1
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/0049ffd0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::start_attack(int attacker, int target, int type, int base_id) {
    field_F924_ = attacker;
    field_F928_ = target;
    field_F92C_ = type;
    field_F930_ = base_id;
    field_F920_ = 0;
    field_F91C_ = 0;
    field_F934_ = 0;
    if (field_F918_ != 0) {
        const int counter = static_cast<int>(--field_F918_);
        const int index = counter % 16;
        if (counter < 16) satellite_live_[index] = 0;
        sat_anim();
        explosion_rect_.top = static_cast<int>(satellite_y_[index]) + field_E2AC_ - 23;
        explosion_rect_.left = static_cast<int>(satellite_x_[index]) + field_E2A8_ - 21;
        explosion_rect_.right = explosion_rect_.left + 43;
        explosion_rect_.bottom = explosion_rect_.top + 47;
        buffer1_.init(43, 47, 0, 0);
        static_cast<Buffer *>(this)->copy(&buffer1_, explosion_rect_.left,
            explosion_rect_.top, 0, 0, 43, 47);
        time3_.init(reinterpret_cast<void (__cdecl *)(int)>(sat_expl_daemon),
                    reinterpret_cast<int>(&ReportWinGlobal), 50, 5);
        time3_.start();
    }
}

// ORIGINAL: 0x004A0100 ?exp_anim@ReportWin@@QAEXXZ 0x004A0100-0x004A0248
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004a0100.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::exp_anim() {
    Buffer *canvas = static_cast<Buffer *>(this);
    canvas->set_clip(&satellite_rect_);
    if (field_F91C_ != 0) {
        time3_.stop();
        if (field_F920_ == 0) {
            field_F920_ = 1;
            buffer1_.draw(canvas, 9, explosion_rect_.left, explosion_rect_.top);
            update(&explosion_rect_, 0);
            action_sat_attack(field_F924_, field_F928_, field_F92C_, field_F930_);
            canvas->set_clip(const_cast<RECT *>(&canvas->clip_rect()));
            return;
        }
    } else {
        buffer1_.draw(canvas, 9, explosion_rect_.left, explosion_rect_.top);
        Sprite &frame = g_XI_BOOM_VEH_SPRITES[48 + field_F934_];
        frame.draw_multi_table_dest(canvas, frame.transparent_index(),
            explosion_rect_.left, explosion_rect_.top, &ExplosionColorTables[1][0][0]);
        if (++field_F934_ >= 16) field_F91C_ = 1;
        update(&explosion_rect_, 0);
    }
    canvas->set_clip(const_cast<RECT *>(&canvas->clip_rect()));
}

// ORIGINAL: 0x004989F0 ?on_group_clicked@ReportWin@@QAEXHHH@Z 0x004989F0-0x00498B06
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004989f0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::on_group_clicked(int group, int, int state) {
    time2_.stop();
    if (group == 4) ReportIfGlobal.field_1E240_ = state;
    redraw();
    if (group != 6) return;
    ReportIfGlobal.reset();
    switch (ReportIfGlobal.field_8_) {
        case 2: ReportIfGlobal.draw_labs(); break;
        case 3: ReportIfGlobal.draw_energy(); break;
        case 4: ReportIfGlobal.draw_ops(); break;
        case 5: ReportIfGlobal.draw_projects(); break;
        case 6: ReportIfGlobal.draw_sat(); break;
        case 7: ReportIfGlobal.draw_security(); break;
        case 8: ReportIfGlobal.draw_score(); break;
        case 0: ReportIfGlobal.draw_intel(); break;
    }
    MainInterfaceGlobal.soft_update();
}

// ORIGINAL: 0x004986D0 ?on_button_clicked@ReportWin@@QAEXH@Z 0x004986D0-0x004989A8
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004986d0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
// symbol    ?on_button_clicked@ReportWin@@UAEXH@Z
void ReportWin::on_button_clicked(int id) {
    const bool datalink_mode = IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 8;
    const int page = ReportIfGlobal.field_8_;
    if (id < 0x378) {
        if (datalink_mode) return;
        if (id == 9) {
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            return;
        }
        if (id != 1) return;
        switch (page) {
            case 0: DatalinkMain.exec(15, ReportIfGlobal.report_selection_); return;
            case 2: DatalinkMain.exec(14, static_cast<int>(field_FAB8_) == -1 ? 62 : field_FAB8_); return;
            case 3: DatalinkMain.exec(0, 43); return;
            case 4: DatalinkMain.exec(10, 29); return;
            case 5: {
                int topic = field_59AC_;
                if (topic < 1024) {
                    topic += 70;
                    if (topic < 70) topic = 71;
                } else {
                    topic = -Bases[topic - 1024].queue_production_id[0];
                }
                DatalinkMain.exec(11, topic);
                return;
            }
            case 6: DatalinkMain.exec(14, Facility[65].preq_tech); return;
            case 7: DatalinkMain.exec(3, 7); return;
            case 8: DatalinkMain.exec(1, 4); return;
        }
        return;
    }
    if (page == 2) {
        const int old_tech = field_FAB8_;
        switch (id) {
            case 0x379: field_FAB8_ = Technology[old_tech].preq_tech_1; break;
            case 0x37A: field_FAB8_ = Technology[old_tech].preq_tech_2; break;
            case 0x37B: case 0x37C: case 0x37D: case 0x37E: {
                int count = 0;
                for (int i = 0; i < 178; ++i) {
                    int preq = (i & 1) ? Technology[i >> 1].preq_tech_2
                                       : Technology[i >> 1].preq_tech_1;
                    if (preq == old_tech && ++count == id - 0x37A) {
                        field_FAB8_ = i >> 1;
                    }
                }
                break;
            }
        }
        ReportIfGlobal.listBox1_.set_selected_id(field_FAB8_);
        if (datalink_mode) DatalinkMain.set_index(field_FAB8_, 0);
        else redraw();
        return;
    }
    if (page == 6 && id == 0xD3E) {
        const int target = ReportIfGlobal.buttonGroup3_.field_84_;
        const int type = static_cast<int>(ReportIfGlobal.buttonGroup4_.field_84_) - 65;
        if (target > 0 && target < 8 && type >= 0 && type < 5) {
            sat_attack(ConsoleGlobal.faction_id_, target, type);
            return;
        }
        flatButton9_.hide();
    }
}

// ORIGINAL: 0x00497AC0 ?right_menu@ReportWin@@QAEXHHH@Z 0x00497AC0-0x00497E69;0x00659420-0x00659435
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/00497ac0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::right_menu(int base_id, int x, int y) {
    PullDown menu;
    if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
        || reportwin_input_blocked()) return;
    client_to_screen(&x, &y);
    menu.init(0);
    menu.add_item(1, label_get(0x894 / 4));
    menu.add_item(100, label_get(0x910 / 4));
    menu.add_separator(99);
    menu.add_item(2, label_get(0x898 / 4));
    menu.add_item(3, label_get(0x89C / 4));
    menu.add_separator(99);
    menu.add_item(4, label_get(0x8A0 / 4));
    menu.add_item(5, label_get(0x8A4 / 4));
    menu.add_item(6, label_get(0x8A8 / 4));
    menu.add_item(7, label_get(0x8AC / 4));
    menu.add_item(8, label_get(0xBFC / 4));
    menu.add_item(9, label_get(0xA54 / 4));
    menu.show(x, y, pop_expire);
    if (GameDataStateStorage.fields_.game_state_ & 0x800) return;
    const int selected = menu.get_selected();
    // The original performs only the signed upper-bound check here.
    const int action = selected < 64 ? menu.item_id(selected) : -1;
    TutWinBaseWindow.soft_zoom(base_id);
    switch (action) {
        case 1: TutWinBaseWindow.zoom(base_id, 0); break;
        case 2:
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            TutWinBaseWindow.production(0, 0);
            return;
        case 3: TutWinBaseWindow.hurry(); break;
        case 4: TutWinBaseWindow.gov_on(0x1000000); break;
        case 5: TutWinBaseWindow.gov_on(0x2000000); break;
        case 6: TutWinBaseWindow.gov_on(0x4000000); break;
        case 7: TutWinBaseWindow.gov_on(0x8000000); break;
        case 8: BaseCurrent->state &= 0x7FFFFFFF; break;
        case 9: TutWinBaseWindow.governor(0); break;
        case 100:
            MapWinTable[0]->set_center(BaseCurrent->x, BaseCurrent->y, 1);
            ReportWinGlobal.release_modal();
            ReportIfGlobal.release_iface_mode();
            break;
    }
    redraw();
}

// ORIGINAL: 0x00497F00 ?on_right_click@ReportWin@@QAEXHH@Z 0x00497F00-0x004980EF
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/00497f00.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::on_right_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int spot_id, payload;
    if (spot_.check(x, y, &spot_id, &payload) < 0) return;
    if (ReportIfGlobal.field_8_ == 4) {
        if (spot_id > 1023) {
            const int first_id = spot_id - 1024;
            if (first_id < 1024) {
                if (Bases[first_id].faction_id_current != ConsoleGlobal.faction_id_) return;
                right_menu(first_id, x, spot_id);
                return;
            }
            const int base_id = spot_id - 2048;
            if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) return;
            log_say("Reports right_click garrison", payload, base_id, 0);
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.garrison_click(payload, x, spot_id, 1);
            return;
        }
        log_say("Base report psych right click", 0, 0, 0);
        TutWinBaseWindow.soft_zoom(spot_id);
        if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
        if (BaseCurrent->specialist_total != 0) --BaseCurrent->specialist_total;
        base_compute(1);
        NetDaemonNet.unlock_base(BaseIDCurrentSelected);
    } else if (ReportIfGlobal.field_8_ == 6
        && (GameDataStateStorage.fields_.game_state_ & 0x40) != 0 && (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
        sat_edit(ReportIfGlobal.report_faction_id_, payload, spot_id);
        redraw();
        return;
    }
    redraw();
}

// ORIGINAL: 0x004A08E0 ?draw_security@ReportWin@@QAEXXZ 0x004A08E0-0x004A1134
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a08e0.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_security() {
    Buffer *canvas = static_cast<Buffer *>(this);
    const int faction_id = ReportIfGlobal.report_faction_id_;
    const PlayerData &faction = PlayersData[faction_id];
    const int filter = ReportIfGlobal.field_1E244_;
    RECT title = { energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                   energy_panel_rect_.right - 3, 0 };
    const int first_y = energy_panel_rect_.top + 9 + field_E268_;
    title.bottom = first_y;
    RECT row = { title.left, first_y + 2, title.right, first_y + 43 };
    canvas->box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas->box_sprite(&title, &InterfaceBoxStyles[1]);
    const int rows = (energy_panel_rect_.bottom - row.top) / 43;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, get_adjective(faction_id));
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAD8 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas->write_cent_l(StringTemp, &title, strlen(StringTemp));
    ++title.top;
    ++title.bottom;
    spot_.kill_type(0);

    int last_row = -1;
    for (int i = 0; i < 128; ++i) {
        const int id = i < 64 ? i : faction_id * 64 - 64 + i;
        if ((VehPrototypes[id].flags & 1) != 0
            && reportwin_security_filter(id, filter)
            && faction.proto_id_lost[id] + (faction.proto_id_queue[id] != 0)
                + (faction.proto_id_active[id] != 0) != 0) ++last_row;
    }
    if (field_C0F8_ == 0) {
        if (rows <= last_row) field_E26C_ = 1;
    } else if (last_row < rows) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_ != 0) {
        field_C108_ = rows - 1;
        field_C0FC_ = 0;
        field_C100_ = 0;
        field_C104_ = last_row;
        scroll_.set_range(0, last_row);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    if (last_row == -1) return;
    int ordinal = -1;
    for (int index = 0; index < 128; ++index) {
        const int id = index < 64 ? index : faction_id * 64 - 64 + index;
        const VehPrototype &prototype = VehPrototypes[id];
        if ((prototype.flags & 1) == 0 || !reportwin_security_filter(id, filter)
            || (faction.proto_id_queue[id] != 0) + (faction.proto_id_active[id] != 0)
                + faction.proto_id_lost[id] == 0) continue;
        ++ordinal;
        const int first = field_C0FC_;
        if (ordinal < first || ordinal >= rows + first) continue;
        if (ordinal != first) { row.top += 43; row.bottom += 43; }
        const bool selected = (selected_prototypes_[index >> 5] & (1u << (index & 31))) != 0;
        if (selected) canvas->box_sprite(&row, &InterfaceBoxStyles[2]);
        canvas->box_sprite(&row, &InterfaceBoxStyles[selected ? 5 : 1]);
        spot_.add(id, 0, row.left, row.top, row.right - row.left, row.bottom - row.top);
        rect1_.top = row.top;
        const int text_y = row.top - field_E268_ / 2 + 21;
        const int vehicle = veh_fake(id, faction_id);
        veh_draw(canvas, vehicle, energy_panel_rect_.left, row.top + 2, -6, 8, 0);
        canvas->set_text_color(0x99, -1, 1, 1);
        canvas->set_text_color2(0xF2, -1, 1, 1);
        canvas->set_text_color3(0xF9, -1, 1, 1);
        canvas->set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_,
                         ReportIfGlobal.field_1E18C_, 0);
        rect1_.left = 102;
        StringTemp[0] = 0;
        if (prototype.obsolete_factions & (1u << (faction_id & 31))) strcat(StringTemp, "-");
        strcat(StringTemp, VehPrototypes[id].veh_name);
        canvas->write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
        rect1_.left = (rect1_.right >> 1) + 40;
        StringTemp[0] = 0;
        say_stats(StringTemp, id, 0);
        canvas->write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        reportwin_append_number(StringTemp, faction.proto_id_active[id] > 250 ? 0 : faction.proto_id_active[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x614 / 4)); strcat(StringTemp, ", ");
        strcat(StringTemp, "}");
        reportwin_append_number(StringTemp, faction.proto_id_queue[id] > 250 ? 0 : faction.proto_id_queue[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x618 / 4)); strcat(StringTemp, ", ");
        strcat(StringTemp, "[");
        reportwin_append_number(StringTemp, faction.proto_id_lost[id]);
        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x61C / 4)); strcat(StringTemp, "]");
        canvas->write_right_l(StringTemp, rect1_.right - 2, text_y, strlen(StringTemp));
        rect1_.top += 43;
    }
}

// ORIGINAL: 0x00496E10 ?on_left_click@ReportWin@@QAEXHH@Z 0x00496E10-0x00497A30
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/00496e10.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::on_left_click(int x, int y) {
    if (reportwin_input_blocked()) return;
    int selected, payload;
    const int hit = spot_.check(x, y, &selected, &payload);
    const int page = ReportIfGlobal.field_8_;
    if (hit == -1) {
        if (page != 4) return;
        field_5A08_ = 0;
        for (int i = 0; i < 512; ++i) {
            ReportSortEntry &entry = ReportIfGlobal.sort_entries_[i];
            entry.base_id = entry.sorted_base_id = i;
            entry.value = entry.sorted_value = 0;
        }
        redraw();
        ReportIfGlobal.draw_ops();
        MainInterfaceGlobal.soft_update();
        return;
    }
    int blink_base = 0;
    bool blink = false;
    if (page == 2) {
        if (selected != 0x29A) return;
        const int tech = PlayersData[ReportIfGlobal.report_faction_id_].tech_id_researching;
        if (tech > -1 && ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 || (GameDataStateStorage.fields_.game_rules_ & 0x200) == 0)) {
            field_FAB8_ = field_59AC_ = tech;
            reset(); draw_labs(); soft_update();
            ReportIfGlobal.reset(); ReportIfGlobal.draw_labs();
            MainInterfaceGlobal.soft_update();
        }
        if ((GameDataStateStorage.fields_.game_rules_ & 0x200) == 0) ConsoleGlobal.set_tech(ConsoleGlobal.faction_id_);
        else ConsoleGlobal.set_ai(ConsoleGlobal.faction_id_);
        reset(); draw_labs(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_labs();
        MainInterfaceGlobal.soft_update();
        return;
    }
    if (page == 4) {
        int last = selected - 1024;
        if (selected < 1024) {
            if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0) {
                if (selected > -1) {
                    TutWinBaseWindow.soft_zoom(selected);
                    const int specialist = selected < BaseCurrent->specialist_total
                        ? BaseCurrent->specialist_total - selected - 1 : -1;
                    TutWinBaseWindow.pop_click(specialist, 0, 0, 0);
                    base_compute(1);
                }
            } else {
                TutWinBaseWindow.soft_zoom(selected);
                log_say("Base report psych click", 0, 0, 0);
                if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) return;
                for (int square = 20; square > 0; --square) {
                    const uint32_t bit = 1u << (square & 31);
                    if (BaseCurrent->resource_sqr_radius & bit) {
                        BaseCurrent->resource_sqr_radius &= ~bit;
                        break;
                    }
                }
                ++BaseCurrent->specialist_total;
                base_compute(1);
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        } else if (last > 1023) {
            int base_id = selected - 2048;
            if (base_id < 1024 ||
                (base_id = selected - 3072, Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_)) {
                if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) return;
                log_say("Reports left_click garrison", base_id, payload, 0);
                TutWinBaseWindow.soft_zoom(base_id);
                TutWinBaseWindow.garrison_click(payload, x, y, 0);
                return;
            }
            log_say("Global EDBC base set", 0, 0, 0);
            if (NetDaemonNet.lock_base(base_id, 0, -1, -1) == 0) {
                TutWinBaseWindow.soft_zoom(base_id);
                const uint32_t governor = BaseCurrent->governor;
                if ((governor & 0x80000000u) == 0 || (governor & 0x8000000u) == 0) {
                    unsigned category;
                    if ((governor & 0x1000000u) != 0) category = 0;
                    else if ((governor & 0x2000000u) != 0) category = 1;
                    else category = ((~governor & 0x4000000u) | 0x8000000u) >> 26;
                    BaseCurrent->governor = (governor | 0x80000000u) & 0xF0FFFFFFu;
                    switch (category) {
                        case 0: BaseCurrent->governor |= 0x2000000u; break;
                        case 1: BaseCurrent->governor |= 0x4000000u; break;
                        case 2: BaseCurrent->governor |= 0x8000000u; break;
                        case 3: BaseCurrent->governor |= 0x1000000u; break;
                    }
                    TutWinBaseWindow.gov_activate();
                } else BaseCurrent->governor = governor & 0x7FFFFFFFu;
                draw_radius(Bases[base_id].x, Bases[base_id].y, 2, 2);
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            }
        } else {
            if (Bases[last].faction_id_current != ConsoleGlobal.faction_id_) return;
            const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
            const bool control = (GetAsyncKeyState(VK_CONTROL) & 0xFF00) != 0;
            if (!shift && control) selected_bases_[last / 32] ^= 1u << (last & 31);
            else if (!control) {
                for (int word = 0; word < 16; ++word) selected_bases_[word] = 0;
                if (!shift || field_59AC_ == 0) selected_bases_[last / 32] |= 1u << (last & 31);
                else {
                    int first = static_cast<int>(field_59AC_) - 1024;
                    field_59AC_ = first;
                    if (last < first) { field_59AC_ = last; last = first; first = field_59AC_; }
                    for (int base_id = first; base_id <= last; ++base_id) {
                        if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_)
                            selected_bases_[base_id / 32] |= 1u << (base_id & 31);
                    }
                }
            }
        }
        int count = 0;
        for (int id = 0; id < 512; ++id) if (selected_bases_[id / 32] & (1u << (id & 31))) ++count;
        field_5A04_ = count == 1;
        field_59AC_ = selected;
        reset(); draw_ops(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_ops();
        MainInterfaceGlobal.soft_update();
        const int base_id = selected % 1024;
        if (field_5A04_ && (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
            || (Bases[base_id].visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) != 0)) { blink_base = base_id; blink = true; }
    } else if (page == 5) {
        field_59AC_ = selected;
        reset(); draw_projects(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_projects();
        MainInterfaceGlobal.soft_update();
        const int base_id = selected < 1024 ? base_project(selected) : selected - 1024;
        if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_
            && (Bases[base_id].visibility & (1u << (ConsoleGlobal.faction_id_ & 31))) == 0) return;
        blink_base = base_id;
        blink = true;
    } else if (page == 7) {
        if (ReportIfGlobal.report_faction_id_ != ConsoleGlobal.faction_id_) return;
        const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
        const bool control = (GetAsyncKeyState(VK_CONTROL) & 0xFF00) != 0;
        const int filter = ReportIfGlobal.field_1E244_;
        const int clicked_triad = Chassis[VehPrototypes[selected].chassis_id].triad;
        if (!control) {
            for (int row = 0; row < 128; ++row) {
                const int id = row < 64 ? row : ReportIfGlobal.report_faction_id_ * 64 - 64 + row;
                const bool matches = filter < 3
                    ? Chassis[VehPrototypes[id].chassis_id].triad == clicked_triad
                        && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0
                    : filter == 3 ? Weapon[VehPrototypes[id].weapon_id].offense_rating == 0 : filter == 4;
                if (matches) selected_prototypes_[row / 32] &= ~(1u << (row & 31));
            }
        }
        if (!shift) {
            const int row = selected % 64 + (selected > 63 ? 64 : 0);
            if (control) selected_prototypes_[row / 32] ^= 1u << (row & 31);
            else selected_prototypes_[row / 32] |= 1u << (row & 31);
        } else {
            int first_id = field_59AC_, last_id = selected;
            if (last_id < first_id) { field_59AC_ = last_id; last_id = first_id; first_id = field_59AC_; }
            int first = first_id % 64 + (first_id > 63 ? 64 : 0);
            const int last_row = last_id % 64 + (last_id > 63 ? 64 : 0);
            for (; first <= last_row; ++first) {
                const int id = first < 64 ? first : ReportIfGlobal.report_faction_id_ * 64 - 64 + first;
                const bool matches = filter < 3
                    ? Chassis[VehPrototypes[id].chassis_id].triad == Chassis[VehPrototypes[last_id].chassis_id].triad
                        && Weapon[VehPrototypes[id].weapon_id].offense_rating != 0
                    : filter == 3 ? Weapon[VehPrototypes[id].weapon_id].offense_rating == 0 : filter == 4;
                if (matches) selected_prototypes_[first / 32] |= 1u << (first & 31);
            }
        }
        field_59AC_ = selected;
        reset(); draw_security(); soft_update();
        ReportIfGlobal.reset(); ReportIfGlobal.draw_security();
        MainInterfaceGlobal.soft_update();
        return;
    }
    if (!blink) return;
    WorldClimateWorldWin.tile_to_pixel(Bases[blink_base].x, Bases[blink_base].y,
        &ReportIfGlobal.animation_x_, &ReportIfGlobal.animation_y_);
    ReportIfGlobal.animation_x_ -= 7;
    ReportIfGlobal.animation_y_ -= 5;
    ReportIfGlobal.animation_state_ = -1;
    ReportIfGlobal.bl_anim();
    time1_.init(reinterpret_cast<void (__cdecl *)(int)>(bl_daemon),
        reinterpret_cast<int>(&ReportIfGlobal), 150, 5);
    time1_.start();
}
int __cdecl X_text_open(char *, const char *);
void __cdecl compute_score(int, int *, int *, int);
void __cdecl diplomacy_check(int, int, int);
int __cdecl most_objectives(int *, int *);
// compute_score/diplomacy_check remain seed-only dependencies requiring owner
// integration. ClimatePopupTitle is an existing legacy storage seam.

static void report_topic_number(int n) {
    char number[80];
    _itoa(n, number, 10);
    strcat(StringTemp, number);
}
static void report_topic_line(Buffer &canvas, RECT &cursor, int x) {
    canvas.write_l(StringTemp, x, cursor.top, strlen(StringTemp));
}
static void report_intel_label(int label) {
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(label));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    strcat(StringTemp, " ");
    strcat(StringTemp, "]");
}
static int report_social_model(const SocialCategory &selection, int category) {
    switch (category) {
    case 0: return selection.models_[SocialCategory::POLITICS];
    case 1: return selection.models_[SocialCategory::ECONOMICS];
    case 2: return selection.models_[SocialCategory::VALUES];
    default: return selection.models_[SocialCategory::FUTURE];
    }
}
static void report_append_leader(const Player &player) {
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    strcat(StringTemp, player.title_leader);
    strcat(StringTemp, " ");
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    strcat(StringTemp, player.name_leader);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2D0 / 4));
    strcat(StringTemp, " ");
    GenderDefault = player.noun_gender;
    PluralityDefault = player.is_noun_plural;
    strcat(StringTemp, player.noun_faction);
}

// ORIGINAL: 0x004A2210 ?draw_intel@ReportWin@@QAEXXZ 0x004A2210-0x004A345F
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a2210.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_intel() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int viewer = ReportIfGlobal.report_faction_id();
    const int subject = ReportIfGlobal.report_selection_;
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    RECT body = {title.left, title.bottom + 2, title.right,
                 energy_panel_rect_.bottom - 3};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[viewer].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAE0 / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    rect1_.left = body.left + 4;
    rect1_.top = body.top + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    const bool informed = (GameDataStateStorage.fields_.game_state_ & 0x80) || spying(subject) ||
        (PlayersData[viewer].diplo_treaties[subject] & 1);
    Sprite &portrait = FactionsArt[subject].council_big;
    portrait.draw(&canvas, portrait.transparent_index(), rect1_.left, rect1_.top);
    const int text_x = rect1_.left + 158;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    report_append_leader(Players[subject]);
    strcat(StringTemp, "]");
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_ * 2;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE98 / 4);
    const unsigned int relation = PlayersData[viewer].diplo_treaties[subject];
    if (relation & 0x10) strcat(StringTemp, label_get(0x358 / 4));
    else if (relation & 1) strcat(StringTemp, label_get(0x34C / 4));
    else if (relation & 2) strcat(StringTemp, label_get(0x350 / 4));
    else if (relation & 4) strcat(StringTemp, label_get(0x354 / 4));
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    diplomacy_check(viewer, subject, 0);
    report_intel_label(0xE94 / 4);
    say_special(StringTemp, StringTable.get(reinterpret_cast<int>(Mood[get_mood(DiploFriction)])), subject);
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5A0 / 4);
    const int fight = PlayersData[subject].ai_fight;
    strcat(StringTemp, label_get(fight < 0 ? 0x5AC / 4 : fight == 0 ? 0x5A8 / 4 : 0x5A4 / 4));
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x5B0 / 4));
    CharUpperA(StringTemp);
    strcat(StringTemp, ":   ]");
    int priorities = 0;
    const PlayerData &data = PlayersData[subject];
    const int interests[4] = {data.ai_growth, data.ai_tech, data.ai_wealth, data.ai_power};
    for (int i = 0; i < 4; ++i) {
        if (interests[i]) {
            if (priorities) strcat(StringTemp, ", ");
            strcat(StringTemp, label_get(0x824 / 4 + i));
            ++priorities;
        }
    }
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0x5C0 / 4);
    const int category = Players[subject].soc_ideology_category;
    const int model = Players[subject].soc_ideology_model;
    if (category >= 0 && model != 0) {
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(SocialCategories[category].type)));
        strcat(StringTemp, ": ");
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(SocialCategories[category].name[model])));
        CharUpperA(StringTemp);
        report_topic_line(canvas, rect1_, text_x);
        rect1_.top += field_E268_;
    }
    int headquarters = -1;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == subject && has_fac(FAC_HEADQUARTERS, base, 0)) {
            headquarters = base;
            break;
        }
    }
    report_intel_label(0x5C4 / 4);
    say_base(StringTemp, headquarters);
    CharUpperA(StringTemp);
    report_topic_line(canvas, rect1_, text_x);
    rect1_.top += field_E268_;
    report_intel_label(0xE4 / 4);
    report_topic_number(PlayersData[subject].energy_reserves);
    if (informed) {
        report_topic_line(canvas, rect1_, text_x);
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x254 / 4));
        strcat(StringTemp, ": ]");
        say_tech(StringTemp, PlayersData[subject].tech_id_researching, 0);
        if (informed) {
            report_topic_line(canvas, rect1_, text_x);
            rect1_.top += field_E268_;
        }
    }
    if (!(Players[subject].rule_flags & RFLAG_ALIEN)) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0xF00 / 4));
        CharUpperA(StringTemp);
        strcat(StringTemp, ":   ]");
        report_topic_number(council_votes(subject));
        report_topic_line(canvas, rect1_, text_x);
    }
    rect1_.top += field_E268_ + 20;
    for (int category_index = 0; category_index < 4; ++category_index) {
        StringTemp[0] = 0;
        strcat(StringTemp, "[");
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(SocialCategories[category_index].type)));
        strcat(StringTemp, ":   ]");
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(SocialCategories[category_index].name[
            report_social_model(PlayersData[subject].soc_category_pending, category_index)])));
        CharUpperA(StringTemp);
        canvas.write_l(StringTemp, rect1_.left + 20,
                       rect1_.top + category_index * field_E268_, strlen(StringTemp));
    }
    for (int faction = 1; faction < 8; ++faction) {
        const unsigned int flags = PlayersData[subject].diplo_treaties[faction];
        if (faction == subject || !(flags & 8) ||
            !(factions_status_byte(1) & (1 << faction)) ||
            (!informed && faction != viewer && !(PlayersData[viewer].diplo_treaties[faction] & 1))) continue;
        StringTemp[0] = 0;
        if (flags & 0x10) {
            strcat(StringTemp, label_get(0x358 / 4)); strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x5C8 / 4));
        } else if (flags & 1) {
            strcat(StringTemp, get_pact(subject)); strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x43C / 4));
        } else {
            if (flags & 2) strcat(StringTemp, label_get(0x350 / 4));
            else {
                if (flags & 0x10000) { strcat(StringTemp, label_get(0x5D0 / 4)); strcat(StringTemp, " "); }
                strcat(StringTemp, label_get(0x354 / 4));
            }
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(0x5CC / 4));
        }
        strcat(StringTemp, " ");
        report_append_leader(Players[faction]);
        report_topic_line(canvas, rect1_, text_x + 125);
        rect1_.top += field_E268_;
    }
}

// Requires coordinator-reviewed PlayerData final-dword migration:
// int score_items_[9] at +0x20A8, replacing unk_110..unk_118 including the
// intervening char[4]. Original4A1318 accesses9signedints;4A16C9 bounds9.
// ORIGINAL: 0x004A1140 ?draw_score@ReportWin@@QAEXXZ 0x004A1140-0x004A2208
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a1140.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_score() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id();
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
                  energy_panel_rect_.right - 3,
                  energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    RECT body = {title.left, title.bottom + 2, title.right, energy_panel_rect_.bottom - 3};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&body, &InterfaceBoxStyles[1]);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xADC / 4));
    strcat(StringTemp, "]");
    --title.top;
    --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    compute_score(faction, 0, 0, 0);
    if (X_text_open(reinterpret_cast<char *>(*ClimatePopupTitle), "SCORING")) return;
    rect1_.left = body.left + 4;
    rect1_.top = body.top + 4;
    rect1_.right = body.right - body.left - 8;
    rect1_.bottom = rect1_.right;
    for (int item = 0; item < 9; ++item) {
        text_get();
        if (PlayersData[faction].score_items_[item] == 0 || item == 5) continue;
        StringTemp[0] = 0;
        if (item == 6) {
            const int own = num_objectives(faction, 0);
            const int total = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
            report_topic_number(own);
            strcat(StringTemp, " ");
            if (total > own) {
                strcat(StringTemp, label_get(0xE0C / 4));
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_;
                StringTemp[0] = 0;
                report_topic_number(total - own);
                strcat(StringTemp, " ");
                strcat(StringTemp, label_get(0x1030 / 4));
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += static_cast<int>(field_E268_ * 3) / 2;
                StringTemp[0] = 0;
            }
            strcat(StringTemp, TextBufferGetPtr);
        } else if (item == 7) {
            if (GameDataStateStorage.fields_.game_state_ & 0x200000) strcat(StringTemp, label_get(0xE70 / 4));
            else if (GameDataStateStorage.fields_.game_state_ & 0x400000) strcat(StringTemp, label_get(0xE88 / 4));
            else if (PlayersData[faction].flags & 0x100000) strcat(StringTemp, label_get(0x1198 / 4));
            else if (!(GameDataStateStorage.fields_.game_state_ & 0x2000)) strcat(StringTemp, label_get(0xE10 / 4));
            else strcat(StringTemp, TextBufferGetPtr);
        } else strcat(StringTemp, TextBufferGetPtr);
        report_topic_line(canvas, rect1_, rect1_.left);
        StringTemp[0] = 0;
        const int value = PlayersData[faction].score_items_[item];
        strcat(StringTemp, value < 0 ? "- " : "+ ");
        report_topic_number(abs(value));
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    if (PlayersData[faction].score_items_[5]) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(MapNativeLifeForms ? 0x908 / 4 : 0x904 / 4));
        strcat(StringTemp, " ");
        strcat(StringTemp, MapNativeLifeForms ? "+25%" : "-25%");
        report_topic_line(canvas, rect1_, rect1_.left);
        StringTemp[0] = 0;
        const int value = PlayersData[faction].score_items_[5];
        strcat(StringTemp, value < 0 ? "- " : "+ ");
        report_topic_number(abs(value));
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400) && !(GameDataStateStorage.fields_.game_state_ & 0x20) && PlayersData[faction].unk_109 > 0) {
        rect1_.top += field_E268_;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1044 / 4));
        CharUpperA(StringTemp);
        report_topic_line(canvas, rect1_, rect1_.left);
        strcpy(StringTemp, "+100%");
        canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                             rect1_.top, strlen(StringTemp));
        rect1_.top += field_E268_;
    }
    rect1_.top += field_E268_;
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp, label_get(0x6C0 / 4));
    strcat(StringTemp, "}");
    report_topic_line(canvas, rect1_, rect1_.left);
    strcpy(StringTemp, "{");
    if (PlayersData[faction].unk_109 < 0) strcat(StringTemp, "- ");
    report_topic_number(abs(PlayersData[faction].unk_107));
    strcat(StringTemp, "}");
    canvas.write_right_l(StringTemp, rect1_.left + rect1_.right,
                         rect1_.top, strlen(StringTemp));
    rect1_.top += field_E268_ * 2;
    if (GameDataStateStorage.fields_.game_state_ & 1) {
        if (GameDataStateStorage.fields_.game_state_ & 0x80000) {
            StringTemp[0] = 0;
            if (PlayersData[faction].score_items_[7]) strcat(StringTemp, label_get(0xE10 / 4));
            else {
                strcat(StringTemp, label_get(0xE14 / 4));
                for (int other = 1; other < 8; ++other) {
                    if (other == faction) continue;
                    compute_score(other, 0, 0, 0);
                    if (PlayersData[other].score_items_[7]) {
                        strcat(StringTemp, " "); strcat(StringTemp, "(");
                        PluralityDefault = 0;
                        GenderDefault = Players[other].is_leader_female;
                        strcat(StringTemp, Players[other].name_leader);
                        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, ")");
                        break;
                    }
                }
                CharUpperA(StringTemp);
                strcat(StringTemp, "!");
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_ * 2;
            }
        } else if (!(GameDataStateStorage.fields_.game_state_ & 0x602000) && ScenarioSave.objective_req_victory <= 9000) {
            StringTemp[0] = 0;
            // Original reuses the faction byte-index local as the outcome.
            // The unmet nonzero objective target with item 8 nonzero leaves it
            // intact; preserve that malformed DRAWSTRING selection path.
            int outcome = faction * sizeof(PlayerData);
            int most = 0;
            if (ScenarioSave.objective_req_victory == 0) {
                const int objectives = num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000);
                int leader, pacts;
                most = most_objectives(&leader, &pacts);
                if (objectives >= (most < 0 ? 0 : most)) {
                    if (pacts) { strcat(StringTemp, label_get(0x102C / 4)); outcome = 2; }
                    else { strcat(StringTemp, label_get(0xE10 / 4)); outcome = 1; }
                } else {
                    outcome = 0;
                    strcat(StringTemp, label_get(0xE14 / 4));
                    if (leader > 0) {
                        strcat(StringTemp, " "); strcat(StringTemp, "(");
                        PluralityDefault = 0;
                        GenderDefault = Players[leader].is_leader_female;
                        strcat(StringTemp, Players[leader].name_leader);
                        strcat(StringTemp, " "); strcat(StringTemp, label_get(0x101C / 4));
                        strcat(StringTemp, ")");
                    }
                }
            } else if (num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000) >= ScenarioSave.objective_req_victory) {
                strcat(StringTemp, label_get(0xE10 / 4)); outcome = 1;
            } else if (PlayersData[faction].score_items_[8] == 0) {
                outcome = 0;
                strcat(StringTemp, label_get(0xE14 / 4));
            }
            CharUpperA(StringTemp);
            strcat(StringTemp, "!");
            report_topic_line(canvas, rect1_, rect1_.left);
            rect1_.top += field_E268_ * 2;
            for (int other = 1; other < 8; ++other) {
                if (!(factions_status_byte(1) & (1 << other)) || other == faction) continue;
                const int target = ScenarioSave.objective_req_victory ? ScenarioSave.objective_req_victory : most;
                if (num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000) < target) continue;
                if (!(factions_status_byte(0) & (1 << other)) && (GameDataStateStorage.fields_.game_rules_ & 0x80000)) continue;
                const char *section = outcome > 1 ? "DRAWSTRING" : outcome ? "WINSTRING2" : "WINSTRING";
                if (X_text_open(reinterpret_cast<char *>(*ClimatePopupTitle), section)) break;
                text_get();
                PluralityDefault = 0; GenderDefault = Players[other].is_leader_female;
                parse_says(0, Players[other].title_leader, -1, -1);
                PluralityDefault = 0; GenderDefault = Players[other].is_leader_female;
                parse_says(1, Players[other].name_leader, -1, -1);
                GenderDefault = Players[other].noun_gender;
                PluralityDefault = Players[other].is_noun_plural;
                parse_says(2, Players[other].noun_faction, -1, -1);
                StringTemp[0] = 0;
                report_topic_number(num_objectives(other, 0));
                const int all = num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000);
                if (all > num_objectives(other, 0)) {
                    strcat(StringTemp, "(");
                    report_topic_number(num_objectives(other, GameDataStateStorage.fields_.game_rules_ & 0x1000));
                    strcat(StringTemp, ")");
                }
                parse_says(3, StringTemp, -1, -1);
                StringTemp[0] = 0;
                parse_string(TextBufferGetPtr, StringTemp);
                report_topic_line(canvas, rect1_, rect1_.left);
                rect1_.top += field_E268_;
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x20) {
        canvas.set_text_color(0xF9, -1, 1, 1);
        canvas.set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1060 / 4));
        strcat(StringTemp, "!");
        canvas.write_cent_l(StringTemp, rect1_.left, energy_panel_rect_.bottom - 50,
                            rect1_.right, 50, strlen(StringTemp));
    }
    text_close();
}
static unsigned int report_sat_count(const PlayerData &data, int kind) {
    switch (kind) {
    case 0: return data.satellites_[0];
    case 1: return data.satellites_[1];
    case 2: return data.satellites_[2];
    default: return data.satellites_[3];
    }
}

// ORIGINAL: 0x0049D540 ?draw_ops@ReportWin@@QAEXXZ 0x0049D540-0x0049EAEB;0x00659435-0x00659447
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0049d540.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_ops() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id_;
    Random rng;
    rng.reseed(timeGetTime());
    field_59A0_ = field_59A4_ = 40;
    const int split = energy_panel_rect_.top + 34 + field_E268_;
    RECT upper = {energy_panel_rect_.left, energy_panel_rect_.top,
        energy_panel_rect_.right, split};
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
        energy_panel_rect_.right - 3, energy_panel_rect_.top + field_E268_ + 9};
    RECT lower = {energy_panel_rect_.left, split + 2,
        energy_panel_rect_.right, energy_panel_rect_.bottom};
    RECT icon = {energy_panel_rect_.left + 3, split + 5,
        energy_panel_rect_.left + 43, split + 45};
    RECT text = {energy_panel_rect_.left + 45, split + 5,
        energy_panel_rect_.right - 3, split + 45};
    canvas.box_sprite(&upper, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    canvas.box_sprite(&lower, &InterfaceBoxStyles[0]);
    const int pitch = field_59A4_ + 2;
    const int rows = (lower.bottom - lower.top - 6) / pitch;
    flatButton4_.show(0); flatButton5_.show(0); flatButton6_.show(0);
    const int count = PlayersData[faction].current_num_bases;
    if (!field_C0F8_) {
        if (rows <= count) field_E26C_ = 1;
    } else if (count < rows) {
        scroll_hide(this, &scroll_);
        field_C0F8_ = 0;
    }
    if (field_E270_) {
        field_C108_ = rows - 1;
        field_C0FC_ = field_C100_ = 0;
        field_C104_ = count - 1;
        scroll_.set_range(0, count - 1);
        scroll_.paging_ = field_C108_;
        scroll_.set_pos(field_C0FC_);
        field_E270_ = 0;
    }
    spot_.kill_type(0);
    spot_.reset_count();
    strcpy(StringTemp, "[");
    strcat(StringTemp, get_adjective(faction));
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xACC / 4));
    if (field_5A08_) {
        strcat(StringTemp, " - ");
        const int labels[8] = {0xA0, 0xA4, 0xE4, 0xAAC, 0xAB0, 0xAB4, 0xFC, 0xAB8};
        if (field_5A08_ >= 1 && field_5A08_ <= 8)
            strcat(StringTemp, label_get(labels[field_5A08_ - 1] / 4));
    }
    CharUpperA(StringTemp);
    strcat(StringTemp, "]");
    --title.top; --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    int ordinal = -1;
    // Original 49DF66/49E0CB assign this only for facilities; 49E0C4 reads
    // it on the unit path too. Preserve the stale/uninitialized value.
    int facility_scratch;
    for (int list_index = 0; list_index < GameDataStateStorage.fields_.base_current_count_; ++list_index) {
        const int base_id = ReportIfGlobal.sort_entries_[list_index].base_id;
        Base &base = Bases[base_id];
        if (base.faction_id_current != faction) continue;
        ++ordinal;
        if (ordinal < static_cast<int>(field_C0FC_) || ordinal >= static_cast<int>(field_C0FC_) + rows) continue;
        if (ordinal > static_cast<int>(field_C0FC_)) {
            text.top += pitch; text.bottom += pitch;
            icon.top += pitch; icon.bottom += pitch;
        }
        report_work_rect_.left = icon.left;
        report_work_rect_.top = text.top;
        report_work_rect_.right = text.right + 1;
        report_work_rect_.bottom = text.bottom;
        spot_.add(base_id + 0x400, 0, &report_work_rect_);
        spot_.add(base_id + 0xC00, 0, &icon);
        const bool selected = (selected_bases_[base_id / 32] & (1u << (base_id & 31))) != 0;
        canvas.box_sprite(&text, &InterfaceBoxStyles[selected ? 2 : 1]);
        if (selected) canvas.box_sprite(&text, &InterfaceBoxStyles[5]);
        canvas.box_sprite(&icon, &InterfaceBoxStyles[selected ? 2 : 1]);
        if (selected) canvas.box_sprite(&icon, &InterfaceBoxStyles[5]);
        if (base.governor & 0x80000000u) {
            for (int priority = 0; priority < 4; ++priority) {
                if (base.governor & (0x1000000u << priority)) {
                    RECT inner = {icon.left + 1, icon.top + 1, icon.right - 1, icon.bottom - 1};
                    canvas.box_sprite(&inner, &InterfaceBoxStyles[36 + 3 * priority]);
                    break;
                }
            }
        }
        const int text_y = pitch / 2 - static_cast<int>(field_E268_) / 2 + text.top;
        const int base_width = MapWinCaptionScale < 1 ? 62 : MapWinCaptionScale * 100 / MapWinCaptionDivisor;
        const int base_height = MapWinCaptionScale < 1 ? 46 : MapWinCaptionScale * 75 / MapWinCaptionDivisor;
        rect1_.left = (static_cast<int>(field_59A0_) - base_width) / 2 + 1 + icon.left;
        rect1_.top = (static_cast<int>(field_59A4_) - base_height) / 2 + 1 + icon.top;
        base_draw(&canvas, base_id, rect1_.left, rect1_.top, -6, 0x40);
        canvas.set_text_color(0xE1, -1, 1, 1);
        canvas.set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E184_, ReportIfGlobal.field_1E18C_, 0);
        set_base(base_id);
        base_compute(0);
        StringTemp[0] = 0;
        say_base(StringTemp, base_id);
        const int matrix = base_project(FAC_TELEPATHIC_MATRIX);
        if (BaseCurrent->talent_total < BaseCurrent->drone_total
            && (matrix < 0 || Bases[matrix].faction_id_current != faction)) {
            canvas.set_text_color(0xF9, -1, 1, 1);
        } else if (!BaseCurrent->drone_total && BaseCurrent->population_size > 2
            && (BaseCurrent->population_size + 1) / 2 <= BaseCurrent->talent_total) {
            strcat(StringTemp, "(!)");
            canvas.set_text_color(0xFF, -1, 1, 1);
        }
        rect1_.left = text.left + 5;
        CharUpperA(StringTemp);
        canvas.write_l(StringTemp, rect1_.left, text.top + 5, strlen(StringTemp));
        StringTemp[0] = 0;
        if (is_objective(base_id) && !(GameDataStateStorage.fields_.game_rules_ & 0x20000000)) {
            // The original indexes colour by list position, not sorted base ID.
            canvas.set_text_color(InfoFactionTextColors[Bases[list_index].faction_id_current], -1, 1, 1);
            strcat(StringTemp, label_get(0xE08 / 4));
            CharUpperA(StringTemp);
        } else if (!(base.governor & 0x80000000u)) {
            canvas.set_text_color(0xE1, -1, 1, 1);
            strcat(StringTemp, label_get(0xBFC / 4));
        } else {
            for (int status_priority = 0; status_priority < 4; ++status_priority) {
                if (base.governor & (0x1000000u << status_priority)) {
                    canvas.set_text_color(mandate_color(status_priority), -1, 1, 1);
                    strcat(StringTemp, label_get(0x824 / 4 + status_priority));
                    break;
                }
            }
        }
        canvas.write_l(StringTemp, rect1_.left + 20, text.bottom - field_E268_ - 1, strlen(StringTemp));
        rect1_.left += 192;
        const int mode = buttonGroup1_.field_84_;
        if (mode == 0x8B3) {
            const int production = BaseCurrent->queue_production_id[0];
            if (production < 0) {
                const int facility = -production;
                facility_scratch = facility;
                Sprite &sprite = facility < 70 ? g_FACILITY_ICON_SPRITES[facility]
                    : g_SECRET_PROJECT_ICON_SPRITES[facility - 70];
                const int denominator = facility < 70 ? g_FACILITY_ICON_SPRITES[0].height()
                    : g_SECRET_PROJECT_ICON_SPRITES[0].height();
                const int color = mandate_color(facility < 70 ? TutWinBaseWindow.prodPicker_.edbc_fac(facility) : 1);
                sprite.draw_mono(&canvas, sprite.transparent_index(), rect1_.left + 8,
                    rect1_.top + 4, color, text.bottom - text.top - 2, denominator);
            } else {
                veh_draw(&canvas, veh_fake(production, BaseCurrent->faction_id_current),
                    rect1_.left, rect1_.top + 4, -8, 0x4C, 0);
            }
            StringTemp[0] = 0;
            canvas.set_font(ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_, ReportIfGlobal.field_1E18C_, 0);
            int item_cost;
            if (production < 0) {
                const int facility = -production;
                item_cost = Facility[facility].cost;
                facility_scratch = facility;
                strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[facility].name)));
                canvas.set_text_color(mandate_color(TutWinBaseWindow.prodPicker_.edbc_fac(facility)), -1, 1, 1);
            } else {
                strcat(StringTemp, VehPrototypes[production].veh_name);
                BOOL prototype_cost;
                item_cost = veh_cost(production, BaseIDCurrentSelected, &prototype_cost);
                canvas.set_text_color(mandate_color(TutWinBaseWindow.prodPicker_.edbc_unit(production)), -1, 1, 1);
            }
            rect1_.left += 60;
            canvas.write_l(StringTemp, rect1_.left, text_y, strlen(StringTemp));
            canvas.set_text_color(0xF2, -1, 1, 1);
            strcpy(StringTemp, "(");
            reportwin_append_number(StringTemp, facility_scratch == 69 ? 0 : BaseCurrent->minerals_accumulated);
            strcat(StringTemp, "/");
            const int cost = cost_factor(BaseCurrent->faction_id_current, 1, -1) * item_cost;
            reportwin_append_number(StringTemp, cost);
            strcat(StringTemp, ")    ");
            const int minerals = BaseCurrent->mineral_surplus_final;
            if (minerals < 1) strcat(StringTemp, label_get(0x10C / 4));
            else {
                strcat(StringTemp, label_get(0xC4 / 4)); strcat(StringTemp, ": "); strcat(StringTemp, " ");
                int turns = 0;
                if (cost > BaseCurrent->minerals_accumulated) {
                    const int divisor = minerals < 1 ? 1 : minerals > 99999 ? 99999 : minerals;
                    turns = (minerals - 1 + cost - BaseCurrent->minerals_accumulated) / divisor;
                    if (turns < 0) turns = 0; else if (turns > 999999) turns = 999999;
                }
                reportwin_append_number(StringTemp, turns);
            }
            canvas.write_right_l(StringTemp, text.right - 4, text_y, strlen(StringTemp));
        } else if (mode == 0x8B4) {
            const int population = BaseCurrent->population_size;
            int spacing = population < 2 ? 34 : (text.right - text.left - 226) / (population - 1);
            if (spacing < 1) spacing = 1; else if (spacing > 34) spacing = 34;
            rect1_.left = spacing - 32 + text.right;
            rect1_.top = text.bottom - pitch / 2 - 18;
            const bool alien = ExpansionEnabled && (Players[faction].rule_flags & RFLAG_ALIEN);
            for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int index = BaseCurrent->specialist_total - specialist - 1;
                const int type = index < 16 ? (BaseCurrent->spec[index >> 3] >> ((index & 7) * 4)) & 15 : best_specialist();
                Sprite &sprite = alien ? g_AL_SPECIALIST_SM_CURSOR_SPRITES[type] : g_SPECIALIST_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            for (int drone = 0; drone < BaseCurrent->drone_total; ++drone) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = 2 + (drone < BaseCurrent->super_drone_total) + (alien ? 0 : rng.get(0, 2) * 4);
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            const int workers = population - BaseCurrent->talent_total - BaseCurrent->specialist_total
                - (BaseCurrent->drone_total > 0 ? BaseCurrent->drone_total : 0);
            for (int worker = 0; worker < workers; ++worker) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = 1 + (alien ? 0 : rng.get(0, 2) * 4);
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
            for (int talent = 0; talent < BaseCurrent->talent_total; ++talent) {
                rect1_.left -= spacing;
                spot_.add(list_index, 0, rect1_.left, rect1_.top, spacing, pitch);
                const int type = alien ? 0 : rng.get(0, 2) * 4;
                Sprite &sprite = alien ? g_AL_CITIZEN_SM_CURSOR_SPRITES[type] : g_CITIZEN_SM_CURSOR_SPRITES[type];
                sprite.draw(&canvas, sprite.transparent_index(), rect1_.left, rect1_.top);
            }
        } else if (mode == 0x8B5) {
            int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
            const int slots = stack_check(vehicle, 1, -1, -1, -1);
            int spacing = slots < 2 ? 40 : (text.right - text.left - 232) / (slots - 1);
            if (spacing < 1) spacing = 1; else if (spacing > 40) spacing = 40;
            rect1_.left = spacing - 45 + text.right;
            rect1_.top = text.top;
            if (vehicle >= 0) {
                while (Vehs[vehicle].prev_veh_id_stack >= 0) vehicle = Vehs[vehicle].prev_veh_id_stack;
                for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                    rect1_.left -= spacing;
                    veh_draw(&canvas, vehicle, rect1_.left, rect1_.top, -8, 8, 0);
                    spot_.add(base_id + 0x800, vehicle, rect1_.left, rect1_.top, spacing, pitch);
                }
            }
        }
    }
}
static void report_sat_number(int value) {
    char number[80];
    _itoa(value, number, 10);
    strcat(StringTemp, number);
}
// ORIGINAL: 0x0049F350 ?draw_sat@ReportWin@@QAEXXZ 0x0049F350-0x0049FE1E
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/0049f350.cpp", "source:src/reportwin.h", "source:src/reportif.h", "image:original ReportWin instructions, callbacks and lifecycle evidence"], "reason": "Recover this report-window operation in its genuine owner using typed members, natural subobject lifetimes and shared source-owned state; retained catalogue identity and spans remain distinct from current emitted symbols."}
void ReportWin::draw_sat() {
    Buffer &canvas = *static_cast<Buffer *>(this);
    const int faction = ReportIfGlobal.report_faction_id();
    RECT title = {energy_panel_rect_.left + 3, energy_panel_rect_.top + 3,
        energy_panel_rect_.right - 3, energy_panel_rect_.top + static_cast<int>(field_E268_) + 9};
    canvas.box_sprite(&energy_panel_rect_, &InterfaceBoxStyles[0]);
    canvas.box_sprite(&title, &InterfaceBoxStyles[1]);
    strcpy(StringTemp, "[");
    strcat(StringTemp, Players[faction].formal_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xAD4 / 4));
    strcat(StringTemp, "]");
    --title.top; --title.bottom;
    canvas.write_cent_l(StringTemp, &title, strlen(StringTemp));
    flatButton7_.show(0);
    flatButton8_.show(0);
    spot_.reset_count();
    bool orbital_tech = false;
    for (int technology_faction = 1; technology_faction < 8; ++technology_faction)
        if (has_tech(Facility[65].preq_tech, technology_faction)) orbital_tech = true;
    if (buttonGroup2_.field_84_ == 0xD3C) {
        flatButton9_.hide();
        if (!orbital_tech) {
            canvas.set_font(ReportIfGlobal.field_1E1A0_, 0, 0, 0);
            canvas.set_text_color(0xE1, -1, 1, 1);
            strcpy(StringTemp, label_get(0xCC4 / 4));
            canvas.write_cent_l(StringTemp, rect1_.left, rect1_.top,
                rect1_.right, rect1_.bottom, strlen(StringTemp));
            return;
        }
        rect1_.top += 50;
        rect1_.bottom -= 50;
        const int column_width = (rect1_.bottom - rect1_.top - 2) / 8;
        const int heading_width = rect1_.right / (ExpansionEnabled ? 5 : 3);
        const int row_width = (rect1_.right - 4 - heading_width) / (ExpansionEnabled ? 5 : 4);
        for (int other = 0; other < 8; ++other) {
            // Genuine original: clears only type 0 on every column iteration.
            spot_.kill_type(0);
            rect1_.left = energy_panel_rect_.left + 2;
            if (other) {
                StringTemp[0] = 0;
                GenderDefault = Players[other].noun_gender;
                PluralityDefault = Players[other].is_noun_plural;
                strcat(StringTemp, Players[other].noun_faction);
                canvas.set_text_color(ChatItemIdTable[other], ChatShadowColors[other], 1, 1);
                report_work_rect_.left = rect1_.left;
                report_work_rect_.top = rect1_.top;
                report_work_rect_.right = rect1_.left + heading_width;
                report_work_rect_.bottom = rect1_.top + column_width;
                canvas.write_l(StringTemp, &report_work_rect_, strlen(StringTemp));
            }
            rect1_.left += heading_width;
            report_work_rect_.left = rect1_.left;
            report_work_rect_.top = rect1_.top;
            report_work_rect_.right = rect1_.left + row_width;
            report_work_rect_.bottom = rect1_.top + column_width;
            field_E29C_ = 0;
            while (static_cast<int>(field_E29C_) < (ExpansionEnabled ? 5 : 4)) {
                const int kind = field_E29C_;
                StringTemp[0] = 0;
                if (!other) {
                    canvas.set_text_color(0xFF, -1, 1, 1);
                    strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[kind == 4 ? 41 : 65 + kind].name)));
                    canvas.wrap_cent(StringTemp, &report_work_rect_);
                } else {
                    int elevators = 0;
                    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base)
                        if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
                    const bool nonzero = kind == 4 ? elevators != 0 : report_sat_count(PlayersData[other], kind) != 0;
                    const int colors[5] = {0x98, 0xDF, 0x62, 0xF9, 0xFF};
                    canvas.set_text_color(nonzero ? colors[kind] : 0xF8, -1, 1, 1);
                    if (kind == 3) {
                        report_sat_number(PlayersData[other].satellites_[3] - PlayersData[other].sat_odp_deployed);
                        strcat(StringTemp, "/");
                    }
                    report_sat_number(kind == 4 ? elevators : report_sat_count(PlayersData[other], kind));
                    canvas.write_cent_l(StringTemp, &report_work_rect_, strlen(StringTemp));
                    spot_.add(kind, other, report_work_rect_.left, report_work_rect_.top,
                        report_work_rect_.right - report_work_rect_.left,
                        report_work_rect_.bottom - report_work_rect_.top);
                }
                report_work_rect_.left += row_width;
                report_work_rect_.right += row_width;
                ++field_E29C_;
            }
            rect1_.top += column_width;
        }
        return;
    }
    field_59A0_ = 576;
    field_59A4_ = 210;
    rect1_.left = (energy_panel_rect_.right + energy_panel_rect_.left - 576) / 2;
    rect1_.top = (energy_panel_rect_.bottom + energy_panel_rect_.top - 210) / 2 + 20;
    satellite_rect_.left = rect1_.left;
    satellite_rect_.top = rect1_.top;
    satellite_rect_.right = rect1_.left + 576;
    satellite_rect_.bottom = rect1_.top + 210;
    flic_.init(&canvas, "satellite.flc", 0, 0, 0);
    field_E2A8_ = (satellite_rect_.right + satellite_rect_.left) / 2;
    field_E2AC_ = (satellite_rect_.bottom + satellite_rect_.top) / 2;
    field_E2A0_ = field_E2A8_ - flic_.decoded_width() / 2;
    field_E2A4_ = field_E2AC_ - flic_.decoded_height() / 2;
    buffer2_.init(flic_.decoded_width(), flic_.decoded_height(), 0, 0);
    buffer2_.fill(9);
    flic_.init(&buffer2_, "satellite.flc", 0, 0, 0);
    canvas.box_sprite(&satellite_rect_, &InterfaceBoxStyles[4]);
    satellite_rect_.left += 3; satellite_rect_.top += 3;
    satellite_rect_.right -= 3; satellite_rect_.bottom -= 3;
    field_F918_ = 0;
    // Original includes factions 1..7 (833..4198 dwords, exclusive endpoint).
    for (int other = 1; other < 8; ++other) {
        field_E29C_ = 0;
        while (field_E29C_ < 4) {
            field_F918_ += report_sat_count(PlayersData[other], field_E29C_);
            ++field_E29C_;
        }
    }
    field_EE30_ = 0.02f;
    field_E29C_ = 0;
    while (field_E29C_ < 16) {
        const int i = field_E29C_;
        const int radius_x = static_cast<int>(field_59A4_) / 2 - 10;
        const int radius_y = radius_x;
        satellite_acceleration_[i] = static_cast<float>(sqrt(static_cast<double>(radius_x * radius_x * 2)));
        const float radius = satellite_acceleration_[i];
        // x87 49FC58..64 multiplies THREE radius factors, unlike seed.
        satellite_acceleration_[i] = static_cast<float>(sqrt(1.0 / (static_cast<double>(radius) * radius * radius)) * 700.0f);
        const float angle = static_cast<float>(i) * 0.0625f;
        const float phase_x = angle * 6.28000020980835f;
        const float phase_y = static_cast<float>((((i & 1) ? 0.125 : -0.125) + angle) * 6.28000020980835f);
        const float speed = satellite_acceleration_[i];
        satellite_x_[i] = static_cast<float>(cos(static_cast<double>(speed) + phase_x) * radius_x);
        satellite_y_[i] = static_cast<float>(sin(static_cast<double>(speed) + phase_y) * radius_y);
        satellite_dx_[i] = static_cast<float>(sin(static_cast<double>(speed) + phase_x) * -radius_x * speed);
        satellite_dy_[i] = static_cast<float>(cos(static_cast<double>(speed) + phase_y) * speed * radius_y);
        satellite_acceleration_[i] = static_cast<float>(static_cast<double>(speed) * field_EE30_ * speed);
        satellite_live_[i] = i < static_cast<int>(field_F918_);
        ++field_E29C_;
    }
    sat_anim();
    time2_.init(reinterpret_cast<void (__cdecl *)(int)>(sat_anim_daemon), reinterpret_cast<int>(this), 4000, 5);
    time2_.start();
}

// ORIGINAL: 0x004A0260 ?sat_attack@@YAXHHH@Z 0x004A0260-0x004A080C;0x00659447-0x00659580
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/unrecovered/004a0260.cpp", "source:src/faction.h", "image:complete original instructions and independent storage/lifecycle evidence"], "reason": "Recover satellite attack selection through a natural Popup lifetime, its owned picture and the genuine four-counter faction array, preserving original signed guards and network/local action ordering."}
void __cdecl sat_attack(int attacker, int target, int kind) {
    Popup popup;
    if (attacker == target || target < 1) return;
    if (!PlayersData[attacker].satellites_[3]) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<const char *>(*ClimatePopupTitle),
                 "NOKILLERS", 0, "space_sm.pcx", 0);
        return;
    }
    if (static_cast<int>(PlayersData[attacker].sat_odp_deployed)
        >= static_cast<int>(PlayersData[attacker].satellites_[3])) {
        if (attacker == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<const char *>(*ClimatePopupTitle),
                 "KILLERSDEPLOYED", 0, "space_sm.pcx", 0);
        return;
    }
    int targets = 0;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == target && has_fac_built(41, base)) ++targets;
    }
    if (kind == 4) {
        if (!targets) return;
    } else {
        // Original guards only the signed upper bound; no new lower guard.
        if (kind >= 4 || !PlayersData[target].satellites_[kind]) return;
    }
    if (break_treaty(attacker, target, 7)) return;
    if (diplo_lock(100)) return;
    if (PlayersData[attacker].satellites_[3]
        && static_cast<int>(PlayersData[attacker].sat_odp_deployed)
            < static_cast<int>(PlayersData[attacker].satellites_[3])) {
        int attack_target = attacker;
        bool attack = false;
        if (kind == 4) {
            if (targets) {
                popup.start(PopupStartCaption, "WHICHGSP", -1, 0, 0x40, 0);
                int width, height;
                if (Buffer::get_pcx_dimensions("satbat_sm.pcx", &width, &height) == 0
                    && popup.owned_picture().init("satbat_sm.pcx", width, height) == 0)
                    popup.select_owned_picture();
                for (int item_base = 0; item_base < GameDataStateStorage.fields_.base_current_count_; ++item_base) {
                    if (Bases[item_base].faction_id_current == target && has_fac_built(41, item_base))
                        popup.dialogs().item(Bases[item_base].name_string, item_base);
                }
                attack_target = popup.exec(0, 0);
                attack = attack_target >= 0;
            }
        } else if (kind < 4 && PlayersData[target].satellites_[kind]) {
            attack = true;
        }
        if (attack) {
            if (IsMultiplayerNet) {
                message_data(0x2412, 0, attacker, target, kind, attack_target);
                NetDaemonNet.await_exec(0);
            } else {
                ReportWinGlobal.start_attack(attacker, target, kind, attack_target);
            }
        }
    }
    diplo_unlock();
    // Popup's genuine destructor closes and tears down all member/base state.
}

/*
// ORIGINAL: 0x0049FFC0 ?sat_anim_daemon@@YAXH@Z 0x0049FFC0-0x0049FFCD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["ReportWin declares sat_anim(); reportwin.cpp already declares this callback and passes it to Time::init with the ReportWin payload.","The original identity's int callback ABI and current pointer-typed source symbol are both retained as facts; native symbol qualification is still required."],"reason":"Move the existing callback body into the ReportWin owner TU, preserving its typed dispatch and historical ABI mismatch evidence."}
// symbol    ?sat_anim_daemon@@YAXPAVReportWin@@@Z
// size      13 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0049FE40
Status: Complete
*/
void __cdecl sat_anim_daemon(ReportWin *a1) {
    a1->sat_anim();
}

/*
// ORIGINAL: 0x004A0250 ?sat_expl_daemon@@YAXH@Z 0x004A0250-0x004A025D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["ReportWin declares exp_anim(); reportwin.cpp already declares this callback for its Time::init callback path.","The original int callback ABI versus pointer-typed source fact remains a native qualification obligation."],"reason":"Move the existing callback into reportwin.cpp unchanged; this does not independently prove the callback's original parameter type."}
// symbol    ?sat_expl_daemon@@YAXPAVReportWin@@@Z
// size      13 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A0100
Status: Complete
*/
void __cdecl sat_expl_daemon(ReportWin *a1) {
    a1->exp_anim();
}
