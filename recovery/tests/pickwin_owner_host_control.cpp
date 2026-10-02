#include <cassert>
#include <array>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#define __cdecl
typedef long LONG;
typedef const char *LPCSTR;
struct RECT { LONG left, top, right, bottom; };
struct BoxSpriteParams { int id; };

struct Trace {
  std::vector<std::string> calls;
  int text_open_error_at = -1;
  int text_open_calls = 0;
  int custom_count = 2;
  int selected_id = -1;
  int callback_id = -1;
  int mode_status = 0;
  int popup_choice = -1;
  int read_faction_calls = 0;
  int buffer_dtor_calls = 0;
  int popup_ctor_calls = 0;
  int popup_dtor_calls = 0;
  int list_item_calls = 0;
  int list_double_callback = 0;
  int list_selection_callback = 0;
  void *dialog_attach_receiver = 0;
  bool dialog_attach_picker_state = false;
  std::vector<std::string> list_items;
  std::vector<int> list_ids;
  std::vector<std::string> menu_items;
  std::vector<std::array<LONG, 4>> extract_rectangles;
  std::vector<std::array<int, 5>> extract_xy;
} trace;

static void note(const char *s) { trace.calls.push_back(s); }
int InterfaceBoxStyles[51];
int TextColourTable[195];
char StringTemp[2048];
const unsigned char WinFillColour = 9;
LPCSTR AlphaxFileID = "ALPHAX";
struct Preferences { int faction_id; } DefaultPrefs = {6};
struct Console { int faction_id_; } ConsoleGlobal = {7};

struct Player {
  char filename[64];
  char search_key[64];
  char formal_name_faction[64];
  char desc_name_faction[64];
};
Player Players[8];

static std::string current_section;
static int text_index;
static int custom_open_count;
static std::string current_item;
int text_open(const char *, const char *section) {
  const int call = trace.text_open_calls++;
  trace.calls.push_back(std::string("text_open:") + section);
  if (call == trace.text_open_error_at) return 1;
  current_section = section;
  text_index = 0;
  if (current_section == "CUSTOMFACTIONS") ++custom_open_count;
  return 0;
}
void text_get() {
  ++text_index;
  note("text_get");
}
char *text_item() {
  static char item[64];
  std::string value;
  if (current_section == "CUSTOMFACTIONS" && custom_open_count == 1) {
    value = text_index > 0 && text_index <= trace.custom_count ? "count" : "";
  } else if (current_section == "CUSTOMFACTIONS") {
    value = "CUSTOM_" + std::to_string(text_index);
  } else if (current_section == "FACTIONS") {
    value = "FACTION_" + std::to_string(text_index);
  } else if (current_section == "NEWFACTIONS") {
    value = "NEWFACTION_" + std::to_string(text_index);
  } else if (current_section == "JENN282") {
    value = "unused";
  } else {
    value = current_section + "_LABEL";
  }
  std::strncpy(item, value.c_str(), sizeof(item) - 1);
  item[sizeof(item) - 1] = 0;
  return item;
}
char *label_get(int id) {
  static char label[64];
  std::snprintf(label, sizeof(label), "LABEL_%x", id);
  return label;
}
void CharUpperA(char *s) { for (; *s; ++s) if (*s >= 'a' && *s <= 'z') *s -= 32; }
void read_faction(int id) {
  ++trace.read_faction_calls;
  std::strcpy(Players[id].formal_name_faction, (std::string("Formal") + std::to_string(id)).c_str());
  std::strcpy(Players[id].desc_name_faction, (std::string("Desc") + std::to_string(id)).c_str());
  note("read_faction");
}
unsigned char factions_status_byte(int) { return static_cast<unsigned char>(trace.mode_status); }

