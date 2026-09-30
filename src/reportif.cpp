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
#include "reportif.h"
#include "base.h"    // BaseCurrentCount bounds the report's fixed sort workspace
#include "general.h" // swap for the original reversed-bound normalization
#include "reportwin.h"  // ReportWinGlobal - the window done() un-modals
#include "console.h"  // ConsoleGlobal.faction_id_ owns original939284
#include "worldwin.h"
#include "menu.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "flatbutton.h"
#include "sprite.h"
#include "buttongroup.h"
#include "spot.h"
#include "time.h"  // SatPanelTimer1/2 - close_sat stops both
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "alpha.h"  // Labels
#include "faction.h"  // Players
#include "game.h"  // say_year
#include "technology.h"  // has_tech_call
#include "strings.h"  // StringTable, StringTemp
#include "vector_teardown.h"
#include "basewin.h"
#include "veh.h"
#include "font.h"
#include "spying_recovery.h"
#include "stringstruct.h"
#include "netdaemon.h"
#include "log.h"
#include "map.h"
#include "text.h"

extern char *DatalinkHelpFile;
// The observed report scratch inputs are six immediate char-addresses, not
// pointer cells. Their original extent/owner is unknown; represent only the
// observed empty NUL-terminated contents, without asserting a larger bound.
// These starts are in the PE loader-zero virtual tail, not file-backed
// initializer bytes. The panel drawers read them as strcpy sources.
// RECOVERY_OBJECT: v1 0x008A624C ?ReportProjectScratchSmall@@3PADA
char ReportProjectScratchSmall[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6250 ?ReportProjectScratchLarge@@3PADA
char ReportProjectScratchLarge[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6254 ?ReportSatelliteScratchUpperText@@3PADA
char ReportSatelliteScratchUpperText[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6258 ?ReportSatelliteScratchUpperLine@@3PADA
char ReportSatelliteScratchUpperLine[1] = {0};
// RECOVERY_OBJECT: v1 0x008A625C ?ReportSatelliteScratchLowerText@@3PADA
char ReportSatelliteScratchLowerText[1] = {0};
// RECOVERY_OBJECT: v1 0x008A6260 ?ReportSatelliteScratchLowerLine@@3PADA
char ReportSatelliteScratchLowerLine[1] = {0};
int __cdecl X_text_open(char *, const char *);
extern BaseWin TutWinBaseWindow;
extern int EnergyBaseMaintenance[MaxBaseNum];
extern int EnergyFacilityCounts[70];
extern int EnergyLabsTotal;
int __cdecl labs_write(GraphicWin *, int, int, RECT *, char *);
void __cdecl scroll_normal();
int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
void __cdecl draw_radius(int x, int y, int mode, int draw_type);

// ORIGINAL: 0x004A8440 ?labs_write@@YAXPAUGraphicWin@@HHPAURECT@@PAD@Z 0x004A8440-0x004A846E FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004a8440.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a8440 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover the ReportIf technology-label writer from its retained seed through the genuine ReportWin/Buffer relationship. Its current emitted symbol is recorded separately from the historical catalogue spelling; this binding adds no claim about executable return decoration."}
// symbol    ?labs_write@@YAHPAVGraphicWin@@HHPAUtagRECT@@PAD@Z
int __cdecl labs_write(GraphicWin *window, int technology, int, RECT *, char *) {
    static_cast<Buffer *>(window)->set_text_color3(
        mandate_color(tech_category(technology)), 0, 1, 1);
    return 0;
}

void __cdecl energy_compute(int faction_id, int mode);
void __cdecl scroll_small();

// Popup's scroll lifecycle callbacks operate on ReportWin's embedded Scroll.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
// on_status reads the distinct reentry cell and writes complete 0/1 dwords.
// Its initial zero comes from the PE loader's virtual tail, not stored bytes.
// RECOVERY_OBJECT: v1 0x008A6240 ?ReportInterfaceStatusReentry@@3HA
int ReportInterfaceStatusReentry = 0;

static void report_iface_button(FlatButton &button, RECT &rect, int id,
                                Win *parent, bool hide) {
    button.init(StringTemp, id, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top, parent, 0);
    static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    if (hide) button.hide();
}

// ORIGINAL: 0x004A41D0 ?on_status@ReportIf@@QAEXH@Z 0x004A41D0-0x004A5ED2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004a41d0.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a41d0 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's status transition through its genuine SubInterface owner and ReportWin's embedded controls, following the retained original behavior. The source's emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_status@ReportIf@@UAEXH@Z
void ReportIf::on_status(int mode) {
    ReportWin &window = ReportWinGlobal;
    if (!mode) {
        ReportInterfaceStatusReentry = 0;
        window.time1_.stop();
        if (window.field_C0F8_) {
            scroll_hide(&window, &window.scroll_); window.field_C0F8_ = 0;
        }
        slide_hide(&window, 0); window.hide();
        window.flatButton3_.close(); window.flatButton2_.close();
        for (int category = 0; category < 7; ++category) flatButtonsB_[category].close();
        for (int faction = 0; faction < 7; ++faction) flatButtonsA_[faction].close();
        switch (field_8_) {
        case 3: window.listBox_.close(); // Original falls through to both local lists.
        case 0: listBox1_.close(); listBox2_.close(); break;
        case 2: listBox1_.close(); flatButton1_.hide(); flatButton2_.hide(); break;
        case 4: close_ops(); break;
        case 6: close_sat(); break;
        case 7: flatButton12_.hide(); flatButton14_.hide(); flatButton13_.hide(); break;
        }
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide(); MainInterfaceGlobal.flat_button_[7].hide();
        return;
    }
    if (ReportInterfaceStatusReentry) { ReportInterfaceStatusReentry = 0; return; }
    report_scroll_blocked_ = 1; field_1E234_ = 1;
    window.field_C0F8_ = 0; report_topic_changed_ = 1;
    window.fill(9); report_faction_id_ = ConsoleGlobal.faction_id_;
    for (int history = 0; history < 19; ++history) report_page_history_[history] = -1;
    window.shared_report_selection() = -1; field_1E1D8_ = 1;
    int research = PlayersData[report_faction_id_].tech_id_researching;
    if (!((GameState & 0x80) || !(GameRules & 0x200)) || research < 0 || research > 89)
        research = 0;
    window.shared_research_focus() = research;
    for (int topic = 0; topic < 7; ++topic) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(0x2B1 + topic));
        FlatButton &button = flatButtonsB_[topic];
        button.init(0, topic + 2, report_category_positions_[topic], report_category_center_,
            spritesA_[0].width(), spritesA_[0].height(), &MainInterfaceGlobal, 0);
        button.set_report_icons(&spritesA_[topic*3], &spritesA_[topic*3+1], &spritesA_[topic*3+2]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw(); button.redraw();
    }
    RECT &frame = window.report_frame_rect_;
    RECT &panel = window.energy_panel_rect_;
    frame.top = panel.bottom + 7; frame.bottom = panel.bottom + 27;
    frame.left = panel.left + 75; frame.right = panel.right - panel.left - 150 + frame.left;
    const int center = (panel.right + panel.left) / 2;
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x21B));
    draw_workspace_rect_.left = frame.left; draw_workspace_rect_.top = frame.top;
    draw_workspace_rect_.right = center - 1; draw_workspace_rect_.bottom = frame.top + 20;
    report_iface_button(window.flatButton3_, draw_workspace_rect_, 1, &window, false);
    window.flatButton3_.redraw();
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x299));
    draw_workspace_rect_.left = center + 1;
    draw_workspace_rect_.right = center + 1 + center - frame.left - 1;
    report_iface_button(window.flatButton2_, draw_workspace_rect_, 9, &window, false);
    window.flatButton2_.redraw();
    // The two paired mode selectors share precisely the same panel geometry.
    ButtonGroup *pairs[2] = {&buttonGroup1_, &buttonGroup2_};
    FlatButton *pair_buttons[2][2] = {{&flatButton1_, &flatButton2_}, {&flatButton3_, &flatButton4_}};
    const int pair_labels[2][2] = {{0x175,0x176},{0x1EE,0x2AA}};
    const int pair_ids[2][2] = {{0x92,0x93},{0xD0A,0xD0B}};
    for (int pair = 0; pair < 2; ++pair) {
        field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 9;
        field_14_ = field_20_.right - field_20_.left - 18;
        pairs[pair]->init(pair == 0 ? 2 : 4, 0);
        const int half = static_cast<int>(field_14_) / 2;
        for (int side = 0; side < 2; ++side) {
            StringTemp[0] = 0; strcat(StringTemp, label_get(pair_labels[pair][side]));
            draw_workspace_rect_.left = field_C_ + (side ? half + 1 : 0);
            draw_workspace_rect_.top = field_10_;
            draw_workspace_rect_.right = side ? draw_workspace_rect_.left + half : field_C_ + half - 1;
            draw_workspace_rect_.bottom = field_10_ + 20;
            report_iface_button(*pair_buttons[pair][side], draw_workspace_rect_, pair_ids[pair][side], &MainInterfaceGlobal, true);
            pairs[pair]->add(pair_buttons[pair][side]);
        }
        pairs[pair]->set(pair_ids[pair][0], 1);
    }
    field_C_ = panel.left + 3; field_10_ = field_1C_ + panel.top + 11;
    field_14_ = panel.right - panel.left - 6;
    window.buttonGroup1_.init(4, 0);
    FlatButton *thirds[3] = {&window.flatButton4_, &window.flatButton5_, &window.flatButton6_};
    const int third_labels[3] = {0x32C,0x2E9,0x3F};
    const int third = static_cast<int>(field_14_) / 3;
    for (int part = 0; part < 3; ++part) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(third_labels[part]));
        draw_workspace_rect_.left = field_C_ + (part == 2 ? third*2+2 : third*part);
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + (part == 0 ? third-2 : part == 1 ? third*2 : field_14_);
        draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*thirds[part], draw_workspace_rect_, 0x8B3+part, &window, false);
        thirds[part]->reset_field_A9C_if_ungrouped(); thirds[part]->Win::hide();
        window.buttonGroup1_.add(thirds[part]);
    }
    window.buttonGroup1_.set(field_1E240_ ? field_1E240_ : 0x8B3, 1);
    field_C_ = panel.left + 3; field_10_ = field_1C_ + panel.top + 11;
    field_14_ = panel.right - panel.left - 6;
    window.buttonGroup2_.init(6, 0);
    FlatButton *halves[2] = {&window.flatButton7_, &window.flatButton8_};
    for (int side = 0; side < 2; ++side) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(0x32C+side));
        const int half = static_cast<int>(field_14_) / 2;
        draw_workspace_rect_.left = field_C_ + (side ? half+1 : 0);
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + (side ? field_14_ : half-1);
        draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*halves[side], draw_workspace_rect_, 0xD3C+side, &window, false);
        halves[side]->reset_field_A9C_if_ungrouped(); halves[side]->Win::hide();
        window.buttonGroup2_.add(halves[side]);
    }
    window.buttonGroup2_.set(0xD3C, 1);
    StringTemp[0] = 0; strcat(StringTemp, label_get(0x32E));
    draw_workspace_rect_.left = frame.left; draw_workspace_rect_.right = frame.right;
    draw_workspace_rect_.top = panel.bottom - 26; draw_workspace_rect_.bottom = panel.bottom - 6;
    report_iface_button(window.flatButton9_, draw_workspace_rect_, 0xD3E, &window, false);
    window.flatButton9_.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46], &InterfaceBoxStyles[45]);
    window.flatButton9_.reset_field_A9C_if_ungrouped(); window.flatButton9_.Win::hide();
    field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 35;
    field_14_ = field_20_.right - field_20_.left - 18;
    FlatButton *orders[7] = {&flatButton5_, &flatButton6_, &flatButton7_, &flatButton8_, &flatButton9_, &flatButton10_, &flatButton11_};
    const int order_labels[7] = {0x2EA,0x2EB,0x2EB,0x2EB,0x2EB,0x2EC,0x2ED};
    for (int order = 0; order < 7; ++order) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(order_labels[order]));
        if (order >= 1 && order <= 4) { strcat(StringTemp, " "); strcat(StringTemp, label_get(0x209+order-1)); }
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*orders[order], draw_workspace_rect_, 0xCEF+order, &MainInterfaceGlobal, true);
        field_10_ += 22;
    }
    field_C_ = field_30_.left + 9; field_10_ = field_30_.top + 32;
    field_14_ = field_30_.right - field_30_.left - 18;
    FlatButton *security[3] = {&flatButton12_, &flatButton14_, &flatButton13_};
    const int security_ids[3] = {0xCF6,0xCF8,0xCF7};
    for (int action = 0; action < 3; ++action) {
        StringTemp[0] = 0; strcat(StringTemp, label_get(action ? 0x2DB : 0x3BB));
        if (action == 2) CharUpperA(StringTemp);
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(*security[action], draw_workspace_rect_, security_ids[action], &MainInterfaceGlobal, true);
        if (action < 2) field_10_ += 22;
    }
    field_C_ = field_20_.left + 9; field_10_ = field_20_.top + 32;
    field_14_ = field_20_.right - field_20_.left - 18;
    buttonGroup3_.init(0x6A, 0);
    for (int other = 1; other < 8; ++other) {
        if (other == ConsoleGlobal.faction_id_) continue;
        int elevators = 0;
        for (int base = 0; base < BaseCurrentCount; ++base)
            if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
        const PlayerData &data = PlayersData[other];
        if (!(has_tech(Facility[65].preq_tech, other)
            || data.satellites_[0]+data.satellites_[1]+data.satellites_[2]+data.satellites_[3]+elevators
            || has_tech(Facility[41].preq_tech, other))) continue;
        StringTemp[0] = 0; strcat(StringTemp, get_noun(other));
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_; draw_workspace_rect_.bottom = field_10_ + 20;
        report_iface_button(flatButtonsC_[other-1], draw_workspace_rect_, other, &MainInterfaceGlobal, true);
        buttonGroup3_.add(&flatButtonsC_[other-1]); field_10_ += 22;
    }
    window.field_59B0_ = 1;
    for (int faction = 1; faction < 8; ++faction) {
        const int current = ConsoleGlobal.faction_id_;
        if (!(PlayersData[faction].diplo_treaties[current] & 1) && !spying(faction)
            && faction != current && !(GameState & 0x80)) continue;
        StringTemp[0] = 0; strcat(StringTemp, get_title(faction)); strcat(StringTemp, " ");
        strcat(StringTemp, get_name(faction)); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xB4)); strcat(StringTemp, " ");
        strcat(StringTemp, get_noun(faction));
        int left, top, width, height;
        if (faction != current) {
            left = report_button_positions_[faction < current ? faction-1 : faction-2];
            top = report_button_positions_[7]; width = 37 + (field_1E230_ ? 5 : 0); height = 29;
        } else if (field_1E230_) {
            left = field_50_.left + 15; top = field_50_.top + 15; width = height = 70;
        } else {
            left = field_50_.left + 9; top = report_button_positions_[7]; width = 37; height = 29;
        }
        draw_workspace_rect_.left = left; draw_workspace_rect_.top = top;
        draw_workspace_rect_.right = left + width; draw_workspace_rect_.bottom = top + height;
        FlatButton &button = flatButtonsA_[faction-1];
        button.init(0, faction+9999, left, top, width, height, &MainInterfaceGlobal, 0);
        button.set_report_icons(faction_button_sprites_[faction-1][0], faction_button_sprites_[faction-1][1], faction_button_sprites_[faction-1][2]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw(); button.redraw();
    }
    flatButtonsA_[ConsoleGlobal.faction_id_-1].set_report_page_state(2);
    flatButtonsA_[ConsoleGlobal.faction_id_-1].redraw();
    window.report_work_rect_.left = window.report_work_rect_.top = 0;
    window.report_work_rect_.right = 32; window.report_work_rect_.bottom = 300;
    window.scroll_.init(&window.report_work_rect_, &window, 0, 0); window.scroll_.hide();
    field_1E234_ = 0;
    ReportIfGlobal.report(ReportIfGlobal.field_8_, ReportIfGlobal.report_faction_id_, ReportIfGlobal.report_selection_);
    slide_show(&window, 0); window.show(3); report_scroll_blocked_ = 0;
    if (window.field_E26C_) {
        scroll_show(&window, &window.scroll_); window.field_E26C_ = 0; window.field_C0F8_ = 1;
    }
    if (WinModalWindow != static_cast<Win *>(&window)) {
        ReportInterfaceStatusReentry = 1; window.set_modal(0, pop_wait, &MainInterfaceGlobal);
    }
}

