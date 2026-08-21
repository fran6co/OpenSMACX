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
#include "original_seam.h"
#include "console.h"
#include "game.h"
#include "general.h"
#include "infowin.h"
#include "map.h"
#include "mapwin.h"
#include "statuswin.h"
#include "temp.h"
#include "fx.h"
#include "hypothesis_layouts.h"
#include "graphicwin.h"
#include "spritebox.h"
#include <cstring>

PrefWin *ConsolePrefWin= (PrefWin *)(0x008578D8);

/*
Purpose: Open the shared preferences window to the preferences page.
// ORIGINAL: 0x00514EF0 ?set_preferences@Console@@QAEXXZ 0x00514EF0-0x00514EFD
// size      13 bytes
// prototype void (__thiscall ?set_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_preferences() {
    ConsolePrefWin->PrefWin::display(0);
}

/*
Purpose: Open the shared preferences window to the automation page.
// ORIGINAL: 0x00514F10 ?set_auto_preferences@Console@@QAEXXZ 0x00514F10-0x00514F1D
// size      13 bytes
// prototype void (__thiscall ?set_auto_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_auto_preferences() {
    ConsolePrefWin->PrefWin::display(3);
}

/*
Purpose: Open the shared preferences window to the base page.
// ORIGINAL: 0x00514F20 ?set_base_preferences@Console@@QAEXXZ 0x00514F20-0x00514F2D
// size      13 bytes
// prototype void (__thiscall ?set_base_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_base_preferences() {
    ConsolePrefWin->PrefWin::display(2);
}

/*
Purpose: Open the shared preferences window to the audiovisual page.
// ORIGINAL: 0x00514F30 ?set_audiovisual@Console@@QAEXXZ 0x00514F30-0x00514F3D
// size      13 bytes
// prototype void (__thiscall ?set_audiovisual@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_audiovisual() {
    ConsolePrefWin->PrefWin::display(4);
}

/*
Purpose: Open the shared preferences window to the map-display page.
// ORIGINAL: 0x00514F40 ?set_map_display@Console@@QAEXXZ 0x00514F40-0x00514F4D
// size      13 bytes
// prototype void (__thiscall ?set_map_display@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_map_display() {
    ConsolePrefWin->PrefWin::display(5);
}

void __fastcall console_set_preferences_redirect(Console *self, void *) {
    self->set_preferences();
}

void __fastcall console_set_auto_preferences_redirect(Console *self, void *) {
    self->set_auto_preferences();
}

void __fastcall console_set_base_preferences_redirect(Console *self, void *) {
    self->set_base_preferences();
}

void __fastcall console_set_audiovisual_redirect(Console *self, void *) {
    self->set_audiovisual();
}

void __fastcall console_set_map_display_redirect(Console *self, void *) {
    self->set_map_display();
}


/*
Purpose: Clear the console's active-group field and drop the highlight bit
         (0x08000000) from every entry of the group table, at its 0x34 stride.
// ORIGINAL: 0x0050F650 ?clear_group@Console@@QAEXXZ 0x0050F650-0x0050F67B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?clear_group@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
Verification note: the sweep's surviving swaps reorder the object-field clear
against the group-count load and one declaration against another; both pairs
touch disjoint state, so the order is unobservable and the mutants are
equivalent by construction.
*/
void Console::clear_group() {
    const int32_t zero = 0;
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x23D1C, &zero, sizeof(zero));
    const int32_t count = *ConsoleGroupCount;
    for (int32_t index = 0; index < count; ++index) {
        uint8_t *const entry = ConsoleGroupTable + index * 0x34;
        uint32_t value;
        std::memcpy(&value, entry, sizeof(value));
        value &= 0xF7FFFFFFu;
        std::memcpy(entry, &value, sizeof(value));
    }
}

void __fastcall console_clear_group_redirect(Console *self, void *) {
    self->clear_group();
}

/*
Purpose: Report whether editing is locked out. Only meaningful in the scenario
         editor; there editing is locked whenever Scroll Lock is toggled on or
         the game is in editor-only mode.
// ORIGINAL: 0x004E1F40 ?edit_lock@Console@@QAEHXZ 0x004E1F40-0x004E1F70
// size      48 bytes
// prototype int (__thiscall ?edit_lock@Console@@QAEHXZ)(Console* this)
// callers   8   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004E1F51
Return Value: 1 when editing is locked, 0 otherwise
Status: Complete
*/
int Console::edit_lock() {
    if (!(GameState & STATE_SCENARIO_EDITOR)) {
        return 0;
    }
    if (((*ConsoleEditKeyStateSlot)(VK_SCROLL) & 1) ||
        (GameState & STATE_EDITOR_ONLY_MODE)) {
        return 1;
    }
    return 0;
}

int __fastcall console_edit_lock_redirect(Console *self, void *) {
    return self->edit_lock();
}

