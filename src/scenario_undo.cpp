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
#include "general.h"
#include "game.h"
#include "alpha.h"
#include "strings.h"
#include <cstring>
#include <cstdlib>

uint32_t ScenEditorUndoPosition = 1; // 0x00690D7C

/*
Purpose: Load a Scenario Editor undo (type: 1) or redo (type: -1) auto-save. TODO: Revisit in the 
         future to fix some of the underlying issues with the undo/redo process.
// ORIGINAL: 0x005ABE40 ?load_undo@@YAXH@Z 0x005ABE40-0x005ABEBF BYTE_EXACT
// LEVER: two real bugs in the committed body, fixed by transcription
//        instead of invention. (1) WRONG CALLEE - `sprintf_s` is not in the
//        image; it's the `strcat`/`_itoa`/`strcat`-into-StringTemp idiom,
//        no ".SAV" suffix, matching `wipe_undo`/`auto_undo` below. (2) the
//        image calls NEITHER an early `return` on `type==-1`, NOR
//        `draw_map` at the end - both were invented, not transcribed; the
//        image's only calls are the two `strcat`s, `_itoa` and
//        `load_daemon`. (3) `static_cast<int>(ScenEditorUndoPosition)` at
//        both comparisons - the global is `uint32_t`, and the image's own
//        `cmp`s are signed (`jle`/`jge`), giving `jbe`/`jae` uncast.
// size      127 bytes
// prototype void (__cdecl ?load_undo@@YAXH@Z)(int type)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005A9760 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl load_undo(int type) {
    if (type < 0 && static_cast<int>(ScenEditorUndoPosition) > 1) {
        ScenEditorUndoPosition--;
    }
    // THE PRE-sprintf IDIOM, same as `wipe_undo`/`auto_undo` immediately
    // below: `strcat`/`_itoa`/`strcat` into StringTemp, no ".SAV" extension
    // appended, and no `draw_map` call - the image's own call targets are
    // just strcat x2, _itoa and load_daemon.
    StringTemp[0] = 0;
    strcat(StringTemp, "saves\\auto\\Scenario Editor Undo ");
    char number[80];
    _itoa(ScenEditorUndoPosition, number, 10);
    strcat(StringTemp, number);
    if (type > 0 && static_cast<int>(ScenEditorUndoPosition) < 9) {
        ScenEditorUndoPosition++;
    }
    load_daemon(StringTemp, false);
}

/*
Purpose: Remove all the existing Scenario Editor undo auto-saves.
// ORIGINAL: 0x005ABEC0 ?wipe_undo@@YAXXZ 0x005ABEC0-0x005ABF14 BYTE_EXACT
// LEVER: WRONG CALLEE - `sprintf_s` is not in the image at all (the image has no C++/CRT sprintf for paths here); it builds the name with `strcat`/`_itoa`/`strcat` into the global `StringTemp` buffer (0x009B86A0) and calls `remove()` directly, with no ".SAV" extension appended - matching auto_undo's own documented idiom immediately below.
// size      84 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x00646950 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl wipe_undo() {
    for (int i = 9; i >= 1; i--) {
        StringTemp[0] = 0;
        strcat(StringTemp, "saves\\auto\\Scenario Editor Undo ");
        char number[80];
        _itoa(i, number, 10);
        strcat(StringTemp, number);
        remove(StringTemp);
    }
}

/*
Purpose: Handle the creation of an undo auto-save when certain Scenario Editor changes are made.
// ORIGINAL: 0x005ABF20 ?auto_undo@@YAXXZ 0x005ABF20-0x005ABFEF BYTE_EXACT
// size      207 bytes
// prototype 
// callers   32   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A94F0 0x006453E0 0x00645460 0x00645470 0x00646950 0x006469F7 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl auto_undo() {
    if (!(GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN)) {
        return;
    }
    // THREE 0x100 BUFFERS AND THE PRE-sprintf IDIOM. The image reserves 0x250
    // bytes at 0x005ABF23 and builds each name with `strcat`, `_itoa`,
    // `strcat` - no `sprintf` anywhere. It also carries the PREVIOUS name
    // forward rather than formatting it twice, and decides between `remove`
    // and `rename` on whether that carried name is empty, which is what makes
    // the first pass delete rather than rename.
    ScenEditorUndoPosition = 1;
    // 0x100, 0x100 and 0x50: the image's three locals sit at [ebp-0x250],
    // [ebp-0x150] and [ebp-0x50], which is where `sub esp, 0x250` comes from.
    char previous[256];
    char current[256];
    char number[80];
    previous[0] = 0;
    for (int i = 9; i >= 1; i--) {
        current[0] = 0;
        strcat(current, "saves\\auto\\Scenario Editor Undo ");
        _itoa(i, number, 10);
        strcat(current, number);
        if (strlen(previous) == 0) {
            remove(current);
        } else {
            // (previous, current), in that order: the image pushes the
            // current name and then the carried one, so the CARRIED name is
            // the first argument - it renames the name it built last pass
            // onto the one it just built, which collapses the history rather
            // than rotating it. That is what the shipped code does.
            rename(previous, current);
        }
        strcpy(previous, current);
    }
    // THE BUILT PATH, not a literal: the image pushes [ebp-0x250] at
    // 0x005ABFE1. The loop's last pass builds "Undo 1" and carries it there,
    // so the name is already in hand.
    save_daemon(previous);
}