struct Buffer {
  Buffer() { note("buffer_ctor"); }
  ~Buffer() { ++trace.buffer_dtor_calls; note("buffer_dtor"); }
  int load_pcx(const char *, int, int, int) { note("load_pcx"); return 0; }
  void set_font(struct Font *, void *, void *, void *) { note("set_font"); }
  void set_text_color(int, int, int, int) { note("set_text_color"); }
  int box_sprite(RECT *, int *) { note("box_sprite"); return 0; }
  int write_cent_l(char *, RECT *, int) { note("write_cent_l"); return 0; }
  void vline(int, int, int, int) { note("vline"); }
  void hline(int, int, int, int) { note("hline"); }
  int put_pixel(int, int, int) { note("put_pixel"); return 0; }
  int restore_clip() { note("restore_clip"); return 0; }
};
struct Win {
  uint32_t iFlags_ = 0;
  virtual ~Win() {}
  void hide() { note("hide"); }
  void show(int) { note("show"); }
  void get_mouse_pos(int *x, int *y) { *x = 11; *y = 12; note("get_mouse_pos"); }
  void client_to_screen(int *, int *) { note("client_to_screen"); }
  void redraw() { note("redraw"); }
  void release_modal() { note("release_modal"); }
};
struct GraphicWin : Win, Buffer {
  int init(int, int, int, int, char *, int, int, int, int) { note("graphic_init"); return 0; }
  void fill(int) { note("fill"); }
};
struct Sprite {
  Sprite() { note("sprite_ctor"); }
  ~Sprite() { note("sprite_dtor"); }
  int extract(Buffer *, unsigned char, RECT *area, int) {
    trace.extract_rectangles.push_back({area->left, area->top, area->right, area->bottom});
    note("sprite_extract_rect"); return 0;
  }
  int extract(Buffer *, unsigned char, int x, int y, int w, int h, int flags) {
    trace.extract_xy.push_back({x, y, w, h, flags});
    note("sprite_extract_xy"); return 0;
  }
  unsigned char transparent_index() const { return 9; }
  int draw(Buffer *, int, int, int) { note("sprite_draw"); return 0; }
};
struct FlatButton : GraphicWin {
  FlatButton() { note("flat_button_ctor"); }
  ~FlatButton() { note("flat_button_dtor"); }
  int init(const char *, int, int, int, int, int, Win *, int) { note("button_init"); return 0; }
  int init(int, int, int, int, int, int, Win *, int) { note("button_init_icon"); return 0; }
  void set_report_icons(Sprite *, Sprite *, Sprite *) { note("set_report_icons"); }
  void set_bubble_text(char *) { note("set_bubble_text"); }
};
struct ButtonGroup {
  uint32_t field_84_ = 0;
  ButtonGroup() { note("button_group_ctor"); }
  ~ButtonGroup() { note("button_group_dtor"); }
  void init(int, int) { note("group_init"); }
  void add(FlatButton *) { note("group_add"); }
  void set(int id, int) { field_84_ = id; note("group_set"); }
};
struct GraphicWin;
struct ListBox;
struct Dialog {
  Dialog() { note("dialog_ctor"); }
  virtual ~Dialog() { note("dialog_dtor"); }
  uint32_t field_20_ = 4;
  uint32_t field_24_ = 13;
  uint32_t field_28_ = 14;
  uint32_t field_40_ = 0, field_44_ = 0, field_48_ = 0, field_B4_ = 0;
  int init(RECT *, int) { note("dialog_init"); return 0; }
  int attach(GraphicWin *, int, int, int);
  int get_selected_id() { note("dialog_get_id"); return trace.selected_id; }
};
typedef void (*WinNotificationCallback)(int);
struct ListBox : public virtual GraphicWin, public virtual Dialog {
  ListBox() { note("listbox_ctor"); }
  ~ListBox() override { note("listbox_dtor"); }
  uint32_t field_2C_ = 0, field_14_ = 0;
  WinNotificationCallback selection_callback_ = 0;
  WinNotificationCallback double_click_callback_ = 0;
  int close() { note("list_close"); return 0; }
  void item(char *text, int id) {
    ++trace.list_item_calls;
    trace.list_items.push_back(text);
    trace.list_ids.push_back(id);
    note("list_item");
  }
  void set_selected_id(int id) { trace.selected_id = id; note("list_set_id"); }
  void set_selected_pos(int, int) { note("list_set_pos"); }
  void set_selection_callback(WinNotificationCallback cb) {
    selection_callback_ = cb;
    trace.list_selection_callback = cb != 0;
    note("list_selection_cb");
  }
  void initialize_picker_state();
};
int Dialog::attach(GraphicWin *, int, int, int) {
  trace.dialog_attach_receiver = this;
  ListBox *list = dynamic_cast<ListBox *>(this);
  trace.dialog_attach_picker_state = list != nullptr && list->field_2C_ == 0 &&
      list->field_14_ == static_cast<uint32_t>(-1) && list->iFlags_ == 0x40 &&
      field_20_ == 0;
  note("dialog_attach");
  return 0;
}
struct Dialogs {
  void item(char *text, int) { trace.menu_items.push_back(text); note("menu_item"); }
};
struct PopMenu {
  Dialogs d;
  PopMenu() { ++trace.popup_ctor_calls; note("popup_ctor"); }
  ~PopMenu() { ++trace.popup_dtor_calls; note("popup_dtor"); }
  int init() { note("popup_init"); return 0; }
  Dialogs &dialogs() { return d; }
  int exec(int, int, int) { note("popup_exec"); return trace.popup_choice; }
};
struct Font {};
struct MainInterface {
 private:
  Font font1_;
 public:
  Font *primary_font() { note("primary_font"); return &font1_; }
} MainInterfaceGlobal;
struct PopupMember {
  PopupMember() { note("popup_member_ctor"); }
  ~PopupMember() { note("popup_member_dtor"); }
};

