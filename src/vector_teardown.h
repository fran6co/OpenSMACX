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

// 0x00402EE0 takes the element callback in ECX and walks the byte-strided
// region without adding constructor-prefix unwind behavior.
typedef void(__fastcall *func_vector_element_callback)(void *element);

// The 0x00402EE0 game helper is distinct from the five-argument CRT ??_L
// iterator above: it receives one typed element callback and performs no array
// unwind/cleanup if a callback throws.
extern "C" void __stdcall sub_402ee0(void *array, unsigned int element_size,
                                      int count, func_vector_element_callback callback);

// Audio and wave owners use the configured runtime's direct global operator new, matching the calls already present in Midi.
