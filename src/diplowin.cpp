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
#include "diplowin.h"
#include "vtable_shim.h"
#include "netdaemon.h"
#include "basepop.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "technology.h"
#include "alpha.h"
#include "maininterface.h"

void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);

// Native redraw passes these distinct addresses as C strings. Both initially
// contain NUL. Only their first byte is established; producer and full string
// extents remain unresolved, so these are provisional minimum storage owners.
// RECOVERY_OBJECT: v2 0x007492D4 0x007492D5 ?DiploLocalizedTradeTitle@@3DA 0x004436C0
char DiploLocalizedTradeTitle;
// RECOVERY_OBJECT: v2 0x007492D8 0x007492D9 ?DiploLocalizedDiplomacyTitle@@3DA 0x004436C0
char DiploLocalizedDiplomacyTitle;

// Native 0x444FC0 constructs both bases, Spot, thirteen FlatButtons, StringBox,
// ListBox and EditBox in declaration order. No scalar initialization follows.
// ORIGINAL: 0x00444FC0 ??0DiploWin@@QAE@XZ 0x00444FC0-0x0044507B;0x006549F0-0x00654A47 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
DiploWin::DiploWin() {}

// ORIGINAL: 0x004436C0 ?on_redraw@DiploWin@@QAEXXZ 0x004436C0-0x00443C25
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
// symbol ?on_redraw@DiploWin@@UAEXXZ
void DiploWin::on_redraw() {
  MainInterfaceGlobal.restore_back(1);

  Buffer &interface_buffer = static_cast<Buffer &>(MainInterfaceGlobal);
  RECT area = MainInterfaceGlobal.right_frame_rect();
  area.left += 4;
  area.top += 4;
  area.right -= 4;
  area.bottom -= 4;
  interface_buffer.box_sprite(&area, &InterfaceBoxStyles[3]);

  area = MainInterfaceGlobal.main_frame_rect();
  area.left += 4;
  area.top += 4;
  area.right -= 4;
  area.bottom -= 4;
  interface_buffer.box_sprite(&area, &InterfaceBoxStyles[3]);

  UnionRect(&area, &portrait_rect_[0], &portrait_rect_[1]);
  UnionRect(&area, &area, &rect_F4C_);
  area.left -= 3;
  area.right += 3;
  area.bottom += 3;
  area.top -= 3;

  Buffer &canvas = static_cast<Buffer &>(*this);
  canvas.box_sprite(&area, &InterfaceBoxStyles[3]);

  UnionRect(&area, &rect_FFC_, &rect_100C_);
  UnionRect(&area, &area, &rect_101C_);
  area.right += 3;
  area.left -= 3;
  area.bottom += 3;
  area.top -= 3;
  canvas.box_sprite(&area, &InterfaceBoxStyles[3]);

  area = rect_F9C_;
  canvas.box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.top += 3;
  area.bottom -= 3;
  canvas.set_font(find_font(14, 1), 0, 0, 0);
  canvas.set_text_color(0x99, -1, 1, 1);

  LPSTR title = Language == 0 ? const_cast<char *>("TRADE")
                              : &DiploLocalizedTradeTitle;
  if (title != 0) {
    canvas.write_cent_l(title, &area, static_cast<int>(strlen(title)));
  }

  area = rect_FAC_;
  canvas.box_sprite(&area, &InterfaceBoxStyles[3]);

  area = rect_FBC_;
  canvas.box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.top += 3;
  area.bottom -= 3;
  title = Language == 0 ? const_cast<char *>("DIPLOMACY")
                        : &DiploLocalizedDiplomacyTitle;
  if (title != 0) {
    canvas.write_cent_l(title, &area, static_cast<int>(strlen(title)));
  }

  area = rect_FCC_;
  canvas.box_sprite(&area, &InterfaceBoxStyles[3]);

  area.top = rect_F4C_.top;
  area.left = rect_F4C_.left;
  area.right = rect_F4C_.right;
  area.bottom = rect_F4C_.bottom;
  canvas.box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.bottom -= 3;
  area.top += 3;

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0x748 / 4));
  strcat(StringTemp, " ");
  strcat(StringTemp, " ");
  strcat(StringTemp, "(");
  strcat(StringTemp, label_get(0x788 / 4));
  strcat(StringTemp, ": ");
  strcat(StringTemp, "{");

  const uint32_t treaties =
      PlayersData[faction_id_].diplo_treaties[faction_id_with_];
  int relation_label;
  int color_code;
  if ((treaties & 1) != 0) {
    relation_label = 0x34C / 4;
    color_code = 0xFA;
  } else if ((treaties & 2) != 0) {
    relation_label = 0x350 / 4;
    color_code = 0xFA;
  } else if ((treaties & 4) != 0) {
    relation_label = 0x354 / 4;
    color_code = 0xFA;
  } else if ((treaties & 0x10) != 0) {
    relation_label = 0x358 / 4;
    color_code = 0xF9;
  } else {
    relation_label = 0x784 / 4;
    color_code = 0xFB;
  }
  strcat(StringTemp, label_get(relation_label));
  canvas.set_text_color2(color_code, -1, 1, 1);
  strcat(StringTemp, "}");
  strcat(StringTemp, ")");
  canvas.write_cent_l(StringTemp, &area, static_cast<int>(strlen(StringTemp)));

  if (MainInterfaceTileFlag != 0) {
    interface_buffer.box_sprite(&rect_F3C_, &InterfaceBoxStyles[0]);
    interface_buffer.box_sprite(&rect_F2C_, &InterfaceBoxStyles[0]);
  }

  draw_who();
  draw_whom();
  draw_offers();
}

