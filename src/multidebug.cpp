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
#include "multidebug.h"
#include "vtable_shim.h"

/*
Purpose: Construct the GraphicWin base, the embedded Font and Time, then
         install MultiDebug's own vtables and zero the session pointer.
// ORIGINAL: 0x005C97F0 ??0MultiDebug@@QAE@XZ 0x005C97F0-0x005C9860;0x00662AF0-0x00662B10 BYTE_EXACT
// LEVER: `new (&font_) Font(); new (&timer_) Time();` - a `new`-expression, not the member-function-call syntax used elsewhere (MCIVideo's Palette) - is what picks up the image's SEH frame here; the member-call-syntax spelling compiled with NO frame at all (0/28 agreeing, diverging at instruction 0).
// size      112 bytes
// prototype void (__thiscall ??0MultiDebug@@QAE@XZ)(MultiDebug* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x00618EA0 0x006161D0
Return Value: n/a
Status: Complete
*/
MultiDebug::MultiDebug() {
    net_ = 0;
}

/*
Purpose: Close the debug window by clearing its single active flag.
// ORIGINAL: 0x005C98E0 ?close@MultiDebug@@QAEXXZ 0x005C98E0-0x005C98EB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?close@MultiDebug@@QAEXXZ)(MultiDebug* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MultiDebug::close() {
    net_ = 0;
}

/*
Purpose: Bring the standing debug window up on its GraphicWin base, then
         arm the pieces that make it a debug window: the face it draws with,
         the text buffer it shows, and the once-a-second tick that drives it.
         Returns GraphicWin::init's verdict; nothing else here can fail.
// ORIGINAL: 0x005C9860 ?init@MultiDebug@@QAEHPAUNet@@PAUWin@@PAE@Z 0x005C9860-0x005C98DB BYTE_EXACT
// symbol    ?init@MultiDebug@@QAEHPAVNet@@PAVWin@@PAE@Z
//   the catalogue's own spelling: both Net and Win are `class`, so the tree
//   emits `PAV` for each, where the forwarder's annotation had carried the
//   `PAU` of a `struct`. Recorded here so the claim follows the symbol.
// size      123 bytes
// prototype int (__thiscall ?init@MultiDebug@@QAEHPAUNet@@PAUWin@@PAE@Z)(MultiDebug* this, Net*, Win*, unsigned char*)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4EF0 0x005D5250 0x00616350 0x00618F40
// LEVER: byte-exact, promoted from the src/unrecovered/005c9860.cpp archive
//   unit, whose `GraphicWinInitShim` is gone with it: graphicwin.h declares
//   `init` returning `int`, so the base call names the base directly. The two
//   stores the image sinks into its calls' argument setup - `net_` ahead of
//   the GraphicWin::init call, the text pointer ahead of Font::init - are
//   plain member stores in source order; VC6 places them, the source does not.
Return Value: GraphicWin::init's, which is 0 when the window came up.
Status: homed on this pass; its forwarder and its archive unit are gone, and
        the `multidebug_init` free-function shape in game.h went with them.
*/
int MultiDebug::init(Net *net, Win *window, unsigned char *text) {
    net_ = net;
    int const result =
        GraphicWin::init(0, 0, 0x96, 0x12c, 0, 0x1000002, window, 0, 0);
    if (result != 0) {
        return result;
    }
    // The text buffer lives in Win::field_A0_, one word the rest of Win
    // treats as storage; see the base class for why it is not typed.
    field_A0_ = reinterpret_cast<uint32_t>(text);
    font_.init("Times New Roman", 0xa, 0);
    fill(0);
    // The tick is passed as the bare `void (__cdecl *)(int)` that
    // Time::start takes, and the function it names is this class's own
    // `void __cdecl` member - so its address is a pointer-to-member and
    // VC6 refuses every conversion to a plain function pointer (C2440,
    // measured on both the `reinterpret_cast` and the C-style cast). The
    // address is spelled directly instead; it is the member above.
    timer_.start(reinterpret_cast<void (__cdecl *)(int)>(0x005C9900),
                 reinterpret_cast<int>(this), 0x3e8, 5);
    return 0;
}


/*
Purpose: Forward the timer tick to the object's own virtual slot 62.
// ORIGINAL: 0x005C9900 ?timer_callback_daemon@MultiDebug@@QAAXH@Z 0x005C9900-0x005C9914 BYTE_EXACT
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005C990C
Return Value: n/a
Status: Complete
*/
void __cdecl MultiDebug::timer_callback_daemon(int a2) {
    if (this) {
        vslot_62();
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiDebug@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005C9EB0 ??_GMultiDebug@@WEEE@AEPAXI@Z 0x005C9EB0-0x005C9EBB BYTE_EXACT
// symbol    ??_EMultiDebug@@WEEE@AEPAXI@Z
// CORRECTED from ??3MultiDebug@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005C9E80` into
//   ??_GMultiDebug@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