// ORIGINAL: 0x004A3460 ?init@ReportIf@@QAEXXZ 0x004A3460-0x004A3927;0x00659580-0x00659595 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004a3460.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a3460 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf initialization from the retained original behavior and the genuine ReportWin control storage; its non-virtual source identity remains distinct from ReportIf's historical virtual callbacks."}
void ReportIf::init() {
    Buffer image;
    field_1E240_ = 0;
    field_1E244_ = 4;
    field_4_ = 5;
    // Registry slot5 is this genuine SubInterface, not another object.
    register_iface_mode(5, this);
    spot_.init(0x100);
    for (int i = 0; i < 0x200; ++i) {
        sort_entries_[i].base_id = i;
        sort_entries_[i].value = 0;
        sort_entries_[i].sorted_base_id = i;
        sort_entries_[i].sorted_value = 0;
    }
    ReportWinGlobal.field_5A08_ = 0;
    for (int dot = 0; dot < 4; ++dot) {
        char number[80];
        StringTemp[0] = 0;
        strcat(StringTemp, "dot");
        _itoa(dot, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ".pcx");
        image.load_pcx(StringTemp, 0, 10, 0xEC);
        spritesB_[dot].extract(&image, WinFillColour, 0, 0, 12, 12, 0);
    }
    image.load_pcx("report.pcx", 0, 10, 0xEC);
    for (int row = 0; row < 7; ++row)
        for (int column = 0; column < 3; ++column)
            spritesA_[row * 3 + column].extract(&image, WinFillColour,
                row * 29, column * 35, 30, 36, 0);
    field_20_ = MainInterfaceGlobal.main_frame_rect();
    field_30_ = MainInterfaceGlobal.right_frame_rect();
    field_40_ = MainInterfaceGlobal.lower_frame_rect();
    field_50_ = MainInterfaceGlobal.info_panel_frame();
    field_1E230_ = WinScreenWidth != 800;
    const int step = field_1E230_ ? 42 : 37;
    for (int slot_index = 0; slot_index < 7; ++slot_index)
        report_button_positions_[slot_index] = slot_index * step;
    for (int faction = 1; faction <= 7; ++faction)
        for (int state = 0; state < 3; ++state)
            faction_button_sprites_[faction - 1][state] =
                faction == ConsoleGlobal.faction_id_ && field_1E230_
                    ? &FactionsArt[faction].report_logo[state == 0]
                    : &FactionsArt[faction].report_logo_small[state];
    int position = field_40_.left + 16 + (field_1E230_ != 0);
    report_category_center_ =
        (field_40_.bottom + field_40_.top - spritesA_[0].height()) / 2;
    for (int category = 0; category < 7; ++category) {
        report_category_positions_[category] = position;
        position += spritesA_[0].width() + 9 + (field_1E230_ ? 3 : 0);
    }
    // Original includes the eighth, previously unwritten slot in this walk
    // before replacing it below; preserve that store sequence.
    for (int slot = 0; slot < 8; ++slot)
        report_button_positions_[slot] += field_50_.left + 46 + (field_1E230_ ? 43 : 0);
    report_button_positions_[7] = field_50_.bottom - 40;
    field_1E180_ = find_font(10, 0);
    field_1E184_ = find_font(12, 0);
    field_1E188_ = find_font(12, 2);
    field_1E18C_ = find_font(12, 1);
    field_1E190_ = find_font(14, 0);
    field_1E194_ = find_font(14, 2);
    field_1E198_ = find_font(14, 1);
    field_1E19C_ = find_font(16, 0);
    field_1E1A0_ = find_font(16, 2);
    field_1E1A4_ = find_font(16, 1);
    field_1E1A8_ = find_font(11, 0);
    field_1E1AC_ = find_font(26, 0);
    field_1E1B0_ = find_font(18, 2);
    field_1E1B4_ = find_font(22, 0);
    field_1E1B8_ = find_font(32, 0);
    field_1C_ = field_1E184_->line_advance();
    ReportWinGlobal.init();
}

// ORIGINAL: 0x004A6050 ?on_iface_selected@ReportIf@@QAEXHH@Z 0x004A6050-0x004A62F3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004a6050.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6050 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's selection callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_selected@ReportIf@@UAEXHH@Z
void ReportIf::on_iface_selected(int item, int) {
    if (ReportIfGlobal.field_8_ == 2) {
        if (item < 0x400) {
            ReportWinGlobal.shared_research_focus() = item;
            ReportWinGlobal.redraw();
        } else {
            TutWinBaseWindow.zoom(item - 0x400, 0);
        }
    } else if (ReportIfGlobal.field_8_ == 3) {
        if (item < 0x400) {
            ReportWinGlobal.shared_report_selection() = item;
            ReportWinGlobal.field_59B0_ = 0;
            listBox1_.set_selected_pos(-1, 1);
        } else {
            ReportWinGlobal.shared_report_selection() = item - 0x400;
            ReportWinGlobal.field_59B0_ = 1;
            listBox2_.set_selected_pos(-1, 1);
        }
        ReportWinGlobal.reset();
        ReportWinGlobal.draw_energy();
        ReportWinGlobal.soft_update();
    } else if (ReportIfGlobal.field_8_ == 4 && item >= 1 && item <= 8) {
        for (int i = 0; i < 0x200; ++i) {
            sort_entries_[i].base_id = i;
            sort_entries_[i].value = 0;
            sort_entries_[i].sorted_base_id = i;
            sort_entries_[i].sorted_value = 0;
        }
        for (int base = 0; base < BaseCurrentCount; ++base) {
            set_base(base);
            base_compute(0);
            int &value = sort_entries_[base].value;
            // Original loads are D4/D0/100/108/104, not the seed's
            // half-sized Ghidra offsets 6A/68/80/84/82.
            switch (item) {
            case 1: value = BaseCurrent->mineral_surplus; break;
            case 2: value = BaseCurrent->nutrient_surplus; break;
            case 3: value = BaseCurrent->economy_total; break;
            case 4: value = BaseCurrent->labs_total; break;
            case 5: value = BaseCurrent->psych_total; break;
            case 6: value = BaseCurrent->population_size; break;
            case 7: {
                int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
                if (vehicle >= 0) {
                    while (Vehs[vehicle].prev_veh_id_stack >= 0)
                        vehicle = Vehs[vehicle].prev_veh_id_stack;
                    for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack)
                        ++value;
                }
                break;
            }
            case 8:
                for (int facility = 0; facility < 70; ++facility)
                    if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8)))
                        ++value;
                for (int project = 0; project < 64; ++project)
                    if (base_project(project) == base) value += 5;
                break;
            }
            sort_entries_[base].sorted_value = value;
        }
        merge_sort(0, BaseCurrentCount - 1);
        ReportWinGlobal.field_5A08_ = item;
        ReportWinGlobal.redraw();
    }
}

