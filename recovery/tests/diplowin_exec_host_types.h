#ifndef DIPLO_EXEC_MOCK_H
#define DIPLO_EXEC_MOCK_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#ifndef __cdecl
#define __cdecl
#endif

struct RECT { int32_t left, top, right, bottom; };
inline int UnionRect(RECT *out, const RECT *a, const RECT *b) {
  const RECT left = *a;
  const RECT right = *b;
  const int left_empty = left.left >= left.right || left.top >= left.bottom;
  const int right_empty = right.left >= right.right || right.top >= right.bottom;
  if (left_empty && right_empty) {
    out->left = out->top = out->right = out->bottom = 0;
    return 0;
  }
  if (left_empty) {
    *out = right;
    return 1;
  }
  if (right_empty) {
    *out = left;
    return 1;
  }
  out->left = left.left < right.left ? left.left : right.left;
  out->top = left.top < right.top ? left.top : right.top;
  out->right = left.right > right.right ? left.right : right.right;
  out->bottom = left.bottom > right.bottom ? left.bottom : right.bottom;
  return 1;
}

typedef void (__cdecl *BufferWriteStringsCallback)(int, int);
struct Font {};
struct BoxSpriteParams {};
struct EventLog {
  int events[128]; int count;
  EventLog() : count(0) {}
  void add(int event) { events[count++] = event; }
};
extern EventLog DiploControlEvents;

struct Buffer {
  void set_write_strings_callback(BufferWriteStringsCallback callback) {
    field_C_ = callback; DiploControlEvents.add(10);
  }
  BufferWriteStringsCallback field_C_;
  int last_text_color[4];
  void set_text_color(int a, int b, int c, int d) {
    last_text_color[0] = a; last_text_color[1] = b;
    last_text_color[2] = c; last_text_color[3] = d;
    DiploControlEvents.add(11);
  }
  void set_font(Font *, Font *, Font *, Font *) { DiploControlEvents.add(12); }
  int box_sprite(RECT *, BoxSpriteParams *) { DiploControlEvents.add(13); return 0; }
};
struct StringStructPayload { char *text; };
struct StringList {
  int entry_count_, current_position_;
  char *pending_text_;
  uint32_t field_20_, field_24_;
  int ids[32]; char *texts[32];
  StringList() : entry_count_(0), current_position_(0), pending_text_(0), field_20_(0), field_24_(0) {}
  int seek_pos(int index) { if (index < 0 || index >= entry_count_) return 0; current_position_ = index; return 1; }
  int current_id() const { return entry_count_ ? ids[current_position_] : 0; }
  StringStructPayload *current_entry() { static StringStructPayload payload; payload.text = entry_count_ ? texts[current_position_] : 0; return entry_count_ ? &payload : 0; }
  char *current_text() { return entry_count_ ? texts[current_position_] : 0; }
  int add(int id) { if (entry_count_ == 32) return 4; ids[entry_count_] = id; texts[entry_count_] = pending_text_; current_position_ = entry_count_++; return 0; }
};
struct Win {
  virtual int set_modal(int, int (__cdecl *)(), Win *) { DiploControlEvents.add(20); return 0; }
};
struct GraphicWin : Win, Buffer {
  int init(int, int, int, int, char *, int, Win *, void *, void *) {
    DiploControlEvents.add(1); return 0;
  }
  void fill(int) { DiploControlEvents.add(2); }
  void redraw() { DiploControlEvents.add(3); }
  void close() { DiploControlEvents.add(30); }
};
struct SubInterface {
 protected: uint32_t field_4_;
 public:
  SubInterface() : field_4_(0) {}
  void release_iface_mode() { DiploControlEvents.add(28); }
  void delete_iface_mode() { DiploControlEvents.add(29); }
};
struct Spot { void init(int value) { if (value == 100) DiploControlEvents.add(0); } };
struct StringBox : GraphicWin {
  StringList list;
  StringList &strings() { return list; }
  void init(int, int, int, int, Win *, void *, int) { DiploControlEvents.add(4); }
  void clear() { list.entry_count_ = 0; DiploControlEvents.add(5); }
  void add(char *text, int id, int) { list.pending_text_ = text; list.add(id); }
};
struct EditBox : GraphicWin {
  uint32_t field_B14_, field_B1C_, field_B20_, field_B24_, field_B30_;
  int init(int, int, int, int, Win *, int) { DiploControlEvents.add(6); return 0; }
  void set_text(char *) { DiploControlEvents.add(7); }
};
struct FlatButton : GraphicWin {
  int index;
  int init(char *, int id, int, int, int, int, Win *, int) {
    index = id; DiploControlEvents.add(40 + id); return 0;
  }
  void set_text_color(int, int, int, int) {}
  void hide() {}
  void set_border_styles(BoxSpriteParams *, BoxSpriteParams *, BoxSpriteParams *) {}
};
struct MainInterface : GraphicWin {
  RECT main_rect, right_rect, lower_rect;
  int panel_height() const { return 220; }
  const RECT &main_frame_rect() const { return main_rect; }
  const RECT &right_frame_rect() const { return right_rect; }
  const RECT &main_layout_rect() const { return rect1_; }
  RECT rect1_;
};
struct Console : virtual GraphicWin {};
struct Label { char *strings_ptr; int count; };
struct StringTableMock { char *get(int) { return const_cast<char *>("mock"); } };
struct OfferResetCell {
  int count_[2]; int state_[2]; int type_[2][8]; int value_[2][8];
};
struct DiploWin : GraphicWin, SubInterface {
  Spot spot_;
  int offer_count_[2]; int field_A24_, field_A28_;
  int faction_id_, faction_id_with_;
  int field_16C_;
  int offer_type_[2][8]; int offer_value_[2][8];
  RECT rect_EDC_, rect_EEC_, rect_EFC_, rect_F0C_, rect_F1C_;
  RECT rect_F2C_, rect_F3C_, rect_F4C_, rect_F5C_, rect_F6C_;
  RECT rect_F7C_, rect_F8C_, rect_F9C_, rect_FAC_, rect_FBC_, rect_FCC_;
  RECT portrait_rect_[2], leader_rect_[2];
  RECT button_rects_[13];
  FlatButton flatButtons_[13]; StringBox stringBox_; EditBox editBox_;
  int calls[16], call_count;
  DiploWin() : field_A24_(0), field_A28_(0), faction_id_(0), faction_id_with_(0), field_16C_(0), call_count(0) {}
  static void __cdecl string_callback(int, int);
  void exec(int, int);
  StringBox &shared_string_box() { return stringBox_; }
  EditBox &shared_edit_box() { return editBox_; }
  void add_offer(int a, int b, int c) { calls[call_count++] = (a << 24) | (b << 16) | (c & 0xffff); }
};

