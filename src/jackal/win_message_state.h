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
#include <stdint.h>

/*
 * AN OBJECT, NOT A `uint32_t *` TO A FIXED ADDRESS. The pointer form costs a
 * load the image does not perform: `MsgStatus |= 0x3F` is
 * `mov ecx, [0x9b7b9c]` in the image and `mov eax, [MsgStatus]` followed by
 * `mov ecx, [eax]` through a pointer, which is what held `flush_timer` at
 * 4 of 44 instructions. It also cannot work outside the shipped image - the
 * address is in terranx.exe's data, unmapped in a standalone build, so every
 * use through it is a wild pointer.
 */
extern uint32_t MsgStatus;  // 0x009B7B9C
