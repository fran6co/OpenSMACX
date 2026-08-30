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
#include "general.h"

extern BOOL IsLoggingDisabled;

 /*
  * Log class: Handles debug logging.
  */
class Log {
 public:
  MEASURED Log() : log_file_(nullptr), is_disabled_(false) {
  } // 00625FB0
  // NOT a constructor: `??__ELogging` reaches this through an ordinary
  // method call (`Logging->construct(...)`), not a `new`-expression. VC6
  // wraps a placement-new'd, non-trivially-destructible object in an SEH
  // frame when its constructor calls an external (non-intrinsic) function,
  // to run the destructor if construction throws - the image has none, so
  // the original reached this some other way. An ordinary member function
  // is just a call, and drops the frame entirely. // 00625FC0
  Log *construct(LPCSTR input) {
      log_file_ = nullptr;
      if (input) {
          void *buf = mem_get(strlen(input) + 1);
          log_file_ = (LPSTR)buf;
          if (buf) {
              *(char *)buf = '\0';
              strcat(log_file_, input);
              FILE *file = env_open(input, "wt");
              if (file) {
                  fclose(file);
              }
          }
      }
      return this;
  }
  MEASURED ~Log() {
      if (log_file_) {
          free(log_file_);
          *reinterpret_cast<LPSTR volatile *>(&log_file_) = nullptr;
      }
  } // 00626020

  int init(LPCSTR input);
  // The image inlines reset/say/say_hex into every `Logging->...()` caller
  // in log.cpp rather than emitting a call to these; defined in-class so
  // this unit's own callers reproduce that. Each still carries its own
  // ORIGINAL marker, moved to log.cpp below the class (see "DEFINED IN THE
  // HEADER, CLAIMED HERE").
  MEASURED void reset() {         // 006260D0
      FILE *file = env_open(log_file_, "wt");
      if (file) {
          fclose(file);
      }
  }
  MEASURED void say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {   // 006260F0
      if (!log_file_ || is_disabled_ || IsLoggingDisabled) {
          return;
      }
      FILE *file = env_open(log_file_, "at");
      if (file) {
          str2 ? fprintf_s(file, "%s %s %d %d %d\n", str1, str2, num1, num2, num3)
              : fprintf_s(file, "%s %d %d %d\n", str1, num1, num2, num3);
          fclose(file);
      }
  }
  MEASURED void say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {   // 00626190
      if (!log_file_ || is_disabled_ || IsLoggingDisabled) {
          return;
      }
      FILE *file = env_open(log_file_, "at");
      if (file) {
          str2 ? fprintf_s(file, "%s %s %04x %04x %04x\n", str1, str2, num1, num2, num3)
              : fprintf_s(file, "%s %04x %04x %04x\n", str1, num1, num2, num3);
          fclose(file);
      }
  }
  // additional functions to assist with encapsulation
  void set_state(BOOL state) { is_disabled_ = (state == 0); }

 private:
  LPSTR log_file_;  // 0x0000
  BOOL is_disabled_;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Log) == 8, "Log layout must match the legacy ABI");
#endif

// global
// THE OBJECT, at 0x009BBFF8 in the shipped image - REAL STORAGE, defined in
// log.cpp. It was `Log *const Logging = (Log *)0x009BBFF8`, naming terranx.exe
// data that is unmapped in a standalone build; every `Logging->` read and
// wrote through that address. The object keeps the folded access the binding
// bought - `mov ecx, 0x9bbff8` receivers become `mov ecx, <reloc>`, and the
// field loads the image folds straight to `[0x9bbff8]` stay direct memory
// operands off the object - while `->` becomes `.`.
extern Log Logging;  // 0x009BBFF8

void __cdecl log_logging();
void __cdecl log_logging_exit();
void __cdecl log_reset();
void __cdecl log_say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
void __cdecl log_say(LPCSTR str1, int num1, int num2, int num3);
void __cdecl log_say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
void __cdecl log_say_hex(LPCSTR str1, int num1, int num2, int num3);
void __cdecl log_set_state(BOOL state);
