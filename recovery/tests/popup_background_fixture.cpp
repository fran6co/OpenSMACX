// Host projection for the actual Popup::on_draw_background body.
// The pytest draft inserts the current source definition at this marker.
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
typedef int BOOL;
typedef char *LPSTR;
#define WINAPI
RECT rect(int left, int top, int right, int bottom) {
  RECT value = {left, top, right, bottom};
  return value;
}
struct Font {};
struct BoxSpriteParams { int id; };

enum { popup_buffer = 1, diplomacy_buffer = 2, interface_buffer = 3 };
struct Event {
  std::string kind;
  int receiver;
  int style;
  int a, b, c, d;
  std::string text;
  Event(const char *k, int r, int s, int aa, int bb, int cc, int dd,
        const char *t = "")
      : kind(k), receiver(r), style(s), a(aa), b(bb), c(cc), d(dd), text(t) {}
};
std::vector<Event> events;

void require(bool condition, const char *message) {
  if (!condition) {
    std::fprintf(stderr, "control failed: %s\n", message);
    std::exit(2);
  }
}

struct Win {
  bool visible;
  Win() : visible(true) {}
  int is_visible() { events.push_back(Event("visible", interface_buffer, 0, 0, 0, 0, 0)); return visible; }
};

struct Buffer {
  int identity;
  RECT working;
  RECT clip;
  Buffer(int id) : identity(id), working(), clip() {}
  int set_clip(RECT *rect) {
    clip = *rect;
    events.push_back(Event("clip", identity, 0, rect->left, rect->top,
                           rect->right, rect->bottom));
    return 0;
  }
  const RECT &clip_rect() const { return clip; }
  RECT *buffer_working_rect() { return &clip; }
  int box_sprite(RECT *rect, BoxSpriteParams *style);
  int set_font(Font *, Font *, Font *, Font *) {
    events.push_back(Event("font", identity, 0, 0, 0, 0, 0)); return 0;
  }
  void set_text_color(int first, int second, int, int) {
    events.push_back(Event("color", identity, 0, first, second, 0, 0));
  }
  int write_cent_l(char *text, RECT *rect, int length) {
    events.push_back(Event("text", identity, 0, rect->left, rect->top,
                           rect->right, rect->bottom, text));
    require(length == static_cast<int>(std::strlen(text)), "caption length");
    return 0;
  }
};

BoxSpriteParams InterfaceBoxStyles[51];
BoxSpriteParams InterfaceStandardPopupBoxStyle = {50};
int style_id(BoxSpriteParams *style) {
  if (style == &InterfaceStandardPopupBoxStyle) return 50;
  for (int i = 0; i < 51; ++i) if (style == &InterfaceBoxStyles[i]) return i;
  return -1;
}
int Buffer::box_sprite(RECT *rect, BoxSpriteParams *style) {
  events.push_back(Event("box", identity, style_id(style), rect->left,
                         rect->top, rect->right, rect->bottom));
  return 0;
}

struct GraphicWin : Win, Buffer {
  GraphicWin(int id) : Buffer(id) {}
  int fill(RECT *rect, int color) {
    events.push_back(Event("fill_rect", identity, 0, rect->left, rect->top,
                           rect->right, rect->bottom));
    require(color == 9, "background fill color");
    return 0;
  }
  void fill(int color) {
    events.push_back(Event("fill", identity, 0, color, 0, 0, 0));
  }
};
struct DiploPop : GraphicWin {
  DiploPop() : GraphicWin(diplomacy_buffer) {}
};
DiploPop DiplomacyPopup;

struct MainInterface : GraphicWin {
  Font font;
  MainInterface() : GraphicWin(interface_buffer) {}
  Font *primary_font() { return &font; }
};
MainInterface MainInterfaceGlobal;

struct Dialog { int field_5C_, field_60_; Dialog() : field_5C_(0), field_60_(0) {} };
struct Dialogs : Dialog {
  int kind_;
  Dialogs() : kind_(-1) {}
};
struct BasePop : GraphicWin {
  Win *external_window_;
  char *owned_text_;
  char *caller_sprite_;
  unsigned int field_30AC_;
  unsigned int field_30D4_, field_30D8_, field_30E0_, field_30E4_, field_30E8_;
  unsigned int field_30F0_, field_21C8_, field_21CC_;
  Dialogs dialogs_;
  BasePop() : GraphicWin(popup_buffer), external_window_(0), owned_text_(0),
      caller_sprite_(0), field_30AC_(0), field_30D4_(0), field_30D8_(0),
      field_30E0_(0), field_30E4_(0), field_30E8_(0), field_30F0_(0),
      field_21C8_(0), field_21CC_(0) {}
};
struct Popup : BasePop { int on_draw_background(); };

