// C++98 host-only fixture. The Python control injects the actual source method
// definitions at ACTUAL_PULLDOWN_BODY and ACTUAL_FLATBUTTON_BODY.
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <cstdio>

int assertion_number = 0;
void require(bool condition) {
  ++assertion_number;
  if (!condition) { std::fprintf(stderr, "renderer assertion %d failed\n", assertion_number); std::exit(41); }
}
struct RECT { int left, top, right, bottom; };
struct Font { int internal_leading() const { return 1; } };
struct Buffer;
struct GraphicWin;
struct Win;
struct Sprite {
  int w, h, draws;
  Sprite(int width = 4, int height = 4) : w(width), h(height), draws(0) {}
  int width() const { return w; }
  int height() const { return h; }
  int transparent_index() const { return 0; }
  int draw(Buffer *, int, int, int, int, int) { ++draws; return 0; }
};
struct Trace {
  int fills, boxes, lines, writes, right_writes, wraps, tiles, sprite_boxes;
  int text_colour, font_result, main_gate, palette_syncs;
  int last_write_left, last_write_right, last_write_top, last_line_left, last_line_right;
  int box_colour_a[8], box_colour_b[8];
  Trace() : fills(0), boxes(0), lines(0), writes(0), right_writes(0),
    wraps(0), tiles(0), sprite_boxes(0), text_colour(0), font_result(0),
    main_gate(1), palette_syncs(0), last_write_left(-1), last_write_right(-1), last_write_top(-1),
    last_line_left(-1), last_line_right(-1) {}
} trace;
struct Palette {};
Palette *PaletteActive = 0;
int HandleMain = 1;
Win *WinCallbackWindow = 0;
const unsigned char WinFillColour = 0;

struct Buffer {
  int field_18_;
  RECT active;
  int width_, height_, foreground_;
  Buffer() : field_18_(0), width_(80), height_(24), foreground_(7) {
    active.left = 0; active.top = 0; active.right = 80; active.bottom = 24;
  }
  RECT clip_rect() const { return active; }
  RECT current_clip() const { return active; }
  int pixel_width() const { return width_; }
  int pixel_height() const { return height_; }
  int foreground_color() const { return foreground_; }
  int set_font(Font *, int, int, int) { return trace.font_result; }
  int text_line_height() const { return 10; }
  int text_width(const char *s, int count = -1) const {
    return count < 0 ? static_cast<int>(std::strlen(s)) * 5 : count * 5;
  }
  int text_height() const { return 8; }
  void set_text_color(int c, int, int, int) { trace.text_colour = c; }
  void write_l(char *, RECT *r, int) {
    ++trace.writes; trace.last_write_left = r->left; trace.last_write_right = r->right; trace.last_write_top = r->top;
  }
  void write_l(const char *, RECT *r, int) {
    ++trace.writes; trace.last_write_left = r->left; trace.last_write_right = r->right; trace.last_write_top = r->top;
  }
  void write_right_l(char *, RECT *r, int) {
    ++trace.right_writes; trace.last_write_left = r->left; trace.last_write_right = r->right;
  }
  void box(RECT *, int a, int b) {
    if (trace.boxes < 8) {
      trace.box_colour_a[trace.boxes] = a;
      trace.box_colour_b[trace.boxes] = b;
    }
    ++trace.boxes;
  }
  void hline(int left, int right, int, int) {
    ++trace.lines; trace.last_line_left = left; trace.last_line_right = right;
  }
  int wrap_height(char *, int) const { return 10; }
  void wrap_cent(char *, int, int, int) { ++trace.wraps; }
  void set_clip(RECT *r) { active = *r; }
  void restore_clip() {
    active.left = 0; active.top = 0; active.right = width_; active.bottom = height_;
  }
  void box_sprite(RECT *, void *) { ++trace.sprite_boxes; }
  void tile(Sprite *, int, int, int, int, int, int) { ++trace.tiles; }
  void sync_to_palette(Palette *) { ++trace.palette_syncs; }
};
struct GraphicWin : Buffer {
  void fill(int) { ++trace.fills; }
  void fill(RECT *, int) { ++trace.fills; }
};