// ORIGINAL: 0x004A6330 ?on_iface_button_clicked@ReportIf@@QAEXH@Z 0x004A6330-0x004A6D10 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004a6330.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6330 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's button callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_button_clicked@ReportIf@@UAEXH@Z
void ReportIf::on_iface_button_clicked(int button) {
    int faction = ConsoleGlobal.faction_id_;
    if (button > 9999) {
        report_topic_changed_ = 1;
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].set_report_page_state(-1);
        flatButtonsA_[ReportIfGlobal.report_faction_id_ - 1].redraw();
        flatButtonsA_[button - 10000].set_report_page_state(2);
        flatButtonsA_[button - 10000].redraw();
        report(ReportIfGlobal.field_8_, button - 9999, report_selection_);
        return;
    }
    if (field_8_ == 4 && report_faction_id_ == faction) {
        switch (button) {
        case 0xCEF: {
            // Selection covers the entire original 512-record base allocation,
            // not merely the currently populated prefix.
            for (int base = 0; base < 512; ++base)
                if (Bases[base].faction_id_current == faction
                    && Bases[base].faction_id_current == ReportIfGlobal.report_faction_id_)
                    ReportWinGlobal.selected_bases_[base >> 5] |= 1u << (base & 31);
            int count = 0;
            for (int selected = 0; selected < 512; ++selected)
                if (ReportWinGlobal.selected_bases_[selected >> 5] & (1u << (selected & 31))) ++count;
            ReportWinGlobal.field_5A04_ = count == 1;
            break;
        }
        case 0xCF0: case 0xCF1: case 0xCF2: case 0xCF3: {
            log_say("Global EDBC base set", 0, 0, 0);
            for (int base = 0; base < BaseCurrentCount; ++base) {
                if (Bases[base].faction_id_current == faction
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))
                    && NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    TutWinBaseWindow.soft_zoom(base);
                    BaseCurrent->governor |= 0x80000000;
                    BaseCurrent->governor &= 0xF0FFFFFF;
                    BaseCurrent->governor |= 0x01000000u << (button - 0xCF0);
                    TutWinBaseWindow.gov_activate();
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        case 0xCF4: {
            log_say("Global governor advanced", 0, 0, 0);
            bool configured = false;
            for (int base = 0; base < BaseCurrentCount; ++base) {
                if (Bases[base].faction_id_current != faction
                    || !(ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))) continue;
                if (!configured) {
                    TutWinBaseWindow.soft_zoom(base);
                    if (TutWinBaseWindow.governor(1) != 0) break;
                    configured = true;
                }
                if (NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    TutWinBaseWindow.soft_zoom(base);
                    BaseCurrent->governor = PlayersData[report_faction_id_].base_governor_adv;
                    TutWinBaseWindow.gov_activate();
                    draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2);
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        case 0xCF5: {
            log_say("Global governor off", 0, 0, 0);
            for (int base = 0; base < BaseCurrentCount; ++base) {
                if (Bases[base].faction_id_current == faction
                    && (ReportWinGlobal.selected_bases_[base >> 5] & (1u << (base & 31)))
                    && NetDaemonNet.lock_base(base, 0, -1, -1) == 0) {
                    Bases[base].governor &= 0x7FFFFFFF;
                    draw_radius(Bases[base].x, Bases[base].y, 2, 2);
                    NetDaemonNet.unlock_base(base);
                }
            }
            break;
        }
        }
        ReportWinGlobal.redraw();
        faction = ConsoleGlobal.faction_id_;
    }
    if (field_8_ == 7 && report_faction_id_ == faction) {
        if (button == 0xCF6 || button == 0xCF7 || button == 0xCF8) {
            for (int row = 0; row < 128; ++row) {
                const int prototype_id = row < 64 ? row : report_faction_id_ * 64 - 64 + row;
                const VehPrototype &prototype = VehPrototypes[prototype_id];
                const int offense = Weapon[prototype.weapon_id].offense_rating;
                const int filter = field_1E244_;
                if (!(filter == 4 || (filter == 3 && offense == 0)
                    || (filter < 3 && Chassis[prototype.chassis_id].triad == filter && offense != 0))) continue;
                if (button == 0xCF6) {
                    ReportWinGlobal.selected_prototypes_[row >> 5] |= 1u << (row & 31);
                    continue;
                }
                if (!(ReportWinGlobal.selected_prototypes_[row >> 5] & (1u << (row & 31)))) continue;
                if (diplo_lock(100) != 0) break;
                for (int vehicle = 0; vehicle < VehCurrentCount; ++vehicle) {
                    Veh &record = Vehs[vehicle];
                    const bool selected = button == 0xCF7
                        ? static_cast<uint8_t>(record.unk_1) == report_faction_id_
                            && record.proto_id == prototype_id
                        : record.faction_id == report_faction_id_
                            && record.proto_id == prototype_id;
                    if (!selected || veh_jail(vehicle) != 0) continue;
                    if (button == 0xCF7) {
                        record.order = 0;
                        record.state &= 0xFCFFBDFF;
                        record.waypoint_count = 0;
                    } else {
                        record.order_auto_type = 0;
                        record.state &= 0xFCFFBFFF;
                        record.order = 0;
                        record.state |= 0x200;
                        record.waypoint_count = 0;
                    }
                    synch_veh(vehicle);
                }
                diplo_unlock();
            }
        }
        ReportWinGlobal.redraw();
    }
    if (button >= 2 && button <= 8) {
        ReportIfGlobal.field_8_ = button;
        if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5)
            ReportIfGlobal.report(button, ReportIfGlobal.report_faction_id_, 0);
        else ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    }
}

// ORIGINAL: 0x004A6D70 ?on_iface_group_clicked@ReportIf@@QAEXHHH@Z 0x004A6D70-0x004A6FD7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004a6d70.cpp retained original identity, full spans and seed history", "recovery/tasks/interface-contract-report-closure.json authorized coupled owner recovery", "original:0x4a6d70 complete control/data-access audit; ReportIf declarations and genuine ReportWin control storage"], "reason": "Recover ReportIf's group callback from the retained seed through its genuine SubInterface owner and ReportWin controls. Its current emitted virtual symbol is recorded separately from the historical catalogue spelling."}
// symbol    ?on_iface_group_clicked@ReportIf@@UAEXHHH@Z
void ReportIf::on_iface_group_clicked(int group, int, int item) {
    if (group == 2 && (item == 0x92 || item == 0x93)) {
        field_1E1D8_ = item == 0x92;
    } else {
        if (group == 4) {
            if (item == 0xD0B) {
                flatButton5_.hide(); flatButton6_.hide(); flatButton7_.hide();
                flatButton8_.hide(); flatButton9_.hide(); flatButton10_.hide();
                flatButton11_.hide();
            } else {
                flatButton5_.show(0); flatButton6_.show(0); flatButton7_.show(0);
                flatButton8_.show(0); flatButton9_.show(0); flatButton10_.show(0);
                flatButton11_.show(0);
            }
        } else if (group == 0x6A) {
            ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
            ReportWinGlobal.flatButton9_.Win::hide();
        } else if (group == 0xCE) {
            ReportWinGlobal.flatButton9_.Win::show(0);
        }
        // The D0B branch jumps directly to the redraw, bypassing map mode.
        if (!(group == 4 && item == 0xD0B)) {
            if (item == 0x3EE) WorldClimateWorldWin.set_world_map();
            else if (item == 0x3EF) WorldClimateWorldWin.set_detail_map();
        }
    }
    if (field_1E234_ == 0) {
        on_redraw();
        MainInterfaceGlobal.soft_update();
    }
}

// ORIGINAL: 0x004A3FC0 ?on_redraw@ReportIf@@QAEXXZ 0x004A3FC0-0x004A401F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004a3fc0.cpp", "image:0x004A3FC0-0x004A401F and dispatch table0x004A4020", "source:src/reportif.h"], "reason": "Recover the report-interface redraw operation through its real category member, reset and owned drawing methods. Original direct calls and nine-entry dispatch preserve the default/no-draw category without vtable or executable-address dispatch."}
void ReportIf::on_redraw() {
    reset();
    switch (field_8_) {
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    case 0: draw_intel(); break;
    }
}



/*
Purpose: Construct the SubInterface base, then every
         sub-object in image order - seven FlatButton runs, three Sprite
         runs, two ListBoxes, four ButtonGroups and fourteen individual
         FlatButtons, and a Spot - before installing ReportIf's own vftable.
// ORIGINAL: 0x004AD170 ??0ReportIf@@QAE@XZ 0x004AD170-0x004AD3AA;0x00658F80-0x00659134
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 34/132 instructions agree - the best of this
//            batch. Layout (offsets, sub-object sizes, construction order)
//            cross-checked directly against the destructor at 0x004ACDA0,
//            which tears the same run down in mirrored order.
// TRIED: real declared members (FlatButton[7]/Sprite[0x15]/FlatButton[7])
//            for flatButtonsA_/spritesA_/flatButtonsB_, built implicitly -
//            measured WORSE, 26/132 (compiled grew to 195 instructions). The
//            `object[0] = vtable` store has to stay explicit body code (its
//            base SubInterface is not parametrised per host class, and that
//            header is out of this batch's scope), and standard C++ always
//            runs implicit member construction before any body statement, so
//            real members push that store after all three instead of ahead
//            of them, unlike the image. Reverted.
// size      1006 bytes
// prototype void (__thiscall ??0ReportIf@@QAE@XZ)(ReportIf* this)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00607CF0 0x00609DB0 0x0062B7C0 0x006456E4 0x006457C2
*/
ReportIf::ReportIf() {
    // Declaration-order construction / reverse-order teardown is automatic.
}

// ListBox's ordinary member teardown includes its complete-object virtual
// bases: GraphicWin at +0x48 and Dialog at +0xA60, matching the original
// adjusted destructor calls (609EC0, 608E10, 5D4DD0).

/*
Purpose: Tear down every sub-object the constructor built, in exactly the
         reverse order.
// ORIGINAL: 0x004ACDA0 ??1ReportIf@@QAE@XZ 0x004ACDA0-0x004AD16C;0x00658D50-0x00658F74
// TRIED: SEH frame - the image has one (flags carry `frame`) but a body
//            built entirely from plain `->method()`/`->~T()` calls at raw
//            offsets never triggers VC6's unwind-protection scaffolding (no
//            local object construction for it to protect), so this tree's
//            compiled body has none. A prior attempt at this same address
//            (src/unrecovered/004acda0.cpp, now deleted - superseded by
//            this body) measured the identical finding in isolation.
//            MISMATCH, 4/235 instructions agree; not chased further given
//            the family-wide cap this batch's brief already names.
// size      1520 bytes
// prototype void (__thiscall ??1ReportIf@@QAE@XZ)(ReportIf* this)
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0062B7F0 0x006456E4
*/
ReportIf::~ReportIf() {
    // Declaration-order construction / reverse-order teardown is automatic.
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FE0 ?on_iface_right_click@ReportIf@@QAEXHH@Z 0x004A5FE0-0x004A5FE3 BYTE_EXACT
// symbol    ?on_iface_right_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FF0 ?on_iface_left_double_click@ReportIf@@QAEXHH@Z 0x004A5FF0-0x004A5FF3 BYTE_EXACT
// symbol    ?on_iface_left_double_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6000 ?on_iface_right_double_click@ReportIf@@QAEXHH@Z 0x004A6000-0x004A6003 BYTE_EXACT
// symbol    ?on_iface_right_double_click@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6010 ?on_iface_left_down@ReportIf@@QAEXHH@Z 0x004A6010-0x004A6013 BYTE_EXACT
// symbol    ?on_iface_left_down@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6020 ?on_iface_right_down@ReportIf@@QAEXHH@Z 0x004A6020-0x004A6023 BYTE_EXACT
// symbol    ?on_iface_right_down@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6030 ?on_iface_mouse_move@ReportIf@@QAEXHH@Z 0x004A6030-0x004A6033 BYTE_EXACT
// symbol    ?on_iface_mouse_move@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6040 ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z 0x004A6040-0x004A6043 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_leave(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6D60 ?on_iface_button_toggled@ReportIf@@QAEXHH@Z 0x004A6D60-0x004A6D63 BYTE_EXACT
// symbol    ?on_iface_button_toggled@ReportIf@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_button_toggled(int, int) {
}

