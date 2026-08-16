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
#pragma comment(lib, "Winmm.lib")
#include <mmsystem.h>

// The two bits of `Time::oneshot_state_`. See the member for what forces
// them to exist: `SetTimer` cannot do one-shot, so it is emulated.
static const int TimeOneShot = 1;
static const int TimeOneShotFired = 2;

 /*
  * Time class
  */
class DLLEXPORT Time {
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
  void stop();
  void close();
  void set_modal();     // 00616860
  void release_modal(); // 00616870

  // eventually make atomic for thread safety
  static Time *TimeModal;
  static int TimeInitCount;
  static void TimerProc(HWND UNUSED(hwnd) hwnd, uint32_t msg, UINT_PTR id_timer, DWORD elapsed);
  static void MultimediaProc(uint32_t timer_id, uint32_t msg, DWORD_PTR dw_user, DWORD_PTR dw1,
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
  void(__cdecl *callback2_)(int, int);
  int cb_param2_; // callback 2nd parameter
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
extern Time *TurnTimer;
extern Time *LineTimer;
extern Time *BlinkTimer;
extern Time *Blink2Timer;
extern Time *GoTimer;
extern Time *ConsoleTimer;

DLLEXPORT void __cdecl start_timers();
DLLEXPORT void __cdecl stop_timers();
DLLEXPORT void __cdecl flush_timer();