extern int TextColourTable[195];

// ORIGINAL: 0x00443E50 ?on_iface_dialog_item_back_draw@DiploWin@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x00443E50-0x00443F0C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
// symbol ?on_iface_dialog_item_back_draw@DiploWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
int DiploWin::on_iface_dialog_item_back_draw(GraphicWin *window, int, int item,
                                             RECT *rect) {
  if (window == 0 || rect == 0) {
    return 0;
  }
  Buffer &canvas = static_cast<Buffer &>(*window);
  canvas.restore_clip();
  RECT local = *rect;

  // These words belong to the text.pcx palette published by textcolor_init.
  // The native addresses are TextColourTable[80]/[81], walked backward as
  // two-int pairs for each item.
  canvas.set_text_color(TextColourTable[80 - 2 * item],
                        TextColourTable[81 - 2 * item], 1, 1);
  if (item == 0) {
    window->fill(rect, 0);
    return 1;
  }
  canvas.box_sprite(&local, &InterfaceBoxStyles[27 + item]);
  return 1;
}

// ORIGINAL: 0x00443C70 ?on_status@DiploWin@@QAEXH@Z 0x00443C70-0x00443D77
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
// symbol ?on_status@DiploWin@@UAEXH@Z
void DiploWin::on_status(int mode) {
  GraphicWin *const front = static_cast<GraphicWin *>(this);
  if (mode != 0) {
    front->on_redraw();
    for (int i = 0; i != 13; ++i) {
      flatButtons_[i].show(0);
    }
    slide_show(front, 2);
    front->Win::show(3);
    stringBox_.show(0);
    editBox_.show(0);
    editBox_.time_.start();
    front->redraw();
    MainInterfaceGlobal.soft_update();
    return;
  }

  stringBox_.hide();
  editBox_.time_.stop();
  editBox_.hide();
  slide_hide(front, 2);
  front->Win::hide();
  for (int i = 0; i != 13; ++i) {
    flatButtons_[i].hide();
  }
}

// SubInterface vtable slots with exact native shared-stub targets. These are
// declarations/bodies to add only after the secondary-base layout is adopted.
// Native secondary table66B5E0 selects these shared return bodies at slots
// 1,2,14,16,17. They do not create additional original identities or claims.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@DiploWin@@UAEXHH@Z 0x00444FC0
void DiploWin::on_iface_scrolled(int, int) {}
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@DiploWin@@UAEXHH@Z 0x00444FC0
void DiploWin::on_iface_scrolling(int, int) {}
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@DiploWin@@UAEHH@Z 0x00444FC0
int DiploWin::on_iface_button_pressed(int) { return 0; }
// RECOVERY_SHARED: v1 0x00406B20 0x00406B23 ?on_iface_group_clicked@DiploWin@@UAEXHHH@Z 0x00444FC0
void DiploWin::on_iface_group_clicked(int, int, int) {}
// RECOVERY_SHARED: v1 0x00443E40 0x00443E45 ?on_iface_dialog_back_draw@DiploWin@@UAEHPAVGraphicWin@@@Z 0x00444FC0
int DiploWin::on_iface_dialog_back_draw(GraphicWin *) { return 0; }

