/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include "checkbox.h"
#include "flatbutton.h"
#include "font.h"
#include "graphicwin.h"
#include "scroll.h"
#include "buttongroup.h"

extern ButtonGroup PrefWinButtonGroup;

// The original object is a GraphicWin with controls embedded in the order
// established by ??0PrefWin and ??1PrefWin.  The UI opens this one object to
// different preference pages; its controls, fonts and backing surface are
// borrowed/owned according to their respective class lifetimes.
class PrefWin : public GraphicWin {
 public:
  PrefWin();
  ~PrefWin();

  void init();
  void display(int page);
  void destroy();
  int on_key_event(int repeat, int key) override;
  void on_group_clicked(int group, int previous, int selected);
  void hide_option_buttons();
  void update_pref_buttons();
  void set_preferences();
  void get_preferences();
  void on_scrolling(int control_id, int position);
  void on_button_clicked(int button_id) override;
  void redraw_option();
  void redraw_volume();
  void change_volume_button_text(int button_id);
  void change_volume_button_state(int button_id);
  void on_redraw() override;

 public:
  int field_A14_;                    // +0xA14
  int field_A18_;                    // +0xA18
  int field_A1C_;                    // +0xA1C
  uint32_t prefsFinal_;              // +0xA20
  uint32_t morePrefsFinal_;          // +0xA24
  uint32_t warningsFinal_;           // +0xA28
  uint32_t field_A2C_;               // +0xA2C
  uint32_t prefsBasicPending_;       // +0xA30
  uint32_t morePrefsBasicPending_;   // +0xA34
  uint32_t prefsAdvPending_;         // +0xA38
  uint32_t morePrefsAdvPending_;     // +0xA3C
  uint32_t prefsAutoPending_;        // +0xA40
  uint32_t morePrefsAutoPending_;    // +0xA44
  uint32_t warningsPending_;         // +0xA48
  uint32_t prefsAVPending_;          // +0xA4C
  uint32_t morePrefsAVPending_;      // +0xA50
  uint32_t prefsMapPending_;         // +0xA54
  uint32_t morePrefsMapPending_;     // +0xA58
  uint32_t field_A5C_;               // +0xA5C

  FlatButton bscPrefButton_;         // +0xA60
  FlatButton advPrefButton_;         // +0x15AC
  FlatButton warnPrefButton_;        // +0x20F8
  FlatButton autoPrefButton_;        // +0x2C44
  FlatButton audioVisPrefButton_;    // +0x3790
  FlatButton mapPrefButton_;         // +0x42DC
  FlatButton gammaButton_;           // +0x4E28

  CheckBox bscPrefCheckBox_;         // +0x5974
  CheckBox advPrefCheckBox_;         // +0x649C
  CheckBox autoPrefCheckBox_;        // +0x6FC4
  CheckBox audioVisPrefCheckBox_;    // +0x7AEC
  CheckBox warnPrefCheckBox_;        // +0x8614
  CheckBox warnContPrefCheckBox_;    // +0x913C
  CheckBox mapPrefCheckBox_;         // +0x9C64

  Scroll volumeMainScroll_;          // +0xA78C
  Scroll volumeVoiceScroll_;         // +0xC8D8
  Scroll volumeSFXScroll_;           // +0xEA24
  Scroll volumeMusicScroll_;         // +0x10B70
  Scroll scroll5_;                   // +0x12CBC

  FlatButton volumeMainToggle_;      // +0x14E08
  FlatButton volumeSFXToggle_;       // +0x15954
  FlatButton volumeVoiceToggle_;     // +0x164A0
  FlatButton volumeMusicToggle_;     // +0x16FEC
  FlatButton flatButton12_;          // +0x17B38
  FlatButton restoreDefaultButton_;  // +0x18684
  FlatButton cancelButton_;          // +0x191D0
  FlatButton okButton_;              // +0x19D1C
  FlatButton moreButton_;            // +0x1A868

  Font *font1_;                      // +0x1B3B4
  Font *font2_;                      // +0x1B3B8
  Font *font3_;                      // +0x1B3BC
  Font *font4_;                      // +0x1B3C0
  Font *font5_;                      // +0x1B3C4
  Font *font6_;                      // +0x1B3C8
  Font *font7_;                      // +0x1B3CC
};

static_assert(sizeof(PrefWin) == 0x1B3D0,
              "PrefWin layout must match the original object extent");
