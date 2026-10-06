/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x00492410 ??_GPrefWin@@WEEE@AEPAXI@Z 0x00492410-0x0049241B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GPrefWin@@WEEE@AEPAXI@Z at 0x00492410 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object PrefWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned PrefWin constructor at 0x004923c2 stores Buffer-view vftable 0x0066d41c at this+0x444; the source owner is the typed PrefWin GraphicWin/Buffer multiple-inheritance view.", "Current src/prefwin.cpp COFF Buffer-view vftable first slot selects generated ??_EPrefWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current PrefWin COFF weak external ??_EPrefWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GPrefWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls PrefWin::~PrefWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_pref_win@@YIPAXPAX0I@Z and ?scalar_delete_pref_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for PrefWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EPrefWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x004923E0 ??_GPrefWin@@UAEPAXI@Z 0x004923E0-0x00492401 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004923E0-0x00492401", "Pinned class constructor ??0PrefWin@@QAE@XZ at 0x004921E0 stores original class/view table VA 0x66d424 at 0x004923BC (mov dword ptr [esi], 0x66d424); the current genuine owner vftable ??_7PrefWin@@6BWin@@@ selects generated ??_GPrefWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPrefWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GPrefWin@@UAEPAXI@Z at 0x004923E0 calls the source-mapped ??1PrefWin@@QAE@XZ at 0x00491DF0, then tests byte ptr [ebp + 8], 1; je 0x4923fa bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/prefwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PrefWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPrefWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "prefwin.h"

#include <string.h>

#include "alpha.h"
#include "ambience.h"
#include "buttongroup.h"
#include "buffer.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "gamma.h"
#include "prefwin_geometry.h"
#include "main.h"
#include "mapwin.h"
#include "palette.h"
#include "popup.h"
#include "text.h"
#include "temp.h"
#include "tutwin.h"
#include "wave_device.h"

int __cdecl X_text_open(char *src_file_id, const char *section_id);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
// The process-wide preference window is a genuine PrefWin object. Its
// original CRT initializer is 0x48D4F0, before ButtonGroup at 0x48D520.
// RECOVERY_OBJECT: v2 0x008578D8 0x00872CA8 ?ConsolePrefWin@@3VPrefWin@@A 0x004921E0
PrefWin ConsolePrefWin;   // 0x008578D8
// The six preference-page tabs have their own ButtonGroup at 0x8577F0,
// immediately before the initialized geometry words beginning at 0x857884.
// RECOVERY_OBJECT: v2 0x008577F0 0x00857884 ?PrefWinButtonGroup@@3VButtonGroup@@A 0x0048D840
ButtonGroup PrefWinButtonGroup;

namespace {
// Original caption storage begins NUL; the writable extent and indirect
// writers at its image location 0x872CAC remain unproved.
char PrefWinCaption[] = "";
}

// The geometry object is recovered in the adjacent global-arithmetic owner.
// Keep these as ordinary object declarations here; they are not executable
// address aliases and their startup initializers remain compiler-owned.
extern int32_t PrefWinDisplayWidth, PrefWinDisplayHeight, PrefWinButtonWidth;
extern int32_t PrefWinTitleButtonHeight, PrefWinToggleWidth, PrefWinToggleHeight;
extern int32_t PrefWinGeometryConstant_686B78;
// Original 0x48D884 writes 50 to this separate DWORD on the 800-pixel path.
// RECOVERY_OBJECT: v2 0x00872CA8 0x00872CAC ?PrefWinNarrowDisplayVerticalOffset@@3HA 0x0048D840
int PrefWinNarrowDisplayVerticalOffset;
extern int MessageWinTextColor, MessageWinTextBackground;
// Mutable preference state begins at 1.0; the 100.0 factor converts the
// stored gamma scalar to the integer value written to the preferences file.
// RECOVERY_OBJECT: v2 0x006846A8 0x006846AC ?PrefWinGammaCorrection@@3MA 0x00491400
float PrefWinGammaCorrection = 1.0f;
// RECOVERY_OBJECT: v2 0x0066D58C 0x0066D590 _PrefWinGammaScale 0x00491400
const float PrefWinGammaScale = 100.0f;

namespace {

Win *console_window() {
  return static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal));
}

Win *preference_window() {
  // Embedded controls belong to the canonical preferences window (8578D8),
  // not its Console parent (9156B0).
  return static_cast<Win *>(static_cast<GraphicWin *>(&ConsolePrefWin));
}

struct CheckEntry {
  int id;
  uint32_t bit;
  uint8_t group;
  bool inverted;
};

