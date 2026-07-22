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
#include "buffer.h"
#include "win.h"

 /*
  * GraphicWin class
  */
class DLLEXPORT GraphicWin : Win {
  friend class Scroll;
  // BaseButton's colour setters drive this buffer directly.
  friend class BaseButton;

 public:
  GraphicWin() { ; }
  ~GraphicWin() { ; }
  void construct();
  uint32_t close();
 private:
  Buffer buffer_;
  uint32_t field_9CC_;
  uint32_t field_9D0_;
  uint32_t field_9D4_;
  uint32_t field_9D8_;
  uint32_t field_9DC_;
  uint32_t field_9E0_;
  uint32_t field_9E4_;
  uint32_t field_9E8_;
  uint32_t field_9EC_;
  uint32_t field_9F0_;
  uint32_t field_9F4_;
  uint32_t field_9F8_;
  uint32_t field_9FC_;
  uint32_t field_A00_;
  uint32_t field_A04_;
  uint32_t field_A08_;
  uint32_t field_A0C_;
  uint32_t field_A10_;
};

static_assert(sizeof(GraphicWin) == 0xA14,
              "GraphicWin layout must match the legacy ABI");

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef void(__thiscall func_subobject_destructor)(void *);
typedef void(__thiscall func_subobject_close)(void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The Buffer subobject destructor is source-owned; the seam stays bindable so
// tests outside the hybrid process can substitute it. The Win subobject
// destructor is still a temporary original dependency.
// tear down GDI and DirectDraw resources through the executable's own CRT and
// stay at their original addresses until that closure is source-owned. Tests
// outside the hybrid process rebind them.
extern func_subobject_destructor *BufferSubobjectDestructor;
extern func_subobject_destructor *WinOriginalDestructor;

// GraphicWin::close keeps Win::close as a temporary original dependency;
// Buffer::close is source-owned. Both remain bindable so source-level tests
// can verify delegation order without entering process-owned teardown code.
extern func_subobject_close *BufferSubobjectClose;
extern func_subobject_close *WinOriginalClose;

// Process default copied into field_A0C_ by close().
extern uint32_t *GraphicWinFieldA0CDefault;

// Original virtual table addresses the destructor installs before delegating.
extern const uint32_t GraphicWinPrimaryVtable;
extern const uint32_t GraphicWinBufferVtable;

GraphicWin *__fastcall graphic_win_destructor_redirect(GraphicWin *self, void *);
GraphicWin *__fastcall graphic_win_construct_redirect(GraphicWin *self, void *);
uint32_t __fastcall graphic_win_close_redirect(GraphicWin *self, void *);

// Test seams: the subobject destructors are original dependencies, so tests
// substitute recording stubs to observe delegation targets and ordering.
void graphic_win_destructor_probe_reset();
int graphic_win_destructor_probe_buffer_calls();
int graphic_win_destructor_probe_win_calls();
void *graphic_win_destructor_probe_buffer_target();
void *graphic_win_destructor_probe_win_target();
int graphic_win_destructor_probe_order();