// The diplomacy window's redraw latch is writable application storage, not a
// pointer into the original executable. on_key_click and the offer paths write it.
// RECOVERY_OBJECT: v2 0x007492D0 0x007492D4 ?DiploOffersDirty@@3HA 0x00442590
int DiploOffersDirty;

// Original initialized .data and literal evidence, inspected from the pinned image.
// RECOVERY_OBJECT: v2 0x0068A5C4 0x0068A5C8 ?DiploKnownTechColor@@3HA 0x00443170
int DiploKnownTechColor = 50;
// RECOVERY_OBJECT: v2 0x006839F4 0x006839F6 ?DiploKnownTechOpen@@3PADA 0x00443170
char DiploKnownTechOpen[2] = "(";
// RECOVERY_OBJECT: v2 0x006839F8 0x006839FA ?DiploKnownTechClose@@3PADA 0x00443170
char DiploKnownTechClose[2] = ")";

// ORIGINAL: 0x00443170 ?draw_offers@DiploWin@@QAEXXZ 0x00443170-0x0044369F
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00443170.cpp"], "reason": "Typed offer-panel rendering, technology/energy/treaty/faction labels and Spot ownership, replacing seed raw offsets."}
void DiploWin::draw_offers() {
    Buffer *surface = this;
    int line_height = find_font(14, 0)->line_advance();
    int side = 0;
    do {
        spot_.kill_type(side);
        surface->set_font(find_font(14, 0), find_font(14, 1), find_font(14, 2), 0);
        RECT panel = portrait_rect_[side];
        int faction = side == 0 ? faction_id_ : faction_id_with_;
        int left = side == 0 ? panel.left + 1 + FactionsArt[faction].datalink.width() : panel.left;
        int right = side == 0 ? panel.right : panel.right - 1 - FactionsArt[faction].datalink.width();
        left += 3;
        int width = right - 3 - left;
        surface->set_text_color(ChatItemIdTable[faction], ChatShadowColors[faction], 1, 1);
        StringTemp[0] = 0;
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        strcat(StringTemp, Players[faction].title_leader);
        strcat(StringTemp, " ");
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        strcat(StringTemp, Players[faction].name_leader);
        strcat(StringTemp, " ");
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(offer_count_[side] == 0 ? 0x734 / 4 : 0x738 / 4));
        surface->write_cent_l(StringTemp, left, panel.top + 3, width, strlen(StringTemp));
        int top = panel.top + 3 + line_height;
        int i = 0;
        while (i < offer_count_[side]) {
            StringTemp[0] = 0;
            faction = side == 0 ? faction_id_ : faction_id_with_;
            surface->set_text_color(ChatItemIdTable[faction], ChatShadowColors[faction], 1, 1);
            int value = offer_value_[side][i];
            int label = -1;
            char text[80];
            switch (offer_type_[side][i]) {
            case 0:
                if (side == 1 && offer_value_[1][i] < 89 && has_tech(offer_value_[1][i], faction_id_)) {
                    surface->set_text_color(DiploKnownTechColor, 0, side, side);
                    strcat(StringTemp, DiploKnownTechOpen);
                    say_tech(text, offer_value_[1][i], side);
                    strcat(StringTemp, text);
                    strcat(StringTemp, DiploKnownTechClose);
                } else {
                    say_tech(text, value, side);
                    strcat(StringTemp, text);
                }
                break;
            case 1:
                _itoa(value, text, 10);
                strcat(StringTemp, text);
                strcat(StringTemp, " ");
                label = 0x90 / 4;
                break;
            case 2: label = 0x34C / 4; break;
            case 3: label = 0x350 / 4; break;
            case 4: label = 0x354 / 4; break;
            case 5:
                strcat(StringTemp, label_get(0x980 / 4));
                strcat(StringTemp, " ");
                GenderDefault = Players[value].noun_gender;
                PluralityDefault = Players[value].is_noun_plural;
                strcat(StringTemp, Players[value].noun_faction);
                break;
            }
            if (label >= 0) strcat(StringTemp, label_get(label));
            surface->write_cent_l(StringTemp, left, top, width, strlen(StringTemp));
            spot_.add(i, side, left, top, width, line_height);
            top += line_height;
            ++i;
        }
        ++side;
    } while (side < 2);
}

