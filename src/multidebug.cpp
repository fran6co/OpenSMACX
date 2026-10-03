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
#include "net_class.h"
#include <stdlib.h>
#include <string.h>

// ORIGINAL: 0x005C97C0 ??__Eg_MULTIDEBUG@@YAXXZ 0x005C97C0-0x005C97D6 FILE
// RECOVERY_BINDING: v1 {"reason":"The original relocated CRT entry constructs the static MultiDebug object and registers its complete teardown; bind the compiler's natural initializer, not a handwritten wrapper.","evidence":["git:24472a5a^:src/init_thunks.cpp","original:CRT slot 0x00682668 points to 0x005C97C0","original:0x005C97C0 constructs 0x009B22F0 and registers 0x005C97E0"]}
// symbol    _$E21

// ORIGINAL: 0x005C97E0 ??__Fg_MULTIDEBUG@@YAXXZ 0x005C97E0-0x005C97EA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The static object's registered teardown tail-calls its complete destructor; the compiler-generated helper owns this lifetime.","evidence":["git:24472a5a^:src/atexit_thunks.cpp","original:0x005C97E0 passes 0x009B22F0 to the complete destructor at 0x005C9E00"]}
// symbol    _$E19

// Original startup constructs MultiDebug here and registers its complete
// destructor. The keyboard path accesses this object's inherited members.
// RECOVERY_OBJECT: v1 0x009B22F0 ?WinBubbleWindow@@3VMultiDebug@@A
MultiDebug WinBubbleWindow;

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

// ORIGINAL: 0x005C9E00 ??1MultiDebug@@QAE@XZ 0x005C9E00-0x005C9E77;0x00662AD0-0x00662AF0 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The retained destructor seed identifies the complete MultiDebug teardown and its Font/GraphicWin cleanup funclets; the declared members own these lifetimes.","evidence":["src/unrecovered/005c9e00.cpp","original:0x005C9E3C clears net_ before Time, Font and GraphicWin teardown","original:0x00662AD0-0x00662AF0 contains the base and font cleanup funclets"]}
// Deleted seed provenance: git:117282ee0186f316fd18d2d25f9ae0223c8186ae:src/unrecovered/005c9e00.cpp
// symbol    ??1MultiDebug@@UAE@XZ
// The complete destructor clears the session before Time, Font and GraphicWin
// teardown. Their reverse declaration order also owns exception cleanup;
// explicitly calling the original destructor here would destroy them twice.
MultiDebug::~MultiDebug() {
    net_ = 0;
}

// ORIGINAL: 0x005C9E80 ??_GMultiDebug@@UAEPAXI@Z 0x005C9E80-0x005C9EA1 FILE BYTE_EXACT
// symbol    ??_GMultiDebug@@UAEPAXI@Z
// Natural scalar deletion calls the complete destructor and conditionally
// releases the object. The Buffer-base entry below adjusts to this receiver.