/*
Purpose: Open the shared preferences window to the advanced page.
// ORIGINAL: 0x00514F00 ?set_adv_preferences@Console@@QAEXXZ 0x00514F00-0x00514F0D
// size      13 bytes
// prototype void (__thiscall ?set_adv_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_adv_preferences() {
    ConsolePrefWin->PrefWin::display(1);
}

/*
Purpose: Undo the last scenario-editor change, by asking load_undo for the
         editor's undo slot.
// ORIGINAL: 0x004E1F20 ?editor_undo@Console@@QAEXXZ 0x004E1F20-0x004E1F29 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_undo@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ABE40
Return Value: n/a
Status: Complete
*/
void Console::editor_undo() {
    load_undo(1);
}

void __fastcall console_set_adv_preferences_redirect(Console *self, void *) {
    self->set_adv_preferences();
}

void __fastcall console_editor_undo_redirect(Console *self, void *) {
    self->editor_undo();
}

StatusWin *ConsoleStatusWin= (StatusWin *)(0x008C5568);

/*
Purpose: Refresh everything the console shows after a selection or turn change:
         hand the change code to the shared InfoWin, redraw the shared
         StatusWin, then push the map's caption into the main interface.
// ORIGINAL: 0x00514880 ?update_data@Console@@QAEXH@Z 0x00514880-0x005148AA
// size      42 bytes
// prototype void (__thiscall ?update_data@Console@@QAEXH@Z)(Console* this, int)
// callers   33   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458900 0x0046FB10 0x004B9EA0
Return Value: n/a
Status: Complete with a temporary StatusWin::redraw original dependency
Verification note: three properties of this body are unobservable at every
oracle tier, because the original's own callees discard them rather than
because the fixtures are weak.
  1. InfoWin::change at 0x00458900 is a bare `ret 4` - it drops both its
     `this` and its argument - so neither the call nor the value of a1 has any
     effect to observe. src/infowin.cpp models it as an empty body.
  2. MapWin::main_caption at 0x0046FB10 overwrites ecx before using it, so the
     MapWin pointer loaded out of ConsoleMapWinSlot never reaches anything
     that reads it.
  3. The original leaves EAX holding main_caption's residue, whose chain is
     already terminated by the committed `void MapWin::main_caption()`, so
     there is no residue available to return and fabricating a constant would
     be wrong. Kept `void`, matching the mangled name.
Mutation-harness survivors for the dropped `change` call and for a perturbed
a1 are therefore equivalent by construction.
*/
void Console::update_data(int a1) {
    // 0x00514883 mov eax,[ebp+8] / 0x00514886 mov ecx,0x7ad2a0 /
    // 0x0051488B push eax / 0x0051488C call 0x458900.
    reinterpret_cast<InfoWin *>(ConsoleInfoWin)->change(a1);
    // 0x00514891 mov ecx,0x8c5568 / 0x00514896 call 0x4b9ea0.
    ConsoleStatusWin->StatusWin::redraw();
    // 0x0051489B mov ecx,[0x7d3c3c] - a load, not an address-of - then
    // 0x005148A1 call 0x46fb10. The slot is read here, never cached.
    reinterpret_cast<MapWin *>(*ConsoleMapWinSlot)->main_caption();
}

// `ret 4` on the original: one 4-byte stack argument plus the ecx `this`. The
// entry needs no this-adjustment - update_data is entered on the Console
// itself, not through a virtual-base adjustor.
void __fastcall console_update_data_redirect(Console *self, void *, int a1) {
    self->update_data(a1);
}

func_console_map_win_focus ConsoleOriginalMapWinFocus =
    original_method<func_console_map_win_focus>(0x0046B310);
Console *ConsoleGlobal= (Console *)(0x009156B0);
func_main_menu_check ConsoleOriginalMainMenuCheck =
    original_method<func_main_menu_check>(0x00460DD0);