// The original range normalization and merge algorithm operate on a fixed
// 0x200-row workspace inside ReportIf. Keys are ordered descending; each
// merge writes the scratch pair, then publishes it back to the primary pair.
// ORIGINAL: 0x004ACC30 ?merge_sort@ReportIf@@QAEXHH@Z 0x004ACC30-0x004ACD98
// RECOVERY_BINDING: v1 {"evidence": ["git:4097a7c0:src/recovered/units/004acc30.cpp;source:src/reportif.h;source:src/base.h:BaseCurrentCount;source:src/general.h:swap"], "reason": "Recover the ReportIf base-ranking merge sort over its measured 0x200-entry, 16-byte scratch workspace, using the source-owned BaseCurrentCount bound and preserving original range normalization and key-prefetch order."}
void ReportIf::merge_sort(int first, int last) {
    const int maximum = BaseCurrentCount - 1;
    if (first < 0 || maximum < 0) {
        first = 0;
    } else if (first > maximum) {
        first = maximum;
    }

    int high = maximum;
    if (last < 0 || maximum < 0) {
        high = 0;
    } else if (last <= high) {
        high = last;
    }

    if (first == high) return;

    int end = high;
    if (high < first) swap(&first, &end);

    const int middle = (end + first) / 2;
    merge_sort(first, middle);
    int right = middle + 1;
    merge_sort(right, end);

    int left = first;
    int output = first;
    // Preserve the original unchecked key prefetch: it reads both cursors'
    // values before testing which half is exhausted, including an endpoint
    // read adjacent to the active range. Do not turn this into a guarded,
    // conventional merge without a separate behavior decision.
    while (output <= end) {
        int selected;
        if (sort_entries_[left].value < sort_entries_[right].value) {
            if (right <= end) {
                selected = right++;
            } else {
                selected = left++;
            }
        } else if (left <= middle) {
            selected = left++;
        } else {
            selected = right++;
        }
        sort_entries_[output].sorted_base_id = sort_entries_[selected].base_id;
        sort_entries_[output].sorted_value = sort_entries_[selected].value;
        ++output;
    }

    for (int i = first; i <= end; ++i) {
        sort_entries_[i].base_id = sort_entries_[i].sorted_base_id;
        sort_entries_[i].value = sort_entries_[i].sorted_value;
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004AC130 ?close_score@ReportIf@@QAEXXZ 0x004AC130-0x004AC131 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close_score@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::close_score() {
}

// Close the project page's global map overlay and the two project-selector
// buttons owned by MainInterface, not by ReportIf.
// ORIGINAL: 0x004AA930 ?close_projects@ReportIf@@QAEXXZ 0x004AA930-0x004AA964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:4097a7c0:src/recovered/004aa930.cpp;source:src/reportif.h;source:src/worldwin.h;source:src/maininterface.h;source:src/flatbutton.h"], "reason": "Close the project report overlay by its real WorldWin and MainInterface owners when the active report is not one of the two map categories."}
void ReportIf::close_projects() {
    const int state = field_8_;
    if (state != 4 && state != 5) {
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide();
        MainInterfaceGlobal.flat_button_[7].hide();
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
// ORIGINAL: 0x004A6320 ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x004A6320-0x004A6328 BYTE_EXACT
// symbol    ?on_iface_dialog_item_back_draw@ReportIf@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      8 bytes
// prototype int (__thiscall ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(ReportIf* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1
Status: Complete
*/
int ReportIf::on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *) {
    return 1;
}


/*
Purpose: Close the two intel list boxes.

             mov esi,ecx / lea ecx,[esi+0xA2D0] / call ListBox::close
                           lea ecx,[esi+0xAE24] / call ListBox::close

         Two subobjects, in that order, and nothing else. ReportIf models no
         fields, so both offsets are documented and raw.

         ListBox::close resolves its own bases through the object's vbtable at
         run time, so each of these has to be a real ListBox carrying a table
         of its own; reaching them by offset is what the original does and is
         what keeps the two independent.
// ORIGINAL: 0x004AC980 ?close_intel@ReportIf@@QAEXXZ 0x004AC980-0x004AC99B BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?close_intel@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_intel() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}


/*
Purpose: Close the energy report's three list boxes.

             mov ecx,0x87BE84     / call ListBox::close
             lea ecx,[esi+0xA2D0] / call ListBox::close
             lea ecx,[esi+0xAE24] / call ListBox::close

         The last two are the same subobjects close_intel closes, in the same
         order. The first is ReportWinGlobal's owned listBox_ at +0x5A0C;
         it is borrowed here and closes first.

         Original 0x0087BE84 = ReportWinGlobal 0x00876478 + 0x5A0C.
         Its construction and teardown belong to ReportWin, not an
         independent global allocation.
// ORIGINAL: 0x004A9020 ?close_energy@ReportIf@@QAEXXZ 0x004A9020-0x004A9045 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?close_energy@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_energy() {
    ReportWinGlobal.listBox_.close();
    listBox1_.close();
    listBox2_.close();
}

// The constructor owns this ListBox and these FlatButtons as members;
// the retained close_labs seed closes/hides those same
// objects in this order. Slot 2 is the inherited Win::hide virtual.
// ORIGINAL: 0x004A8470 ?close_labs@ReportIf@@QAEXXZ 0x004A8470-0x004A849E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004a8470.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/listbox.h"], "reason": "Close the ReportIf lab list and hide its two placement-constructed FlatButtons through their typed Win virtual method."}
void ReportIf::close_labs() {
    listBox1_.close();
    flatButton1_.hide();
    flatButton2_.hide();
}

// The three FlatButtons are genuine ReportIf members. The seed
// calls Win slot 2 on them in this order; keep the original object identities
// while using the source-owned virtual method instead of a vtable shim.
// ORIGINAL: 0x004ABE30 ?close_security@ReportIf@@QAEXXZ 0x004ABE30-0x004ABE62 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004abe30.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/win.h"], "reason": "Hide the three ReportIf security FlatButtons through Win slot 2 in retained-seed order."}
void ReportIf::close_security() {
    flatButton12_.hide();
    flatButton14_.hide();
    flatButton13_.hide();
}

// ReportWin owns these three controls as members; close_ops clears
// each ungrouped state word before hiding it. ReportIf's own controls and the
// final MainInterface pair remain their respective typed owners.
// ORIGINAL: 0x004A9F90 ?close_ops@ReportIf@@QAEXXZ 0x004A9F90-0x004AA0AF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004a9f90.cpp", "source:src/reportif.h", "source:src/reportwin.h", "source:src/basebutton.h"], "reason": "Close the ReportIf and ReportWin operation controls through their named owners, preserve conditional map cleanup, and clear only ungrouped ReportWin BaseButton state through an encapsulated helper."}
void ReportIf::close_ops() {
    listBox1_.close();
    flatButton3_.hide();
    flatButton4_.hide();
    flatButton5_.hide();
    flatButton6_.hide();
    flatButton7_.hide();
    flatButton8_.hide();
    flatButton9_.hide();
    flatButton10_.hide();
    flatButton11_.hide();

    FlatButton *button = &ReportWinGlobal.flatButton4_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();
    button = &ReportWinGlobal.flatButton5_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();
    button = &ReportWinGlobal.flatButton6_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();

    if (field_8_ != 4 && field_8_ != 5) {
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide();
        MainInterfaceGlobal.flat_button_[7].hide();
    }
}

// Animation storage is the ReportIf sprite run; the destination is the
// Buffer base of the global WorldClimateWorldWin. Its timeout lives in the
// genuine ReportWin-owned Time member.
// ORIGINAL: 0x004A4060 ?bl_anim@ReportIf@@QAEXXZ 0x004A4060-0x004A4191
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004a4060.cpp", "source:src/reportif.h", "source:src/worldwin.h", "source:src/reportwin.h", "source:src/sprite.h"], "reason": "Advance ReportIf animation with the declared sprite run and coordinate state, drawing to the WorldClimateWorldWin Buffer base and stopping the ReportWin-owned Time at terminal frame 10."}
void ReportIf::bl_anim() {
    WorldClimateWorldWin.redraw();
    Buffer *const destination = static_cast<Buffer *>(&WorldClimateWorldWin);
    switch (animation_state_) {
    case -1:
    case 0:
    case 8:
    case 9:
        spritesB_[0].draw(destination, spritesB_[0].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 1:
    case 7:
        spritesB_[1].draw(destination, spritesB_[1].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 2:
    case 4:
    case 6:
        spritesB_[2].draw(destination, spritesB_[2].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 3:
    case 5:
        spritesB_[3].draw(destination, spritesB_[3].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 10:
        ReportWinGlobal.time1_.stop();
        break;
    }
    ++animation_state_;
}


/*
// ORIGINAL: 0x004A3930 ?done@ReportIf@@QAEXXZ 0x004A3930-0x004A3946 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?done@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005EE280
Status: Complete
*/
// The window whose modal ReportIf::done releases, 0x00876478 - now IDENTIFIED
// as the process-wide ReportWin singleton reportwin.cpp defines: the original
// dynamic initializer at 0x00496930 constructs it (`mov ecx, 0x876478; call
// ??0ReportWin@@QAE@XZ`, followed by the atexit registration), and
// ReportWin::on_key_click closes the same object with Win::release_modal.
// done() releases its modal loop and gives up the active interface mode.
void ReportIf::done() {
    ReportWinGlobal.Win::release_modal();
    reinterpret_cast<SubInterface *>(this)->release_iface_mode();
}

// The seams and definitions for the unrecovered bodies, at the end of the file
// so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// reportif.cpp(219), which is the release_iface_mode call inside ReportIf::done,
// and done's 22-byte BYTE_EXACT match is a `call rel32` there.


/*
 * SubInterface's two called methods, hosted here for the reason spelled out in
 * subinterface.h: that class has no translation unit of its own, and this is
 * the file that already sees the header and already calls one of them.
 */
func_sub_interface_iface_mode SubInterfaceSetIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D310);
func_sub_interface_iface_mode SubInterfaceReleaseIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D380);

/*
Purpose: Make this interface the active one. The body at 0x0045D310 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::set_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceSetIfaceMode)();
}

/*
Purpose: Give up the active interface mode. The body at 0x0045D380 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::release_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceReleaseIfaceMode)();
}
#pragma auto_inline(on)

// Research state87BE24/885F30 is ReportWinGlobal+59AC/+FAB8.
// The energy list87BE84 is ReportWinGlobal+5A0C. No duplicate storage.

// The process-wide ReportIf itself, 0x00885F38 - see reportif.h. The
// image's dynamic initializer at 0x00496900 constructs it through the real
// recovered constructor above, so the storage sits here.
// Original constructor 0x004AD170 initializes this singleton; its two list
// receivers at 0x00890208/0x00890D5C are owned +0xA2D0/+0xAE24 members.
// This start-only binding does not infer a complete original allocation.
// RECOVERY_OBJECT: v1 0x00885F38 ?ReportIfGlobal@@3VReportIf@@A
ReportIf ReportIfGlobal;  // 0x00885F38

/*
 * Panel drawing uses the shared mutable InterfaceBoxStyles records and the
 * Buffer subobject owned by MainInterfaceGlobal. Its working rectangle is
 * exposed by GraphicWin::buffer_working_rect().
 */

// ORIGINAL: 0x004A3D20 ?reset@ReportIf@@QAEXXZ 0x004A3D20-0x004A3FBF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/maininterface.h:MainInterfaceGlobal", "source:src/graphicwin.h:GraphicWin::buffer_working_rect", "source:src/datalink.cpp:1933", "docs/recovery/member-accesses.csv:ReportIf offsets 0x20, 0x30, 0x40, 0x50"], "reason": "Draw through the MainInterfaceGlobal Buffer base and its owned working clip rectangle, with typed ReportIf panel rectangles."}
// size      671 bytes
// prototype void (__thiscall ?reset@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   5   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DEF90 0x005E2DD3
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004A3D20
// measured tier  MISMATCH
// divergence     43
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a3d20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?reset@ReportIf@@QAEXXZ  at 0x004A3D20  (671 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void ReportIf::reset() {
    Buffer *buf = static_cast<Buffer *>(&MainInterfaceGlobal);

    RECT *r1 = &field_20_;
    buf->box_sprite(r1, &InterfaceBoxStyles[33]);
    RECT *r2 = &field_30_;
    buf->box_sprite(r2, &InterfaceBoxStyles[33]);
    RECT *r3 = &field_50_;
    buf->box_sprite(r3, &InterfaceBoxStyles[33]);
    RECT *r4 = &field_40_;
    buf->set_clip(r4);
    r4->left -= 7;
    buf->box_sprite(r4, &InterfaceBoxStyles[33]);
    r4->left += 7;
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    r1->left += 3;
    r1->right -= 3;
    r1->top += 3;
    r1->bottom -= 3;
    r2->left += 3;
    r2->top += 3;
    r2->right -= 3;
    r2->bottom -= 3;
    r3->top += 4;
    r3->left += 4;
    r3->right -= 4;
    r3->bottom -= 4;
    r4->left += 4;
    r4->top += 4;
    r4->right -= 4;
    r4->bottom -= 4;
    r4->left -= 3;

    buf->box_sprite(r1, &InterfaceBoxStyles[3]);
    buf->box_sprite(r2, &InterfaceBoxStyles[3]);
    buf->box_sprite(r3, &InterfaceBoxStyles[3]);
    buf->box_sprite(r4, &InterfaceBoxStyles[3]);

    r3->left += 3;
    r3->right -= 3;
    r3->top += 3;
    r3->bottom -= 3;
    r4->left += 3;
    r4->right -= 3;
    r4->top += 3;
    r4->bottom -= 3;

    buf->box_sprite(r3, &InterfaceBoxStyles[0]);
    buf->box_sprite(r4, &InterfaceBoxStyles[0]);

    r4->left += 3;
    r1->left -= 3;
    r1->right += 3;
    r1->top -= 3;
    r1->bottom += 3;
    r2->left -= 3;
    r2->right += 3;
    r2->top -= 3;
    r2->bottom += 3;
    r3->left -= 7;
    r3->right += 7;
    r3->top -= 7;
    r3->bottom += 7;
    r4->left -= 7;
    r4->right += 7;
    r4->top -= 7;
    r4->bottom += 7;

    buf->set_font(field_1E180_, field_1E184_, field_1E184_, 0);
    buf->set_text_color(0x99, -1, 1, 1);
    buf->set_text_color2(0x99, -1, 1, 1);
    buf->set_text_color3(0xe1, -1, 1, 1);
    buf->clear_links();
}

// ORIGINAL: 0x004ABE70 ?draw_score@ReportIf@@QAEXXZ 0x004ABE70-0x004AC127
// The report renderer uses the ReportIf-owned panel rectangles and fonts,
// MainInterfaceGlobal's Buffer base, and the source-owned faction/string
// tables. The original's two static separators are represented by ordinary
// literals; their bytes at 0x00682820 and 0x00682E90 are respectively " "
// and ", ".
void ReportIf::draw_score() {
    Buffer *buf = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT *first_panel = &field_20_;

    buf->set_clip(first_panel);
    field_C_ = first_panel->left + 6;
    field_10_ = first_panel->top + 6;
    field_14_ = first_panel->right - first_panel->left - 0xC;
    field_18_ = first_panel->bottom - first_panel->top - 0xC;

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buf->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *score_panel = &field_50_;
    buf->set_clip(score_panel);
    field_10_ = score_panel->top;
    const int left_inset = field_1E230_ != 0 ? 0x4C : 0;
    field_C_ = score_panel->left + left_inset;
    field_14_ = score_panel->right - left_inset - score_panel->left;
    field_18_ = score_panel->bottom - score_panel->top;

    buf->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buf->set_text_color(0xE1, -1, 1, 1);

    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B7));

    if (field_1E230_ != 0) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }

    buf->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                      static_cast<int>(strlen(StringTemp)));

    buf->set_font(field_1E180_, field_1E180_, field_1E180_, nullptr);
    buf->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *last_panel = &field_30_;
    buf->set_clip(last_panel);
    field_C_ = last_panel->left + 6;
    field_10_ = last_panel->top + 6;
    field_14_ = last_panel->right - last_panel->left - 0xC;
    field_18_ = last_panel->bottom - last_panel->top - 0xC;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buf->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// MainInterface owns the interface-slot array and active interface ID.

