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
 * The four globals WinMain writes before anything else in the process runs.
 *
 * They are declared here rather than in the header of whichever subsystem
 * later reads them because the startup order is the fact worth keeping: every
 * one of these is set by 0x0045F950 and read by code that runs after it, so
 * "who owns this" is answered by the entry point and nothing else.
 *
 * Each is an OBJECT, not a pointer to the address in the shipped image. See
 * the note on `g_JACKAL_FONT` in atexit_thunks.h: the pointer form is what a
 * DLL injected into terranx.exe needed, and this is no longer that.
 *
 * WHERE THEIR INITIAL VALUES COME FROM, which is a question with a mechanical
 * answer. `.data` runs 0x00682000-0x009C21F8 but the executable only STORES
 * the first 0x26000 bytes of it, so:
 *
 *     below 0x006A8000   the image holds the bytes. Read them and write the
 *                        value into the source - `DefaultFontFace` in font.h
 *                        is 0x00691B2C holding 0x00691BD4, which is "Arial",
 *                        and `DialogDefaultStyle` below is a stored 3.
 *     at or above        zero-fill. No bytes exist in the file, so C++
 *                        zero-initialisation at namespace scope IS the
 *                        recovered value, exactly, and there is nothing to
 *                        look up.
 *
 * Three of the four below are above the line. That is not a gap in the
 * recovery; it is the recovery.
 */

// 0x007D3970, and ABOVE the line - a zero-filled buffer, so this declaration
// already carries its whole initial state. WinMain empties it and appends its
// `lpCmdLine`; `control_game` (0x0052AA30) is the only other reader in the
// image.
//
// The BOUND is derived and not proved, and being zero-fill is why it cannot
// be: there are no bytes to find an end in. 0x007D3A78 is the next address
// the recovered corpus references, which caps the array at 0x108 and does not
// establish that it is that large - an unreferenced neighbour would sit
// inside this range and never show.
extern char CommandLineText[0x108];

// 0x00696ECC. Which popup allocator the game uses. See the comment in
// src/main.cpp on why WinMain stores into it twice.
class BasePop;
typedef BasePop *(__cdecl func_popup_alloc)();
extern func_popup_alloc *PopupAllocHook;

// 0x009469FC. Non-zero while a modal `BasePop::exec` is on the stack;
// `Popup::calculate_dimensions` (0x00404B10) and `thumb_routine` (0x005898A0)
// are the other two functions in the corpus that touch it. WinMain clears it.
extern int PopupModalActive;

// 0x006970DC, the last word of the four-int run at 0x006970D0 that `Dialog`'s
// constructor copies into itself as `field_20_`. WinMain sets bit 2 of it
// before any dialog exists, so it is a default rather than live state;
// `PrefWin::display`, `top_menu` and `load_game` are its other users.
//
// BELOW the line, so its initial value is in the image and recovered here
// rather than assumed: the run reads [1, 1, 1, 3] and this is the 3.
extern int DialogDefaultStyle;
