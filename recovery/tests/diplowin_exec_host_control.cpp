#include "diplowin_exec_host_types.h"
#include <assert.h>
#include <stdio.h>

EventLog DiploControlEvents;
DiploWin DiploWindow;
MainInterface MainInterfaceGlobal;
Console ConsoleGlobal;
SubInterface *InterfaceRegistry[12];
BoxSpriteParams InterfaceBoxStyles[51];
int DiploDefaultFlag93A968;
int WinScreenWidth = 1024, WinScreenHeight = 768;
int IsMultiplayerPBEM, IsMultiplayerNet;
// Narrow behavioral double: only fields accessed by the extracted methods.
// It models the current field path, not the complete owner layout or ABI.
struct GameDataLockMasksHostDouble { uint32_t enable_mask_; };
struct GameDataFieldsHostDouble {
    int turn_control_duration_milliseconds_;
    GameDataLockMasksHostDouble lock_masks_;
    uint32_t net_turn_flags_;
    int net_turn_faction_;
    int turn_current_num_;
};
struct GameDataStorageHostDouble { GameDataFieldsHostDouble fields_; };
GameDataStorageHostDouble GameDataStateStorage = {{100, {0}, 0, 0, 0}};

int CommlinkPending, DiploOffersDirty;
uint32_t PendingDiplomacyFlags[8][8];
int ChatItemIdTable[8], ChatShadowColors[8];
Label Labels;
StringTableMock StringTable;
OfferResetCell OfferResetCells[8][8];
StringList DiplomacyTranscriptLists[8][8];
int HistoryStageCalls, MessageDataCalls, MessageDataOpcode, MessageDataTarget;
int RandValues[32], RandValueCount, RandValueIndex;

extern "C" int rand(void) {
  if (RandValueIndex < RandValueCount)
    return RandValues[RandValueIndex++];
  return 0;
}

/* DIPLOWIN_EXEC_FUNCTIONS */
/* DIPLOWIN_TRANSCRIPT_FUNCTION */