int TextColourTable[195];
LPSTR WINAPI CharUpperA(LPSTR text) {
  events.push_back(Event("upper", 0, 0, 0, 0, 0, 0, text));
  for (char *p = text; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
  return text;
}
BOOL WINAPI UnionRect(RECT *out, const RECT *first, const RECT *second) {
  events.push_back(Event("union", popup_buffer, 0, 0, 0, 0, 0));
  out->left = first->left < second->left ? first->left : second->left;
  out->top = first->top < second->top ? first->top : second->top;
  out->right = first->right > second->right ? first->right : second->right;
  out->bottom = first->bottom > second->bottom ? first->bottom : second->bottom;
  return 1;
}

// ACTUAL_SOURCE_BODY

Event box_event(int index) {
  int seen = 0;
  for (std::size_t i = 0; i < events.size(); ++i) {
    if (events[i].kind == "box") {
      if (seen == index) return events[i];
      ++seen;
    }
  }
  require(false, "expected box draw event");
  return Event("", 0, 0, 0, 0, 0, 0);
}
int box_count() {
  int count = 0;
  for (std::size_t i = 0; i < events.size(); ++i) if (events[i].kind == "box") ++count;
  return count;
}
Event last_box() { return box_event(box_count() - 1); }
void clear_events() { events.clear(); }

void check_local_popup_anchor() {
  clear_events();
  Popup popup;
  popup.working = rect(10, 20, 210, 120);
  popup.clip = popup.working;
  popup.field_30AC_ = 60000;
  popup.field_30D4_ = 5;
  popup.field_30D8_ = 3;
  popup.field_30E0_ = 2;
  popup.field_30E4_ = 6;
  popup.field_30E8_ = 20;
  popup.field_30F0_ = 7;
  popup.field_21C8_ = 4;
  popup.dialogs_.kind_ = -1;
  char caption[] = "hello";
  popup.owned_text_ = caption;

  require(popup.on_draw_background() == 1, "null external window result");
  require(std::strcmp(caption, "HELLO") == 0, "caption uppercased in place");
  require(box_count() == 5, "standard frame, title pair, anchor, final border");
  require(box_event(0).receiver == popup_buffer && box_event(0).style == 50,
          "type-six standard box style uses popup buffer");
  require(box_event(1).style == 3 && box_event(2).style == 6,
          "title styles retain order");
  require(box_event(3).style == 0, "anchor style");
  Event border = last_box();
  require(border.receiver == popup_buffer && border.style == 3,
          "local final border receiver and style");
  require(border.a == 10 && border.b == 16 && border.c == 197 && border.d == 30,
          "local final border has three-pixel expansion");
  bool found_local_colors = false;
  for (std::size_t i = 0; i < events.size(); ++i) {
    if (events[i].kind == "color") {
      found_local_colors = events[i].receiver == popup_buffer
          && events[i].a == 68 && events[i].b == 69;
    }
  }
  require(found_local_colors, "local caption uses color table cells 68/69");
}

void check_diplomacy_final_receiver() {
  clear_events();
  Popup popup;
  popup.working = rect(1, 2, 301, 202);
  popup.clip = popup.working;
  DiplomacyPopup.clip = rect(100, 50, 500, 250);
  popup.external_window_ = &DiplomacyPopup;
  popup.field_30F0_ = 20;
  popup.field_30D8_ = 4;
  popup.field_30E0_ = 3;
  popup.field_30E8_ = 20;
  popup.field_21C8_ = 6;

  require(popup.on_draw_background() == 0, "diplomacy external window result");
  require(box_count() == 2, "diplomacy frame and final border");
  require(box_event(0).receiver == diplomacy_buffer && box_event(0).style == 9,
          "diplomacy frame uses diplomacy buffer");
  Event border = last_box();
  require(border.receiver == diplomacy_buffer && border.style == 3,
          "diplomacy final border keeps diplomacy Buffer receiver");
  require(border.a == 27 && border.b == 17 && border.c == 441 && border.d == 26,
          "diplomacy final border geometry");
  bool popup_clip = false;
  for (std::size_t i = 0; i < events.size(); ++i) {
    if (events[i].kind == "clip") popup_clip = events[i].receiver == popup_buffer;
  }
  require(popup_clip, "diplomacy path resets Popup clip before drawing external Buffer");
}

int main() {
  for (int i = 0; i < 51; ++i) InterfaceBoxStyles[i].id = i;
  TextColourTable[68] = 68;
  TextColourTable[69] = 69;
  TextColourTable[180] = 1800;
  TextColourTable[181] = 1810;
  MainInterfaceGlobal.visible = true;
  check_local_popup_anchor();
  check_diplomacy_final_receiver();
  return 0;
}
