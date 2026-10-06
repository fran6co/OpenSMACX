#pragma once

#include "graphicwin.h"
#include "popup.h"
#include "sprite.h"

// Provisional role name only: pinned receiver/layout evidence establishes a
// GraphicWin-derived faction profile owner, but not its historical C++ name
// or the site that constructs/registers it.
// RECOVERY_BASE: v1 FactionProfileWin GraphicWin 0x0 0x00589760
class FactionProfileWin : public GraphicWin {
 public:
  void initialize(int faction_id, int profile_mode);
  void set_faction_id(int faction_id);
  int show_profile();

 private:
  // RECOVERY_MEMBER: v1 FactionProfileWin field_A14_ 0xA14 0x00589760
  uint32_t field_A14_;  // 0xA14
  // RECOVERY_MEMBER: v1 FactionProfileWin field_A18_ 0xA18 0x00589760
  uint32_t field_A18_;  // 0xA18
  // RECOVERY_MEMBER: v1 FactionProfileWin width_ 0xA1C 0x00589760
  int width_;           // 0xA1C
  // RECOVERY_MEMBER: v1 FactionProfileWin height_ 0xA20 0x00589760
  int height_;          // 0xA20
  // RECOVERY_MEMBER: v2 FactionProfileWin bounds_ tagRECT 0xA24 0x00589760
  RECT bounds_;         // 0xA24
  // RECOVERY_MEMBER: v1 FactionProfileWin faction_id_ 0xA34 0x00589760
  int faction_id_;      // 0xA34
  // RECOVERY_MEMBER: v1 FactionProfileWin profile_mode_ 0xA38 0x00589760
  int profile_mode_;    // 0xA38
  // RECOVERY_MEMBER: v2 FactionProfileWin popup_ Popup 0xA3C 0x00589560
  Popup popup_;         // 0xA3C
};

static_assert(sizeof(FactionProfileWin) == 0x5DB8,
              "proposed faction profile layout from pinned receiver offsets");
