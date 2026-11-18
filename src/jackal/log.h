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
  Log(); // 00625FB0
  Log(LPCSTR input);
  ~Log(); // 00626020

  int init(LPCSTR input);
  void reset();
  void say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
  void say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
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

void __cdecl log_reset();
void __cdecl log_say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
void __cdecl log_say(LPCSTR str1, int num1, int num2, int num3);
void __cdecl log_say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);
void __cdecl log_say_hex(LPCSTR str1, int num1, int num2, int num3);
void __cdecl log_set_state(BOOL state);
