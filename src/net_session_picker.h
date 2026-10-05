#pragma once

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

#include "alphanet.h"
#include "font.h"
#include "graphicwin.h"
#include "heap.h"
#include "listbox.h"
#include "net_class.h"
#include "pushbutton.h"
#include "time.h"

// Descriptive, provisional name: the original class name is not established.
// Field/member order is from the pinned constructor/destructor and the
// corresponding owner-body accesses. The last accessed word is +0x37A4;
// that proves an observed prefix, not the complete original allocation.
struct NetSessionPickerTail {
  RECT panel_rect_;             // owner +0x36E8, Dialog::init bounds
  RECT first_label_rect_;       // owner +0x36F8
  uint32_t field_3708_;
  uint32_t field_370c_;
  uint32_t field_3710_;
  uint32_t field_3714_;
  uint32_t field_3718_;
  RECT frame_rect_;             // owner +0x371C
  uint32_t frame_color_;        // owner +0x372C
  uint32_t box_color_a_;        // owner +0x3730
  uint32_t box_color_b_;        // owner +0x3734
  int row_count_;               // owner +0x3738
  uint32_t field_373c_;
  uint32_t field_3740_;
  uint32_t field_3744_;
  uint32_t field_3748_;
  RECT rows_rect_;              // owner +0x374C
  RECT second_label_rect_;      // owner +0x375C
  int row_height_;              // owner +0x376C
  uint32_t selected_row_color_; // owner +0x3770
  uint32_t row_box_color_a_;    // owner +0x3774
  uint32_t row_box_color_b_;    // owner +0x3778
  int boxes_per_row_;           // owner +0x377C
  uint32_t field_3780_;
  uint32_t selected_text_color_a_; // owner +0x3784
  uint32_t selected_text_color_b_; // owner +0x3788
  uint32_t selected_text_color_c_; // owner +0x378C
  uint32_t selected_text_color_d_; // owner +0x3790
  uint32_t field_3794_;
  int client_width_;            // owner +0x3798
  int client_height_;           // owner +0x379C
  int rows_visible_;            // owner +0x37A0
  int action_result_;           // owner +0x37A4
};

static_assert(sizeof(NetSessionPickerTail) == 0xC0,
              "tail fields cover the observed owner interval");
static_assert(offsetof(NetSessionPickerTail, first_label_rect_) == 0x10,
              "first label rectangle offset");
static_assert(offsetof(NetSessionPickerTail, frame_rect_) == 0x34,
              "frame rectangle offset");
static_assert(offsetof(NetSessionPickerTail, rows_rect_) == 0x64,
              "rows rectangle offset");
static_assert(offsetof(NetSessionPickerTail, row_count_) == 0x50,
              "row count offset");
static_assert(offsetof(NetSessionPickerTail, action_result_) == 0xBC,
              "last observed owner word offset");

// RECOVERY_BASE: v1 NetSessionPickerWin GraphicWin 0x0 0x0063BB20
class NetSessionPickerWin : public GraphicWin {
 public:
  NetSessionPickerWin();
  virtual ~NetSessionPickerWin();

  // Original table 0x670DDC slot 14 -> 0x63C9D0.
  virtual void vslot_14();
  // Original table 0x670DDC slot 44 -> 0x63C3A0; `ret 4` confirms this API.
  virtual void on_button_clicked(int action);

  // Original 0x63BF10. Name and virtual slot identity remain unresolved.
  int initialize(Net *network, char *caption, Font *font,
                 int width, int height, int rows);
  // Original 0x63BD10 reusable UI close/reset; not the complete destructor.
  void close();
  // Original 0x63C510 interactive create/join helper.
  int connect_session();
  // Original 0x63C7C0 timer/list refresh.
  void refresh_player_list();
  // Original 0x63CAA0 selected-row rendering helper.
  void draw_selected_rows();
  // Original 0x63CD50 outer frame/fill loop.
  void draw_frame();
  // Original 0x63C340 timer-start operation.
  int start_refresh_timer();
  // Original 0x63C4E0 timer callback bridge, with original integer ABI.
  static void __cdecl refresh_timer_callback(int context);
  // Separate nullary thiscall identity at 0x63C390; no virtual slot has been
  // established, so keep this as an ordinary member pending dispatch proof.
  void refresh_if_focused();

  // Source members in the order independently fixed by constructor offsets.
  // RECOVERY_MEMBER: v2 NetSessionPickerWin refresh_timer_ Time 0xA14 0x0063BB20
  Time refresh_timer_;
  // RECOVERY_MEMBER: v2 NetSessionPickerWin row_heap_ Heap 0xA3C 0x0063BB20
  Heap row_heap_;
  // RECOVERY_MEMBER: v2 NetSessionPickerWin sessions_view_ ListBox 0xA50 0x0063BB20
  ListBox sessions_view_;
  // RECOVERY_MEMBER: v2 NetSessionPickerWin create_button_ PushButton 0x15A4 0x0063BB20
  PushButton create_button_; // owner +0x15A4; native action ID 0
  // RECOVERY_MEMBER: v2 NetSessionPickerWin action_button_ PushButton 0x20A4 0x0063BB20
  PushButton action_button_; // owner +0x20A4; native action ID -1
  // RECOVERY_MEMBER: v2 NetSessionPickerWin cancel_button_ PushButton 0x2BA4 0x0063BB20
  PushButton cancel_button_;
  // RECOVERY_MEMBER: v3 NetSessionPickerWin net_ Net* 0x36A4 0x0063BF10
  Net *net_;
  // RECOVERY_MEMBER: v2 NetSessionPickerWin sessions_ AlphaNetLedger 0x36A8 0x0063BB20
  AlphaNetLedger sessions_;
  // RECOVERY_MEMBER: v1 NetSessionPickerWin tail_ 0x36E8 0x0063BB20
  NetSessionPickerTail tail_;
};
