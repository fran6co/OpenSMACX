/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <stdint.h>

// The pinned image has an observed 148-byte initial-data run at 0x0069704C;
// that run does not independently establish the historical allocation bound.
// The final word at 0x006970DC is shared: Dialog construction/reset read it,
// startup/preferences/menu/load update its flag bits, and color slot 3 writes
// it as DialogDefaults[33 + 3] (original store 0x00609D84). Keep all accesses
// on this one table cell; do not model the style role as separate storage or
// alter the original slot-3 write.
extern uint32_t DialogDefaults[37];

inline uint32_t &dialog_default_style() {
    return DialogDefaults[36];
}
