#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#define __cdecl
struct RECT { int left, top, right, bottom; };

struct Time {
  int stop_count;
  void stop();
};
static int time_stops;
static Time *last_stopped;
void Time::stop() { ++stop_count; ++time_stops; last_stopped = this; }

static int union_calls;
static RECT union_left, union_right, union_result;
static int update_calls;
static void *last_update_owner;
static RECT last_update_rect;

struct GraphicWin {
  void soft_update(RECT *area) {
    ++update_calls;
    last_update_owner = this;
    last_update_rect = *area;
  }
};

struct MainInterface : GraphicWin {
  char pad_to_animation[0xDE0];
  RECT menu_animation_bounds_;
  RECT multiplayer_animation_bounds_;
  char pad_to_alert_[0x216F0 - 0xE00];
  uint32_t alert_;
  char pad_to_time1_[0x25040 - 0x216F4];
  Time time1_;
  char pad_to_time2_[0x25068 - 0x25044];
  Time time2_;
};

static_assert(offsetof(MainInterface, menu_animation_bounds_) == 0xDE0,
              "host layout: menu rectangle");
static_assert(offsetof(MainInterface, multiplayer_animation_bounds_) == 0xDF0,
              "host layout: multiplayer rectangle");
static_assert(offsetof(MainInterface, alert_) == 0x216F0,
              "host layout: alert field");
static_assert(offsetof(MainInterface, time1_) == 0x25040,
              "host layout: first timer");
static_assert(offsetof(MainInterface, time2_) == 0x25068,
              "host layout: second timer");
static_assert(sizeof(RECT) == 16 && sizeof(Time) == 4,
              "host layout assumptions");

int UnionRect(RECT *out, const RECT *left, const RECT *right) {
  ++union_calls;
  union_left = *left;
  union_right = *right;
  out->left = left->left < right->left ? left->left : right->left;
  out->top = left->top < right->top ? left->top : right->top;
  out->right = left->right > right->right ? left->right : right->right;
  out->bottom = left->bottom > right->bottom ? left->bottom : right->bottom;
  union_result = *out;
  return 1;
}

void __cdecl timer_callback_daemon(int, int);

/* CALLBACK_SOURCE */

static MainInterface *make_low_instance() {
  void *storage = mmap(0, sizeof(MainInterface), PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
  assert(storage != MAP_FAILED);
  assert(reinterpret_cast<uintptr_t>(storage) <= 0x7fffffffu);
  memset(storage, 0, sizeof(MainInterface));
  return static_cast<MainInterface *>(storage);
}

int main() {
  MainInterface *owner = make_low_instance();
  MainInterface *other = make_low_instance();
  owner->menu_animation_bounds_ = RECT{10, 20, 40, 50};
  owner->multiplayer_animation_bounds_ = RECT{5, 25, 60, 45};
  other->menu_animation_bounds_ = RECT{100, 100, 110, 110};
  other->multiplayer_animation_bounds_ = RECT{101, 101, 111, 111};

  timer_callback_daemon(0, 0);
  assert(time_stops == 0 && update_calls == 0 && union_calls == 0);

  const int owner_context = static_cast<int>(reinterpret_cast<uintptr_t>(owner));
  const int other_context = static_cast<int>(reinterpret_cast<uintptr_t>(other));
  timer_callback_daemon(9, owner_context);
  assert(time_stops == 0 && update_calls == 0 && union_calls == 0);

  timer_callback_daemon(0, owner_context);
  assert(owner->time1_.stop_count == 1 && owner->time2_.stop_count == 0);
  assert(other->time1_.stop_count == 0 && time_stops == 1);
  assert(last_stopped == &owner->time1_);
  assert(update_calls == 0 && union_calls == 0);

  owner->alert_ = 0xffffffffu;
  timer_callback_daemon(1, owner_context);
  assert(owner->time2_.stop_count == 1 && owner->alert_ == 0xffffffffu);
  assert(time_stops == 2 && last_stopped == &owner->time2_);
  assert(update_calls == 0 && union_calls == 0);

  owner->alert_ = 7;
  timer_callback_daemon(1, other_context);
  assert(other->alert_ == 1 && owner->alert_ == 7);
  assert(update_calls == 1 && union_calls == 1);
  assert(union_left.left == other->menu_animation_bounds_.left);
  assert(union_right.left == other->multiplayer_animation_bounds_.left);
  assert(last_update_owner == static_cast<GraphicWin *>(other));
  assert(last_update_rect.left == 100 && last_update_rect.top == 100);
  assert(last_update_rect.right == 111 && last_update_rect.bottom == 111);

  other->alert_ = 0xfffffffeu;
  timer_callback_daemon(1, other_context);
  assert(other->alert_ == 0xffffffffu);
  assert(update_calls == 2 && union_calls == 2);

  munmap(owner, sizeof(MainInterface));
  munmap(other, sizeof(MainInterface));
  return 0;
}