struct PickWin : GraphicWin {
  PickWin();
  ~PickWin();
  void init(void (__cdecl *callback)(int), int mode, int faction);
  void on_redraw();
  void on_group_clicked(int source, int unused, int mode);
  void on_button_clicked(int button);
  int on_dialog_item_back_draw(GraphicWin *, int, int, RECT *);
  uint32_t field_A14_=0, field_A18_=0, field_A1C_=0, field_A20_=0;
  PopupMember popup_;
  uint32_t field_5DA0_=0, field_5DA4_=0, field_5DA8_=0, field_5DAC_=0;
  int32_t field_5DB0_=0;
  uint32_t field_5DB4_=0;
  char faction_files_[24][0x18] = {};
  char faction_menu_items_[24][0x18] = {};
  void (__cdecl *field_6238_)(int) = 0;
  uint32_t field_623C_=0, field_6240_=0, field_6244_=0, field_6248_=0;
  uint32_t field_624C_=0, field_6250_=0, field_6254_=0, field_6258_=0;
  uint32_t field_625C_=0, field_6260_=0, field_6264_=0, field_6268_=0;
  uint32_t field_626C_=0, field_6270_=0, field_6274_=0, field_6278_=0;
  uint32_t field_627C_=0, field_6280_=0, field_6284_=0, field_6288_=0;
  FlatButton flatButton1_, flatButton2_, flatButton3_;
  Sprite sprites1_[3], sprites2_[3], sprites3_[3], sprites4_[3], sprite5_;
  uint32_t field_86AC_=0, field_86B0_=0, field_86B4_=0, field_86B8_=0;
  ListBox listBox_;
  uint32_t field_9210_=0, field_9214_=0, field_9218_=0, field_921C_=0;
  uint32_t field_9220_=0, field_9224_=0, field_9228_=0, field_922C_=0;
  uint32_t field_9230_=0, field_9234_=0, field_9238_=0, field_923C_=0;
  uint32_t field_9240_=0, field_9244_=0, field_9248_=0, field_924C_=0;
  uint32_t field_9250_=0, field_9254_=0, field_9258_=0, field_925C_=0;
  uint32_t field_9260_=0, field_9264_=0, field_9268_=0, field_926C_=0;
  FlatButton flatButton4_, flatButton5_, flatButton6_, flatButton7_, flatButton8_, flatButton9_;
  RECT picker_button_rects_[7] = {};
  FlatButton flatButtons10_[7];
  ButtonGroup buttonGroup_;
};