// Dialog ids and preference words are kept together so that the two
// directions use one correspondence table.  Group numbers select the
// preference words in PrefWin rather than raw receiver-relative storage.
const CheckEntry kBasic[] = {
    {7, PREF_BSC_TUTORIAL_MSGS, 0, false},
    {8, PREF_BSC_AUTO_DESIGN_VEH, 0, false},
    {9, MPREF_BSC_AUTO_PRUNE_OBS_VEH, 1, false},
    {10, PREF_BSC_MOUSE_EDGE_SCROLL_VIEW, 0, false},
    {11, PREF_BSC_AUTOSAVE_EACH_TURN, 0, false},
    {12, PREF_BSC_PAUSE_END_TURN, 0, false},
    {13, PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH, 0, true},
    {14, PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH, 0, true},
};
const CheckEntry kAdvanced[] = {
    {15, PREF_ADV_RADIO_BTN_NOT_SEL_SING_CLK, 2, true},
    {16, MPREF_ADV_CLICK_VEH_CANCELS_ORDERS, 3, false},
    {17, MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING, 3, false},
    {18, MPREF_ADV_PAUSE_AFTER_BATTLES, 3, false},
    {19, PREF_ADV_FAST_BATTLE_RESOLUTION, 2, false},
    {28, MPREF_ADV_QUICK_MOVE_VEH_ORDERS, 3, false},
    {20, MPREF_ADV_QUICK_MOVE_ALL_VEH, 3, false},
    {21, MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP, 3, false},
    {23, MPREF_ADV_RIGHT_CLICK_POPS_UP_MENU, 3, false},
    {24, MPREF_ADV_DETAIL_MAIN_MENUS, 3, false},
    {25, MPREF_ADV_DETAIL_RIGHT_CLICK_MENUS, 3, false},
    {26, MPREF_AV_SLIDING_SCROLLBARS, 3, false},
};
const CheckEntry kAutomation[] = {
    {29, PREF_AUTO_AIR_VEH_RET_HOME_FUEL_RNG, 4, false},
    {30, PREF_AUTO_FORMER_PLANT_FORESTS, 4, false},
    {31, PREF_AUTO_FORMER_RAISE_LWR_TERRAIN, 4, false},
    {32, PREF_AUTO_FORMER_BUILD_ADV, 4, false},
    {33, MPREF_AUTO_FORMER_REMOVE_FUNGUS, 5, false},
    {42, MPREF_AUTO_FORMER_CANT_BUILD_ROADS, 5, true},
    {34, MPREF_AUTO_FORMER_BUILD_SENSORS, 5, false},
    {35, PREF_AUTO_END_MOVE_SPOT_VEH_PACT, 4, false},
    {36, PREF_AUTO_END_MOVE_SPOT_VEH_TREATY, 4, false},
    {37, PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE, 4, false},
    {38, PREF_AUTO_END_MOVE_SPOT_VEH_WAR, 4, false},
    {39, PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD, 4, false},
    {40, PREF_AUTO_WAKE_VEH_TRANS_REACH_LAND, 4, false},
    {41, MPREF_AUTO_ALWAYS_INSPECT_MONOLITH, 5, false},
};
const CheckEntry kAudioVisual[] = {
    {0x40, MPREF_AV_VOICEOVER_STOP_CLOSE_POPUP, 7, false},
    {0x41, PREF_AV_SECRET_PROJECT_MOVIES, 6, false},
    {0x42, PREF_AV_INTERLUDES_DISABLED, 6, true},
    {0x47, MPREF_AV_MONUMENTS_DISABLED, 7, true},
    {0x43, PREF_AV_MAP_ANIMATIONS, 6, false},
    {0x44, PREF_AV_SLIDING_WINDOWS, 6, false},
    {0x45, MPREF_AV_SLIDING_SCROLLBARS, 7, false},
    {0x16, MPREF_AV_WHOLE_VEH_BLINKS, 7, false},
};
const CheckEntry kMap[] = {
    {0x48, MPREF_MAP_SHOW_FOG_WAR, 9, false},
    {0x49, PREF_MAP_SHOW_GRID, 8, false},
    {0x4E, MPREF_MAP_SHOW_GRID_OCEAN_SQ, 9, false},
    {0x4A, PREF_MAP_SHOW_BASE_GRID, 8, false},
    {0x4B, MPREF_MAP_SHOW_BASE_NAMES, 9, false},
    {0x4C, MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES, 9, false},
    {0x4D, MPREF_MAP_SHOW_FLAT_TERRAIN, 9, false},
    {0x4F, MPREF_MAP_HIDE_ACTIVE_VEH_GOTO_PATH, 9, true},
};
const CheckEntry kWarnings[] = {
    {0x2B, WARN_STOP_NEW_FAC_BUILT, 10, false},
    {0x2C, WARN_STOP_NON_COMBAT_VEH_BUILT, 10, false},
    {0x2D, WARN_STOP_COMBAT_VEH_BUILT, 10, false},
    {0x2E, WARN_STOP_PROTOTYPE_COMPLETE, 10, false},
    {0x2F, WARN_STOP_BUILT_VIA_GOV_QUEUE, 10, false},
    {0x30, WARN_STOP_DRONE_RIOTS, 10, false},
    {0x31, WARN_STOP_DRONE_RIOTS_END, 10, false},
    {0x32, WARN_STOP_GOLDEN_AGE, 10, false},
    {0x33, WARN_STOP_GOLDEN_AGE_END, 10, false},
    {0x34, WARN_STOP_NUTRIENT_SHORTAGE, 10, false},
    {0x36, WARN_STOP_BUILD_OUT_OF_DATE, 10, false},
    {0x37, WARN_STOP_POP_LIMIT_REACHED, 10, false},
    {0x38, WARN_STOP_DELAY_IN_TRANSCEND, 10, false},
};
const CheckEntry kWarningContinue[] = {
    {0x39, WARN_STOP_STARVATION, 10, false},
    {0x3A, WARN_STOP_MINERAL_SHORTAGE, 10, false},
    {0x3B, WARN_STOP_ENERGY_SHORTAGE, 10, false},
    {0x3C, WARN_STOP_RANDOM_EVENT, 10, false},
};
const CheckEntry kAudioDialog[] = {
    {0x40, 0, 10, false}, {0x41, 0, 10, false}, {0x42, 0, 10, false},
    {0x47, 0, 10, false}, {0x43, 0, 10, false}, {0x44, 0, 10, false},
    {0x45, 0, 10, false}, {0x16, 0, 10, false},
};

uint32_t &pref_word(PrefWin *self, unsigned int group) {
  switch (group) {
    case 0: return self->prefsBasicPending_;
    case 1: return self->morePrefsBasicPending_;
    case 2: return self->prefsAdvPending_;
    case 3: return self->morePrefsAdvPending_;
    case 4: return self->prefsAutoPending_;
    case 5: return self->morePrefsAutoPending_;
    case 6: return self->prefsAVPending_;
    case 7: return self->morePrefsAVPending_;
    case 8: return self->prefsMapPending_;
    default: return self->morePrefsMapPending_;
  }
}

void read_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,
                   unsigned int count) {
  for (unsigned int i = 0; i < count; ++i) {
    const bool set = (pref_word(self, items[i].group) & items[i].bit) != 0;
    box.set_state_id(items[i].id, (set != items[i].inverted) ? 1 : 0);
  }
}

void write_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,
                    unsigned int count) {
  for (unsigned int i = 0; i < count; ++i) {
    const bool set = box.get_state_id(items[i].id) != 0;
    if (set != items[i].inverted)
      pref_word(self, items[i].group) |= items[i].bit;
  }
}

