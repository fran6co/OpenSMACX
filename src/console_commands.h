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

#include "console.h"
#include "strings.h"
#include <cstdlib>
#include <cstring>

// Shared Console state retains one definition in the UI owner.
extern char TutorialPopupCaptionText[6];
extern char *TutorialPopupCaption;
extern int ConsoleGroupAction;
extern int ConsoleControlTurnActive;
extern Console ConsoleGlobal;

typedef int (__cdecl *NullCallback)(void);

// Local source utility, not a separately mapped original entry: both
// original UI/editor callers repeat the formatting operation.
static void parse_field_number(int index, int value) {
    StringTemp[0] = 0;
    say_num(value);
    parse_says(index, StringTemp, -1, -1);
}