extern DiploWin DiploWindow;
extern MainInterface MainInterfaceGlobal;
extern Console ConsoleGlobal;
extern SubInterface *InterfaceRegistry[12];
extern BoxSpriteParams InterfaceBoxStyles[51];
extern RECT DiploInitPanelRect, DiploInitScreenRect, DiploInitLowerRect;
extern int DiploDefaultFlag93A968, WinScreenWidth, WinScreenHeight;
extern int IsMultiplayerPBEM, IsMultiplayerNet, TurnControlDurationMilliseconds;
extern uint32_t LockEnableMask;
extern int CommlinkPending, DiploOffersDirty;
extern uint32_t PendingDiplomacyFlags[8][8];
extern int ChatItemIdTable[8], ChatShadowColors[8];
extern Label Labels;
extern StringTableMock StringTable;
extern OfferResetCell OfferResetCells[8][8];
extern StringList DiplomacyTranscriptLists[8][8];
extern int HistoryStageCalls, MessageDataCalls, MessageDataOpcode, MessageDataTarget;
extern int DiploOffersDirty;
inline void swap(int *left, int *right) { int value = *left; *left = *right; *right = value; }

inline char *label_get(int) { return const_cast<char *>("mock"); }
inline Font *find_font(int, int) { static Font f; return &f; }
inline int bit_count(int value) { int n = 0; while (value) { n += value & 1; value >>= 1; } return n; }
inline uint32_t __cdecl message_data(int opcode, int target, int, int, int, int) {
  ++MessageDataCalls; MessageDataOpcode = opcode; MessageDataTarget = target; return 0;
}
inline int __cdecl pop_expire() { return 0; }
extern "C" void __cdecl sub_441100(int, int);
#endif


