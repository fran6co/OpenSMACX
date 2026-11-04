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
#include "game.h"
#include "interface_palette.h"
#include "console.h"
#include "multiwin.h"
#include "statuswin.h"
#include "tutwin.h"
#include "worldwin.h"

#include "alpha.h"
#include "basepop.h"
#include "strings.h"
#include "time.h"

// The teardown half of control_game's wheel, homed 2026-08-29 from
// src/recovered/units/ (0058eff0, 00589b20, 0058f430). Their forwarders in
// pending_bodies.cpp are gone; these definitions own the symbols now.
// ---------------------------------------------------------------------------

/*
Purpose: Tear the desktop down at the end of each main-loop pass.
// ORIGINAL: 0x0058EFF0 ?desktop_close@@YAXXZ 0x0058EFF0-0x0058F031 FILE BYTE_EXACT
// size      65 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x005D4E40
// LEVER: byte-exact on promotion. The fixed windows are REAL OBJECTS now
//   (TutWinDesWindow, TutWinBaseWindow in their domain .cpps; ConsoleStatusWin
//   in statuswin.cpp; MainInterfaceGlobal in mapwin.cpp for TutWinIfaceWindow)
//   and still fold each receiver to `mov ecx, imm32`, the displacement
//   relocated; `Win::hide` is qualified to keep the slot-2 virtual a direct
//   tail `jmp`, and `GraphicWin::close` on the WorldWin and on MultiWindow's
//   front is qualified so a later base-class declaration cannot steal the
//   call from the direct E8 the image has. WorldClimateWorldWin is declared
//   in worldwin.h and defined in map.cpp, so both bodies name the object -
//   same fold as the old by-value world_climate_window() helper, one binding
//   name instead of two.
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_close() {
    TutWinDesWindow.close();
    TutWinBaseWindow.close();
    mapwin_system_shutdown();
    ConsoleStatusWin.close();
    WorldClimateWorldWin.GraphicWin::close();
    // The MultiWin object's GraphicWin front - the image closes that base
    // directly (`call 0x5d4e40` on `mov ecx, 0x7fd648`).
    MultiWindow.GraphicWin::close();
    MainInterfaceGlobal.Win::hide();
}


/*
Purpose: Publish the Console's Win base for popup parenting, then destroy
         the owned opening window and clear its pointer.
// ORIGINAL: 0x00589B20 ?close_opening@@YAXXZ 0x00589B20-0x00589B57 FILE BYTE_EXACT
// size      55 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00589B4A
// The original converts Console through MapWin's virtual GraphicWin base.
// At 0x00589B4A it calls the opening object's scalar deleting destructor
// with flag one, not on_dialog_focus. GraphicWin's original primary table
// at 0x0066FC50 selects 0x005D7140, which destroys and frees the allocation.
Return Value: n/a
*/
void __cdecl close_opening() {
    Win *console_window = &ConsoleGlobal;
    GraphicWin *opening = opening_window();
    // Publish the ordinary C++ conversion to the fallback parent window.
    BasePopDefaultField322C = console_window;
    if (opening != 0) {
        delete opening;
        opening_window() = 0;
    }
}


// ORIGINAL: 0x00589B60 ?init_opening@@YAHPBD@Z 0x00589B60-0x00589D29;0x0066026A-0x0066028A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:opening-window-lifetime-owner", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589B60..589D29;66026A..66028A", "source:src/unrecovered/00589b60.cpp complete original identity/spans and seed lessons retained; Buffer::pixel_height already negates the top-down DIB height."], "reason": "Recover opening-window construction, loading, presentation and popup-parent publication through genuine GraphicWin and Buffer lifetimes. Each callback-sensitive shared-slot reload is independent; no added null guard or failure-path behavior change."}
int __cdecl init_opening(const char *filename) {
    if (OpeningWindow == 0) {
        GraphicWin *const window = new GraphicWin();
        OpeningWindow = window;
        if (window == 0) {
            return 4;
        }

        const int result = window->init(0, 0, WinScreenWidth, WinScreenHeight,
                                        0, 0x400000, 0, 0,
                                        0);
        if (result != 0) {
            close_opening();
            return result;
        }
    }

    OpeningWindow->fill(0);
    {
        Buffer local_buffer;
        if (OpeningWindow != 0) {
            local_buffer.load_pcx(filename, 0, 10, 0xEC);

            // Native reload after load_pcx supplies the copy destination and
            // its metrics. The null arm only nulls the copy destination; the
            // following metrics reads are unconditional in the original.
            GraphicWin *const copy_window = OpeningWindow;
            Buffer *const destination =
                copy_window != 0 ? static_cast<Buffer *>(copy_window) : 0;
            const int target_width = copy_window->pixel_width();
            const int target_height = copy_window->pixel_height();
            local_buffer.copy(destination, 0, 0, local_buffer.pixel_width(),
                              local_buffer.pixel_height(), 0, 0, target_width,
                              target_height);

            // The native body reloads the slot after copy, independently of
            // the receiver used above, before virtual on_update dispatch.
            OpeningWindow->on_update();
        }
    } // Buffer's natural destructor runs before palette realization.

    g_PALETTE1.set();
    OpeningWindow->show(0);
    do_all_draws();
    BasePopDefaultField322C = static_cast<Win *>(OpeningWindow);
    return 0;
}


/*
Purpose: Shut the game state down: timers, label strings, then the string
         table. The mode parameter is read by none of it.
// ORIGINAL: 0x0058F430 ?game_close@@YAXH@Z 0x0058F430-0x0058F444 FILE BYTE_EXACT
// size      20 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0050F440 0x006169D0
// LEVER: byte-exact on promotion. StringTable is strings.h's header binding,
//   so the receiver folds to `mov ecx, 0x9b90d8`; the image's closing `jmp`
//   into Strings::shutdown comes free from the call being the last statement
//   of a void function, and the discarded int result costs nothing.
// Historical LEVER (2026-09-03): temporary stop_timers_call and
//   labels_shutdown_call forwarders once restored these call boundaries.
Return Value: n/a
Status: Complete
*/
void __cdecl game_close(int mode) {
    stop_timers();
    labels_shutdown();
    StringTable.shutdown();
}