static RECT rect(int left, int top, int right, int bottom) {
  RECT value = {left, top, right, bottom};
  return value;
}
static void expect_rect(const RECT &actual, int left, int top,
                        int right, int bottom) {
  assert(actual.left == left && actual.top == top
         && actual.right == right && actual.bottom == bottom);
}
static void seed_button_sentinels(DiploWin &win) {
  win.button_rects_[0] = rect(-101, -102, -103, -104);
  win.button_rects_[1] = rect(-201, -202, -203, -204);
}
static void expect_button_sentinels(const DiploWin &win) {
  expect_rect(win.button_rects_[0], -101, -102, -103, -104);
  expect_rect(win.button_rects_[1], -201, -202, -203, -204);
}
static void expect_default_geometry(const DiploWin &win) {
  assert(win.init_left == 112 && win.init_top == 172
         && win.init_width == 832 && win.init_height == 376);
  expect_rect(win.rect_EDC_, 23, 0, 777, 376);
  expect_rect(win.rect_F4C_, 30, 7, 770, 27);
  expect_rect(win.leader_rect_[0], 30, 30, 399, 50);
  expect_rect(win.leader_rect_[1], 401, 30, 770, 50);
  expect_rect(win.button_rects_[2], 130, 330, 308, 350);
  expect_rect(win.button_rects_[3], 310, 330, 488, 350);
  expect_rect(win.button_rects_[4], 490, 330, 668, 350);
  expect_rect(win.button_rects_[5], 30, 278, 212, 298);
  expect_rect(win.button_rects_[6], 214, 278, 396, 298);
  expect_rect(win.button_rects_[7], 30, 300, 212, 320);
  expect_rect(win.button_rects_[8], 214, 300, 396, 320);
  expect_rect(win.button_rects_[9], 404, 278, 586, 298);
  expect_rect(win.button_rects_[10], 588, 278, 770, 298);
  expect_rect(win.button_rects_[11], 404, 300, 586, 320);
  expect_rect(win.button_rects_[12], 588, 300, 770, 320);
  expect_rect(win.rect_F9C_, 30, 256, 396, 276);
  expect_rect(win.rect_FBC_, 404, 256, 770, 276);
  expect_rect(win.rect_FAC_, 27, 253, 399, 323);
  expect_rect(win.rect_FCC_, 401, 253, 773, 323);
  expect_rect(win.portrait_rect_[0], 30, 53, 399, 246);
  expect_rect(win.portrait_rect_[1], 401, 53, 770, 246);
  expect_rect(win.rect_EEC_, 107, 87, 693, 107);
  expect_rect(win.rect_EFC_, 107, 110, 693, 593);
  expect_rect(win.rect_F0C_, 727, 87, 1193, 107);
  expect_rect(win.rect_F1C_, 727, 110, 1193, 593);
  expect_rect(win.rect_F3C_, 14, 24, 496, 373);
  expect_rect(win.rect_F2C_, 14, 376, 496, 396);
  expect_button_sentinels(win);
}
static void expect_signed_edge_geometry(const DiploWin &win) {
  // Native's signed half calculation yields 0 for the -1 width difference.
  assert(win.init_left == 0 && win.init_top == 5
         && win.init_width == 832 && win.init_height == 395);
  expect_rect(win.rect_EDC_, 23, 0, 777, 395);
  expect_rect(win.rect_F4C_, 30, 7, 770, 27);
  expect_rect(win.leader_rect_[0], 30, 30, 399, 50);
  expect_rect(win.leader_rect_[1], 401, 30, 770, 50);
  expect_rect(win.button_rects_[2], 130, 349, 308, 369);
  expect_rect(win.button_rects_[3], 310, 349, 488, 369);
  expect_rect(win.button_rects_[4], 490, 349, 668, 369);
  expect_rect(win.button_rects_[5], 30, 297, 212, 317);
  expect_rect(win.button_rects_[6], 214, 297, 396, 317);
  expect_rect(win.button_rects_[7], 30, 319, 212, 339);
  expect_rect(win.button_rects_[8], 214, 319, 396, 339);
  expect_rect(win.button_rects_[9], 404, 297, 586, 317);
  expect_rect(win.button_rects_[10], 588, 297, 770, 317);
  expect_rect(win.button_rects_[11], 404, 319, 586, 339);
  expect_rect(win.button_rects_[12], 588, 319, 770, 339);
  expect_rect(win.rect_F9C_, 30, 275, 396, 295);
  expect_rect(win.rect_FBC_, 404, 275, 770, 295);
  expect_rect(win.rect_FAC_, 27, 272, 399, 342);
  expect_rect(win.rect_FCC_, 401, 272, 773, 342);
  expect_rect(win.portrait_rect_[0], 30, 53, 399, 265);
  expect_rect(win.portrait_rect_[1], 401, 53, 770, 265);
  expect_rect(win.rect_EEC_, 107, 87, 693, 107);
  expect_rect(win.rect_EFC_, 107, 110, 693, 593);
  expect_rect(win.rect_F0C_, 727, 87, 1193, 107);
  expect_rect(win.rect_F1C_, 727, 110, 1193, 593);
  expect_rect(win.rect_F3C_, 14, 24, 496, 373);
  expect_rect(win.rect_F2C_, 14, 376, 496, 396);
  expect_button_sentinels(win);
}