/*
Purpose: Point the map windows at one tile on behalf of one faction. Build a
         view-tag mask from the faction argument, walk the eight map-window
         slots, and hand the coordinate to every window whose tag the mask
         selects. The primary window, slot 0, is handled quite differently from
         the rest, and only its success counts: when it moves, the queued input
         is flushed and 1 is returned.
// ORIGINAL: 0x005108A0 ?focus@Console@@QAEXHHH@Z 0x005108A0-0x005109A3
// symbol    ?focus@Console@@QAEHHHH@Z
// size      259 bytes
// prototype void (__thiscall ?focus@Console@@QAEXHHH@Z)(Console* this, int xCoord, int yCoord, int factionID)
// callers   32   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046B310 0x005109B0 0x005FD120
Return Value: 1 when the primary map window took the coordinate, 0 otherwise
Status: Complete with temporary Console::cursor_next, MapWin::focus,
        MapWin::draw_map and flush_input original dependencies
Verification note: the catalogued mangled name ?focus@Console@@QAEXHHH@Z says
        void and is wrong. EAX is loaded from the [ebp-4] flag slot at
        0x0051098B and 0x0051099A, and 12 of the 61 call sites branch on it two
        instructions later - 0x004F7D7D, 0x00508838 and 0x0050D606 among them.
        Declared int; do not "fix" it back on the strength of the name.
Verification note: slot 0 is special in FOUR distinct ways, each transcribed at
        the address that decides it.
          1. 0x005108ED / 0x005108EF - slot 0 skips the per-window activity gate
             at 0x1DD74 entirely, the same exemption draw_tile makes.
          2. 0x0051090C / 0x0051090E - an UNTAGGED slot 1..7 is dropped, while
             an untagged slot 0 falls through to 0x00510910 and gets a second
             chance from the control-turn global.
          3. 0x0051091B / 0x0051091D - a TAGGED slot 1..7 goes straight to the
             plain focus call at 0x00510964, while a tagged slot 0 runs the
             console cursor and the survey-overlay latch first.
          4. 0x00510976 / 0x00510978 - a successful focus on slots 1..7 does NOT
             raise the flag. Only slot 0 can set it, so only slot 0 can reach
             flush_input or make this function return 1.
Verification note: the reload of the flag slot at 0x0051099A, after flush_input
        returns, is a register-clobber reload of a local this frame owns.
        flush_input is a cdecl no-argument function and cannot touch it, so the
        reload is not observable and is transcribed as one `return focused;`.
Verification note: the mask is computed once at 0x005108C1-0x005108D5 and then
        reloaded from the [ebp+0x10] argument slot at 0x005108FF on every
        iteration, and `this` is reloaded from [ebp-8] at 0x0051092B. Nothing
        this body calls can write a caller-pushed argument slot or another
        frame's local, so both are register-allocation artifacts and the single
        locals below are faithful. The MapWinTable reloads are NOT in that
        class and are kept - see their own comments.
Verification note: the original has no null check other than the slot test at
        0x005108E5. It does not defend against a null `this`, a null
        ConsoleGlobal, a null seam, or a null MapWinTable[0] at 0x00510946 and
        0x00510955 - the last two genuinely reachable if a callee empties the
        slot mid-iteration, and the original would fault. That absence is
        deliberate and is not repaired here.
*/
int Console::focus(int x_coord, int y_coord, int faction_id) {
    // 0x005108AD `cmp eax, esi` against an already-zeroed ESI, then 0x005108B6
    // `jge`: a negative faction takes 0x60000000 and matches both view tags at
    // once. Otherwise the neg/sbb/and/add idiom at 0x005108C1-0x005108D5 folds
    // one equality into a constant, giving 0x40000000 for a different faction
    // and 0x20000000 for the local one. LocalFaction is read only on the
    // non-negative path, exactly as the original's branch structure has it.
    uint32_t mask;
    if (faction_id < 0) {
        mask = 0x60000000U;
    } else {
        mask = (faction_id != LocalFaction) ? 0x40000000U : 0x20000000U;
    }
    // 0x005108B3 `mov [ebp-4], esi` with ESI zero. The literal 1 at 0x0051097A
    // is the ONLY store into that slot, so this is a plain 0/1 flag and the
    // returns are exactly 0 and 1, never a callee's residue.
    int focused = 0;
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        // 0x005108DE `mov eax, [esi*4 + 0x7d3c3c]`. Re-read every iteration,
        // do NOT hoist: cursor_next, MapWin::focus and MapWin::draw_map below
        // can republish the table, and later slots must see that.
        MapWin *const window = MapWinTable[slot];
        // 0x005108E5 test / 0x005108E7 je. The null test runs FIRST, so an
        // empty slot 0 is skipped despite every exemption below.
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x00510902 reads the window's wide flag dword, one dword ahead of the
        // activity field mapwin.h already names. Kept as a documented literal
        // rather than a constant in another class's header: MapWin owns the
        // field, and console.h is not the place to name it. 0x00510908
        // `test ecx, edx` is a bitwise AND against the tag bits, not an
        // equality, so a window carrying 0x60000000 matches either mask.
        const uint32_t flags = *reinterpret_cast<const volatile uint32_t *>(
            reinterpret_cast<const uint8_t *>(window) + 0x1DD70);
        const bool tagged = (mask & flags) != 0;
        if (slot == 0) {
            // Specialisation 2. Short-circuit order matches the original: the
            // control-turn global is read only when the tag missed.
            if (!tagged && *ConsoleControlTurnActive == 0) {
                continue;
            }
            // Specialisation 3. cursor_next runs on the process-wide Console at
            // 0x009156B0, NOT on `this`. Every call site happens to enter focus
            // with the same object, but the constant is in the instruction
            // stream and is transcribed as one.
            ConsoleGlobal->cursor_next(x_coord, y_coord);
            // 0x0051092B reloads `this`, 0x0051092E reads the survey-overlay
            // latch. Read AFTER the call, do NOT hoist: this comes from `this`
            // while cursor_next was handed ConsoleGlobal, and cursor_next
            // writes its own object's fields in this region. Held as a raw
            // offset into the unmapped derived storage, the way clear_group
            // holds 0x23D1C.
            volatile uint32_t *const overlay =
                reinterpret_cast<volatile uint32_t *>(
                    reinterpret_cast<uint8_t *>(this) + 0x23C00);
            if (*overlay != 0) {
                // 0x0051093C clears the latch BEFORE the table load at
                // 0x00510946 and before the call at 0x0051094C. The clear is
                // ordered against MapWin::focus, not folded into it.
                *overlay = 0;
                // 0x00510946 reads slot 0 by absolute address, with no [esi*4]
                // index, and as a fresh load. Do NOT reuse `window`:
                // cursor_next may have republished the table.
                MapWin *const primary = MapWinTable[0];
                if ((ORIGINAL(primary)->*ConsoleOriginalMapWinFocus)(x_coord, y_coord) != 0) {
                    focused = 1;
                    continue;
                }
                // 0x00510955 reads slot 0 a SECOND time, after MapWin::focus
                // returned. Re-read, do NOT hoist and do not share with the
                // load above. The literal 1 is the draw type.
                MapWin *const repaint = MapWinTable[0];
                (ORIGINAL(repaint)->*MapWinOriginalDrawMap)(1);
                continue;
            }
            // Latch already clear (0x00510936 je 0x510964): fall through to the
            // plain focus call, exactly as a tagged slot 1..7 does.
        } else if (!tagged) {
            continue;
        }
        // 0x00510964 - the third table read this iteration can make. Re-read,
        // do NOT hoist: on the slot-0 path cursor_next has already run between
        // this and the load at 0x005108DE.
        MapWin *const target = MapWinTable[slot];
        // Specialisation 4: a successful focus on slots 1..7 is discarded; only
        // the primary window raises the flag.
        if ((ORIGINAL(target)->*ConsoleOriginalMapWinFocus)(x_coord, y_coord) != 0
            && slot == 0) {
            focused = 1;
        }
    }
    // 0x00510991 test / 0x00510993 je / 0x00510995 call - once, after the whole
    // eight-slot walk, never inside it.
    if (focused != 0) {
        flush_input();
    }
    return focused;
}