void add_dialog_items(CheckBox &box, const CheckEntry *items,
                      unsigned int count, const int *label_offsets) {
  for (unsigned int i = 0; i < count; ++i) {
    StringTemp[0] = '\0';
    text_get();
    const char *suffix = label_offsets ? label_get(label_offsets[i] / 4) : nullptr;
    if (suffix) strcat(StringTemp, suffix);
    if (TextBufferGetPtr) strcat(StringTemp, TextBufferGetPtr);
    box.item(StringTemp, items[i].id, 0);
  }
}

int label_index(int byte_offset) { return byte_offset / 4; }

void set_control_text(FlatButton &button, const char *text) {
  if (text) button.set_name(text);
}

}  // namespace

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004921E0 ??0PrefWin@@QAE@XZ 0x004921E0-0x004923DA;0x00658930-0x00658ABC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004921e0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
PrefWin::PrefWin() : GraphicWin() {}

// ORIGINAL: 0x00491DF0 ??1PrefWin@@QAE@XZ 0x00491DF0-0x004921DF;0x006586C0-0x00658924 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00491df0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED: real nested member declarations (arrays/Scroll/CheckBox/NetWin in true layout order) letting the compiler auto-cascade the teardown and build the SEH frame - instead used explicit reinterpret_cast<T*>(self+offset)->close()/~T() calls matching the disasm's two FlatButton arrays (stride 0xB4C), 5 Scrolls, one CheckBox+Dialog+GraphicWin triple, and 6-element NetWin array (stride 0xB28, closed via UNK4()) in program order.
// symbol    ??1PrefWin@@UAE@XZ
PrefWin::~PrefWin() {}

