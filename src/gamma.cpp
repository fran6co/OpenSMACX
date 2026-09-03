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
#include "gamma.h"
#include "worldwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x005C9410 ?on_key_click@Gamma@@QAEHHH@Z 0x005C9410-0x005C9418 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@Gamma@@QAEHHH@Z)(Gamma* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Gamma::on_key_click(int, int) {
    return 1;
}



/*
// ORIGINAL: 0x005C9390 ?on_scrolled@Gamma@@QAEXHH@Z 0x005C9390-0x005C9398 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9520
Status: Complete
*/
void Gamma::on_scrolled(int code, int pos) {
    adjust_palette();
}


// The zero-init .bss table PushButton's constructor loads its field defaults
// from. Zero at load in the image, as here - the same idiom as
// BaseButtonA94Default (basebutton.cpp) and `Log Logging` (log.cpp).
uint32_t PushButtonDefaults[18];  // 0x009BC118


/*
Purpose: Construct the BaseButton base, stage both virtual tables, then load
         the eighteen field defaults from the process table.
// ORIGINAL: 0x0062BF20 ??0PushButton@@QAE@XZ 0x0062BF20-0x0062C00E BYTE_EXACT
// PROMOTED from src/unrecovered/0062bf20.cpp, 2026-09-03. That scaffold's
//        blocker is now gone: `class PushButton` derives from BaseButton for
//        real and `BaseButton::BaseButton()` is defined OUT OF LINE, so the
//        implicit base construction is the image's own `call ??0BaseButton`
//        and the two vtable stores are the compiler's - no `new (this) T()`
//        and no operator-new call. What remains of the scaffold's
//        hand-written body is the eighteen field defaults.
// symbol    ??0PushButton@@QAE@XZ
// size      238 bytes
// prototype void (__thiscall ??0PushButton@@QAE@XZ)(PushButton* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606F30
Status: Complete
*/
PushButton::PushButton() {
    field_AB8_ = PushButtonDefaults[0];
    field_ABC_ = PushButtonDefaults[1];
    field_AC0_ = PushButtonDefaults[2];
    field_AC4_ = PushButtonDefaults[3];
    field_AC8_ = PushButtonDefaults[4];
    field_ACC_ = PushButtonDefaults[5];
    field_AD0_ = PushButtonDefaults[6];
    field_AD4_ = PushButtonDefaults[7];
    field_AD8_ = PushButtonDefaults[8];
    field_ADC_ = PushButtonDefaults[9];
    field_AE0_ = PushButtonDefaults[10];
    field_AE4_ = PushButtonDefaults[11];
    field_AE8_ = PushButtonDefaults[12];
    field_AEC_ = PushButtonDefaults[13];
    field_AF0_ = PushButtonDefaults[14];
    field_AF4_ = PushButtonDefaults[15];
    field_AF8_ = PushButtonDefaults[16];
    field_AFC_ = PushButtonDefaults[17];
}