int __fastcall console_focus_redirect(Console *self, void *, int x_coord,
                                      int y_coord, int faction_id) {
    return self->focus(x_coord, y_coord, faction_id);
}

/*
Purpose: Record that the console was closed from the system menu, by raising
         the flag the turn loop watches.
// ORIGINAL: 0x0051D7C0 ?on_sys_close@Console@@QAEXXZ 0x0051D7C0-0x0051D7CB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?on_sys_close@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Console::on_sys_close() {
    *ConsoleExitTurnLoop = 1;
}

/*
// ORIGINAL: 0x004E0FA0 ?editor_climate@Console@@QAEXXZ 0x004E0FA0-0x004E0FC3 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?editor_climate@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0058C2A0 0x005ABF20 0x005C5A30
Status: Complete
*/
void Console::editor_climate() {
    auto_undo();
    if (!custom_planet(0, 0)) {
        world_climate();
        // QUALIFIED: `Console : MapWin` inherits `MapWin::draw_map`, so the
        // unqualified name binds to the member and compiles a `__thiscall`
        // where the image has a `__cdecl` call to the FREE draw_map at
        // 0x0046B190. The `::` is what makes the two the same call.
        ::draw_map(1);
    }
}

/*
// ORIGINAL: 0x004E1F30 ?editor_redo@Console@@QAEXXZ 0x004E1F30-0x004E1F39 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_redo@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005ABE40
Status: Complete
*/
void Console::editor_redo() {
    load_undo(-1);
}

/*
// ORIGINAL: 0x0051D740 ?menu_update@Console@@QAEXXZ 0x0051D740-0x0051D753
// size      19 bytes
// prototype void (__thiscall ?menu_update@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00460DD0
Status: Complete
*/
void Console::menu_update() {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x23bdc);
    (ORIGINAL(self + 0x22a2c)->*ConsoleOriginalMainMenuCheck)(v);
}

/*
// ORIGINAL: 0x004E0F80 ?editor_polar@Console@@QAEXXZ 0x004E0F80-0x004E0F98 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?editor_polar@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005C58E0 0x005C5A30
Status: Complete
*/
void Console::editor_polar() {
    auto_undo();
    world_polar_caps();
    world_climate();
    ::draw_map(1);   // the FREE draw_map; see Console::editor_climate above
}
