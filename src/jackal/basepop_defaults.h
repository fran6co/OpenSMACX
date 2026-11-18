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

#include <stddef.h>
#include <stdint.h>

// Shared implementation defaults used by BasePop and the Popup producer.
struct BasePopButtonStyle {
  uint8_t button_color;
  uint8_t unresolved_31A1_[3];
  uint32_t bevel_upper;
  uint32_t bevel_lower;
  uint32_t bevel_thickness;
};
static_assert(sizeof(BasePopButtonStyle) == 0x10,
              "BasePop button-style defaults size");
static_assert(offsetof(BasePopButtonStyle, button_color) == 0x00,
              "BasePop button color offset");
static_assert(offsetof(BasePopButtonStyle, unresolved_31A1_) == 0x01,
              "BasePop unresolved bytes offset");
static_assert(offsetof(BasePopButtonStyle, bevel_upper) == 0x04,
              "BasePop upper bevel offset");
static_assert(offsetof(BasePopButtonStyle, bevel_lower) == 0x08,
              "BasePop lower bevel offset");
static_assert(offsetof(BasePopButtonStyle, bevel_thickness) == 0x0C,
              "BasePop bevel thickness offset");
extern BasePopButtonStyle BasePopButtonDefaults;
