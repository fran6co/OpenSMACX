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

// The 0x00402EE0 helper passes the current element to its callback in ECX
// and walks the byte-strided region without constructor-prefix unwind.
typedef void(__fastcall *ArrayElementCallback)(void *element);

// The mapped 0x00402EE0 helper is distinct from the five-argument CRT ??_L
// iterator: it receives one typed element callback and performs no array
// unwind/cleanup if a callback throws.
// This shape is also consistent with MSVC's non-EH ??_H constructor iterator.
// Compiler/runtime provenance is unproved; this component name describes the
// observed operation, not an established original application-owned utility.
extern "C" void __stdcall initialize_strided_array(
    void *array, unsigned int element_size, int count,
    ArrayElementCallback callback);
