#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
#define __cdecl
static std::vector<std::string> calls;
extern int PullDownBorderPadding;
extern int PullDownDefaultSelectedTextShadow;
static int line_height = 10;
static int configured_text_height = 8;
static int text_width_value = 4;
static int line_height_calls = 0;
static std::vector<int> line_height_sequence;
static int callback_mode = -1;
static int PullDownTextPadding = 3;
int PullDownBorderPadding = 2;
static int width_value = 100;
static int padding_after_line_height = -1;
static int width_after_line_height = -1;
static int padding_after_fill = -1;
static bool mutate_separator_after_first_hline = false;
static int border_after_first_hline = -1;
static int width_after_first_hline = -1;
static int line_height_after_first_hline = -1;
static int shadow_after_first_hline = -1;
static bool mutate_item_on_fill = false;
static char *replacement_item_text = nullptr;
static char *replacement_item_right = nullptr;
static char *replacement_item_mnemonic = nullptr;
static std::vector<std::string> written_left_text;
static std::vector<std::string> written_right_text;

struct Buffer {
  int text_line_height() {
    calls.push_back("line-height");
    if (padding_after_line_height >= 0) PullDownTextPadding = padding_after_line_height;
    if (width_after_line_height >= 0) width_value = width_after_line_height;
    const int call = line_height_calls++;
    if (call < static_cast<int>(line_height_sequence.size())) return line_height_sequence[call];
    return line_height;
  }
  int text_height() { calls.push_back("text-height"); return configured_text_height; }
  int pixel_width() const { calls.push_back("pixel-width"); return width_value; }
  void hline(int x1, int x2, int y, int color) {
    calls.push_back("hline:" + std::to_string(x1) + "," + std::to_string(x2) + "," + std::to_string(y) + "," + std::to_string(color));
    if (mutate_separator_after_first_hline) {
      mutate_separator_after_first_hline = false;
      if (border_after_first_hline >= 0) PullDownBorderPadding = border_after_first_hline;
      if (width_after_first_hline >= 0) width_value = width_after_first_hline;
      if (line_height_after_first_hline >= 0) line_height = line_height_after_first_hline;
      if (shadow_after_first_hline >= 0) PullDownDefaultSelectedTextShadow = shadow_after_first_hline;
    }
  }
  int text_width(const char *, int n) { calls.push_back("text-width:" + std::to_string(n)); return n * text_width_value; }
  int foreground_color_ = 0;
  void set_text_color(int c, int, int, int) { foreground_color_ = c; calls.push_back("color:" + std::to_string(c)); }
  int foreground_color() const { calls.push_back("foreground"); return foreground_color_; }
  void write_l(const char *text, RECT *r, int n) {
    written_left_text.emplace_back(text, static_cast<size_t>(n));
    calls.push_back("left:" + std::to_string(n) + ":" + std::to_string(r->left));
  }
  void write_right_l(const char *text, RECT *, int n) {
    written_right_text.emplace_back(text, static_cast<size_t>(n));
    calls.push_back("right:" + std::to_string(n));
  }
};
struct GraphicWin : public Buffer {
  RECT rect2_{0, 0, 20, 20};
  void fill(RECT *r, int color) {
    calls.push_back("fill:" + std::to_string(r->left) + "," + std::to_string(r->right) + "," + std::to_string(color));
    if (padding_after_fill >= 0) PullDownTextPadding = padding_after_fill;
  }
  void soft_update(RECT *r) { calls.push_back("soft:" + std::to_string(r->left) + "," + std::to_string(r->right) + "," + std::to_string(r->top) + "," + std::to_string(r->bottom)); }
  void close() { calls.push_back("close"); }
  void client_to_screen(int *x, int *y) { *x += 100; *y += 200; calls.push_back("client-to-screen"); }
  void redraw() { calls.push_back("slot62"); }
  const RECT *buffer_working_rect() const { return &rect2_; }
};
struct Sprite {
  int iHeight_ = 4, iWidth_ = 6, cTransparentIndex_ = 9;
  int height() const { return iHeight_; }
  int width() const { return iWidth_; }
  int transparent_index() const { return cTransparentIndex_; }
  int draw(Buffer *, int, int x, int y, int, int);
};
struct Font {
  int internal_leading() const { calls.push_back("font-leading"); return 1; }
  int line_advance() const { return 10; }
};
struct PullDownItem { char *text; char *right_text; int id; uint32_t flags; char *mnemonic; };
typedef int (__cdecl *PullDownFillCallback)(GraphicWin *, int, RECT *);
typedef int (__cdecl *PullDownPoll)();
typedef int (__cdecl *PullDownBackfillCallback)(GraphicWin *);
struct Menu {
  int move_x = -1, move_y = -1;
  void mouse_move(int x, int y) { move_x = x; move_y = y; calls.push_back("menu-move"); }
};
static int poll_callback() { calls.push_back("poll"); return 0; }
class PullDown : public GraphicWin {
 public:
  int selected_ = 0;
  uint32_t field_F18_ = 0;
  PullDownPoll poll_ = nullptr;
  int item_count_ = 0;
  int width_ = 0;
  int visible_count_ = 0;
  Menu *menu_ = nullptr;
  uint8_t dirty_ = 0;
  uint8_t dirty_padding_[3]{};
  PullDownFillCallback field_F38_ = nullptr;
  PullDownBackfillCallback field_F3C_ = nullptr;
  PullDownItem items_[64]{};
  void UNK1();
  void on_mouse_move(int x, int y);
  void mouse_move(int x, int y);
  void pulldown_update(int old_selected, int new_selected);
  void UNK4() { calls.push_back("clear-items"); }
  int slot062() { calls.push_back("slot62"); return 0; }
};
static PullDownFillCallback PullDownFieldF38Default = nullptr;
static PullDownBackfillCallback PullDownFieldF3CDefault = nullptr;
int PullDownDefaultTextColor = 11;
int PullDownDefaultTextShadow = 12;
int PullDownDefaultSelectedTextColor = 13;
int PullDownDefaultSelectedTextShadow = 14;
int PullDownDefaultDisabledTextColor = 15;
int PullDownDefaultDisabledTextShadow = 16;
int PullDownDefaultBackground = 17;
Sprite icon_sprite;
Sprite *CheckBoxDefault2 = &icon_sprite;
static bool switch_sprite_after_draw = false;
static Sprite *sprite_after_draw = nullptr;
Font font_object;
Font *PullDownDefaultFont = &font_object;
int Sprite::draw(Buffer *, int, int x, int y, int, int) {
  calls.push_back("icon:" + std::to_string(x) + "," + std::to_string(y));
  if (switch_sprite_after_draw) {
    switch_sprite_after_draw = false;
    CheckBoxDefault2 = sprite_after_draw;
  }
  return 0;
}
static int fill_callback(GraphicWin *window, int value, RECT *r) {
  calls.push_back("callback:" + std::to_string(value) + ":" + std::to_string(r->left));
  if (mutate_item_on_fill) {
    PullDown *const menu = static_cast<PullDown *>(window);
    menu->items_[0].text = replacement_item_text;
    menu->items_[0].right_text = replacement_item_right;
    menu->items_[0].mnemonic = replacement_item_mnemonic;
    mutate_item_on_fill = false;
  }
  return callback_mode;
}
void do_all_draws() { calls.push_back("draws"); }
bool in_box(int x, int y, const RECT *r) {
  return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}