char PickWinCaptionBuffer[1] = {};
extern PickWin PickWinGlobal;
void __cdecl list_was_dblclicked(int);

// The Python test runner replaces these markers with balanced-brace extracts
// from src/pickwin.cpp and the inline ListBox method from src/listbox.h.
// PICKWIN_OWNER_BODY_EXTRACTS
// LISTBOX_PICKER_STATE_EXTRACT
// PICKWIN_GLOBAL_EXTRACT

static int callback_count;
static void __cdecl selection_callback(int id) {
  ++callback_count;
  trace.callback_id = id;
  note("selection_callback");
}
static void reset_trace() {
  trace = Trace();
  current_section.clear();
  text_index = 0;
  custom_open_count = 0;
  callback_count = 0;
  for (int i = 0; i < 8; ++i) {
    std::memset(&Players[i], 0, sizeof(Players[i]));
  }
}
static bool has_call(const char *wanted) {
  for (const std::string &call : trace.calls) if (call == wanted) return true;
  return false;
}
static int call_index(const char *wanted) {
  for (int i = 0; i < static_cast<int>(trace.calls.size()); ++i)
    if (trace.calls[static_cast<size_t>(i)] == wanted) return i;
  return -1;
}
static std::vector<std::string> member_lifecycle_calls() {
  std::vector<std::string> out;
  for (const std::string &call : trace.calls) {
    if (call == "popup_member_ctor" || call == "popup_member_dtor" ||
        call == "flat_button_ctor" || call == "flat_button_dtor" ||
        call == "sprite_ctor" || call == "sprite_dtor" ||
        call == "listbox_ctor" || call == "listbox_dtor" ||
        call == "button_group_ctor" || call == "button_group_dtor")
      out.push_back(call);
  }
  return out;
}