// Source-owned faction/network operations; shared declarations await owner review.
int __cdecl diplo_lock(int timeout);
void __cdecl diplo_unlock();

// ORIGINAL: 0x004426F0 ?check_done@DiploWin@@QAEXXZ 0x004426F0-0x00442736 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/004426f0.cpp"], "reason": "Rehome seed into typed DiploWin owner; states and counterpart are source members."}
void DiploWin::check_done() {
    if (field_A24_ == 1 && field_A28_ == 1) {
        message_data(0x150A, faction_id_with_, 0, 0, 0, 0);
        if (IsMultiplayerPBEM != 0) {
            do_it();
        }
    }
}

// ORIGINAL: 0x00442740 ?do_it@DiploWin@@QAEXXZ 0x00442740-0x0044299D
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00442740.cpp"], "reason": "Rehome exchange execution into source-owned DiploWin arrays and faction/network globals."}
void DiploWin::do_it() {
    if ((faction_id_ <= faction_id_with_ || IsMultiplayerPBEM != 0)
        && CommlinkPending < 2) {
        CommlinkPending = 2;
        if (diplo_lock(200) == 0) {
            int side = 0;
            do {
                int i = 0;
                while (i < offer_count_[side]) {
                    int giver = side == 0 ? faction_id_ : faction_id_with_;
                    int recipient = side == 0 ? faction_id_with_ : faction_id_;
                    int value = offer_value_[side][i];
                    switch (offer_type_[side][i]) {
                    case 0:
                        if (value > 88 || !has_tech(value, recipient)) {
                            net_tech(recipient, value, giver, 1);
                        }
                        break;
                    case 1: {
                        int reserves = PlayersData[giver].energy_reserves;
                        int amount = value;
                        if (value < 0 || reserves < 0) {
                            amount = 0;
                        } else if (reserves < value) {
                            amount = reserves;
                        }
                        net_energy(giver, -amount, recipient, amount, 1);
                        break;
                    }
                    case 2: net_treaty_on(giver, recipient, 1, 1); break;
                    case 3: net_treaty_on(giver, recipient, 2, 1); break;
                    case 4: net_treaty_on(giver, recipient, 4, 1); break;
                    case 5:
                        if (((1 << (value & 31)) & static_cast<unsigned char>(LockEnableMask)) != 0
                            && !(PlayersData[giver].diplo_treaties[value] & 0x10)) {
                            message_data(0x150F, value, giver, recipient, 0, 0);
                        }
                        net_treaty_on(giver, value, 8, 0);
                        net_double_cross(giver, value, recipient, 1);
                        break;
                    }
                    ++i;
                }
                ++side;
            } while (side < 2);
            diplo_unlock();
            CommlinkPending = 1;
            field_A28_ = 3;
            field_A24_ = 3;
            offer_count_[1] = 0;
            offer_count_[0] = 0;
            message_data(0x150D, faction_id_with_, 0, 0, 0, 0);
            redraw();
        }
    }
}


// ORIGINAL: 0x00442A10 ?draw_who@DiploWin@@QAEXXZ 0x00442A10-0x00442D98
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00442a10.cpp"], "reason": "Typed DiploWin party rendering using source-owned FactionArt datalink portraits, rectangles and label storage."}
void DiploWin::draw_who() {
    if (faction_id_ <= 0) return;
    int faction = faction_id_;
    Buffer *surface = this;
    surface->set_font(find_font(14, 1), 0, 0, 0);
    StringTemp[0] = 0;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].title_leader);
    strcat(StringTemp, " ");
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].name_leader);
    surface->set_text_color(ChatItemIdTable[faction], ChatShadowColors[faction], 1, 1);
    RECT area = leader_rect_[0];
    surface->box_sprite(&area, &InterfaceBoxStyles[6]);
    area.left += 3; area.top += 3; area.right -= 3; area.bottom -= 3;
    string_upper_slot()(StringTemp);
    surface->write_cent_l(StringTemp, &area, strlen(StringTemp));
    area = portrait_rect_[0];
    surface->box_sprite(&area, &InterfaceBoxStyles[0]);
    Sprite *portrait = &FactionsArt[faction].datalink;
    area.top += 1;
    area.left = area.left + 1;
    area.right = area.left + portrait->width();
    area.bottom = area.top + portrait->height();
    portrait->draw(surface, portrait->transparent_index(), area.left, area.top, 1, 1);
    area.left += 4; area.bottom -= 4; area.right -= 4;
    area.top = area.bottom - 20;
    surface->set_font(find_font(14, 1), 0, 0, 0);
    StringTemp[0] = 0;
    int label = -1;
    switch (field_A24_) {
    case 0: label = 0x73C / 4; break;
    case 1: label = 0x740 / 4; break;
    case 2: label = 0x744 / 4; break;
    case 3: label = 0x780 / 4; break;
    case 4: label = 0x968 / 4; break;
    case 5: label = 0x95C / 4; break;
    case 6: label = 0x960 / 4; break;
    }
    int state = field_A24_;
    if (state == 2 || state == 4 || state == 5)
        surface->set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
    if (state == 6) surface->set_text_color(251, 0, 1, 1);
    if (label >= 0) strcat(StringTemp, label_get(label));
    string_upper_slot()(StringTemp);
    surface->map_colors(&area, ShadowColorTable);
    area.top += 3; area.left += 3; area.right -= 3; area.bottom -= 3;
    surface->write_cent_l(StringTemp, &area, strlen(StringTemp));
}

