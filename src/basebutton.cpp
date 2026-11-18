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
#include "basebutton.h"

// Application-region copy of a JACKAL.LIB class member. The image places
// BaseButton::hide at 0x00406860 inside the game object around Popup
// (0x00404860..0x004072CE), not in BaseButton's library member
// (src/jackal/basebutton.cpp, 0x00606F30..). The original was a header-inline
// COMDAT emitted by that game object. It stays out of line in this
// application unit until the header form is restored.

// ORIGINAL: 0x00406860 ?hide@BaseButton@@QAEXXZ 0x00406860-0x00406879 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original FlatButton constructor0x00607CFB installs table0x00669754; its slot2 targets the complete25-byte function0x00406860-0x00406879.", "Original0x00406860 tests the ButtonGroup pointer at+0xAA8, clears the dword at+0xA9C when null, then jumps to Win::hide0x005EDCD0. Existing BaseButton members group_ and field_A9C_ own these offsets.", "Historical seed git:5605e9af:src/recovered/units/00406860.cpp supplies the preserved original identity and span; the product override uses the existing GraphicWin/Win inheritance rather than raw self offsets."], "reason": "FlatButton primary vtable selects the BaseButton hide override, which conditionally clears its ungrouped state before invoking Win::hide."}
// symbol    ?hide@BaseButton@@UAEXXZ
void BaseButton::hide() {
    if (group_ == 0) {
        field_A9C_ = 0;
    }
    Win::hide();
}