int main() {
  MainInterfaceGlobal.main_rect.left = 100;
  MainInterfaceGlobal.main_rect.top = 80;
  MainInterfaceGlobal.main_rect.right = 700;
  MainInterfaceGlobal.main_rect.bottom = 600;
  MainInterfaceGlobal.right_rect.left = 720;
  MainInterfaceGlobal.right_rect.top = 80;
  MainInterfaceGlobal.right_rect.right = 1200;
  MainInterfaceGlobal.right_rect.bottom = 600;
  MainInterfaceGlobal.rect1_.left = 10;
  MainInterfaceGlobal.rect1_.top = 20;
  MainInterfaceGlobal.rect1_.right = 500;
  MainInterfaceGlobal.rect1_.bottom = 400;

  // Odd random selector chooses energy; subsequent selector 1 falls through
  // to offer types 3 and 2. The final selector is 2.
  RandValues[0] = 1; RandValues[1] = 1; RandValues[2] = 1;
  RandValues[3] = 2; RandValues[4] = 0; RandValues[5] = 2;
  RandValueCount = 6;
  DiploWin win;
  seed_button_sentinels(win);
  win.exec(2, 5);
  expect_default_geometry(win);
  assert(win.call_count == 3);
  assert(win.calls[0] == ((1 << 24) | (1 << 16) | 50));
  assert(win.calls[1] == ((1 << 24) | (3 << 16)));
  assert(win.calls[2] == ((1 << 24) | (2 << 16)));
  assert(MessageDataCalls == 1 && MessageDataOpcode == 0x1505
         && MessageDataTarget == 5);
  assert(InterfaceRegistry[11] == static_cast<SubInterface *>(&win));

  // Even random selector chooses technology. A zero switch selector
  // intentionally falls through offer types 4, 3, then 2.
  RandValues[0] = 2; RandValues[1] = 7; RandValues[2] = 1; RandValues[3] = 0;
  RandValueCount = 4; RandValueIndex = 0;
  DiploWin second;
  WinScreenWidth = 799; WinScreenHeight = 601;
  MainInterfaceGlobal.panel_height_value = 201;
  seed_button_sentinels(second);
  second.exec(1, 6);
  expect_signed_edge_geometry(second);
  WinScreenWidth = 1024; WinScreenHeight = 768;
  MainInterfaceGlobal.panel_height_value = 220;
  assert(second.call_count == 5);
  assert(second.calls[0] == ((1 << 24) | 7));
  assert(second.calls[1] == ((1 << 24) | 0));
  assert(second.calls[2] == ((1 << 24) | (4 << 16)));
  assert(second.calls[3] == ((1 << 24) | (3 << 16)));
  assert(second.calls[4] == ((1 << 24) | (2 << 16)));
  assert(second.field_A28_ == 1);

  // Pair ordering is asymmetric: saved dialogue uses [low][high], while
  // offer reset state uses the original faction order [current][counterpart].
  IsMultiplayerPBEM = 1;
  MessageDataCalls = 0; CommlinkPending = 0;
  OfferResetCell &cell = OfferResetCells[5][2];
  cell.count_[0] = 2; cell.count_[1] = 1;
  cell.state_[0] = 4; cell.state_[1] = 5;
  cell.type_[0][0] = 1; cell.type_[0][1] = 4;
  cell.value_[0][0] = 10; cell.value_[0][1] = 20;
  cell.type_[1][0] = 3; cell.value_[1][0] = 30;
  StringList &saved = DiplomacyTranscriptLists[2][5];
  StringList &opposite = DiplomacyTranscriptLists[5][2];
  opposite.pending_text_ = const_cast<char *>("opposite"); opposite.add(99);
  saved.pending_text_ = const_cast<char *>("first"); saved.add(11);
  saved.pending_text_ = const_cast<char *>("second"); saved.add(12);
  saved.pending_text_ = const_cast<char *>("third"); saved.add(13);
  saved.pending_text_ = const_cast<char *>("fourth"); saved.add(14);
  DiploWindow.exec(5, 2);
  assert(DiploWindow.offer_count_[0] == 2 && DiploWindow.offer_count_[1] == 1);
  assert(DiploWindow.field_A24_ == 4 && DiploWindow.field_A28_ == 5);
  assert(cell.type_[0][1] == 4 && cell.value_[1][0] == 30);
  assert(saved.entry_count_ == 7 && saved.current_id() == 14);
  assert(opposite.entry_count_ == 1 && opposite.current_id() == 99);
  StringList &visible = DiploWindow.stringBox_.strings();
  assert(visible.entry_count_ == 3);
  assert(visible.seek_pos(0) && visible.current_id() == 12);
  assert(visible.seek_pos(1) && visible.current_id() == 13);
  assert(visible.seek_pos(2) && visible.current_id() == 14);
  assert(MessageDataCalls == 1 && MessageDataTarget == 2);

  BufferWriteStringsCallback callback = &DiploWin::string_callback;
  assert(DiploWindow.stringBox_.field_C_ == callback);
  ChatItemIdTable[3] = 77; ChatShadowColors[3] = 88;
  callback(3, 0x1234);
  assert(DiploWindow.stringBox_.last_text_color[0] == 77);
  assert(DiploWindow.stringBox_.last_text_color[1] == 88);
  assert(DiploWindow.stringBox_.last_text_color[2] == 1);
  assert(DiploWindow.stringBox_.last_text_color[3] == 1);
  return 0;
}