// ORIGINAL: 0x00442DC0 ?draw_whom@DiploWin@@QAEXXZ 0x00442DC0-0x00443149
// RECOVERY_BINDING: v1 {"evidence": ["git:3f6ab06bdf3b791aa23454e19f71b4e8b5935caf:src/recovered/units/00442dc0.cpp"], "reason": "Typed DiploWin party rendering using source-owned FactionArt datalink portraits, rectangles and label storage."}
void DiploWin::draw_whom() {
    if (faction_id_ <= 0) return;
    int faction = faction_id_with_;
    Buffer *surface = this;
    surface->set_font(find_font(14, 1), 0, 0, 0);
    StringTemp[0] = 0;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].title_leader);
    strcat(StringTemp, " ");
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].name_leader);
    surface->set_text_color(ChatItemIdTable[faction], ChatShadowColors[faction], 1, 1);
    RECT area = leader_rect_[1];
    surface->box_sprite(&area, &InterfaceBoxStyles[6]);
    area.left += 3; area.top += 3; area.right -= 3; area.bottom -= 3;
    string_upper_slot()(StringTemp);
    surface->write_cent_l(StringTemp, &area, strlen(StringTemp));
    area = portrait_rect_[1];
    surface->box_sprite(&area, &InterfaceBoxStyles[0]);
    Sprite *portrait = &FactionsArt[faction].datalink;
    area.top += 1;
    area.left = area.right - portrait->width() - 1;
    area.right = area.left + portrait->width();
    area.bottom = area.top + portrait->height();
    portrait->draw(surface, portrait->transparent_index(), area.left, area.top, 1, 1);
    area.left += 4; area.bottom -= 4; area.right -= 4;
    area.top = area.bottom - 20;
    surface->set_font(find_font(14, 1), 0, 0, 0);
    StringTemp[0] = 0;
    int label = -1;
    switch (field_A28_) {
    case 0: label = 0x73C / 4; break;
    case 1: label = 0x740 / 4; break;
    case 2: label = 0x744 / 4; break;
    case 3: label = 0x780 / 4; break;
    case 4: label = 0x968 / 4; break;
    case 5: label = 0x95C / 4; break;
    case 6: label = 0x960 / 4; break;
    }
    int state = field_A28_;
    if (state == 2 || state == 4 || state == 5)
        surface->set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
    if (state == 6) surface->set_text_color(251, 0, 1, 1);
    if (label >= 0) strcat(StringTemp, label_get(label));
    string_upper_slot()(StringTemp);
    surface->map_colors(&area, ShadowColorTable);
    area.top += 3; area.left += 3; area.right -= 3; area.bottom -= 3;
    surface->write_cent_l(StringTemp, &area, strlen(StringTemp));
}

// ORIGINAL: 0x004412D0 ?UNK1@DiploWin@@QAEXXZ 0x004412D0-0x004412ED BYTE_EXACT
void DiploWin::UNK1() {
    stringBox_.clear();
    editBox_.set_text(0);
}

