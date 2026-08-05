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

/*
 * The CRT's vector destructor iterator: walk an array, calling one teardown
 * per element. Several recovered teardowns dispatch through it - the atexit
 * array thunks, the FX effect bank, the FontQueue - so the seam lives in its
 * own translation unit with no further dependencies, and each caller carries
 * its own per-element teardown seam.
 */
typedef void(__thiscall func_thiscall_teardown)(void *object);
typedef void(__stdcall func_vector_dtor_iterator)(
    void *array, unsigned int element_size, int count,
    func_thiscall_teardown *teardown);
extern func_vector_dtor_iterator *VectorDtorIterator;

// Its construction-side companion: walk an array calling one constructor per
// element, with the destructor along for exception unwind (unreachable here,
// but passed faithfully).
typedef void(__stdcall func_vector_ctor_iterator)(
    void *array, unsigned int element_size, int count,
    func_thiscall_teardown *ctor, func_thiscall_teardown *dtor);
extern func_vector_ctor_iterator *VectorCtorIterator;

// The game's own operator new, ??2@YAPAXI@Z at 0x0064558A - a 14-byte
// _nh_malloc(size, 1) forwarder that answers null rather than raising.
// Anything the recovered code allocates and the original frees has to come
// from here, so it lives beside the iterators for the same reason they do: it
// has three callers already (Wave, Sound, Wave_Device) and now a fourth in
// GraphicWin::init, and it must reach every one of them without dragging the
// audio closure into their link. The name records the recovery that first
// bound it, not an ownership claim - it is the process-wide allocator.
typedef void *(__cdecl func_operator_new)(unsigned int size);
extern func_operator_new *WaveOperatorNew;