/*
Purpose: Close the debug session by clearing its network pointer.
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
    timer_.start(MultiDebug::timer_callback_daemon,
                 reinterpret_cast<int>(this), 0x3e8, 5);
    return 0;
}


/*
Purpose: Forward the timer tick to the object's own virtual slot 62.
// ORIGINAL: 0x005C9900 ?timer_callback_daemon@MultiDebug@@QAAXH@Z 0x005C9900-0x005C9914 BYTE_EXACT
// symbol    ?timer_callback_daemon@MultiDebug@@SAXH@Z
// The original loads its sole payload from [ebp+8] and returns with plain
// ret. The catalogue's Q spelling is not evidence of a hidden C++ receiver.
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
void __cdecl MultiDebug::timer_callback_daemon(int context) {
    MultiDebug *self = reinterpret_cast<MultiDebug *>(context);
    if (self != 0) {
        self->redraw();
    }
}

// ORIGINAL: 0x005C98F0 ?timer_callback@MultiDebug@@QAEXXZ 0x005C98F0-0x005C98F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover MultiDebug's existing instance callback as a typed call to its inherited redraw slot, replacing the seed-only vtable-shaped shim.","evidence":["original:0x005C98F0 dispatches the receiver through virtual slot 62", "source:src/win.h declares redraw at slot 62 and src/graphicwin.h provides the inherited redraw implementation", "source:src/multidebug.cpp timer_callback_daemon already dispatches self->redraw() on the same MultiDebug receiver"]}
void MultiDebug::timer_callback() {
    redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// _itoa's scaffold declaration above is nullary (a fallback); the real
// 3-argument call is declared here under a different name. The relocation
// it needs is masked by the comparison regardless of the symbol it names.
// ORIGINAL: 0x005C9920 ?on_redraw@MultiDebug@@QAEXXZ 0x005C9920-0x005C9DF9 FILE
// symbol    ?on_redraw@MultiDebug@@UAEXXZ
// The original primary table's slot 12 selects this painter. The timer's
// inherited GraphicWin::redraw reaches it through ordinary virtual dispatch.
void MultiDebug::on_redraw() {
    Buffer::set_font(&font_, 0, 0, 0);
    Buffer::set_text_color(255, -1, 1, 1);
    GraphicWin::fill(0);

    char message[256];
    char number[80];
    message[0] = 0;
    strcat(message, "Host ID: ");
    _itoa(static_cast<int>(net_->field_764_), number, 10);
    strcat(message, number);
    Buffer::write_l(message, 3, 3, strlen(message));
    int y = 3 + Buffer::text_line_height();

    message[0] = 0;
    strcat(message, "Local ID: ");
    _itoa(static_cast<int>(net_->field_760_), number, 10);
    strcat(message, number);
    Buffer::write_l(message, 3, y, strlen(message));
    y += (Buffer::text_line_height() * 3) / 2;

    for (int index = 0; index < 16; ++index) {
        if (net_->players_[index].field_0_ == 0) {
            continue;
        }

        message[0] = 0;
        _itoa(index, number, 10);
        strcat(message, number);
        strcat(message, ". ");
        strcat(message, " ");
        int value_x = Buffer::write_l(message, 3, y, strlen(message));
        char *name = net_->players_[index].name_and_tail_;
        if (name != 0) {
            Buffer::write_l(name, value_x, y, strlen(name));
        }
        y += Buffer::text_line_height();

        // Keep the original independent conversions and fresh session reads.
        // A cached NetPlayer reference would outlive the intervening draws.
        char guaranteed[80];
        message[0] = 0;
        strcat(message, "Guaranteed count: ");
        _itoa(static_cast<int>(net_->players_[index].field_4_), guaranteed, 10);
        strcat(message, guaranteed);
        Buffer::write_l(message, value_x, y, strlen(message));
        y += Buffer::text_line_height();

        char sending[80];
        message[0] = 0;
        strcat(message, "Sending: ");
        _itoa(static_cast<int>(net_->players_[index].field_8_), sending, 10);
        strcat(message, sending);
        Buffer::write_l(message, value_x, y, strlen(message));
        y += Buffer::text_line_height();

        char average[80];
        message[0] = 0;
        strcat(message, "Average reply time: ");
        _itoa(static_cast<int>(net_->players_[index].field_C_), average, 10);
        strcat(message, average);
        Buffer::write_l(message, value_x, y, strlen(message));
        y += Buffer::text_line_height();

        char last[80];
        message[0] = 0;
        strcat(message, "Last time: ");
        _itoa(static_cast<int>(net_->players_[index].stamp_10_), last, 10);
        strcat(message, last);
        Buffer::write_l(message, value_x, y, strlen(message));
        y += Buffer::text_line_height();

        char flags[80];
        message[0] = 0;
        strcat(message, "Flags: ");
        _itoa(net_->players_[index].flags_14_, flags, 10);
        strcat(message, flags);
        Buffer::write_l(message, value_x, y, strlen(message));
        y += Buffer::text_line_height();
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