// ORIGINAL: 0x004412F0 ?new_chat@DiploWin@@QAEXPADH@Z 0x004412F0-0x004413A7
// Historical seed evidence: the faction record is indexed by its 0x59C stride;
// the seed's remaining register-order difference was not a structural clue.
void DiploWin::new_chat(char *text, int faction_id) {
    char line[256];
    line[0] = 0;
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    strcat(line, Players[faction_id].name_leader);
    string_upper_slot()(line);
    strcat(line, ": ");
    strcat(line, " ");
    strcat(line, text);
    stringBox_.add(line, faction_id, 1);
    DiploOffersDirty = 1;
}

// Slot 62, 0xF8 - the DiploWin front vtable's update entry, the one
// done_deal tails into with the image's own `jmp dword ptr [eax + 0xf8]`.
namespace {
typedef void (__fastcall *func_diplo_win_slot062)(void *);
}

// ORIGINAL: 0x00442560 ?on_key_down@DiploWin@@QAEHH@Z 0x00442560-0x0044258A BYTE_EXACT
int DiploWin::on_key_down(int key) {
    if (key == 0x0D || key == 0x1B) {
        return 0;
    }
    return editBox_.vslot_26(key);
}

// ORIGINAL: 0x004429C0 ?done_deal@DiploWin@@QAEXXZ 0x004429C0-0x004429E7 BYTE_EXACT
void DiploWin::done_deal() {
    field_A28_ = 3;
    field_A24_ = 3;
    offer_count_[1] = 0;
    offer_count_[0] = 0;
    redraw();
}

// Historical TRIED entries below describe the scaffolded seed, not this typed member body.
// ORIGINAL: 0x00442590 ?on_key_click@DiploWin@@QAEHHH@Z 0x00442590-0x004426ED
// symbol    ?on_key_click@DiploWin@@UAEHHH@Z
// TRIED: Seed provenance: 0.74 mnemonic similarity - the edit box's slot-25 vtable call needed a shim in the scaffold; the tree's own Win::vslot_25 declares the same two-int signature, and Win slot 58 is release_modal, so both dispatch through the real declarations here. The private DiploWin state stays raw-offset (0xAB8, the 0xE490 edit text, stringBox_, editBox_).
// TRIED: DAT_00946a50/DAT_00946a84 are per-faction record fields (stride 1436 = 0x59C) whose address itself is indexed - declared as extern arrays instead of the auto-scaffold's plain const-pointer globals (which would fold the stride multiply away), per the same lever as 0x00423000's g_faction_table. slot 25 on the edit_box_ sub-object takes two ints (a1,a2) - the auto-emitted VCall shim is nullary, so a second small vtable-shim class (VCall25) declares only the 26 ordinals needed with the right signature on the last one; a bare __thiscall function-pointer cast is C4234 on VC6. Landed at 0.74 mnemonic similarity.
int DiploWin::on_key_click(int repeat, int key) {
    if (key == 0x1B) {
        release_modal();
        return 1;
    }
    if (key == 0x0D) {
        if (strlen(editBox_.text_) != 0) {
            const int faction = ConsoleGlobal.faction_id_;
            message_chat(0x1E00, faction_id_with_, editBox_.text_);

            char line[256];
            line[0] = 0;
            PluralityDefault = 0;
            GenderDefault = Players[faction].is_leader_female;
            strcat(line, Players[faction].name_leader);
            string_upper_slot()(line);
            strcat(line, ": ");
            strcat(line, " ");
            strcat(line, editBox_.text_);

            stringBox_.add(line, faction, 1);
            DiploOffersDirty = 1;
            editBox_.set_text(0);
            return 1;
        }
        release_modal();
        return 1;
    }
    editBox_.on_key_click(repeat, key);
    return 1;
}


