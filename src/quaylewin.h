#pragma once

#include "graphicwin.h"
#include "flatbutton.h"
#include "spot.h"
#include "listbox.h"
#include "buffer.h"

// Construction at 49682F enters GraphicWin; its Win branch begins at zero
// and its inherited Buffer branch at +444. These are real bases, not arrays.
// RECOVERY_BASE: v1 QuayleWin GraphicWin 0x0 0x00496810
class QuayleWin : public GraphicWin {
 public:
  QuayleWin();
  ~QuayleWin();

  int init();
  virtual void on_redraw();
  void close();
  void on_left_click(int x, int y);
  void on_selected(int selection, int unused);
  void on_button_clicked(int button_id) override;
  virtual void on_left_click_event(int x, int y) override;
  int on_key_click(int repeat, int key) override;
  virtual void vslot_53(int selection, int unused) override;

  void set_chapter(int chapter) { field_2144_ = chapter; }
  void set_page(int page) { field_2148_ = page; }

 private:
  // RECOVERY_MEMBER: v1 QuayleWin field_A14_ 0xA14 0x004951A0
  uint32_t field_A14_;
  // RECOVERY_MEMBER: v1 QuayleWin field_A18_ 0xA18 0x004951A0
  uint32_t field_A18_;
  // RECOVERY_MEMBER: v1 QuayleWin field_A1C_ 0xA1C 0x004951A0
  uint32_t field_A1C_;
  // RECOVERY_MEMBER: v1 QuayleWin field_A20_ 0xA20 0x004951A0
  uint32_t field_A20_;
  // RECOVERY_MEMBER: v2 QuayleWin window_rect_ tagRECT 0xA24 0x004951A0
  RECT window_rect_;              // +0xA24: translated canvas clip
  // RECOVERY_MEMBER: v2 QuayleWin chapter_list_rect_ tagRECT 0xA34 0x004951A0
  RECT chapter_list_rect_;        // +0xA34: multilingual chapter list/dialog
  // RECOVERY_MEMBER: v2 QuayleWin illustration_rect_ tagRECT 0xA44 0x004951A0
  RECT illustration_rect_;        // +0xA44: art buffer destination and clip
  // RECOVERY_MEMBER: v2 QuayleWin navigation_button_rect_ tagRECT 0xA54 0x004951A0
  RECT navigation_button_rect_;   // +0xA54: chapter/pages button bounds
  // RECOVERY_MEMBER: v2 QuayleWin title_rect_ tagRECT 0xA64 0x004951A0
  RECT title_rect_;               // +0xA64: centered title bounds
  // RECOVERY_MEMBER: v2 QuayleWin introduction_rect_ tagRECT 0xA74 0x004951A0
  RECT introduction_rect_;        // +0xA74: introduction text bounds
  // RECOVERY_MEMBER: v2 QuayleWin rating_rect_ tagRECT 0xA84 0x004951A0
  RECT rating_rect_;              // +0xA84: rating text bounds
  // RECOVERY_MEMBER: v2 QuayleWin text_scratch_rect_ tagRECT 0xA94 0x004956D0
  RECT text_scratch_rect_;        // +0xA94: persistent hotspot/page layout RECT
  // RECOVERY_MEMBER: v1 QuayleWin line_advance_ 0xAA4 0x004951A0
  uint32_t line_advance_;         // +0xAA4: ReportIfGlobal font line advance
  // RECOVERY_MEMBER: v1 QuayleWin monitor_mode_ 0xAA8 0x004951A0
  uint32_t monitor_mode_;         // +0xAA8: ReportIfGlobal multi-monitor mode
  // RECOVERY_MEMBER: v2 QuayleWin chapter_button_ FlatButton 0xAAC 0x00496810
  FlatButton chapter_button_;
  // RECOVERY_MEMBER: v2 QuayleWin pages_button_ FlatButton 0x15F8 0x00496810
  FlatButton pages_button_;
  // RECOVERY_MEMBER: v1 QuayleWin field_2144_ 0x2144 0x00496540
  int32_t field_2144_;
  // RECOVERY_MEMBER: v1 QuayleWin field_2148_ 0x2148 0x00496670
  int32_t field_2148_;
  // RECOVERY_MEMBER: v2 QuayleWin spot_ Spot 0x214C 0x00496810
  Spot spot_;
  // RECOVERY_MEMBER: v2 QuayleWin list_box_ ListBox 0x2158 0x00496810
  ListBox list_box_;
  // RECOVERY_MEMBER: v2 QuayleWin buffer1_ Buffer 0x2CAC 0x00496810
  Buffer buffer1_;
  // RECOVERY_MEMBER: v2 QuayleWin buffer2_ Buffer 0x3234 0x00496810
  Buffer buffer2_;
};

static_assert(sizeof(QuayleWin) == 0x37BC,
              "QuayleWin layout follows its constructor subobjects");

extern QuayleWin QuayleWinGlobal;