int main() {
  // All three entry-time resource failures return normally and destroy the
  // local Buffer once, without initializing the window or controls.
  reset_trace(); trace.text_open_error_at = 0;
  PickWin early;
  early.init(selection_callback, 0, 0);
  assert(trace.buffer_dtor_calls == 1 && !has_call("graphic_init"));

  reset_trace(); trace.text_open_error_at = 1;
  early.init(selection_callback, 0, 0);
  assert(trace.buffer_dtor_calls == 1 && !has_call("graphic_init"));

  reset_trace(); trace.text_open_error_at = 2;
  early.init(selection_callback, 0, 0);
  assert(trace.buffer_dtor_calls == 1 && !has_call("graphic_init"));

  reset_trace(); trace.text_open_error_at = 3;
  early.init(selection_callback, 0, 0);
  assert(trace.buffer_dtor_calls == 1 && !has_call("graphic_init"));

  // Successful initialization validates both callbacks, 14 base strings,
  // two custom names, 16 localized menu labels, geometry extraction and the
  // final callback/redraw order using the actual candidate body text.
  reset_trace(); trace.custom_count = 2;
  early.init(selection_callback, 0, 3);
  assert(early.field_5DB0_ == 2);
  assert(std::strcmp(early.faction_files_[0], "FACTION_1") == 0);
  assert(std::strcmp(early.faction_files_[7], "NEWFACTION_1") == 0);
  assert(std::strcmp(early.faction_files_[14], "CUSTOM_1") == 0);
  assert(std::strcmp(early.faction_menu_items_[0], "FACTION_1_LABEL") == 0);
  assert(std::strcmp(early.faction_menu_items_[15], "CUSTOM_2_LABEL") == 0);
  assert(early.listBox_.double_click_callback_ == list_was_dblclicked);
  const std::array<LONG, 4> expected_rects[12] = {
    {1,1,259,91}, {260,1,518,91}, {519,1,777,91},
    {1,92,259,182}, {260,92,518,182}, {519,92,777,182},
    {1,183,259,273}, {260,183,518,273}, {519,183,777,273},
    {1,313,17,329}, {1,330,17,346}, {1,347,17,363}
  };
  assert(trace.extract_rectangles.size() == 12);
  for (int i = 0; i < 12; ++i) assert(trace.extract_rectangles[i] == expected_rects[i]);
  assert(trace.extract_xy.size() == 1);
  assert((trace.extract_xy[0] == std::array<int, 5>{1,364,7,13,0}));
  assert(early.listBox_.selection_callback_ == 0);
  assert(early.listBox_.field_2C_ == 0);
  assert(early.listBox_.field_14_ == static_cast<uint32_t>(-1));
  assert(early.listBox_.iFlags_ == 0x40);
  assert(callback_count == 0);
  assert(early.listBox_.field_20_ == 0);
  assert(trace.popup_ctor_calls == 0);
  assert(trace.buffer_dtor_calls >= 1);
  assert(has_call("primary_font") && has_call("set_font"));

  // Double-click callback range and receiver are the actual source proposal.
  reset_trace();
  list_was_dblclicked(0);
  assert(trace.popup_ctor_calls == 0);
  list_was_dblclicked(8);
  assert(trace.popup_ctor_calls == 0);
  assert(PickWinGlobal.field_5DA8_ == 0);
  list_was_dblclicked(1);
  assert(PickWinGlobal.field_5DA8_ == 1);
  assert(has_call("release_modal"));
  reset_trace();
  list_was_dblclicked(7);
  assert(PickWinGlobal.field_5DA8_ == 7 && has_call("release_modal"));

  // Every command constructs/destroys the PopMenu frame local.  The 0x102
  // route sets all seven opponents to JENN282 then rebuilds and notifies.
  reset_trace(); trace.custom_count = 0;
  PickWin buttons;
  buttons.init(selection_callback, 1, 0);
  reset_trace();
  buttons.on_button_clicked(0x102);
  assert(trace.popup_ctor_calls == 1 && trace.popup_dtor_calls == 1);
  assert(trace.read_faction_calls == 0 && trace.list_item_calls == 7);
  for (int i = 1; i < 8; ++i) {
    assert(std::strcmp(Players[i].filename, "JENN282") == 0);
    assert(std::strcmp(Players[i].search_key, "JENN282") == 0);
  }
  assert(buttons.listBox_.field_20_ == 0);
  assert(buttons.listBox_.get_selected_id() == 6 && trace.callback_id == 6);
  assert(buttons.listBox_.selection_callback_ == selection_callback);
  assert(trace.dialog_attach_receiver == static_cast<Dialog *>(&buttons.listBox_));
  assert(trace.dialog_attach_picker_state);
  assert(call_index("dialog_init") < call_index("dialog_attach"));
  assert(call_index("dialog_attach") < call_index("list_set_id"));
  assert(call_index("list_set_id") < call_index("list_selection_cb"));
  assert(call_index("list_selection_cb") < call_index("selection_callback"));

  // Mode 2 filters exactly the faction-status bits; it does not change the
  // text or selection callback contract.
  reset_trace(); trace.mode_status = 1 << 2;
  buttons.field_5DA0_ = 2;
  buttons.on_button_clicked(0x102);
  assert(trace.list_item_calls == 6);
  for (int id : trace.list_ids) assert(id != 2);

  // Popup selection returns a row index; selection 15 picks the second
  // custom faction, reads it for player 1 and rebuilds the list.
  reset_trace(); trace.custom_count = 2; trace.popup_choice = 15;
  buttons.init(selection_callback, 1, 0);
  reset_trace(); trace.popup_choice = 15;
  buttons.on_button_clicked(0x3039);
  assert(trace.popup_ctor_calls == 1 && trace.popup_dtor_calls == 1);
  assert(trace.menu_items.size() == 17);
  assert(std::strcmp(Players[1].filename, "CUSTOM_2") == 0);
  assert(trace.read_faction_calls == 1 && trace.list_item_calls == 7);

  // Cancellation redraws but preserves the selected faction and never
  // rebuilds the list; 0x103 releases modal state with no selection write.
  reset_trace(); trace.popup_choice = -1;
  const uint32_t old_selected = buttons.field_5DA8_;
  buttons.on_button_clicked(0x3039);
  assert(buttons.field_5DA8_ == old_selected);
  assert(trace.popup_ctor_calls == 1 && trace.popup_dtor_calls == 1);
  assert(trace.list_item_calls == 0 && has_call("redraw"));
  reset_trace();
  buttons.on_button_clicked(0x103);
  assert(trace.popup_ctor_calls == 1 && trace.popup_dtor_calls == 1);
  assert(has_call("release_modal"));

  // The other three actual bodies exercise no-op gating, state transfer,
  // row redraw modes, typed palette access and normal returns.
  reset_trace();
  buttons.field_5DB4_ = 5;
  reset_trace();
  buttons.listBox_.set_selected_id(3);
  trace.calls.clear();
  buttons.on_group_clicked(1, 0, 0x105);
  assert(!has_call("redraw") && buttons.field_5DB4_ == 5);
  buttons.on_group_clicked(0, 0, 0x105);
  assert(buttons.field_5DB4_ == 3 && has_call("list_set_pos") && has_call("redraw"));
  reset_trace();
  buttons.on_group_clicked(0, 0, 0x104);
  assert(trace.selected_id == 3 && has_call("redraw"));
  reset_trace();
  buttons.buttonGroup_.field_84_ = 0x105;
  buttons.listBox_.set_selected_id(2);
  reset_trace();
  buttons.on_redraw();
  assert(has_call("fill") && has_call("box_sprite") && has_call("write_cent_l"));
  assert(has_call("vline") && has_call("hline") && has_call("sprite_draw"));
  assert(has_call("show"));
  buttons.buttonGroup_.field_84_ = 0x104;
  reset_trace();
  buttons.listBox_.set_selected_id(4);
  trace.calls.clear();
  buttons.on_redraw();
  assert(has_call("fill") && has_call("hline") && has_call("sprite_draw"));
  RECT area = {1,2,30,40};
  reset_trace();
  assert(buttons.on_dialog_item_back_draw(&buttons, 0, 1, &area) == 1);
  assert(has_call("restore_clip") && has_call("box_sprite") && has_call("set_text_color"));
  assert(buttons.on_dialog_item_back_draw(nullptr, 0, 1, &area) == 0);

  // The global object and each automatic subobject obey ordinary C++ lifetime:
  // constructor order follows declaration order and teardown reverses it.
  reset_trace();
  {
    PickWin lifecycle_probe;
    std::vector<std::string> expected = {"popup_member_ctor",
      "flat_button_ctor","flat_button_ctor","flat_button_ctor",
      "sprite_ctor","sprite_ctor","sprite_ctor","sprite_ctor","sprite_ctor",
      "sprite_ctor","sprite_ctor","sprite_ctor","sprite_ctor","sprite_ctor",
      "sprite_ctor","sprite_ctor","sprite_ctor","listbox_ctor",
      "flat_button_ctor","flat_button_ctor","flat_button_ctor","flat_button_ctor",
      "flat_button_ctor","flat_button_ctor","flat_button_ctor","flat_button_ctor",
      "flat_button_ctor","flat_button_ctor","flat_button_ctor","flat_button_ctor",
      "flat_button_ctor",
      "button_group_ctor"};
    if (member_lifecycle_calls() != expected) {
      for (const std::string &s : member_lifecycle_calls()) std::fprintf(stderr, "%s ", s.c_str());
      std::fprintf(stderr, "\nexpected %zu\n", expected.size());
      assert(false);
    }
  }
  std::vector<std::string> teardown = member_lifecycle_calls();
  assert(teardown.size() == 64);
  assert(teardown[32] == "button_group_dtor");
  assert(teardown[33] == "flat_button_dtor" && teardown[45] == "flat_button_dtor");
  assert(teardown[46] == "listbox_dtor");
  assert(teardown[59] == "sprite_dtor");
  assert(teardown[60] == "flat_button_dtor" && teardown[62] == "flat_button_dtor");
  assert(teardown[63] == "popup_member_dtor");
}