/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00442A00 ?UNK5@DiploWin@@QAEXXZ 0x00442A00-0x00442A01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK5() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004429F0 ?UNK4@DiploWin@@QAEXH@Z 0x004429F0-0x004429F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK4(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D80 ?on_iface_left_click@DiploWin@@QAEXHH@Z 0x00443D80-0x00443D83 BYTE_EXACT
// symbol ?on_iface_left_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D90 ?on_iface_left_up@DiploWin@@QAEXHH@Z 0x00443D90-0x00443D93 BYTE_EXACT
// symbol ?on_iface_left_up@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_up@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DA0 ?on_iface_right_click@DiploWin@@QAEXHH@Z 0x00443DA0-0x00443DA3 BYTE_EXACT
// symbol ?on_iface_right_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DB0 ?on_iface_left_down@DiploWin@@QAEXHH@Z 0x00443DB0-0x00443DB3 BYTE_EXACT
// symbol ?on_iface_left_down@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DC0 ?on_iface_right_down@DiploWin@@QAEXHH@Z 0x00443DC0-0x00443DC3 BYTE_EXACT
// symbol ?on_iface_right_down@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DD0 ?on_iface_mouse_move@DiploWin@@QAEXHH@Z 0x00443DD0-0x00443DD3 BYTE_EXACT
// symbol ?on_iface_mouse_move@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DE0 ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z 0x00443DE0-0x00443DE3 BYTE_EXACT
// symbol ?on_iface_mouse_leave@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DF0 ?on_iface_selected@DiploWin@@QAEXHH@Z 0x00443DF0-0x00443DF3 BYTE_EXACT
// symbol ?on_iface_selected@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E00 ?on_iface_left_double_click@DiploWin@@QAEXHH@Z 0x00443E00-0x00443E03 BYTE_EXACT
// symbol ?on_iface_left_double_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E10 ?on_iface_right_double_click@DiploWin@@QAEXHH@Z 0x00443E10-0x00443E13 BYTE_EXACT
// symbol ?on_iface_right_double_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E20 ?on_iface_button_clicked@DiploWin@@QAEXH@Z 0x00443E20-0x00443E23 BYTE_EXACT
// symbol ?on_iface_button_clicked@DiploWin@@UAEXH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E30 ?on_iface_button_toggled@DiploWin@@QAEXHH@Z 0x00443E30-0x00443E33 BYTE_EXACT
// symbol ?on_iface_button_toggled@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00443E40 ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z 0x00443E40-0x00443E45 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int DiploWin::on_iface_dialog_back_draw(int) {
    return 0;
}


/*
Purpose: Clear the two cached values at 0xA24 and 0xA28.
// ORIGINAL: 0x004413B0 ?UNK2@DiploWin@@QAEXXZ 0x004413B0-0x004413BE
// size      14 bytes
// prototype void (__thiscall ?UNK2@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK2() {
    field_A24_ = 0;
    field_A28_ = 0;
}


/*
Purpose: Clear one entry of the array at 0xA1C, then clear the two words at
         0xA24 and 0xA28.

             mov dword [ecx+eax*4+0xA1C],0
             lea eax,[ecx+0xA24] / xor ecx,ecx
             mov [eax],ecx / mov [eax+4],ecx

         The index is NOT bounds checked - the original stores through
         `eax*4` whatever the caller passed - so this does not add a guard the
         program does not have.

         Note the array at 0xA1C overlaps the two words cleared afterwards:
         index 2 IS the word at 0xA24, and index 3 the one at 0xA28. So calling
         with 2 or 3 writes the same slot twice, which is visible only because
         the fixture checks exact bytes rather than the cleared entry alone.
// ORIGINAL: 0x004413C0 ?UNK3@DiploWin@@QAEXH@Z 0x004413C0-0x004413E2
// size      34 bytes
// prototype void (__thiscall ?UNK3@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK3(int a1) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<int32_t *>(self + 0xA1C + a1 * 4) = 0;
    int32_t *const pair = reinterpret_cast<int32_t *>(self + 0xA24);
    pair[0] = 0;
    pair[1] = 0;
}


/*
// ORIGINAL: 0x00443C30 ?show@DiploWin@@QAEXH@Z 0x00443C30-0x00443C4B BYTE_EXACT
// symbol    ?show@DiploWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void DiploWin::show(int visible) {
    if (!is_visible()) {
        set_iface_mode();
    }
}

/*
// ORIGINAL: 0x00443C50 ?hide@DiploWin@@QAEXXZ 0x00443C50-0x00443C68 BYTE_EXACT
// symbol    ?hide@DiploWin@@UAEXXZ
// size      24 bytes
// prototype void (__thiscall ?hide@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x00443C60
Status: Complete
*/
void DiploWin::hide() {
    if (is_visible()) {
        release_modal();
    }
}

// DiploOffersDirty (0x007492D0) - the diplomacy-window redraw latch. add_offer,
// kill_offer, new_chat and on_button_clicked all set it to 1 as they change the
// offer lists; exec (0x00443F10) tests it and clears it. It is now owned storage.