// close_sat borrows ReportWinGlobal's buttons9/7/8 and timers2/3.
// Original881A24/88038C/880ED8 are876478+B5AC/9F14/AA60.
// Their +AA8/+A9C fields are BaseButton.group_/field_A9C_; the existing
// typed reset_field_A9C_if_ungrouped preserves the original inline condition.
// Original884738/884760 are876478+E2C0/E2E8, not timer pointers/globals.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

// The eight menu commands all use the same already-owned report window and
// interface mode. Retained seeds at 0x004AC9A0..0x004ACC29 establish each
// category value and the modal-versus-direct branch.
// ORIGINAL: 0x004AC9A0 ?report_labs@@YAXXZ 0x004AC9A0-0x004AC9EC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004ac9a0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_labs seed identifies this free report command and its original extent. Category 2 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_labs() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 2;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(2, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004AC9F0 ?report_energy@@YAXXZ 0x004AC9F0-0x004ACA3C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004ac9f0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_energy seed identifies this free report command and its original extent. Category 3 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_energy() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 3;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(3, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA40 ?report_ops@@YAXXZ 0x004ACA40-0x004ACA8C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004aca40.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_ops seed identifies this free report command and its original extent. Category 4 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_ops() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 4;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(4, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA90 ?report_projects@@YAXXZ 0x004ACA90-0x004ACADC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aca90.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_projects seed identifies this free report command and its original extent. Category 5 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_projects() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 5;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(5, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACAE0 ?report_sat@@YAXXZ 0x004ACAE0-0x004ACB2C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acae0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_sat seed identifies this free report command and its original extent. Category 6 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_sat() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 6;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(6, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB30 ?report_security@@YAXXZ 0x004ACB30-0x004ACB7C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acb30.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_security seed identifies this free report command and its original extent. Category 7 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_security() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 7;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(7, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB80 ?report_score@@YAXH@Z 0x004ACB80-0x004ACBCC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acb80.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_score seed identifies this free report command and its original extent. Category 8 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_score(int) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 8;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(8, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACBD0 ?report_intel@@YAXH@Z 0x004ACBD0-0x004ACC29
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acbd0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_intel seed identifies this free report command and its original extent. Category 0 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_intel(int selection) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 0;
    ReportIfGlobal.set_report_selection(selection);
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(0, ReportIfGlobal.report_faction_id(), selection);
    }
}

/*
Purpose: Show the report window modally with pop_wait as its poll callback,
         or re-report the intel category when the active interface is the
         world map's slot 5.

             cmp  [esi+8], 7 / jne ret          - intel panel only
             push y / push x / lea ecx,[esi+0x1E1BC] / call Spot::check
                                                - (x, y, &y, &x): the spot
                                                  index lands in y, the type
                                                  in x
             inc eax / je ret                   - -1 means "no spot"
             [esi+0x1E244] = y                  - the clicked topic
             [0x008846E8] = 1
             [0x00885F40] = 7                   - switch to the intel report
             cmp IfaceIdToSlot[ActiveIfaceId], 5 - world map keeps its own
                                                  modal owner
             set_modal(0, pop_wait, MainInterfaceGlobal) - slot 0xE4
             report(7, report_faction_id_, 0) on the 0x00885F38 singleton
// ORIGINAL: 0x004A5F40 ?on_iface_left_click@ReportIf@@QAEXHH@Z 0x004A5F40-0x004A5FD6
// symbol    ?on_iface_left_click@ReportIf@@UAEXHH@Z
// size      150 bytes
// prototype void (__thiscall ?on_iface_left_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A3950 0x005FAB00
// indirect  0x004A5FB1
// Homed from src/unrecovered/004a5f40.cpp.
*/
void ReportIf::on_iface_left_click(int x_coord, int y_coord) {
    if (field_8_ != 7) {
        return;
    }

    // check() answers the hit index and hands the spot's position and type
    // back through the two argument slots - the push order is &y_coord then
    // &x_coord, so the position lands in the y argument, the type in the x.
    if (spot_.check(x_coord, y_coord,
                                               &y_coord, &x_coord) == -1) {
        return;
    }
    field_1E244_ = y_coord;

    ReportWinGlobal.select_intel_topic();
    field_8_ = 7;

    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return;
    }

    ReportIfGlobal.report(7, report_faction_id_, 0);
}

/*
Purpose: Close the satellite report: hide every faction's panel button
         except the local player's, hide the five bottom buttons, clear-and-
         hide the three satellite windows, stop both timers.

             esi = 1; edi = self + 0x13778 (flatButtonsC_)
             do { if (esi != LocalFaction) slot002(edi)      - FlatButton::
                                                                close, the
                                                                virtual
                  esi++; edi += 0xB4C } while (esi < 8)
             esi = self + 0x18720 (flatButtonsD_); five slot002 calls at
             0xB4C strides
             the three (flag, state) pairs, then the three Win::hide calls -
             DIRECT `call 0x5EDCD0`, the qualified-base spelling
             Time::stop on 0x00884738 and 0x00884760
// ORIGINAL: 0x004AB550 ?close_sat@ReportIf@@QAEXXZ 0x004AB550-0x004AB5F9
// size      169 bytes
// prototype void (__thiscall ?close_sat@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x00616730
// indirect  0x004AB56C 0x004AB58A
// Homed from src/unrecovered/004ab550.cpp. The seed carried FILE
// BYTE_EXACT; the token is the recorder's to re-bank on this body once
// this file's link-only placeholder (ReportIf::report) is implemented, so
// it is not hand-carried here.
*/
void ReportIf::close_sat() {
    // Both runs are the image's do/while over a walking pointer: the first
    // steps flatButtonsC_ (esi = 1, 0xB4C strides, `cmp esi, 8` at the
    // bottom), keeping the local player's own faction button; the second
    // counts flatButtonsD_ down from 5.
    FlatButton *button = flatButtonsC_;
    int faction = 1;
    do {
        if (faction != ConsoleGlobal.faction_id_) {
            button->close();
        }
        ++button;
        ++faction;
    } while (faction < 8);

    button = flatButtonsD_;
    int count = 5;
    do {
        button->close();
        ++button;
        --count;
    } while (count != 0);

    ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton9_.Win::hide();
    ReportWinGlobal.flatButton7_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton7_.Win::hide();
    ReportWinGlobal.flatButton8_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton8_.Win::hide();
    ReportWinGlobal.time2_.stop();
    ReportWinGlobal.time3_.stop();
}

