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

#include "original_seam.h"

/*
 * The CRT's vector destructor iterator: walk an array, calling one teardown
 * per element. Several recovered teardowns dispatch through it - the atexit
 * array thunks, the FX effect bank, the FontQueue - so the seam lives in its
 * own translation unit with no further dependencies, and each caller carries
 * its own per-element teardown seam.
 */
typedef void (OriginalObject::*func_thiscall_teardown)();
typedef void(__stdcall func_vector_dtor_iterator)(
    void *array, unsigned int element_size, int count,
    const void *teardown);
func_vector_dtor_iterator *const VectorDtorIterator = (func_vector_dtor_iterator *)0x006456E4;

// Its construction-side companion: walk an array calling one constructor per
// element, with the destructor along for exception unwind (unreachable here,
// but passed faithfully).
// THE CALLBACKS ARE ADDRESSES, not pointer-to-member VARIABLES. The image
// pushes them as immediates - `push 0x4c67c0` at 0x00445450 - and a
// `func_thiscall_teardown` built by `original_method` at load time is a
// variable, so every caller pushed `dword ptr [...]` instead.
typedef void(__stdcall func_vector_ctor_iterator)(
    void *array, unsigned int element_size, int count,
    const void *ctor, const void *dtor);
func_vector_ctor_iterator *const VectorCtorIterator = (func_vector_ctor_iterator *)0x006457C2;

// THE LAST TWO INSTRUCTIONS OF EVERY ARRAY THUNK, and there is no C++ for
// them. `??_L@YGXPAXIHP6EX0@Z1@Z` and `??_M@...` are the compiler's own vector
// constructor and destructor iterators: VC6 emits a `call rel32` to them when
// it generates an array construction, and nothing a program can WRITE names
// them - a declaration cannot carry a `??_` mangling, and array placement new
// is not equivalent because MSVC stores a count cookie ahead of the array and
// returns a shifted pointer.
//
// So the pointer stays, and 101 array thunks are capped two instructions short
// of the image. What was fixable IS fixed: the element callbacks below are
// addresses rather than pointer-to-member variables, because the image pushes
// them as immediates - `push 0x4c67c0` - and a variable built by
// `original_method` at load time pushed `dword ptr [...]` instead.

// The game's own operator new, ??2@YAPAXI@Z at 0x0064558A - a 14-byte
// _nh_malloc(size, 1) forwarder that answers null rather than raising.
// Anything the recovered code allocates and the original frees has to come
// from here, so it lives beside the iterators for the same reason they do: it
// has three callers already (Wave, Sound, Wave_Device) and now a fourth in
// GraphicWin::init, and it must reach every one of them without dragging the
// audio closure into their link. The name records the recovery that first
// bound it, not an ownership claim - it is the process-wide allocator.
typedef void *(__cdecl func_operator_new)(unsigned int size);
func_operator_new *const WaveOperatorNew = (func_operator_new *)0x0064558A;