// ORIGINAL: 0x004413F0 ?kill_offer@DiploWin@@QAEXHH@Z 0x004413F0-0x0044148C FILE
// TRIED: `int idx = a2;` declared inside the inner `if (a2 < count - 1)` (mismatch, idx assign shifted 5 instructions late) and hoisted above it with a separate `countMinus1` temp (MNEMONIC_ONLY, worse). Landed the inner-if placement: MISMATCH #23, mnemonic similarity 0.98, same byte count (156/156).
// working copy - scaffold materialised by --work
// size      156 bytes
// prototype void (__thiscall ?kill_offer@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592EE0
// indirect  0x0044147F

void DiploWin::kill_offer(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        message_data(0x1507, *reinterpret_cast<int *>(self + 0xAB8), a2, 0, 0, 0);
    }
    int count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
    if (a2 < count) {
        if (a2 < count - 1) {
            char *rec = self + 0xA2C + a1 * 32 + a2 * 4;
            int idx = a2;
            do {
                idx = idx + 1;
                *reinterpret_cast<int *>(rec) = *reinterpret_cast<int *>(rec + 4);
                *reinterpret_cast<int *>(rec + 0x40) = *reinterpret_cast<int *>(rec + 0x44);
                rec += 4;
                count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
            } while (idx < count - 1);
        }
        count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) - 1;
        *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) = count;
        DiploOffersDirty = 1;
        *reinterpret_cast<int *>(self + 0xA24) = 0;
        *reinterpret_cast<int *>(self + 0xA28) = 0;
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }

}

// ORIGINAL: 0x00441490 ?add_offer@DiploWin@@QAEXHHH@Z 0x00441490-0x004415B9 FILE
// size      297 bytes
// prototype void (__thiscall ?add_offer@DiploWin@@QAEXHHH@Z)(DiploWin* this, int, int, int)
// callers   3   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x00592EE0
// indirect  0x00441598
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00441490
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00441490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_offer@DiploWin@@QAEXHHH@Z  at 0x00441490  (297 bytes)
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


void DiploWin::add_offer(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int *count_arr = reinterpret_cast<int *>(self + 0xa1c);
    int *type_arr = reinterpret_cast<int *>(self + 0xa2c);
    int *val_arr = reinterpret_cast<int *>(self + 0xa6c);

    DiploOffersDirty = 1;

    int i;
    for (i = 0; i < count_arr[a1]; i++) {
        if (a2 == 4 || a2 == 2 || a2 == 3) {
            int existing = type_arr[i + a1 * 8];
            if (existing == 4 || existing == 2 || existing == 3) {
                goto found;
            }
        } else if (a2 == 1) {
            if (type_arr[i + a1 * 8] == 1) {
                if (a3 <= 0) {
                    kill_offer(a1, i);
                    return;
                }
                goto found;
            }
        } else if (a2 == 0) {
            if (type_arr[i + a1 * 8] == 0 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        } else if (a2 == 5) {
            if (type_arr[i + a1 * 8] == 5 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        }
    }

    {
        int count = count_arr[a1];
        if (count >= 8) {
            return;
        }
        count_arr[a1] = count + 1;
        i = count;
    }

found:
    {
        int slot = i + a1 * 8;
        type_arr[slot] = a2;
        val_arr[slot] = a3;
        *reinterpret_cast<int *>(self + 0xa24) = 0;
        *reinterpret_cast<int *>(self + 0xa28) = 0;
        if (a1 == 0) {
            message_data(0x1506, *reinterpret_cast<int *>(self + 0xab8),
                        a2, a3, a1, a1);
        }
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }
}

// ORIGINAL: 0x00442510 ?on_left_click@DiploWin@@QAEXHH@Z 0x00442510-0x00442558 FILE BYTE_EXACT
// LEVER: homed from units/00442510.cpp once kill_offer (0x004413F0) landed in
// this TU - the call was the only thing blocking it. The declared `spot_`
// member at 0xED0 and the same vtable_slot dispatch done_deal uses replaced
// the artifact's cast of `this` and its VCall shim.
// size      72 bytes
// prototype void (__thiscall ?on_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x005FAB00
// indirect  0x0044254D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00442510
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00442510/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@DiploWin@@QAEXHH@Z  at 0x00442510  (72 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DiploWin::on_left_click(int a1, int a2) {
    // `check`'s two int* out-params reuse the parameter slots, so the same
    // names come back carrying the spot position and the button kind.
    if (spot_.check(a1, a2, &a1, &a2) >= 0 && a2 == 0) {
        kill_offer(0, a1);
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }
}
