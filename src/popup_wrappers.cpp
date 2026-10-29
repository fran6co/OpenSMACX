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
#include "stdafx.h"
#include "popup.h"
#include "xpops.h"

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C320 ?popt@@YAHPADPBDHPADH@Z 0x0048C320-0x0048C353 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0048c320.cpp", "image:terranx_original.exe:0x0048C320"], "reason": "Recover tutorial popup configuration around the existing X_pop builder, restoring normal configuration after capturing the int result. Original call order establishes ownership and corrects a stale pointer-return declaration."}
// symbol    ?popt@@YAHPADPBDH0H@Z
int __cdecl popt(char *caption, const char *label, int value, char *text, int flags) {
    popups_tutorial();
    const int result = X_pop(caption, label, value, text, flags, nullptr);
    popups_normal();
    return result;
}