typedef int (*PullDownFillCallback)(GraphicWin *, int, RECT *);
typedef int (*PullDownBackfillCallback)(GraphicWin *);
int fill_callback_calls = 0, backfill_callback_calls = 0;
int fill_callback(GraphicWin *, int selected, RECT *rect) {
  ++fill_callback_calls;
  if (selected) ++rect->left;
  return 0;
}
int backfill_callback(GraphicWin *) { ++backfill_callback_calls; return 0; }
struct PullDownItem { char *text, *right_text; int id; unsigned flags; char *mnemonic; };
Font font;
Font *FontDefault = &font;
Font *PullDownDefaultFont = 0;
Sprite checked(6, 8);
Sprite *CheckBoxDefault2 = &checked;
PullDownFillCallback PullDownFieldF38Default = 0;
PullDownBackfillCallback PullDownFieldF3CDefault = 0;
int PullDownBorderPadding = 1, PullDownTextPadding = 2;
int PullDownDefaultTextColor = 10, PullDownDefaultTextShadow = 11;
int PullDownDefaultSelectedTextColor = 12, PullDownDefaultSelectedTextShadow = 13;
int PullDownDefaultDisabledTextColor = 14, PullDownDefaultDisabledTextShadow = 15;
int PullDownDefaultBackground = 16;
struct PullDown : GraphicWin {
  PullDownItem items_[64]; int item_count_, selected_;
  PullDownFillCallback field_F38_; PullDownBackfillCallback field_F3C_;
  void on_redraw();
};

struct Win {
  bool suppress_focus;
  Win() : suppress_focus(false) {}
  virtual ~Win() {}
  virtual int vslot_05(int, int, int) { return 0; }
  bool has_dialog_focus_flag() const { return suppress_focus; }
};
struct BaseButton : GraphicWin {
  bool dialog_focus;
  Win *parent_; char *name_; int field_A18_, field_A74_, field_A78_;
  int field_A9C_, field_A94_, field_AB8_;
  unsigned bevel_upper_, bevel_lower_, bevel_thickness_;
  BaseButton() : dialog_focus(false), parent_(0), name_(0), field_A18_(0), field_A74_(0), field_A78_(0),
    field_A9C_(0), field_A94_(-1), field_AB8_(0), bevel_upper_(1),
    bevel_lower_(2), bevel_thickness_(1) {}
  Win *parent_window() const { return parent_; }
  int is_dialog_focus() const { return dialog_focus ? 1 : 0; }
};
struct BoxSpriteParams { int tag; };
struct FlatButton : BaseButton {
  unsigned field_ABC_[3]; BoxSpriteParams *border_styles_[3];
  Sprite *field_AD4_[3], *config[27];
  void on_redraw();
};

// ACTUAL_PULLDOWN_BODY
// ACTUAL_FLATBUTTON_BODY