/*
Purpose: Switch report pages using the existing interface and window owners,
         preserving the original research-selection transition and redraw.
// ORIGINAL: 0x004A3950 ?report@ReportIf@@QAEXHHH@Z 0x004A3950-0x004A3CD3 FILE
// size      899 bytes
// prototype void (__thiscall ?report@ReportIf@@QAEXHHH@Z)(ReportIf* this, int, int, int)
// callers   12   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x0045E540 0x004A3D20 0x004A7000 0x004A84A0 0x004A9050 0x004A9F90 0x004AA0B0 0x004AA970 0x004AB550 0x004AB600 0x004ABE70 0x004AC140 0x004C3A40 0x005B9F20 0x005D5890 0x005D5A70 0x00609F20
Status: Report dispatch and state transition recovered; seven referenced page
        renderers remain explicit, link-only placeholders.
*/
void ReportIf::report(int report_id, int faction_id, int selection) {
    const int previous_page = report_page_history_[0];
    switch (previous_page) {
    case 2:
        listBox1_.close();
        flatButton1_.hide();
        flatButton2_.hide();
        break;
    case 3:
        ReportWinGlobal.listBox_.close();
        // Fall through: energy and intel share both embedded lists.
    case 0:
        listBox1_.close();
        listBox2_.close();
        break;
    case 4:
        close_ops();
        break;
    case 5:
        if (field_8_ != 4 && field_8_ != 5) {
            WorldClimateWorldWin.hide_all();
            MainInterfaceGlobal.flat_button_[6].hide();
            MainInterfaceGlobal.flat_button_[7].hide();
        }
        break;
    case 6:
        close_sat();
        break;
    case 7:
        flatButton12_.hide();
        flatButton14_.hide();
        flatButton13_.hide();
        break;
    }

    // The original category run is 2..8; category 1 has no report page or
    // typed control in the class layout. Zero denotes the intel list.
    if (previous_page >= 2 && previous_page <= 8) {
        FlatButton &old_button = flatButtonsB_[previous_page - 2];
        old_button.set_report_page_state(-1);
        old_button.redraw();
    }
    if (report_id >= 2 && report_id <= 8) {
        FlatButton &new_button = flatButtonsB_[report_id - 2];
        new_button.set_report_page_state(2);
        new_button.redraw();
    }

    report_selection_ = selection;
    field_8_ = report_id;
    report_faction_id_ = faction_id;

    if (previous_page != 2 && ReportWinGlobal.shared_report_selection() != -1) {
        int next_focus;
        if (!(GameState & STATE_OMNISCIENT_VIEW)
            && (GameRules & RULES_BLIND_RESEARCH)) {
            if (ReportWinGlobal.shared_research_focus() >= 0 && ReportWinGlobal.shared_research_focus() <= 0x59
                && has_tech_call(ReportWinGlobal.shared_research_focus(), faction_id)) {
                next_focus = ReportWinGlobal.shared_research_focus();
            } else {
                next_focus = ReportWinGlobal.shared_research_focus();
                for (int tech_id = 0; tech_id < 0x59; ++tech_id) {
                    if (has_tech_call(tech_id, faction_id)) {
                        next_focus = tech_id;
                        break;
                    }
                }
            }
        } else {
            next_focus = PlayersData[faction_id].tech_id_researching;
        }
        ReportWinGlobal.shared_research_focus() = next_focus;
        if (ReportWinGlobal.shared_research_focus() < 0 || ReportWinGlobal.shared_research_focus() > 0x59) {
            ReportWinGlobal.shared_research_focus() = 0;
        }
        ReportWinGlobal.shared_report_selection() = -1;
    }

    if (faction_id <= 0) {
        return;
    }

    for (int index = 18; index > 0; --index) {
        report_page_history_[index] = report_page_history_[index - 1];
    }
    report_page_history_[0] = field_8_;
    const int current_page = report_page_history_[0];
    if (current_page == 4 || current_page == 5 || current_page == 7) {
        if (current_page != report_page_history_[1] || report_topic_changed_) {
            ReportWinGlobal.field_E270_ = 1;
            report_topic_changed_ = 0;
        }
    } else if (ReportWinGlobal.field_C0F8_ != 0) {
        scroll_hide(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E270_ = 0;
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 0;
    }

    reset();
    switch (field_8_) {
    case 0: draw_intel(); break;
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    }
    MainInterfaceGlobal.soft_update();
    ReportWinGlobal.redraw();
    if (ReportWinGlobal.field_E26C_ != 0 && !report_scroll_blocked_) {
        scroll_show(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 1;
    }
}

// These panel renderers belong to report()'s ReportIf owner. The retained
// seeds provide their original identities and cross-function evidence.
// ORIGINAL: 0x004A7000 ?draw_labs@ReportIf@@QAEXXZ 0x004A7000-0x004A842F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a7000.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/tech.h"], "reason": "Original panel identity and accesses identify ReportIf's laboratory renderer, using its lists and selection state, source-owned base rows and technology records; preserve the seed's row IDs and technology sentinel rather than substitute a different panel."}
void ReportIf::draw_labs() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    flatButton1_.show(0);
    flatButton2_.show(0);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + 26;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    field_10_ += 32;
    field_18_ -= 32;
    draw_workspace_rect_.left = field_C_ + 3;
    draw_workspace_rect_.top = field_10_ + 3;
    draw_workspace_rect_.right = field_C_ + field_14_ - 3;
    draw_workspace_rect_.bottom = field_10_ + field_18_ - 3;
    scroll_small();
    // The complete-object ListBox owns its compiler-adjusted virtual bases.
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    list.set_report_text_color(0xF2);
    list.set_write_callback(labs_write);
    char number[80];
    if (field_1E1D8_ == 0) {
        for (int base = 0; base < BaseCurrentCount; ++base) {
            if (Bases[base].faction_id_current != report_faction_id_) continue;
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|"); strcat(StringTemp, "+");
            const int labs = Bases[base].labs_total;
            _itoa(labs, number, 10); strcat(StringTemp, number);
            strcat(StringTemp, " "); strcat(StringTemp, "(");
            int research = PlayersData[report_faction_id_].soc_effect_pending.research;
            if (research < -5) research = -5;
            else if (research > 5) research = 5;
            const int scaled = (research + 10) * labs * 10;
            _itoa(scaled / 100, number, 10); strcat(StringTemp, number);
            if (scaled % 100 != 0) {
                strcat(StringTemp, ".");
                _itoa((scaled % 100) / 10, number, 10); strcat(StringTemp, number);
            }
            strcat(StringTemp, ")"); strcat(StringTemp, label_get(0x17B));
            list.item(StringTemp, base + 0x400);
        }
    } else {
        int levels[MaxTechnologyNum + 1];
        for (int tech = 0; tech < MaxTechnologyNum; ++tech)
            levels[tech] = has_tech(tech, report_faction_id_)
                ? tech_category(tech) + tech_recurse(tech, 0) * 4 : 0;
        levels[MaxTechnologyNum] = 999999;
        for (int iteration = 0; iteration < MaxTechnologyNum; ++iteration) {
            int best = MaxTechnologyNum;
            for (int candidate = 0; candidate < MaxTechnologyNum; ++candidate)
                if (levels[candidate] != 0 && levels[candidate] < levels[best]) best = candidate;
            if (best < MaxTechnologyNum) {
                StringTemp[0] = 0;
                say_tech(StringTemp, best, 0);
                strcat(StringTemp, "|"); strcat(StringTemp, "[");
                strcat(StringTemp, label_get(0x275 + (levels[best] & 3)));
                strcat(StringTemp, " ");
                _itoa(levels[best] >> 2, number, 10); strcat(StringTemp, number);
                strcat(StringTemp, "]");
                list.item(StringTemp, best);
                levels[best] = 0;
            }
        }
    }
    list.reset_report_selection();
    dialog.field_B4_ = 0;
    list.attach(&MainInterfaceGlobal, dialog.field_24_, dialog.field_28_, 0x80020);
    if (field_1E1D8_ == 0) list.set_selected_pos(-1, 1);
    else list.set_selected_id(ReportWinGlobal.shared_research_focus());
    static_cast<GraphicWin &>(list).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction); CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B1));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_, strlen(StringTemp));
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x178)); strcat(StringTemp, ": ");
    _itoa(tech_rate(report_faction_id_), number, 10); strcat(StringTemp, number);
    strcat(StringTemp, "    "); strcat(StringTemp, label_get(0x179)); strcat(StringTemp, ": ");
    _itoa(PlayersData[report_faction_id_].tech_accumulated, number, 10); strcat(StringTemp, number);
    if (field_1E230_ == 0) {
        buffer->write_cent_l(StringTemp, field_C_, field_10_, field_14_, strlen(StringTemp));
        field_10_ += field_1C_;
        StringTemp[0] = 0;
    } else strcat(StringTemp, "    ");
    strcat(StringTemp, label_get(0x17A)); strcat(StringTemp, ": ");
    energy_compute(report_faction_id_, 0);
    _itoa(EnergyLabsTotal / 100, number, 10); strcat(StringTemp, number);
    if (EnergyLabsTotal % 100 != 0) {
        strcat(StringTemp, ".");
        _itoa((EnergyLabsTotal % 100) / 10, number, 10); strcat(StringTemp, number);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_, field_14_, strlen(StringTemp));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->set_font(field_1E184_, field_1E18C_, field_1E198_, 0);
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_30_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0; strcat(StringTemp, "[");
    const int theory = PlayersData[report_faction_id_].theory_of_everything;
    if (theory == 0) {
        buffer->set_text_color3(0xE1, -1, 1, 1);
        strcat(StringTemp, label_get(0x2C0));
    } else {
        buffer->set_text_color3(mandate_color(1), -1, 1, 1);
        say_tech(StringTemp, PlayersData[report_faction_id_].tech_id_researching, 0);
        CharUpperA(StringTemp); strcat(StringTemp, " ");
        _itoa(theory, number, 10); strcat(StringTemp, number);
    }
    strcat(StringTemp, "]");
    field_18_ -= 4; field_10_ += 4;
    buffer->write_cent_l(StringTemp, field_C_, field_10_, field_14_, strlen(StringTemp));
    field_18_ -= field_1C_; field_10_ += field_1C_;
    buffer->set_text_color3(0xE1, -1, 1, 1);
    field_18_ -= 2; field_C_ += 3;
    const int bar_height = static_cast<int>(field_18_) / 4 - 15;
    field_14_ -= 6;
    int total[4] = {0, 0, 0, 0};
    int owned[4] = {0, 0, 0, 0};
    // The original pre-requisite cursor stops before Transcendent Thought.
    for (int technology = 0; technology < MaxTechnologyNum - 1; ++technology) {
        if (Technology[technology].preq_tech_1 == -2 || Technology[technology].preq_tech_2 == -2) continue;
        const int category = tech_category(technology);
        if (category >= 0 && category <= 3) {
            ++total[category];
            if (has_tech(technology, report_faction_id_)) ++owned[category];
        }
    }
    field_10_ += 8;
    for (int category = 0; category < 4; ++category) {
        StringTemp[0] = 0;
        draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + 15;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[38 + category * 3]);
        // No zero-denominator guard exists in the original technology bars.
        draw_workspace_rect_.right = field_C_ + static_cast<int>(field_14_) * owned[category] / total[category];
        buffer->set_clip(&draw_workspace_rect_);
        draw_workspace_rect_.right = field_C_ + field_14_;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[36 + category * 3]);
        buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
        field_10_ += 15;
        buffer->set_text_color3(mandate_color(category), -1, 1, 1);
        strcat(StringTemp, "["); strcat(StringTemp, label_get(0x209 + category)); strcat(StringTemp, "   ");
        _itoa(owned[category] * 100 / total[category], number, 10); strcat(StringTemp, number);
        strcat(StringTemp, "%"); strcat(StringTemp, "]");
        buffer->write_l(StringTemp, field_C_ + 2,
            static_cast<int>(field_10_) + (bar_height - static_cast<int>(field_1C_)) / 2, strlen(StringTemp));
        if (category != 3) field_10_ += bar_height;
    }
}

