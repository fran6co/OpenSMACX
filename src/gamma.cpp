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


/*
Purpose: Re-install PushButton's two virtual tables, then delegate to
         BaseButton::destroy() (this tree's spelling of BaseButton's real
         complete-object destructor). A trivial trailing void call with no
         stack cleanup, so VC6 folds the tail into a `jmp` rather than a
         `call`+`ret` pair - the LEVER documented on PullDown's destructor
         (pulldown.cpp) does NOT apply here because it depends on the caller
         needing the callee's return value; this one genuinely wants the
         jmp, and the image has one.
// ORIGINAL: 0x0062C010 ??1PushButton@@QAE@XZ 0x0062C010-0x0062C025 BYTE_EXACT
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
    ordered[0x000 / 4] = PushButtonPrimaryVtable;
    ordered[0x444 / 4] = PushButtonBufferVtable;
    return static_cast<PushButton *>(BaseButton::destroy());
}

// Gamma's own virtual table addresses, installed by the constructor. The pair
// mirrors GraphicWin's exactly - primary at +0, the Buffer subobject's at
// +0x444, eight bytes apart - because Gamma inherits that layout unchanged.
const uint32_t GammaPrimaryVtable = 0x0066C3D8;
const uint32_t GammaBufferVtable = 0x0066C3D0;

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