// ORIGINAL: 0x0048D840 ?init@PrefWin@@QAEXXZ 0x0048D840-0x0048F9F5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0048d840.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED: full hand transcription from raw disassembly (Ghidra had only one benign unreachable-block warning and was trustworthy here). The six "BASICPREFS/ADVANCEDPREFS/AUTOMATIONPREFS/AUDIOVISUALPREFS/ BASEWARNINGS/(unconditional)/MAPPREFS" tab pages each resolve an embedded CheckBox's own vtable slot at +8 (a stored byte offset to an embedded Dialog sub-object, not a function pointer) and dispatch that Dialog's vtable slot 1/2 - confirmed against raw asm receiver setup (`lea ecx,[esi+N]` before each call), not trusted from Ghidra's frequently-dropped implicit `this`. Two VCall shims used since slot002 is nullary on one dispatch chain and takes one int on the sibling chain. First divergence is instruction #5 (prologue scheduling) - not byte-verified beyond the receiver/signature spot-checks cited above.
void PrefWin::init() {
  field_A14_ = field_A18_ = field_A1C_ = field_A5C_ = 0;

  const int width = PrefWinDisplayWidth + 0x4E;
  const int height = PrefWinDisplayHeight;
  int top = WinScreenHeight - 0x26A;
  int frame_adjustment = 0;
  if (WinScreenWidth == 800) {
    PrefWinGeometryValue_8578D0 -= 0x32;
    PrefWinNarrowDisplayVerticalOffset = 0x32;
    top = 0;
    frame_adjustment = 0x50;
  }
  // The original passes 0x872CAC as a mutable caption address. Its first
  // byte is observed NUL, but extent and indirect writers are unresolved.
  GraphicWin::init(WinScreenWidth / 2 - PrefWinDisplayWidth / 2, top,
                   width, height, PrefWinCaption,
                   0x1000000, console_window(), nullptr,
                   nullptr);
  GraphicWin::fill(9);

  font1_ = find_font(16, 0);
  font2_ = find_font(12, 0);
  font3_ = find_font(12, 2);
  font4_ = find_font(12, 1);
  font5_ = find_font(14, 0);
  font6_ = find_font(14, 2);
  font7_ = find_font(14, 1);
  Buffer::set_font(font2_, font2_, font5_, 0);
  Buffer::set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);

  FlatButton *tabs[] = {&bscPrefButton_, &advPrefButton_, &warnPrefButton_,
                        &autoPrefButton_, &audioVisPrefButton_, &mapPrefButton_,
                        &gammaButton_};
  const int tabIds[] = {0, 1, 2, 3, 4, 5, 6};
  const int tabLabels[] = {3000, 0xBBC, 0xBC0, 0xBC4, 0xBC8, 0x10D4, 0x10D8};
  const int tabY[] = {0, 0x16, 0x2C, 0x42, 0x58, 0x6E, 0x9A};
  for (unsigned int tab_index = 0; tab_index < 7; ++tab_index) {
    const char *label = label_get(label_index(tabLabels[tab_index]));
    tabs[tab_index]->init(const_cast<char *>(label ? label : ""), tabIds[tab_index],
                  PrefWinGeometryValue_85788C,
                  PrefWinGeometryValue_857890 + tabY[tab_index],
                  PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
                  preference_window(), 0);
    // Original 6E84B4 is the tutorial BaseWin's constructed Font member,
    // not find_font(12, 0). Its owner constructs and tears it down.
    tabs[tab_index]->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    tabs[tab_index]->redraw();
  }
  PrefWinButtonGroup.init(0x5D, 0);
  for (unsigned int tab_member = 0; tab_member < 6; ++tab_member)
    PrefWinButtonGroup.add(tabs[tab_member]);
  PrefWinButtonGroup.set(0, 1);

  CheckBox *boxes[] = {&bscPrefCheckBox_, &advPrefCheckBox_, &autoPrefCheckBox_,
                       &audioVisPrefCheckBox_, &warnPrefCheckBox_,
                       &warnContPrefCheckBox_, &mapPrefCheckBox_};
  const CheckEntry *entries[] = {kBasic, kAdvanced, kAutomation, kAudioDialog,
                                 kWarnings, kWarningContinue, kMap};
  const char *sections[] = {"BASICPREFS", "ADVANCEDPREFS", "AUTOMATIONPREFS",
      "AUDIOVISUALPREFS", "BASEWARNINGS", "BASEWARNINGS", "MAPPREFS"};
  const unsigned int counts[] = {sizeof(kBasic) / sizeof(kBasic[0]),
      sizeof(kAdvanced) / sizeof(kAdvanced[0]),
      sizeof(kAutomation) / sizeof(kAutomation[0]),
      sizeof(kAudioDialog) / sizeof(kAudioDialog[0]),
      sizeof(kWarnings) / sizeof(kWarnings[0]),
      sizeof(kWarningContinue) / sizeof(kWarningContinue[0]),
      sizeof(kMap) / sizeof(kMap[0])};
  const int basicLabels[] = {0x6BA4,0x6BA8,0x6BAC,0x6BB0,0x6BB4,0x6BB8,0x6BBC,0x6BC0};
  const int advancedLabels[] = {0x6BD4,0x6BD8,0x6BDC,0x6BE0,0x6BE4,0x6BE8,0x6BEC,0x6BF0,0x6BF4,0x6BF8,0x6BFC,0x6C00};
  const int automationLabels[] = {0x6C14,0x6C18,0x6C1C,0x6C20,0x6C24,0x6C28,0x6C2C,0x6C30,0x6C34,0x6C38,0x6C3C,0x6C40,0x6C44,0x6C48};
  const int audioLabels[] = {0x6C60,0x6C64,0x6C68,0x6C6C,0x6C70,0x6C74,0x6C78,0x6C7C};
  const int warningLabels[] = {0x6C90,0x6C94,0x6C98,0x6C9C,0x6CA0,0x6CA4,0x6CA8,0x6CAC,0x6CB0,0x6CB4,0x6CB8,0x6CBC,0x6CC0};
  const int warningContinueLabels[] = {0x6CC4,0x6CC8,0x6CCC,0x6CD0};
  const int mapLabels[] = {0x6CE0,0x6CE4,0x6CE8,0x6CEC,0x6CF0,0x6CF4,0x6CF8,0x6CFC};
  const int *itemLabels[] = {basicLabels, advancedLabels, automationLabels,
      audioLabels, warningLabels, warningContinueLabels, mapLabels};
  const int y[] = {PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578C4,
      PrefWinGeometryValue_8578C4, PrefWinGeometryValue_8578C4,
      PrefWinGeometryValue_8578A4 - PrefWinGeometryConstant_686B78 -
          PrefWinGeometryValue_8577E0 - frame_adjustment,
      PrefWinGeometryValue_8578A4 - PrefWinGeometryConstant_686B78 -
          PrefWinGeometryValue_8577E0 + 0x28 - frame_adjustment,
      PrefWinGeometryValue_8578A4};
  for (unsigned int box_index = 0; box_index < 7; ++box_index) {
    // The warning-continuation controls are appended to the still-open
    // BASEWARNINGS text section; the original does not open it a second time.
    if (box_index != 5 && X_text_open(ScriptPopupFile, sections[box_index]) != 0) return;
    boxes[box_index]->close();
    Dialog *dialog = static_cast<Dialog *>(boxes[box_index]);
    dialog->init(PrefWinGeometryValue_8578C8, y[box_index],
                 PrefWinGeometryValue_8578A0, PrefWinGeometryValue_8578A4, 0);
    add_dialog_items(*boxes[box_index], entries[box_index], counts[box_index], itemLabels[box_index]);
    boxes[box_index]->attach(&ConsolePrefWin, PrefWinGeometryValue_8578C8,
                     PrefWinGeometryValue_8578C4, 0x1000120);
    if (box_index != 4) text_close();
  }

  Scroll *scrolls[] = {&volumeMainScroll_, &volumeVoiceScroll_,
      &volumeSFXScroll_, &volumeMusicScroll_};
  for (int scroll_index = 0; scroll_index < 4; ++scroll_index) {
    scrolls[scroll_index]->init_horz(PrefWinGeometryValue_8578D0 + scroll_index * 0x16,
        PrefWinGeometryValue_8578B8, 0x50 + scroll_index, preference_window(), 0);
    scrolls[scroll_index]->set_range(0, 0x7F);
  }
  volumeMainScroll_.set_pos(prefs_get("Main Volume", 0x7F, 0));
  volumeVoiceScroll_.set_pos(prefs_get("Voice Volume", 0x55, 0));
  volumeSFXScroll_.set_pos(prefs_get("SFX Volume", 0x5F, 0));
  if (FactionMusic)
    volumeMusicScroll_.set_pos(prefs_get("Music Volume", 0x3F, 0));
  else
    volumeMusicScroll_.set_pos(0);
  on_scrolling(0x50, volumeMainScroll_.position());
  on_scrolling(0x51, volumeVoiceScroll_.position());
  on_scrolling(0x52, volumeSFXScroll_.position());
  if (FactionMusic) on_scrolling(0x53, volumeMusicScroll_.position());
  for (int scroll_hide_index = 0; scroll_hide_index < 4; ++scroll_hide_index)
    scrolls[scroll_hide_index]->hide();
  for (int box_hide_index = 0; box_hide_index < 6; ++box_hide_index)
    boxes[box_hide_index]->hide();

  FlatButton *volume_toggles[] = {&volumeMainToggle_, &volumeVoiceToggle_,
      &volumeSFXToggle_, &volumeMusicToggle_};
  const int volume_ids[] = {0x55, 0x57, 0x56, 0x58};
  const int volume_labels[] = {0x10E0, 0x10E8, 0x10F0, 0x10F8};
  for (unsigned int toggle_index = 0; toggle_index < 4; ++toggle_index) {
    const char *label = label_get(label_index(volume_labels[toggle_index]));
    volume_toggles[toggle_index]->init(const_cast<char *>(label ? label : ""),
        volume_ids[toggle_index], PrefWinGeometryValue_8578B4,
        PrefWinGeometryValue_8578BC - PrefWinNarrowDisplayVerticalOffset +
            static_cast<int>(toggle_index) * 0x16,
        PrefWinToggleWidth, PrefWinToggleHeight, preference_window(), 0);
  }
  for (unsigned int toggle_redraw = 0; toggle_redraw < 4; ++toggle_redraw) {
    volume_toggles[toggle_redraw]->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    volume_toggles[toggle_redraw]->redraw();
  }
  // The state stores precede all hide callbacks, in main/SFX/voice/music order.
  volumeMainToggle_.field_A18_ = 0;
  volumeSFXToggle_.field_A18_ = 0;
  volumeVoiceToggle_.field_A18_ = 0;
  volumeMusicToggle_.field_A18_ = 0;
  for (unsigned int toggle_hide = 0; toggle_hide < 4; ++toggle_hide)
    volume_toggles[toggle_hide]->hide();

  const char *restore_label = label_get(label_index(0x10DC));
  restoreDefaultButton_.init(const_cast<char *>(restore_label ? restore_label : ""),
      0x5A, PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_8578C0 - PrefWinNarrowDisplayVerticalOffset -
          0x45 + PrefWinGeometryValue_857884,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
      preference_window(), 0);
  const char *ok_label = label_get(label_index(0x194));
  okButton_.init(const_cast<char *>(ok_label ? ok_label : ""), 0x5C,
      PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_8578C0 - PrefWinNarrowDisplayVerticalOffset -
          0x2E + PrefWinGeometryValue_857884,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
      preference_window(), 0);
  const char *cancel_label = label_get(label_index(0x198));
  cancelButton_.init(const_cast<char *>(cancel_label ? cancel_label : ""), 0x5B,
      PrefWinGeometryValue_85788C,
      PrefWinGeometryValue_8578C0 - PrefWinNarrowDisplayVerticalOffset -
          0x17 + PrefWinGeometryValue_857884,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
      preference_window(), 0);
  FlatButton *footer_buttons[] = {&restoreDefaultButton_, &okButton_, &cancelButton_};
  for (unsigned int footer_index = 0; footer_index < 3; ++footer_index) {
    footer_buttons[footer_index]->Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    footer_buttons[footer_index]->redraw();
  }
  restoreDefaultButton_.field_A18_ = 0;
  okButton_.field_A18_ = 0;
  cancelButton_.field_A18_ = 0;
  for (unsigned int footer_hide = 0; footer_hide < 3; ++footer_hide)
    footer_buttons[footer_hide]->hide();

  const char *more_label = label_get(label_index(0x1100));
  moreButton_.init(const_cast<char *>(more_label ? more_label : ""),
      0x5E, PrefWinGeometryValue_8578C8,
      PrefWinGeometryValue_857898 - 0x17 + PrefWinGeometryValue_8578A4,
      PrefWinGeometryValue_857894, PrefWinTitleButtonHeight,
      preference_window(), 0);
  moreButton_.field_A18_ = 0;
  moreButton_.hide();
  Win::hide();
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FA00 ?display@PrefWin@@QAEXH@Z 0x0048FA00-0x0048FCC5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048fa00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::display(int page) {
  ConsoleGlobal.alpha_menu_.hide();
  prefsFinal_ = morePrefsFinal_ = warningsFinal_ = 0;
  prefsBasicPending_ = prefsAdvPending_ = prefsAutoPending_ = prefsAVPending_ = prefsMapPending_ = GameDataStateStorage.fields_.game_preferences_;
  morePrefsBasicPending_ = morePrefsAdvPending_ = morePrefsAutoPending_ = morePrefsAVPending_ = morePrefsMapPending_ = GameDataStateStorage.fields_.game_more_preferences_;
  warningsPending_ = GameDataStateStorage.fields_.game_warnings_;
  RECT outer = {0x17, 0, PrefWinDisplayWidth + 0x17, PrefWinDisplayHeight};
  Buffer::box_sprite(&outer, &InterfaceBoxStyles[9]);
  RECT inner = {0x1B, 4, PrefWinDisplayWidth + 0x13, PrefWinDisplayHeight - 4};
  Buffer::box_sprite(&inner, &InterfaceBoxStyles[9]);
  RECT buttons = {PrefWinGeometryValue_857888, PrefWinGeometryValue_857884,
      PrefWinButtonWidth + PrefWinGeometryValue_857888,
      PrefWinGeometryValue_8578C0 + PrefWinGeometryValue_857884};
  Buffer::box_sprite(&buttons, &InterfaceBoxStyles[18]);
  get_preferences();
  volumeMainToggle_.field_A18_ = volumeVoiceToggle_.field_A18_ ||
      volumeSFXToggle_.field_A18_ || volumeMusicToggle_.field_A18_;
  change_volume_button_text(0x55);
  redraw_option();
  redraw_volume();
  restoreDefaultButton_.show(0);
  okButton_.show(0);
  cancelButton_.show(0);
  if (page >= 0) {
    field_A18_ = field_A14_;
    field_A14_ = page;
    PrefWinButtonGroup.button_click(page);
  }
  redraw();
  slide_show(this, 0);
  show(3);
  Win::set_def_focus(preference_window());
  set_modal(0, pop_wait, nullptr);
  if (!(GameDataStateStorage.fields_.game_preferences_ & 0x20000000))
    DialogDefaultStyle |= 4;
  else
    DialogDefaultStyle &= ~4;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FDD0 ?destroy@PrefWin@@QAEXXZ 0x0048FDD0-0x0048FF51 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048fdd0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::destroy() {
  if (field_A5C_) {
    set_preferences();
    prefsFinal_ |= prefsBasicPending_ | prefsAdvPending_ | prefsAutoPending_ |
                  prefsAVPending_ | prefsMapPending_;
    morePrefsFinal_ |= morePrefsBasicPending_ | morePrefsAdvPending_ |
                      morePrefsAutoPending_ | morePrefsAVPending_ | morePrefsMapPending_;
    warningsFinal_ |= warningsPending_;
    AlphaIniPrefs.preferences = prefsFinal_;
    AlphaIniPrefs.more_preferences = morePrefsFinal_;
    AlphaIniPrefs.announce = warningsFinal_;
    prefs_use();
    prefs_save(0);
    prefs_put("Main Volume", volumeMainScroll_.position(), 0);
    prefs_put("Voice Volume", volumeVoiceScroll_.position(), 0);
    prefs_put("Music Volume", volumeMusicScroll_.position(), 0);
    prefs_put("SFX Volume", volumeSFXScroll_.position(), 0);
    field_A5C_ = 0;
  }
  draw_map(1);
  UnkBitfield1 |= 4;
  okButton_.field_A9C_ = 0;
  cancelButton_.field_A9C_ = 0;
  slide_hide(this, 0);
  hide();
  Win::set_def_focus(console_window());
  release_modal();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FCD0 ?on_key_click@PrefWin@@QAEHHH@Z 0x0048FCD0-0x0048FD98 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0048fcd0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_key_click@PrefWin@@UAEHHH@Z
int PrefWin::on_key_click(int repeat, int key) {
  (void)repeat;
  if (key == VK_RETURN) {
    field_A5C_ = 1;
    destroy();
    return 1;
  }
  if (key == VK_ESCAPE) {
    destroy();
    return 1;
  }
  if (key == VK_UP || key == VK_DOWN) {
    int next;
    if (key == VK_UP)
      next = field_A14_ < 1 ? 5 : field_A14_ - 1;
    else
      next = field_A14_ < 5 ? field_A14_ + 1 : 0;
    field_A18_ = field_A14_;
    field_A14_ = next;
    field_A1C_ = 1;
    PrefWinButtonGroup.button_click(next);
    return 1;
  }
  return 1;
}

// ORIGINAL: 0x0048FF60 ?on_group_clicked@PrefWin@@QAEXHHH@Z 0x0048FF60-0x0048FFA1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0048ff60.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void PrefWin::on_group_clicked(int group, int previous, int selected) {
  (void)group;
  if (!field_A1C_ && previous >= 0 && previous < 6) field_A18_ = previous;
  else field_A1C_ = 0;
  field_A14_ = selected;
  on_button_clicked(selected);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048FFB0 ?hide_option_buttons@PrefWin@@QAEXXZ 0x0048FFB0-0x00490123 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048ffb0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::hide_option_buttons() {
  switch (field_A18_) {
    case 0: bscPrefCheckBox_.hide(); break;
    case 1: advPrefCheckBox_.hide(); break;
    case 2:
      warnPrefCheckBox_.hide();
      moreButton_.hide();
      break;
    case 3: autoPrefCheckBox_.hide(); break;
    case 4:
      audioVisPrefCheckBox_.hide();
      volumeMainScroll_.hide();
      volumeVoiceScroll_.hide();
      volumeSFXScroll_.hide();
      volumeMusicScroll_.hide();
      volumeMainToggle_.hide();
      volumeVoiceToggle_.hide();
      volumeSFXToggle_.hide();
      volumeMusicToggle_.hide();
      break;
    case 5: mapPrefCheckBox_.hide(); break;
    case 6:
      warnContPrefCheckBox_.hide();
      moreButton_.hide();
      break;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00490140 ?update_pref_buttons@PrefWin@@QAEXXZ 0x00490140-0x00490326 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00490140.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::update_pref_buttons() {
  switch (field_A14_) {
    case 0: bscPrefCheckBox_.show(3); break;
    case 1: advPrefCheckBox_.show(3); break;
    case 2:
      warnPrefCheckBox_.show(3);
      moreButton_.set_name(label_get(label_index(0x1100)));
      moreButton_.show(3);
      break;
    case 3: autoPrefCheckBox_.show(3); break;
    case 4:
      audioVisPrefCheckBox_.show(3);
      volumeMainScroll_.show(3);
      volumeVoiceScroll_.show(3);
      volumeSFXScroll_.show(3);
      volumeMusicScroll_.show(3);
      volumeMainToggle_.show(3);
      volumeVoiceToggle_.show(3);
      volumeSFXToggle_.show(3);
      volumeMusicToggle_.show(3);
      break;
    case 5: mapPrefCheckBox_.show(3); break;
    case 6:
      warnContPrefCheckBox_.show(3);
      StringTemp[0] = '\0';
      if (const char *label = label_get(label_index(0xA60)))
        strcat(StringTemp, label);
      moreButton_.set_name(StringTemp);
      moreButton_.show(3);
      break;
  }
}

// ORIGINAL: 0x00490350 ?set_preferences@PrefWin@@QAEXXZ 0x00490350-0x00490AFA FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00490350.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED: declared each CheckBox* group pointer lazily, right before its first get_state_id() call (matching the original's `lea edi, [esi+OFFSET]` sitting immediately before that group's first push), instead of all six upfront. Moved divergence #1(sim0.98)->#63 on the frameless build (original prologue is `push esi; mov esi,ecx; push edi` with no `push ebp`, so that is the row that matters, not the framed one the ranking line shows). Not chased further given size (1962 bytes / ~71 mechanical get_state_id calls).
void PrefWin::set_preferences() {
  prefsBasicPending_ = morePrefsBasicPending_ = 0;
  prefsAdvPending_ = morePrefsAdvPending_ = 0;
  prefsAutoPending_ = morePrefsAutoPending_ = 0;
  prefsAVPending_ = morePrefsAVPending_ = 0;
  prefsMapPending_ = morePrefsMapPending_ = 0;
  warningsPending_ = 0;
  write_controls(this, bscPrefCheckBox_, kBasic, sizeof(kBasic) / sizeof(kBasic[0]));
  write_controls(this, advPrefCheckBox_, kAdvanced, sizeof(kAdvanced) / sizeof(kAdvanced[0]));
  write_controls(this, autoPrefCheckBox_, kAutomation, sizeof(kAutomation) / sizeof(kAutomation[0]));
  write_controls(this, audioVisPrefCheckBox_, kAudioVisual, sizeof(kAudioVisual) / sizeof(kAudioVisual[0]));
  if (volumeVoiceToggle_.field_A18_)
    morePrefsAVPending_ |= MPREF_AV_VOICEOVER_TECH_FAC;
  if (volumeSFXToggle_.field_A18_)
    prefsAVPending_ |= PREF_AV_VOLUME_SFX_TOGGLE;
  if (volumeMusicToggle_.field_A18_)
    prefsAVPending_ |= PREF_AV_VOLUME_MUSIC_TOGGLE;
  write_controls(this, mapPrefCheckBox_, kMap, sizeof(kMap) / sizeof(kMap[0]));
  for (unsigned int warning_index = 0;
       warning_index < sizeof(kWarnings) / sizeof(kWarnings[0]); ++warning_index)
    if (warnPrefCheckBox_.get_state_id(kWarnings[warning_index].id))
      warningsPending_ |= kWarnings[warning_index].bit;
  if (warnPrefCheckBox_.get_state_id(0x35)) warningsPending_ |= 0x100;
  for (unsigned int continue_index = 0;
       continue_index < sizeof(kWarningContinue) / sizeof(kWarningContinue[0]); ++continue_index)
    if (warnContPrefCheckBox_.get_state_id(kWarningContinue[continue_index].id))
      warningsPending_ |= kWarningContinue[continue_index].bit;
}

// ORIGINAL: 0x00490B00 ?get_preferences@PrefWin@@QAEXXZ 0x00490B00-0x004911FD FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00490b00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// TRIED: 7 do/while loops over switch(i){id,flag} pairs calling CheckBox::set_state_id, matching Ghidra's structure; the three byte-indexed jump-table dispatches (0x490DCE/0x490F03, whose table bytes are not in the brief) were simplified to a uniform field test per loop, dropping the change_volume_button_text special cases - MISMATCH #2 (prologue) not chased further.
void PrefWin::get_preferences() {
  read_controls(this, bscPrefCheckBox_, kBasic, sizeof(kBasic) / sizeof(kBasic[0]));
  read_controls(this, advPrefCheckBox_, kAdvanced, sizeof(kAdvanced) / sizeof(kAdvanced[0]));
  read_controls(this, autoPrefCheckBox_, kAutomation, sizeof(kAutomation) / sizeof(kAutomation[0]));
  // The original reads these three volume controls separately; they are
  // not items in the eight-entry audio/visual CheckBox.
  volumeVoiceToggle_.field_A18_ =
      (morePrefsAVPending_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0;
  change_volume_button_text(0x57);
  volumeSFXToggle_.field_A18_ =
      (prefsAVPending_ & PREF_AV_VOLUME_SFX_TOGGLE) != 0;
  change_volume_button_text(0x56);
  volumeMusicToggle_.field_A18_ =
      (prefsAVPending_ & PREF_AV_VOLUME_MUSIC_TOGGLE) != 0;
  change_volume_button_text(0x58);
  read_controls(this, audioVisPrefCheckBox_, kAudioVisual, sizeof(kAudioVisual) / sizeof(kAudioVisual[0]));
  read_controls(this, mapPrefCheckBox_, kMap, sizeof(kMap) / sizeof(kMap[0]));
  for (unsigned int warning_index = 0;
       warning_index < sizeof(kWarnings) / sizeof(kWarnings[0]); ++warning_index)
    warnPrefCheckBox_.set_state_id(kWarnings[warning_index].id,
        (warningsPending_ & kWarnings[warning_index].bit) != 0);
  for (unsigned int continue_index = 0;
       continue_index < sizeof(kWarningContinue) / sizeof(kWarningContinue[0]); ++continue_index)
    warnContPrefCheckBox_.set_state_id(kWarningContinue[continue_index].id,
        (warningsPending_ & kWarningContinue[continue_index].bit) != 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491380 ?on_scrolling@PrefWin@@QAEXHH@Z 0x00491380-0x004913E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491380.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::on_scrolling(int control_id, int position) {
  switch (control_id) {
    case 0x50: WaveDeviceGlobal.set_volume(position); break;
    case 0x51: WaveDeviceGlobal.set_group_volume(0, position); break;
    case 0x52: WaveDeviceGlobal.set_group_volume(1, position); break;
    case 0x53: if (FactionMusic) FactionMusic->set_volume(position); break;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491400 ?on_button_clicked@PrefWin@@QAEXH@Z 0x00491400-0x00491794;0x00658AC0-0x00658B6C FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491400.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_button_clicked@PrefWin@@UAEXH@Z
void PrefWin::on_button_clicked(int button_id) {
  Gamma gamma;
  switch (button_id) {
    case 6:
      g_PALETTE1.copy_from(&g_PALETTE2);
      PrefWinGammaCorrection = static_cast<float>(
          gamma.exec(&g_PALETTE1, PrefWinGammaCorrection, 0));
      prefs_put("Gamma Correction",
          static_cast<int>(PrefWinGammaCorrection * PrefWinGammaScale), 0);
      break;
    case 0x5A: // Restore the rule and warning defaults; retain live UI controls.
      prefsBasicPending_ = default_prefs();
      prefsAdvPending_ = prefsAutoPending_ = prefsAVPending_ = prefsMapPending_ = prefsBasicPending_;
      morePrefsBasicPending_ = default_prefs2();
      morePrefsAdvPending_ = morePrefsAutoPending_ = morePrefsAVPending_ = morePrefsMapPending_ = morePrefsBasicPending_;
      warningsPending_ = default_warn();
      volumeMainToggle_.field_A18_ = 1;
      volumeVoiceToggle_.field_A18_ = 1;
      volumeSFXToggle_.field_A18_ = 1;
      volumeMusicToggle_.field_A18_ = 1;
      volumeMainScroll_.set_pos(0x7F);
      WaveDeviceGlobal.set_volume(0x7F);
      volumeVoiceScroll_.set_pos(0x7F);
      WaveDeviceGlobal.set_group_volume(0, 0x7F);
      volumeSFXScroll_.set_pos(0x5F);
      WaveDeviceGlobal.set_group_volume(1, 0x5F);
      volumeMusicScroll_.set_pos(0x3F);
      if (FactionMusic) FactionMusic->set_volume(0x3F);
      get_preferences();
      break;
    case 0x5B:
      field_A5C_ = 0;
      destroy();
      break;
    case 0x5C:
      field_A5C_ = 1;
      destroy();
      break;
    case 0x5E:
      field_A18_ = field_A14_;
      field_A14_ = field_A14_ == 2 ? 6 : 2;
      hide_option_buttons();
      break;
  }
  change_volume_button_state(button_id);
  change_volume_button_text(button_id);
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491810 ?redraw_option@PrefWin@@QAEXXZ 0x00491810-0x00491857 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491810.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::redraw_option() {
  RECT rect = {PrefWinGeometryValue_85789C, PrefWinGeometryValue_857898,
      PrefWinGeometryValue_8578A8 + PrefWinGeometryValue_85789C,
      PrefWinGeometryValue_8578A4 + PrefWinGeometryValue_857898};
  Buffer::box_sprite(&rect, &InterfaceBoxStyles[18]);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491860 ?redraw_volume@PrefWin@@QAEXXZ 0x00491860-0x004918BA FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491860.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::redraw_volume() {
  if (field_A14_ != 4) return;
  RECT rect = {PrefWinGeometryValue_8577D8,
      PrefWinGeometryValue_8577DC - PrefWinNarrowDisplayVerticalOffset,
      PrefWinGeometryValue_8577E4 + PrefWinGeometryValue_8577D8,
      PrefWinGeometryValue_8577E0 + PrefWinGeometryValue_8577DC -
          PrefWinNarrowDisplayVerticalOffset};
  Buffer::box_sprite(&rect, &InterfaceBoxStyles[9]);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004918C0 ?change_volume_button_text@PrefWin@@QAEXH@Z 0x004918C0-0x004919DD FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004918c0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::change_volume_button_text(int button_id) {
  FlatButton *button = nullptr;
  int offset = 0;
  switch (button_id) {
    case 0x55:
      button = &volumeMainToggle_;
      offset = volumeMainToggle_.field_A18_ ? 0x10E4 : 0x10E0;
      break;
    case 0x57:
      button = &volumeVoiceToggle_;
      offset = volumeVoiceToggle_.field_A18_ ? 0x10EC : 0x10E8;
      break;
    case 0x56:
      button = &volumeSFXToggle_;
      offset = volumeSFXToggle_.field_A18_ ? 0x10F4 : 0x10F0;
      break;
    case 0x58:
      button = &volumeMusicToggle_;
      offset = volumeMusicToggle_.field_A18_ ? 0x10FC : 0x10F8;
      break;
  }
  if (!button) return;
  const char *text = label_get(label_index(offset));
  set_control_text(*button, text);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004919F0 ?change_volume_button_state@PrefWin@@QAEXH@Z 0x004919F0-0x00491BE9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004919f0.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
void PrefWin::change_volume_button_state(int button_id) {
  switch (button_id) {
    case 0x57:
      if (volumeVoiceToggle_.field_A18_) {
        volumeVoiceToggle_.field_A18_ = 0;
        WaveDeviceGlobal.disable_group(0);
        return;
      }
      volumeVoiceToggle_.field_A18_ = 1;
      if (!volumeMainToggle_.field_A18_) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      WaveDeviceGlobal.enable_group(0);
      return;
    case 0x56:
      if (volumeSFXToggle_.field_A18_) {
        WaveDeviceGlobal.disable_group(1);
        volumeSFXToggle_.field_A18_ = 0;
        return;
      }
      volumeSFXToggle_.field_A18_ = 1;
      if (!volumeMainToggle_.field_A18_) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      WaveDeviceGlobal.enable_group(1);
      return;
    case 0x58:
      if (volumeMusicToggle_.field_A18_) {
        if (FactionMusic) {
          delete FactionMusic;
          FactionMusic = nullptr;
        }
        volumeMusicToggle_.field_A18_ = 0;
        return;
      }
      volumeMusicToggle_.field_A18_ = 1;
      if (!volumeMainToggle_.field_A18_) {
        volumeMainToggle_.field_A18_ = 1;
        change_volume_button_text(0x55);
      }
      prefsAVPending_ |= PREF_AV_VOLUME_MUSIC_TOGGLE;
      load_music(ConsoleGlobal.faction_id_);
      if (FactionMusic) FactionMusic->begin();
      return;
    case 0x55:
      if (volumeMainToggle_.field_A18_) {
        volumeMainToggle_.field_A18_ = 0;
        volumeMusicToggle_.field_A18_ = 0;
        volumeVoiceToggle_.field_A18_ = 0;
        volumeSFXToggle_.field_A18_ = 0;
        WaveDeviceGlobal.disable();
        prefsAVPending_ &= ~0xC00u;
        morePrefsAVPending_ &= ~0x4000u;
      } else {
        volumeMainToggle_.field_A18_ = 1;
        volumeMusicToggle_.field_A18_ = 1;
        volumeVoiceToggle_.field_A18_ = 1;
        volumeSFXToggle_.field_A18_ = 1;
        WaveDeviceGlobal.enable();
      }
      change_volume_button_text(0x58);
      change_volume_button_text(0x57);
      change_volume_button_text(0x56);
      return;
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00491C00 ?on_redraw@PrefWin@@QAEXXZ 0x00491C00-0x00491DEE FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00491c00.cpp", "recovery/tasks/prefwin-owner-coverage-wave.json", "Original control parent 8578D8; ctor/destructor member order, 800-pixel branch, checkbox text-context and callback order independently reviewed."], "reason": "Recover the complete PrefWin ownership group through a genuine GraphicWin base and compiler-owned embedded control lifetimes; preserve seed identity, original spans and unresolved discoveries without granting byte claims."}
// symbol    ?on_redraw@PrefWin@@UAEXXZ
void PrefWin::on_redraw() {
  redraw_option();
  redraw_volume();
  hide_option_buttons();
  update_pref_buttons();
  bscPrefCheckBox_.on_redraw();
  advPrefCheckBox_.on_redraw();
  warnPrefCheckBox_.on_redraw();
  warnContPrefCheckBox_.on_redraw();
  autoPrefCheckBox_.on_redraw();
  audioVisPrefCheckBox_.on_redraw();
  mapPrefCheckBox_.on_redraw();
}