// ORIGINAL: 0x004A84A0 ?draw_energy@ReportIf@@QAEXXZ 0x004A84A0-0x004A9017
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a84a0.cpp", "source:src/reportif.h", "image:0x004A84AE-0x004A84BC pushes mode zero then report faction for the cdecl energy_compute call"], "reason": "Original energy-panel identity maps to ReportIf's list, selection, frame and facility rows. The original cdecl push order establishes energy_compute(report_faction_id_, 0), correcting the seed's reversed displayed arguments without changing the calculation owner."}
void ReportIf::draw_energy() {
    energy_compute(report_faction_id_, 0);
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0xBE));
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2E6)); strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.right = field_14_ + field_C_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    scroll_small();
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    list.set_report_text_color(0xF2);
    for (int base = 0; base < BaseCurrentCount; ++base) {
        if (Bases[base].faction_id_current != report_faction_id_) continue;
        char number[80];
        const int income = Bases[base].economy_total;
        const int maintenance = EnergyBaseMaintenance[base];
        StringTemp[0] = 0;
        say_base(StringTemp, base);
        strcat(StringTemp, "|"); strcat(StringTemp, "+");
        _itoa(income, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, "/"); strcat(StringTemp, "-");
        _itoa(maintenance, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, " = ");
        if (income >= maintenance) strcat(StringTemp, "+");
        _itoa(income - maintenance, number, 10); strcat(StringTemp, number);
        strcat(StringTemp, label_get(0x17B));
        list.item(StringTemp, base + 0x400);
    }
    list.reset_report_selection();
    dialog.field_B4_ = 0;
    list.attach(&MainInterfaceGlobal, dialog.field_24_, dialog.field_28_, 0x80020);
    if (ReportWinGlobal.field_59B0_) {
        if (ReportWinGlobal.shared_report_selection() >= 0x400)
            list.set_selected_id(ReportWinGlobal.shared_report_selection());
        else list.set_selected_pos(0, 1);
    } else list.set_selected_pos(-1, 1);
    static_cast<GraphicWin &>(list).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction); CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B2));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_, strlen(StringTemp));
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x39)); strcat(StringTemp, ": ");
    char number[80];
    _itoa(PlayersData[report_faction_id_].energy_reserves, number, 10); strcat(StringTemp, number);
    buffer->write_cent_l(StringTemp, field_C_, field_10_, field_14_, strlen(StringTemp));
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x3C2)); strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox2_.close();
    Dialog &fac_dialog = static_cast<Dialog &>(listBox2_);
    fac_dialog.init(&draw_workspace_rect_, 0);
    fac_dialog.field_48_ = field_1C_ + 3;
    fac_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    fac_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    fac_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    fac_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int facility = 0; facility < 70; ++facility) {
        const int count = EnergyFacilityCounts[facility];
        // Zero counts skip this row, not the remainder of the table.
        if (count == 0) continue;
        StringTemp[0] = 0;
        _itoa(count, number, 10); strcat(StringTemp, number); strcat(StringTemp, " ");
        strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[facility].name)));
        strcat(StringTemp, "|"); strcat(StringTemp, "-");
        const int project_base = SecretProject.planetary_energy_grid;
        if (project_base >= 0 && Bases[project_base].faction_id_current == report_faction_id_) {
            const int half = fac_maint(facility, report_faction_id_) * count;
            const int cost = fac_maint(facility, report_faction_id_) * count;
            _itoa(cost / 2, number, 10); strcat(StringTemp, number);
            if (half & 1) strcat(StringTemp, ".5");
        } else {
            _itoa(fac_maint(facility, report_faction_id_) * count, number, 10);
            strcat(StringTemp, number);
        }
        strcat(StringTemp, label_get(0x17B));
        listBox2_.item(StringTemp, facility);
    }
    listBox2_.reset_report_selection();
    fac_dialog.field_B4_ = 0;
    listBox2_.attach(&MainInterfaceGlobal, fac_dialog.field_24_, fac_dialog.field_28_, 0x80020);
    if (ReportWinGlobal.field_59B0_) listBox2_.set_selected_pos(-1, 1);
    else listBox2_.set_selected_id(ReportWinGlobal.shared_report_selection());
    static_cast<GraphicWin &>(listBox2_).show(0);
    scroll_normal();
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004A9050 ?draw_ops@ReportIf@@QAEXXZ 0x004A9050-0x004A9F85
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a9050.cpp", "source:src/reportif.h", "source:src/veh.h"], "reason": "Original operations-panel identity belongs to ReportIf's selection and list state; original indexed accesses select source-owned vehicle and prototype records, with the same row filtering and interface controls rather than a frame-only substitute."}
void ReportIf::draw_ops() {
    Buffer &canvas = *static_cast<Buffer *>(&MainInterfaceGlobal);
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        MainInterfaceGlobal.flat_button_[6].show(0);
        MainInterfaceGlobal.flat_button_[7].show(0);
    }
    canvas.set_clip(&field_20_);
    field_C_ = field_20_.left + 9;
    field_10_ = field_20_.top + 40;
    field_14_ = field_20_.right - field_20_.left - 18;
    field_18_ = field_20_.bottom - field_20_.top - 46;
    draw_workspace_rect_.left = field_20_.left + 6;
    draw_workspace_rect_.top = field_20_.top + 6;
    draw_workspace_rect_.right = field_20_.right - 6;
    draw_workspace_rect_.bottom = field_20_.top + 32;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    flatButton3_.show(0);
    flatButton4_.show(0);
    if (buttonGroup2_.field_84_ == 0xD0B) {
        scroll_small();
        draw_workspace_rect_.left = field_C_;
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + field_18_;
        listBox1_.close();
        Dialog &dialog = static_cast<Dialog &>(listBox1_);
        dialog.init(&draw_workspace_rect_, 0);
        dialog.field_48_ = field_1C_ + 3;
        dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
        dialog.set_dialog_text_color(0x99, -1, 1, 1);
        dialog.set_dialog_text_color2(0x99, -1, 1, 1);
        dialog.set_dialog_text_color3(0x99, -1, 1, 1);
        listBox1_.set_report_text_color(0xF2);
        const int labels[8] = {0xA0/4, 0xA4/4, 0xE4/4, 0xAAC/4,
                               0xAB0/4, 0xAB4/4, 0xFC/4, 0xAB8/4};
        // Preserve eight distinct base-computation passes and first-base
        // tie selection. Negative values still select the first own base.
        for (int row = 0; row < 8; ++row) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(labels[row]));
            if (row < 2) CharUpperA(StringTemp);
            strcat(StringTemp, ": ");
            int winner = -1;
            int best = 0;
            for (int base = 0; base < BaseCurrentCount; ++base) {
                if (static_cast<int>(Bases[base].faction_id_current) != report_faction_id_) continue;
                set_base(base);
                base_compute(0);
                int value;
                switch (row) {
                case 0: value = BaseCurrent->mineral_surplus; break;
                case 1: value = BaseCurrent->nutrient_surplus; break;
                case 2: value = BaseCurrent->economy_total; break;
                case 3: value = BaseCurrent->labs_total; break;
                case 4: value = BaseCurrent->psych_total; break;
                case 5: value = BaseCurrent->population_size; break;
                case 6: {
                    int vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);
                    value = 0;
                    if (vehicle >= 0) {
                        while (Vehs[vehicle].prev_veh_id_stack >= 0)
                            vehicle = Vehs[vehicle].prev_veh_id_stack;
                        while (vehicle >= 0) {
                            ++value;
                            vehicle = Vehs[vehicle].next_veh_id_stack;
                        }
                    }
                    break;
                }
                default: {
                    value = 0;
                    // Native4A99A4..4A99EA explicitly counts bits0..69;
                    // there is no facility-prerequisite or reputation filter.
                    for (int facility = 0; facility < 70; ++facility)
                        if (BaseCurrent->facilities_built[facility / 8] & (1 << (facility % 8))) ++value;
                    for (int project = 0; project < 64; ++project)
                        if (base_project(project) == base) value += 5;
                    break;
                }
                }
                if (value > best || winner == -1) {
                    winner = base;
                    best = value;
                }
            }
            strcat(StringTemp, "|");
            // Original calls say_base even when winner==-1. Do not silently
            // remove or guard its original no-base/error path here.
            say_base(StringTemp, winner);
            listBox1_.item(StringTemp, row + 1);
        }
        listBox1_.reset_report_selection();
        dialog.field_B4_ = 0;
        listBox1_.attach(&MainInterfaceGlobal, dialog.field_24_, dialog.field_28_, 0x80020);
        listBox1_.set_selected_id(ReportWinGlobal.shared_report_selection());
        static_cast<GraphicWin &>(listBox1_).show(0);
        scroll_normal();
        canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    } else {
        listBox1_.close();
        flatButton5_.show(0);
        flatButton6_.show(0);
        flatButton7_.show(0);
        flatButton8_.show(0);
        flatButton9_.show(0);
        flatButton10_.show(0);
        flatButton11_.show(0);
    }
    canvas.set_clip(&field_50_);
    field_10_ = field_50_.top;
    const int inset = field_1E230_ != 0 ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_14_ = field_50_.right - inset - field_50_.left;
    field_18_ = field_50_.bottom - field_50_.top;
    StringTemp[0] = 0;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    canvas.set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0xACC/4));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0xB68/4));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    canvas.write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_, strlen(StringTemp));
    canvas.set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    canvas.set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xE4/4));
    strcat(StringTemp, ": ");
    char number[80];
    _itoa(PlayersData[report_faction_id_].energy_reserves, number, 10);
    strcat(StringTemp, number);
    canvas.write_cent_l(StringTemp, field_C_, field_10_, field_14_, strlen(StringTemp));
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    canvas.box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    InflateRect(&draw_workspace_rect_, 3, 3);
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    InflateRect(&draw_workspace_rect_, -3, -3);
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0xF0C/4));
    strcat(StringTemp, "]");
    canvas.write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA0B0 ?draw_projects@ReportIf@@QAEXXZ 0x004AA0B0-0x004AA921;0x00659595-0x006595C9
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aa0b0.cpp", "source:src/reportif.h", "source:src/base.h", "source:src/stringlist.h"], "reason": "Retained spans identify ReportIf's project renderer and its cleanup code. A real local StringList owns the selected-project description; source-owned project/base records, member rectangles and controls express the original panel relationships. Empty scratch-string starts do not establish their original bounds."}
void ReportIf::draw_projects() {
    StringList description;
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        MainInterfaceGlobal.flat_button_[6].show(0);
        MainInterfaceGlobal.flat_button_[7].show(0);
    }
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;

    // The seed draws a narrow title frame followed by a second frame around
    // the selected project's description.  Both use the shared work rectangle.
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x32A));
    strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, draw_workspace_rect_.left, field_10_ + 4,
                         field_14_, static_cast<int>(strlen(StringTemp)));

    int project = ReportWinGlobal.shared_report_selection();
    if (project >= 0x400)
        project = -70 - Bases[project - 0x400].queue_production_id[0];
    if (project == -1) {
        if (ReportWinGlobal.shared_report_selection() == -1) {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x32B));
            buffer->set_text_color(0xE1, -1, 1, 1);
            buffer->wrap(StringTemp, draw_workspace_rect_.left,
                draw_workspace_rect_.top,
                draw_workspace_rect_.right - draw_workspace_rect_.left);
        }
    } else {
        // Preserve the original copies even though their destinations are
        // not subsequently consumed by this panel.
        char copied_small[1024];
        char copied_large[256];
        strcpy(copied_small, ReportProjectScratchSmall);
        strcpy(copied_large, ReportProjectScratchLarge);
        char number[80];
        StringTemp[0] = 0;
        strcat(StringTemp, "HELPPROJ");
        _itoa(project, number, 10);
        strcat(StringTemp, number);
        if (description.load(DatalinkHelpFile, StringTemp, 1, 0) == 0) {
            const int width = draw_workspace_rect_.right - draw_workspace_rect_.left;
            if (draw_workspace_rect_.bottom - draw_workspace_rect_.top
                < buffer->write_strings_height(&description, width, 0)) {
                Font *italic = find_font(10, 1);
                Font *bold = find_font(10, 2);
                Font *normal = find_font(10, 0);
                buffer->set_font(normal, bold, italic, 0);
            }
            buffer->set_text_color(0xE1, -1, 1, 1);
            buffer->write_strings(&description, draw_workspace_rect_.left,
                draw_workspace_rect_.top, width, 0);
        }
    }

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    field_10_ = field_50_.top;
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B4));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());

    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_ - 3;
    draw_workspace_rect_.top = field_10_ - 3;
    draw_workspace_rect_.right = field_C_ + field_14_ + 3;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 9;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3C3));
    strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA970 ?draw_sat@ReportIf@@QAEXXZ 0x004AA970-0x004AB54B
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aa970.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "Retained original identity maps to ReportIf's satellite panel, including its member rectangles and ReportWin help-selection control. Original scratch addresses are empty-string source starts, not writable destination arrays with inferred bounds."}
void ReportIf::draw_sat() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    const bool help = ReportWinGlobal.buttonGroup2_.field_84_ == 0xD3C;
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + (help ? field_18_ : field_1C_ + 6);
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    if (help) {
        field_C_ += 4; field_10_ += 4; field_14_ -= 8;
        for (int faction = 1; faction < 8; ++faction)
            if (faction != ConsoleGlobal.faction_id_) flatButtonsC_[faction-1].hide();
        char paragraph[1024], line[256];
        strcpy(paragraph, ReportSatelliteScratchUpperText);
        strcpy(line, ReportSatelliteScratchUpperLine);
        StringTemp[0] = 0; strcat(StringTemp, "SAT0");
        if (X_text_open(reinterpret_cast<char *>(*ClimatePopupTitle), StringTemp)) {
            text_close();
        } else {
            buffer->set_font(field_1E184_, 0, 0, 0);
            buffer->wrap_flying(paragraph, field_C_, field_10_, field_14_);
            buffer->set_text_color(0xE1, -1, 1, 1);
            for (;;) {
                char *text = text_get();
                if (!text) { text_close(); break; }
                if (!*text) {
                    if (strlen(paragraph)) buffer->wrap_flying(paragraph);
                    buffer->wrap_flying_flush(); text_close(); break;
                }
                line[0] = 0; parse_string(text, line);
                strcat(paragraph, line); strcat(paragraph, " ");
            }
        }
    } else {
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.left += 3; draw_workspace_rect_.top += 3;
        draw_workspace_rect_.right -= 3; draw_workspace_rect_.bottom -= 3;
        buffer->set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0; strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C6)); strcat(StringTemp, "]");
        buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
        for (int other = 1; other < 8; ++other) {
            if (other == ConsoleGlobal.faction_id_) continue;
            int elevators = 0;
            for (int base = 0; base < BaseCurrentCount; ++base)
                if (Bases[base].faction_id_current == other && has_fac(41, base, 0)) ++elevators;
            const PlayerData &data = PlayersData[other];
            if (data.satellites_[0] + data.satellites_[1] + data.satellites_[2]
                + data.satellites_[3] + elevators)
                flatButtonsC_[other-1].show(0);
            else flatButtonsC_[other-1].hide();
        }
    }
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int indent = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + indent;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - indent;
    field_18_ = field_50_.bottom - field_50_.top;
    StringTemp[0] = 0;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp); strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B5));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " "); say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_, strlen(StringTemp));
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6; field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + (help ? field_18_ : field_1C_ + 6);
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    if (help) {
        field_C_ += 4; field_10_ += 4; field_14_ -= 8;
        for (int button = 0; button < 5; ++button) flatButtonsD_[button].hide();
        char paragraph[1024], line[256];
        strcpy(paragraph, ReportSatelliteScratchLowerText);
        strcpy(line, ReportSatelliteScratchLowerLine);
        StringTemp[0] = 0; strcat(StringTemp, "SAT1");
        if (X_text_open(reinterpret_cast<char *>(*ClimatePopupTitle), StringTemp)) {
            text_close();
        } else {
            buffer->set_font(field_1E184_, 0, 0, 0);
            buffer->wrap_flying(paragraph, field_C_, field_10_, field_14_);
            buffer->set_text_color(0xE1, -1, 1, 1);
            for (;;) {
                char *text = text_get();
                if (!text) { text_close(); break; }
                if (!*text) {
                    if (strlen(paragraph)) buffer->wrap_flying(paragraph);
                    buffer->wrap_flying_flush(); text_close(); break;
                }
                line[0] = 0; parse_string(text, line);
                strcat(paragraph, line); strcat(paragraph, " ");
            }
        }
    } else {
        draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
        // The original lower panel deliberately uses the upper panel's bottom.
        draw_workspace_rect_.bottom = field_20_.bottom - 6;
        buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
        draw_workspace_rect_.left += 3; draw_workspace_rect_.top += 3;
        draw_workspace_rect_.right -= 3; draw_workspace_rect_.bottom -= 3;
        buffer->set_font(field_1E184_, field_1E194_, field_1E198_, 0);
        StringTemp[0] = 0; strcat(StringTemp, "[");
        strcat(StringTemp, label_get(0x3C7)); strcat(StringTemp, "]");
        buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
        field_C_ = field_30_.left + 9; field_10_ = field_30_.top + 32;
        field_14_ = field_30_.right - field_30_.left - 18;
        int target = buttonGroup3_.field_84_;
        if (target < 1 || target >= 8) target = -1;
        if (target != -1) {
            field_1E234_ = 1;
            int selected = buttonGroup4_.field_84_;
            if (selected < 65 || selected >= 70) selected = -1;
            buttonGroup4_.init(0xCE, 0);
            int elevators = 0;
            for (int kind = 0; kind < 5; ++kind) {
                if (kind == 4 && !ExpansionEnabled) { flatButtonsD_[4].hide(); break; }
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(reinterpret_cast<int>(Facility[kind == 4 ? 41 : 65+kind].name)));
                draw_workspace_rect_.left = field_C_; draw_workspace_rect_.top = field_10_;
                draw_workspace_rect_.right = field_C_ + field_14_;
                draw_workspace_rect_.bottom = field_10_ + 20;
                FlatButton &button = flatButtonsD_[kind];
                button.init(StringTemp, kind + 65, draw_workspace_rect_.left,
                    draw_workspace_rect_.top, field_14_, 20, &MainInterfaceGlobal, 0);
                static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
                button.redraw(); button.hide();
                elevators = 0;
                for (int base = 0; base < BaseCurrentCount; ++base)
                    if (Bases[base].faction_id_current == target && has_fac(41, base, 0)) ++elevators;
                if (kind == 4 ? elevators != 0 : PlayersData[target].satellites_[kind] != 0) {
                    button.show(0); buttonGroup4_.add(&button); field_10_ += 22;
                }
            }
            if (!ExpansionEnabled) {
                // Original skips selection restoration entirely in this mode.
            } else if (selected != -1 && (selected - 65 == 4 ? elevators != 0
                : selected - 65 < 4 && PlayersData[target].satellites_[selected - 65] != 0)) {
                buttonGroup4_.set(selected, 1);
            } else {
                ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
                ReportWinGlobal.flatButton9_.Win::hide();
            }
            field_1E234_ = 0;
        }
    }
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004AB600 ?draw_security@ReportIf@@QAEXXZ 0x004AB600-0x004ABE24
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004ab600.cpp", "source:src/reportif.h", "source:src/spot.h"], "reason": "Retained identity and indexed accesses identify ReportIf's five-category security panel. Its source-owned lists and Spot rows express the original filters, count caps and lost totals; the final frame uses the first panel's bottom edge as in the original, not a geometrically normalized replacement."}
void ReportIf::draw_security() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x38A));
    strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));

    buffer->set_font(field_1E18C_, 0, 0, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ = draw_workspace_rect_.top + 3;
    field_C_ += 4;
    field_14_ -= 8;
    for (int filter = 0; filter < 5; ++filter) {
        int active = 0, queued = 0, lost = 0;
        buffer->set_text_color(filter == field_1E244_ ? 0xF2 : 0x99, -1, 1, 1);
        for (int row = 0; row < 128; ++row) {
            const int prototype_id = row < 64 ? row : report_faction_id_ * 64 - 64 + row;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            if (!(prototype.flags & 1)) continue;
            const int offense = Weapon[prototype.weapon_id].offense_rating;
            if (!(filter == 4 || (filter == 3 && offense == 0)
                || (filter < 3 && Chassis[prototype.chassis_id].triad == filter && offense != 0)))
                continue;
            const PlayerData &faction = PlayersData[report_faction_id_];
            if (faction.proto_id_active[prototype_id] < 250)
                active += faction.proto_id_active[prototype_id];
            if (faction.proto_id_queue[prototype_id] < 250)
                queued += faction.proto_id_queue[prototype_id];
            lost += faction.proto_id_lost[prototype_id];
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x38B + filter));
        buffer->write_l(StringTemp, field_C_, field_10_, strlen(StringTemp));
        char number[80];
        StringTemp[0] = 0;
        _itoa(active, number, 10); strcat(StringTemp, number); strcat(StringTemp, "-");
        _itoa(queued, number, 10); strcat(StringTemp, number); strcat(StringTemp, "-");
        _itoa(lost, number, 10); strcat(StringTemp, number);
        buffer->write_right_l(StringTemp, field_C_ + field_14_, field_10_, strlen(StringTemp));
        draw_workspace_rect_.left = field_C_;
        draw_workspace_rect_.top = field_10_;
        draw_workspace_rect_.right = field_C_ + field_14_;
        draw_workspace_rect_.bottom = field_10_ + field_1C_;
        spot_.add(filter, 0, draw_workspace_rect_.left, draw_workspace_rect_.top,
            draw_workspace_rect_.right - draw_workspace_rect_.left,
            draw_workspace_rect_.bottom - draw_workspace_rect_.top);
        if (filter == 3) {
            field_10_ += field_1C_ - 1;
            buffer->line(field_C_ - 1, field_10_, field_C_ + field_14_, field_10_, 0x99);
            field_10_ += 5;
        } else field_10_ += field_1C_;
    }

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B6));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_font(field_1E180_, field_1E180_, field_1E180_, 0);
    buffer->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    // Original intentionally uses the FIRST panel's bottom for this frame.
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, "[");
    strcat(StringTemp, label_get(0x3BB));
    strcat(StringTemp, "]");
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    flatButton12_.show(0);
    flatButton14_.show(0);
    flatButton13_.show(0);
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004AC140 ?draw_intel@ReportIf@@QAEXXZ 0x004AC140-0x004AC977
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004ac140.cpp", "source:src/reportif.h", "source:src/faction.h", "source:src/tech.h"], "reason": "Retained identity identifies ReportIf's intelligence panel. Original faction-indexed treaty accesses and technology iteration map to source-owned player data and technology records; the informed-view conditions select the same information rather than treating the panel as a link-only placeholder."}
void ReportIf::draw_intel() {
    Buffer &canvas = static_cast<Buffer &>(MainInterfaceGlobal);
    const int target = report_selection_;
    const bool informed = (GameState & STATE_OMNISCIENT_VIEW)
        || spying(target) != 0
        || (PlayersData[report_faction_id_].diplo_treaties[target] & 1);
    canvas.set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x175)); strcat(StringTemp, "]");
    canvas.write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox1_.close();
    Dialog &tech_dialog = static_cast<Dialog &>(listBox1_);
    tech_dialog.init(&draw_workspace_rect_, 0);
    tech_dialog.field_48_ = field_1C_ + 3;
    tech_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    tech_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    tech_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    tech_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox1_.set_report_text_color(0xF2);
    if (informed) {
        for (int tech = 0; tech < 89; ++tech) {
            if (has_tech(tech, target)) {
                // The clear precedes say_tech at4AC3E1, despite seed ordering.
                StringTemp[0] = 0;
                say_tech(StringTemp, tech, 0);
                if (!has_tech(tech, report_faction_id_)) strcat(StringTemp, " (*)");
                listBox1_.item(StringTemp, tech);
            }
        }
    }
    listBox1_.reset_report_selection();
    tech_dialog.field_B4_ = 0;
    listBox1_.attach(&MainInterfaceGlobal, tech_dialog.field_24_, tech_dialog.field_28_, 0x80020);
    static_cast<GraphicWin &>(listBox1_).show(0);
    scroll_normal();
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_50_);
    const int inset = field_1E230_ ? 76 : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    canvas.set_font(field_1E190_, field_1E194_, field_1E198_, 0);
    canvas.set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " "); strcat(StringTemp, label_get(0x2B8));
    if (field_1E230_) {
        strcat(StringTemp, ", "); strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA)); strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    canvas.write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_, strlen(StringTemp));
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
    canvas.set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    StringTemp[0] = 0;
    strcat(StringTemp, "["); strcat(StringTemp, label_get(0x176)); strcat(StringTemp, "]");
    canvas.write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_, strlen(StringTemp));
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    canvas.box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3; draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3; draw_workspace_rect_.bottom -= 3;
    scroll_small();
    listBox2_.close();
    Dialog &base_dialog = static_cast<Dialog &>(listBox2_);
    base_dialog.init(&draw_workspace_rect_, 0);
    base_dialog.field_48_ = field_1C_ + 3;
    base_dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    base_dialog.set_dialog_text_color(0x99, -1, 1, 1);
    base_dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    base_dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    listBox2_.set_report_text_color(0xF2);
    for (int base = 0; base < BaseCurrentCount; ++base) {
        const Base &record = Bases[base];
        if (record.faction_id_current == target && (informed
            || record.faction_id_current == report_faction_id_
            || (record.visibility & (1 << (report_faction_id_ & 31))))) {
            char number[96];
            StringTemp[0] = 0;
            say_base(StringTemp, base);
            strcat(StringTemp, "|");
            // Original movzx intentionally treats current population as unsigned.
            _itoa(informed ? static_cast<unsigned char>(record.population_size)
                : record.faction_pop_size_intel[report_faction_id_], number, 10);
            strcat(StringTemp, number);
            listBox2_.item(StringTemp, base);
        }
    }
    listBox2_.reset_report_selection();
    base_dialog.field_B4_ = 0;
    listBox2_.attach(&MainInterfaceGlobal, base_dialog.field_24_, base_dialog.field_28_, 0x80020);
    static_cast<GraphicWin &>(listBox2_).show(0);
    scroll_normal();
    canvas.set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// Original interface table 0x0066D700 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_scrolled(int, int) {}
// Original interface table 0x0066D700 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_scrolling(int, int) {}
// Original interface table 0x0066D700 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@ReportIf@@UAEXHH@Z 0x004A41D0
void ReportIf::on_iface_left_up(int, int) {}

// Original interface table 0x0066D700 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@ReportIf@@UAEHH@Z 0x004A41D0
int ReportIf::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066D700 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@ReportIf@@UAEHPAVGraphicWin@@@Z 0x004A41D0
int ReportIf::on_iface_dialog_back_draw(::GraphicWin *) {
    return 0;
}