int main() {
  PullDown menu;
  std::memset(menu.items_, 0, sizeof(menu.items_));
  menu.item_count_ = 2; menu.selected_ = 1;
  char selected[] = "Choice", right[] = "R";
  menu.items_[0].text = selected; menu.items_[0].right_text = right;
  menu.items_[0].mnemonic = selected + 1; menu.items_[0].flags = 1 | 2 | 8;
  menu.items_[1].text = selected; menu.items_[1].flags = 1 | 2;
  menu.on_redraw();
  require(trace.fills == 2 && trace.boxes == 1 && trace.writes == 2);
  require(trace.right_writes == 1 && trace.lines == 1 && checked.draws == 1);
  require(trace.text_colour == PullDownDefaultTextColor);

  trace = Trace(); menu.field_F3C_ = PullDownFieldF3CDefault =
      (PullDownBackfillCallback)0;
  menu.field_F38_ = PullDownFieldF38Default =
      (PullDownFillCallback)0;
  menu.item_count_ = 0; menu.on_redraw();
  require(trace.fills == 1 && trace.boxes == 1);

  trace = Trace(); fill_callback_calls = backfill_callback_calls = 0;
  PullDownFieldF38Default = fill_callback;
  PullDownFieldF3CDefault = backfill_callback;
  menu.item_count_ = 1; menu.selected_ = 0;
  menu.items_[0].flags = 1 | 2;
  menu.on_redraw();
  require(backfill_callback_calls == 1 && fill_callback_calls == 1);
  require(trace.fills == 0 && trace.writes == 1);
  require(trace.last_write_left == PullDownTextPadding + PullDownBorderPadding + 1);
  PullDownFieldF38Default = 0; PullDownFieldF3CDefault = 0;

  trace = Trace(); FlatButton button; Win parent; button.parent_ = &parent;
  std::memset(button.field_ABC_, 0, sizeof(button.field_ABC_));
  std::memset(button.border_styles_, 0, sizeof(button.border_styles_));
  std::memset(button.field_AD4_, 0, sizeof(button.field_AD4_));
  std::memset(button.config, 0, sizeof(button.config));
  button.field_AB8_ = 0; button.name_ = 0;
  Sprite center[3] = {Sprite(20, 12), Sprite(21, 13), Sprite(22, 14)};
  Sprite right_edge[3] = {Sprite(5, 12), Sprite(6, 13), Sprite(7, 14)};
  Sprite bottom[3] = {Sprite(20, 4), Sprite(21, 5), Sprite(22, 6)};
  for (int state = 0; state < 3; ++state) {
    button.field_AB8_ = state;
    for (int reset = 0; reset < 3; ++reset) {
      center[reset].draws = right_edge[reset].draws = bottom[reset].draws = 0;
    }
    button.config[12 + state] = &center[state];
    button.config[15 + state] = &right_edge[state];
    button.config[18 + state] = &bottom[state];
    trace = Trace();
    button.on_redraw();
    require(trace.palette_syncs == 1 && trace.fills == 1);
    require(center[state].draws == 1 && right_edge[state].draws == 1
            && bottom[state].draws == 1);
    for (int other = 0; other < 3; ++other) {
      if (other != state) {
        require(center[other].draws == 0 && right_edge[other].draws == 0
                && bottom[other].draws == 0);
      }
    }
  }

  trace = Trace(); FlatButton fallback; fallback.parent_ = &parent;
  std::memset(fallback.field_ABC_, 0, sizeof(fallback.field_ABC_));
  std::memset(fallback.border_styles_, 0, sizeof(fallback.border_styles_));
  std::memset(fallback.field_AD4_, 0, sizeof(fallback.field_AD4_));
  std::memset(fallback.config, 0, sizeof(fallback.config));
  char long_label[40]; std::memset(long_label, 'L', sizeof(long_label) - 1);
  long_label[sizeof(long_label) - 1] = 0;
  fallback.name_ = long_label; fallback.field_AB8_ = 0;
  fallback.dialog_focus = true;
  fallback.field_A94_ = 3; fallback.field_A9C_ = 0; fallback.bevel_thickness_ = 2;
  fallback.on_redraw();
  require(trace.boxes == 3 && trace.wraps == 0 && trace.writes == 2);
  require(trace.last_write_left == 59 && trace.last_write_right == 112);
  require(trace.last_write_top == 3);
  require(trace.box_colour_a[1] == 3 && trace.box_colour_b[1] == 3);

  trace = Trace(); parent.suppress_focus = true;
  fallback.on_redraw();
  require(trace.boxes == 3 && trace.writes == 2);
  require(trace.box_colour_a[1] == 1 && trace.box_colour_b[1] == 2);
  return 0;
}