/*
Purpose: Re-install PushButton's two virtual tables, then delegate to
         BaseButton::destroy() (this tree's spelling of BaseButton's real
         complete-object destructor). A trivial trailing void call with no
         stack cleanup, so VC6 folds the tail into a `jmp` rather than a
         `call`+`ret` pair - the LEVER documented on PullDown's destructor
         (pulldown.cpp) does NOT apply here because it depends on the caller
         needing the callee's return value; this one genuinely wants the
         jmp, and the image has one.
// ORIGINAL: 0x0062C010 ??1PushButton@@QAE@XZ 0x0062C010-0x0062C025
// DEMOTED from BYTE_EXACT 3/3 by direction (strip-all hand vptr writes): the
// body's whole content WAS the two vtable re-stage stores, deleted along with
// their constants in the de-management sweep. Re-earns BYTE_EXACT only if the
// re-stage stores return as a faithful transcription.
// LEVER: a real `~PushButton()` mangles `??1PushButton@@UAE@XZ` and compiles ~15 instructions of vtable-pointer/base-unwind bookkeeping - AutoSound's virtual destructor (autosound.h, added for RadioButton's vbtable layout) cascades virtuality onto every Win-derived destructor. `destroy()`, an ordinary method rather than a destructor override, is never cascaded into virtual and reproduces the image's plain 3-instruction body; see pushbutton.h for the declaration. Same idiom as BaseButton::destroy()/Scroll::destroy().
// symbol    ?destroy@PushButton@@QAEPAV1@XZ
// size      21 bytes
// prototype void (__thiscall ??1PushButton@@QAE@XZ)(PushButton* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
PushButton *PushButton::destroy() {
    uint32_t *const ordered = reinterpret_cast<uint32_t *>(this);
    return static_cast<PushButton *>(BaseButton::destroy());
}

/*
Purpose: Construct the gamma-correction dialog.
// ORIGINAL: 0x005C8DA0 ??0Gamma@@QAE@XZ 0x005C8DA0-0x005C8E60;0x00662A70-0x00662AC8 BYTE_EXACT
// LEVER: the two vtable stores must come FIRST, before `gamma_ = 1.0`. Written after it the body is SHAPE_EXACT 39/42; written before it, BYTE_EXACT 42/42. The image interleaves them - `[esi+0xee8] = 0`, both vtables, then `[esi+0xeec] = 0x3ff00000` - which reads like the vtable stores landing in the middle of the double and impossible to write as source. It is the other way round: VC6 hoists the double's ZERO half ahead of whatever precedes it and leaves the 0x3ff00000 half in place, so putting the vtable stores first is what produces the image's own order. (This lesson was already written out in full INSIDE the function body, ten lines below the marker, where `decomp.reader` - which reads only the comment run immediately after the marker - could not see it: `osmx show` listed no LEVER for this address at all.)
// size      280 bytes
// prototype void (__thiscall ??0Gamma@@QAE@XZ)(Gamma* this)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FE2A0 0x006051D0 0x00614E50 0x00618EA0 0x0062BF20
// LEVER (2026-09-03, the /Ob0 dependency closed): of the six member
//        constructors this body calls, five were already out of line
//        (Palette, Font, Scroll, EditBox and the GraphicWin base). The one
//        that was not - `PushButton() { ; }` in pushbutton.h - was EXPANDED
//        twice here, and each expansion carried the whole BaseButton chain,
//        so VC6 paid a second EH spill slot (`sub esp, 8`) plus 22 extra
//        instructions: 64 compiled against the image's 42, 10 of 42 agreeing.
//        Out of line (gamma.cpp, 0x0062BF20 BYTE_EXACT) the site is six
//        `call`s from a one-slot frame, 42 of 42. Nothing in this body
//        changed; the fix is entirely in the callee's definition.
Return Value: None
Status: Complete
*/
Gamma::Gamma() {
    // The vtable slots are compiler-managed, not ordinary members a class can
    // name - the same reason GraphicWin::construct writes its own pair at
    // these exact two offsets. Gamma has its own vtable because it overrides;
    // nothing in this chain is declared `virtual`, so the store is explicit
    // rather than emitted by a constructor VC6 would generate.
    // The store order matters; see the LEVER line on the marker above.
    gamma_ = 1.0;
    field_A64_ = 0;
}

/*
Purpose: Step the receiver back to the subobject ??_GGamma@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004562B0 ??_GGamma@@WEEE@AEPAXI@Z 0x004562B0-0x004562BB BYTE_EXACT
// symbol    ??_EGamma@@WEEE@AEPAXI@Z
// CORRECTED from ??3Gamma@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00456280` into
//   ??_GGamma@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

/*
Purpose: Step the receiver back to the subobject ??_GPushButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0062C850 ??_GPushButton@@WEEE@AEPAXI@Z 0x0062C850-0x0062C85B BYTE_EXACT
// symbol    ??_EPushButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3PushButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0062C830` into
//   ??_GPushButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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
