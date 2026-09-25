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
#include "newtechwin.h"
#include "game.h"

// Pending technology hyperlink selected by4838E0; original exec483F90
// consumes it after releasing its modal loop and resets it to-1. That exec
// body is still a seed: this binding does not establish working popup flow.
// RECOVERY_OBJECT: v2 0x00686960 0x00686964 _PendingTechnologyLink 0x004838E0
static int PendingTechnologyLink = -1;

// ORIGINAL: 0x004838E0 ?on_link_clicked@NewTechWin@@QAEHHPAD@Z 0x004838E0-0x0048391F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original table66CE5C selects4838E0 in slot88. Full body checks IsMultiplayerNet93F660 and nonnull text; atoi645DA7 result is stored686960 before virtual release_modal58; returns0 only for admitted links, otherwise1.", "PendingTechnologyLink is the source owner of initial-1 DWORD686960. Consumer/reset48479F/4847CE in unrecovered NewTechWin::exec483F90 remain explicit restoration work; this isolated handler is not a working popup or game acceptance."], "reason": "Recover original GraphicWin link-hook overrides through the genuine slot88 hierarchy, preserving seed identity and historical obligations."}
// symbol    ?on_link_clicked@NewTechWin@@UAEHHPAD@Z
int NewTechWin::on_link_clicked(int, char *link) {
    if (IsMultiplayerNet == 0 && link != 0) {
        PendingTechnologyLink = atoi(link);
        release_modal();
        return 0;
    }
    return 1;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00483E90 ?on_left_click@NewTechWin@@QAEXHH@Z 0x00483E90-0x00483E93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_click@NewTechWin@@QAEXHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NewTechWin::on_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x00483EA0 ?on_key_click@NewTechWin@@QAEHHH@Z 0x00483EA0-0x00483EA8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@NewTechWin@@QAEHHH@Z)(NewTechWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int NewTechWin::on_key_click(int, int) {
    return 1;
}

// Original initializer 0x483880 constructs six Sprites at stride 0x2C;
// initial storage is zero. Registered teardown 0x4838B0 tests/sets mask0x01 of
// byte0x822604 before destruction; this initial-data binding does not establish
// that guard or complete startup/lifetime equivalence.
// RECOVERY_OBJECT: v2 0x00822610 0x00822718 ?g_NEWTECHWIN_SPRITES@@3PAVSprite@@A 0x00453A70
Sprite g_NEWTECHWIN_SPRITES[6];
