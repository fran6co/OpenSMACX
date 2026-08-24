/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <mmsystem.h>

// `palette.h` includes THIS header, so it cannot be included back - forward
// declared for the one pointer-typed field below.
class Palette;

// The two bits of `Time::oneshot_state_`. See the member for what forces
// them to exist: `SetTimer` cannot do one-shot, so it is emulated.
// WHAT THE IN-CLASS BODIES BELOW REACH FOR. `flush_timer` is declared at
// the foot of this header and `HandleMain` in temp.h, both of them after
// the point `Time::stop` needs them - and `stop` has to be in-class,
// because the image inlines it into `close` and `close` into `~Time`. A
// second declaration is safe: the compiler rejects one that disagrees.
void __cdecl flush_timer();
extern HWND HandleMain;

static const int TimeOneShot = 1;
static const int TimeOneShotFired = 2;

 /*
  * Time class
  */
class Time {
  // The timer posts WM_USER+1 to the main window and `Win::window_proc`
  // runs the tick: it clears `tick_posted_`, tests and sets `oneshot_state_`, and calls
  // `callback1_`/`callback2_` with the stored parameters. The original has
  // all of that INLINE in the procedure rather than in a `Time` method, so
  // it stays inline and `Win` is a friend; a `Time::tick()` would be one
  // `call` where the image has the whole sequence.
  friend class Win;

 public:
  Time(); // 006161D0
  ~Time(); // 00616200
  
  void init(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res);
  void init(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, uint32_t res);
  uint32_t start(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res);
  uint32_t start(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, 
                 uint32_t res);
  uint32_t pulse(void(__cdecl *callback)(int), int param, uint32_t cnt, uint32_t res);
  uint32_t pulse(void(__cdecl *callback)(int, int), int param, int param2, uint32_t cnt, 
                 uint32_t res);
  uint32_t start();
  uint32_t pulse();
  // IN-CLASS so `close` and `~Time` inline them, which is what the image
  // does: the destructor is the whole teardown, not a call to it.
  MEASURED void stop() {         // 00616730
    // HOISTED, as the image has it: the flush test is read BEFORE the
    // kill block, not after.
    const int had_flush = ~oneshot_state_ & TimeOneShot;
    if (id_event_) {
      if (count_ < 50) {
        timeKillEvent(id_event_);
      } else {
        KillTimer(HandleMain, id_event_);
      }
      id_event_ = 0;
    }
    if (had_flush) {
      flush_timer();
    }
  }
  MEASURED void close() {        // 00616780
    // IMAGE ORDER: oneshot_state_ after callback1_, resolution_ before
    // tick_posted_.
    stop();
    callback1_ = 0;
    oneshot_state_ = 0;
    callback2_ = 0;
    cb_param2_ = 0;
    cb_param1_ = 0;
    count_ = 0;
    resolution_ = 5;
    tick_posted_ = 0;
    unk_2_ = 0;
  }
  void set_modal();     // 00616860
  void release_modal(); // 00616870

  // eventually make atomic for thread safety
  static Time *TimeModal;
  static int TimeInitCount;
  // `CALLBACK`, i.e. __stdcall. These are handed to `SetTimer` and
  // `timeSetEvent`, which require it, and the image agrees: both end
  // `ret 0x10` and `ret 0x14`, cleaning their own arguments. Declared
  // `static` alone they are __cdecl and end a bare `ret`, which was the one
  // instruction each was short.
  static void CALLBACK TimerProc(HWND UNUSED(hwnd) hwnd, uint32_t msg,
                                 UINT_PTR id_timer, DWORD elapsed);
  static void CALLBACK MultimediaProc(uint32_t timer_id, uint32_t msg,
                                      DWORD_PTR dw_user, DWORD_PTR dw1,
                                      DWORD_PTR dw2);
  static int __cdecl init_class();  // 00616880
  static void __cdecl close_class(); // 00616890

 private:
  // ONE-SHOT EMULATION FOR THE `SetTimer` PATH, and nothing more general
  // than that. `timeSetEvent` has `TIME_ONESHOT` and `TIME_PERIODIC` and
  // picks between them directly; `SetTimer` has no one-shot mode at all, so
  // when `count_` is 50 or more and the timer goes through `SetTimer`, the
  // one-shot has to be emulated in software:
  //
  //   `pulse()`  sets   TimeOneShot        (one shot wanted)
  //   `start()`  clears TimeOneShot        (periodic wanted)
  //   the tick   sets   TimeOneShotFired   on the first tick, and calls
  //                                        `stop()` when it sees it again
  //   `stop()`   flushes queued ticks only when TimeOneShot is CLEAR - a
  //              timer that stopped itself has nothing to flush
  //
  // The tick lives in `Win::window_proc`'s WM_USER+1 arm, which is why
  // `Win` is a friend above.
  int oneshot_state_;
  UINT_PTR id_event_;
  void(__cdecl *callback1_)(int);
  // PRIVATE STORAGE ONLY - the PUBLIC two-parameter `init`/`start`/`pulse`
  // overloads below keep `void(__cdecl *)(int, int)` and `int param2`,
  // because that is what the image's own mangled names
  // (`?init@Time@@QAEXP6AXHH@ZHHHH@Z` and siblings) prove Time's wire
  // format to be. But `Palette::timer_callback` (0x005FEAD0, the only
  // caller of the two-parameter overload) documents that the value
  // actually carried is a `Palette *` - `t->init(Palette::timer_callback,
  // key, this, ...)` passes `this` where the public signature reads
  // `int param2`. Typing the STORAGE this way, with an explicit cast where
  // each public overload stores into it, records what the bytes cannot:
  // the same four bytes either way, since a pointer and an `int` are both
  // one word here.
  void(__cdecl *callback2_)(int, Palette *);
  Palette *cb_param2_; // callback 2nd parameter
  int cb_param1_; // callback 1st parameter
  uint32_t count_; // either delay (timeSetEvent) or elapsed (SetTimer) value
  // A TICK IS ALREADY IN FLIGHT. `TimerProc` and `MultimediaProc` are the
  // OS callbacks: each posts WM_USER+1 to the main window only when this is
  // zero, and sets it as it posts. `Win::window_proc` clears it as the
  // first thing it does with the message. So a timer whose ticks arrive
  // faster than the message loop drains them queues exactly one, instead of
  // a backlog.
  int tick_posted_;
  uint32_t resolution_;
  int unk_2_;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Time) == 0x28, "Time layout must match the legacy ABI");
#endif

// global
Time *const TurnTimer = (Time *)0x00915628;
Time *const LineTimer = (Time *)0x00915658;
Time *const BlinkTimer = (Time *)0x00915688;
Time *const Blink2Timer = (Time *)0x00939EB0;
Time *const GoTimer = (Time *)0x00939E60;
Time *const ConsoleTimer = (Time *)0x00939E88;

void __cdecl start_timers();
void __cdecl flush_timer();

MEASURED inline void __cdecl stop_timers() {
    Blink2Timer->close();
    BlinkTimer->close();
    LineTimer->close();
    // missing MP TurnTimer. TODO: In future determine if timer should be closed on stop
}
