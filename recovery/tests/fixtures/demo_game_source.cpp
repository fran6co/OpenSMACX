// C++98 host fixture template. pytest inserts method definitions extracted
// from the tracked src/game.cpp owner into the five EXTRACT markers below.
#include <string.h>
#define __cdecl
#define int long
struct RECT { long left, top, right, bottom; };
RECT DemoFourthHotspot;
long WinScreenWidth, WinScreenHeight;
unsigned char WinFillColour;
void *HandleMain;
enum { SW_HIDE = 0, SW_SHOWNORMAL = 1 };

long buffer_destructions;
struct Buffer {
  long loads, closes;
  char last_file[64];
  Buffer() : loads(0), closes(0) { last_file[0] = 0; }
  ~Buffer() { ++buffer_destructions; }
  long load_pcx(char *, long, long, long);
  void close() { ++closes; }
};
struct Sprite {
  long draws, extracts;
  Sprite() : draws(0), extracts(0) {}
  unsigned char transparent_index() const { return 7; }
  long draw(Buffer *, long, long, long, long, long) { ++draws; return 0; }
  long extract(Buffer *, unsigned char, long, long, long, long, long);
};
struct Time {
  void (__cdecl *callback)(long);
  long receiver, first, period, starts;
  Time() : callback(0), receiver(0), first(0), period(0), starts(0) {}
  long start(void (__cdecl *fn)(long), long payload, long a, long b) {
    callback = fn; receiver = payload; first = a; period = b; ++starts;
    return 1;
  }
};
long show_calls, shell_calls, modal_calls, release_calls;
long box_calls, modal_x, modal_y, modal_click_enabled;
char last_url[160];
long ShowWindow(void *, long) { ++show_calls; return 1; }
void ShellExecuteA(void *, const char *, const char *url,
                   const char *, const char *, long) {
  ++shell_calls; long n = 0;
  while (url[n] && n < 159) { last_url[n] = url[n]; ++n; }
  last_url[n] = 0;
}
long in_box(long x, long y, RECT *r) {
  ++box_calls;
  return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}
long Buffer::load_pcx(char *path, long, long, long) {
  ++loads; long n = 0;
  while (path[n] && n < 63) { last_file[n] = path[n]; ++n; }
  last_file[n] = 0; return 0;
}
long Sprite::extract(Buffer *, unsigned char, long, long, long, long, long) {
  ++extracts; return 0;
}
struct GraphicWin : Buffer {
  long init(long, long, long, long, long, long, long, long, long) { return 0; }
  long load_pcx(char *p, long a, long b, long c) {
    return Buffer::load_pcx(p, a, b, c);
  }
  void soft_update(RECT *) {}
  virtual void redraw() {}
  virtual void on_left_down(long, long) {}
  virtual long set_modal(long, long, long);
  void release_modal() { ++release_calls; }
};
class Demo : public GraphicWin {
 public:
  long init();
  virtual void redraw();
  virtual void on_left_down(long, long);
  static void __cdecl timer_callback(long);
  long selected_link_;
  long timer_phase_;
  Time timer_;
  RECT splash_rects_[3];
  Sprite splash_sprites_[2];
};
long GraphicWin::set_modal(long, long, long) {
  ++modal_calls;
  if (modal_click_enabled) on_left_down(modal_x, modal_y);
  return 0;
}

/* EXTRACT_SET_DEMO_RECT */
/* EXTRACT_DEMO_REDRAW */
/* EXTRACT_DEMO_SELECT */
/* EXTRACT_DEMO_TIMER */
/* EXTRACT_DEMO_INIT */

#undef int
struct UnknownSelectionDemo : Demo {
  long injected;
  virtual long set_modal(long, long, long) { selected_link_ = injected; return 0; }
};
struct RedrawOverrideDemo : Demo {
  long calls;
  RedrawOverrideDemo() : calls(0) {}
  virtual void redraw() { ++calls; }
};
int main() {
  WinScreenWidth = 0x320; WinScreenHeight = 0x258;
  WinFillColour = 0; HandleMain = reinterpret_cast<void *>(1);
  modal_click_enabled = 1; modal_x = 0x23B; modal_y = 0x200;
  Demo compact;
  if (compact.init() != 1 || compact.selected_link_ != 2) return 1;
  if (compact.splash_rects_[0].left != 0x21 ||
      compact.splash_rects_[0].top != 0x1E1) return 2;
  if (compact.timer_.starts != 1 || compact.timer_.first != 600 ||
      compact.timer_.period != 5 || compact.splash_sprites_[0].extracts != 1 ||
      compact.splash_sprites_[1].extracts != 1) return 3;
  if (shell_calls != 1 || show_calls != 1 || modal_calls != 1) return 4;
  if (strcmp(last_url, "http:\\\\www.amazon.com\\exec\\obidos\\ASIN\\B00000DMA7\\ref=ad_cgm1") != 0) return 5;
  compact.timer_.callback(compact.timer_.receiver);
  if (compact.timer_phase_ != 1 || compact.splash_sprites_[0].draws != 1) return 6;
  modal_x = 0x1C6; modal_y = 0x23B;
  Demo fourth;
  if (fourth.init() != 1 || fourth.selected_link_ != 3) return 7;
  fourth.on_left_down(0x300, 0x300);
  if (fourth.selected_link_ != 0 || release_calls != 3) return 8;
  if (DemoFourthHotspot.left != 0x1C5 || DemoFourthHotspot.top != 0x23A ||
      DemoFourthHotspot.right != 0x262 || DemoFourthHotspot.bottom != 0x24E) return 9;
  const long previous_shows = show_calls, previous_shells = shell_calls;
  UnknownSelectionDemo unknown;
  unknown.injected = 4;
  if (unknown.init() != 1 || show_calls != previous_shows || shell_calls != previous_shells) return 10;
  unknown.injected = -1;
  if (unknown.init() != 1 || show_calls != previous_shows || shell_calls != previous_shells) return 11;
  unknown.injected = 0;
  if (unknown.init() != 0 || show_calls != previous_shows || shell_calls != previous_shells) return 12;
  WinScreenWidth = 0x400; modal_click_enabled = 0;
  Demo wide;
  if (wide.init() != 0 || DemoFourthHotspot.left != 0x243 || DemoFourthHotspot.top != 0x2DA ||
      DemoFourthHotspot.right != 0x30C || DemoFourthHotspot.bottom != 0x2F1) return 13;
  if (buffer_destructions != 6) return 14;
  RedrawOverrideDemo override_demo;
  override_demo.timer_phase_ = 0;
  Demo::timer_callback(reinterpret_cast<long>(&override_demo));
  if (override_demo.timer_phase_ != 1 || override_demo.calls != 1) return 15;
  return 0;
}