// Actual product member bodies are injected here by the source-extract test.
// The host projection keeps callbacks, fields, and rendering effects observable.
// __ACTUAL_BODIES_SLOT__

static int count_prefix(const std::string &prefix) {
  int n = 0;
  for (const auto &s : calls) if (s.compare(0, prefix.size(), prefix) == 0) ++n;
  return n;
}
static size_t call_index(const std::string &call, size_t occurrence = 0) {
  for (size_t i = 0; i < calls.size(); ++i) {
    if (calls[i] == call && occurrence-- == 0) return i;
  }
  return calls.size();
}
static void reset_state() {
  calls.clear();
  line_height = 10;
  line_height_calls = 0;
  line_height_sequence.clear();
  CheckBoxDefault2 = &icon_sprite;
  switch_sprite_after_draw = false;
  sprite_after_draw = nullptr;
  mutate_separator_after_first_hline = false;
  border_after_first_hline = width_after_first_hline = -1;
  line_height_after_first_hline = shadow_after_first_hline = -1;
  PullDownDefaultSelectedTextShadow = 14;
  PullDownTextPadding = 3;
  PullDownBorderPadding = 2;
  width_value = 100;
  padding_after_line_height = -1;
  width_after_line_height = -1;
  padding_after_fill = -1;
  mutate_item_on_fill = false;
  mutate_item_on_fill = false;
  replacement_item_text = replacement_item_right = replacement_item_mnemonic = nullptr;
  written_left_text.clear();
  written_right_text.clear();
}
static void init_item(PullDown &p, int i, uint32_t flags, char *text = nullptr,
                      char *right = nullptr, char *mnemonic = nullptr) {
  p.items_[i] = {text, right, i, flags, mnemonic};
}
static void test_unk1() {
  reset_state();
  PullDown p; Menu menu; p.selected_ = 9; p.field_F18_ = 7; p.poll_ = poll_callback;
  p.item_count_ = 8; p.width_ = 90; p.visible_count_ = 7; p.menu_ = &menu;
  p.field_F38_ = fill_callback; p.field_F3C_ = nullptr;
  p.UNK1();
  assert(calls.size() >= 2 && calls[0] == "clear-items" && calls[1] == "close");
  assert(p.selected_ == -1 && p.field_F18_ == 0 && p.poll_ == nullptr);
  assert(p.item_count_ == 0 && p.width_ == 0 && p.visible_count_ == 0 && p.menu_ == nullptr);
  assert(p.field_F38_ == PullDownFieldF38Default && p.field_F3C_ == PullDownFieldF3CDefault);
}
static void test_on_mouse_move() {
  reset_state();
  PullDown p; p.rect2_ = {0, 0, 10, 10}; p.item_count_ = 1;
  init_item(p, 0, 1); p.selected_ = -1;
  p.on_mouse_move(2, 3);
  assert(p.selected_ == 0 && count_prefix("draws") == 1);
  assert(count_prefix("client-to-screen") == 0 && count_prefix("slot62") == 0);

  reset_state();
  PullDown outside; outside.rect2_ = {0, 0, 10, 10};
  outside.selected_ = 4; Menu menu; outside.menu_ = &menu;
  outside.on_mouse_move(20, 30);
  assert(outside.selected_ == -1 && count_prefix("slot62") == 1);
  assert(menu.move_x == 120 && menu.move_y == 230);
  assert(count_prefix("client-to-screen") == 1 && count_prefix("menu-move") == 1);
}
static void test_mouse_move() {
  reset_state();
  PullDown p; p.item_count_ = 3; p.selected_ = 6;
  init_item(p, 0, 1); init_item(p, 1, 0); init_item(p, 2, 1);
  p.mouse_move(0, 12);
  assert(p.selected_ == 2 && count_prefix("draws") == 1);

  reset_state();
  PullDown separator; separator.item_count_ = 3; separator.selected_ = 1;
  init_item(separator, 0, 1); init_item(separator, 1, 0); init_item(separator, 2, 1|4);
  separator.mouse_move(0, 12);
  assert(separator.selected_ == -1 && count_prefix("draws") == 1);

  reset_state();
  PullDown outside; outside.item_count_ = 2; outside.selected_ = 0;
  init_item(outside, 0, 1); init_item(outside, 1, 1);
  outside.mouse_move(0, 100);
  assert(outside.selected_ == -1 && count_prefix("draws") == 1);
}
static void test_item_fields_are_snapshotted_before_fill_callback() {
  reset_state();
  char original[] = "Open";
  char original_right[] = "Ctrl+O";
  char replacement[] = "After";
  char replacement_right[] = "Alt+X";
  PullDown p; p.selected_ = 0; p.field_F38_ = fill_callback;
  init_item(p, 0, 1|2, original, original_right, original + 1);
  mutate_item_on_fill = true;
  replacement_item_text = replacement;
  replacement_item_right = replacement_right;
  replacement_item_mnemonic = replacement + 3;
  p.pulldown_update(-1, 0);
  assert(written_left_text.size() == 1 && written_left_text[0] == "After");
  assert(written_right_text.size() == 1 && written_right_text[0] == "Ctrl+O");
  assert(count_prefix("hline:9,13,") == 1); // original mnemonic offset +1, plus the 5px row inset
  assert(call_index("foreground") < call_index("text-width:1"));
  assert(call_index("font-leading") < call_index("text-width:1"));
  assert(call_index("text-width:1") < call_index("text-height"));
  assert(call_index("text-height") < call_index("text-width:1", 1));
}
static void test_row_height_is_reloaded_per_row() {
  reset_state();
  line_height_sequence = {10, 20};
  PullDown p; p.selected_ = 1;
  init_item(p, 0, 1); init_item(p, 1, 1);
  p.pulldown_update(-1, 1);
  assert(count_prefix("line-height") == 2);
  assert(count_prefix("soft:2,98,22,32") == 1);
}
static void test_icon_width_is_reloaded_after_draw() {
  reset_state();
  Sprite replacement; replacement.iWidth_ = 11;
  char text[] = "X";
  PullDown p; p.selected_ = 0; init_item(p, 0, 1|2|8, text);
  sprite_after_draw = &replacement;
  switch_sprite_after_draw = true;
  p.pulldown_update(-1, 0);
  assert(count_prefix("left:1:16") == 1); // 5px inset + replacement sprite's 11px width
}
static void test_separator_reloads_each_line_geometry() {
  reset_state();
  line_height_sequence = {10, 10, 6};
  PullDown p; p.selected_ = 0; init_item(p, 0, 1|4);
  mutate_separator_after_first_hline = true;
  border_after_first_hline = 7;
  width_after_first_hline = 130;
  line_height_after_first_hline = 14;
  shadow_after_first_hline = 33;
  p.pulldown_update(-1, 0);
  assert(count_prefix("hline:2,98,5,15") == 1);
  assert(count_prefix("hline:7,123,10,33") == 1);
}
int main() {
  test_unk1();
  test_on_mouse_move();
  test_mouse_move();
  test_item_fields_are_snapshotted_before_fill_callback();
  test_row_height_is_reloaded_per_row();
  test_icon_width_is_reloaded_after_draw();
  test_separator_reloads_each_line_geometry();
  {
    reset_state();
    PullDown p; init_item(p, 0, 0); init_item(p, 1, 0);
    p.pulldown_update(0, 1);
    assert(count_prefix("soft:") == 0); // both hidden rows skip their row invalidation
    assert(count_prefix("draws") == 1);
  }
  {
    reset_state();
    char label[] = "Open"; char right[] = "Ctrl+O";
    PullDown p; p.selected_ = 0; init_item(p, 0, 1|2, label, right, label+1);
    p.pulldown_update(-1, 0);
    assert(count_prefix("fill:") == 1);
    assert(count_prefix("left:4:") == 1 && count_prefix("right:6") == 1);
    assert(count_prefix("text-height") == 1); // selected/new mnemonic uses text_height
    assert(count_prefix("line-height") == 2); // entry geometry plus selected-row observation
    assert(count_prefix("soft:") == 1 && count_prefix("draws") == 1);
  }
  {
    reset_state();
    PullDown p; init_item(p, 0, 1|4); p.field_F38_ = fill_callback;
    p.pulldown_update(-1, 0);
    assert(count_prefix("hline:2,98,") == 2);
    assert(count_prefix("callback:") == 0 && count_prefix("soft:") == 1);
  }
  {
    reset_state();
    PullDown p; init_item(p, 0, 1|8|2);
    p.pulldown_update(-1, 0);
    assert(count_prefix("icon:5,") == 1);
    assert(count_prefix("left:") == 0 && count_prefix("soft:") == 1);
  }
  {
    reset_state();
    PullDown p; p.selected_ = 0; p.field_F38_ = fill_callback;
    init_item(p, 0, 1|2, nullptr);
    p.pulldown_update(-1, 0);
    assert(count_prefix("callback:1:") == 1 && count_prefix("fill:") == 0);
    assert(count_prefix("color:11") == 1);
  }
  {
    reset_state();
    char label[] = "A";
    padding_after_line_height = 5;
    width_after_line_height = 120;
    padding_after_fill = 9;
    PullDown p; p.selected_ = 0; init_item(p, 0, 1|2, label);
    p.pulldown_update(-1, 0);
    assert(calls[0] == "pixel-width" && calls[1] == "line-height");
    assert(count_prefix("fill:2,98,17") == 1); // row extents use the entry padding snapshot; fill uses live padding
    assert(count_prefix("left:1:11") == 1); // restoration rereads padding=9 after fill
    assert(count_prefix("soft:2,98,2,12") == 1); // final invalidation also uses live padding=9
  }
  {
    reset_state();
    padding_after_fill = 9;
    PullDown p; p.selected_ = 1; init_item(p, 0, 1);
    p.pulldown_update(0, -1);
    assert(count_prefix("fill:2,98,11") == 1); // old/noncurrent expands before text-color fill
    assert(count_prefix("soft:2,98,2,12") == 1);
  }
  {
    reset_state();
    padding_after_fill = 9;
    PullDown p; p.selected_ = 1; init_item(p, 0, 1);
    p.pulldown_update(-1, 0);
    assert(count_prefix("fill:5,95,11") == 1); // native new/noncurrent branch fills the unexpanded row
    assert(count_prefix("soft:-4,104,2,12") == 1); // but invalidation uses live padding afterward
  }
  {
    reset_state();
    PullDown p; init_item(p, 0, 1);
    p.pulldown_update(0, 0);
    assert(count_prefix("soft:") == 1 && count_prefix("fill:") == 1);
    assert(count_prefix("draws") == 1);
  }
  return 0;
}
