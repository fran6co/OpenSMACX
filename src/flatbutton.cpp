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
#include "flatbutton.h"

const uint32_t FlatButtonPrimaryVtable = 0x00669754;
const uint32_t FlatButtonBufferVtable = 0x0066974C;
// The nine-group defaults table read by both the constructor and close():
// runtime .data that load_interface (0x00455610, unrecovered - its reference
// sits at 0x455c05) populates alongside the constructor, so it STAYS
// image-addressed - real storage here would fork the state the image code
// writes. TRIED: an accessor returning the pointer - identical 28/41: cl
// materializes the row base either way (`mov ecx, 0x9b8e50 / mov edx,
// [ecx - 0xc]`) where the image keeps the index in ecx against the
// constant (`xor ecx,ecx / mov edx,[ecx + 0x9b8e44]`).
static int *const FlatButtonDefaultsTable = (int *)0x009B8E44;

// A real, out-of-line forwarder so this call site gets an actual `call`
// where `time1_.close()` directly would inline away under /O2 - same
// idiom as `sleep_call`/`base_cost_call` in veh.cpp. `Time::close()` is
// itself already claimed BYTE_EXACT at 0x00616780 (time.cpp); this only
// keeps THIS caller from inlining it.
void __fastcall time_close_call(Time *timer) {
    timer->close();
}

/*
Purpose: Close the primary Time member, reset FlatButton orientation state,
         close the BaseButton base, and reload all nine three-entry tables.
// ORIGINAL: 0x00607DA0 ?close@FlatButton@@QAEXXZ 0x00607DA0-0x00607E41
// LEVER: two fixes, stacked. (1) a real out-of-line `time_close_call`
//        forwarder (same idiom as `sleep_call`/`base_cost_call` in
//        veh.cpp) so `time1_.close()` compiles the `call rel32` the image
//        has instead of inlining away under /O2 - `Time::close()` is
//        itself separately claimed BYTE_EXACT at 0x00616780. (2)
//        `FlatButtonDefaultsTable` (a real constant pointer to the fixed image address)
//        in place of dereferencing the `FlatButtonDefaults` VARIABLE,
//        which read a runtime pointer VALUE where the image folds a
//        compile-time address - same table the constructor already binds
//        this way; `FlatButtonDefaults` itself is now unused and removed.
//        0/41 (70 instructions, unrelated indirect calls from the inlined
//        `Time::close()`) -> 0.557 similar (38 instructions, matching
//        shape).
// TRIED: an explicit 9-way unroll of the inner group loop, matching
//            what the CONSTRUCTOR's own identical table copy compiles to
//            - here it makes things WORSE (80 instructions: the two real
//            calls ahead of the unrolled block push VC6 to hoist nine
//            `sub reg,esi` address computations before the loop even
//            starts). The plain nested `for` loop the constructor also
//            uses is closer here despite not being unrolled - the two
//            functions are NOT under the same codegen heuristic once the
//            forwarder call is added.
// LEVER: three more, stacked on top of the two above and worth 0 -> 28 of 41
//   with the instruction count now exactly the image's 41.
//   (3) `time_close_call` is `__fastcall`, not `__cdecl`. The image reaches
//   Time::close as a thiscall - `lea ecx, [esi + 0xa1c]; call 0x616780` - so a
//   __cdecl forwarder costs the `push`/`add esp, 4` pair around it (38 -> 36
//   instructions).
//   (4) THE DESTINATION IS A WALKING POINTER, and that is what un-blocks the
//   9-way unroll the TRIED below rejected. `object[(0xAE0 / 4) + g * 3 +
//   index]` gives VC6 one induction variable for nine destinations and nine
//   `mov reg, <table>; sub reg, esi` differences hoisted before the loop (70-80
//   instructions). `uint32_t *dest = object + 0xAE0 / 4; ... ++dest;` with the
//   nine reads written out gives the image's TWO induction variables - EAX
//   walking the object, ECX indexing the table - and drops EBX/EBP, hence the
//   image's bare `push esi; push edi` prologue.
//   (5) RETURN THE WALKER, not the address. `dest` ends at this + 0xAEC, which
//   is what the image leaves in EAX as pure loop residue; returning
//   `reinterpret_cast<uintptr_t>(this) + 0xAEC` instead emits a `lea eax,
//   [esi + 0xaec]` the image does not have (42 -> 41 instructions).
// TRIED: the residual 13 instructions are ONE strength-reduction choice and
//   no source form moved it. The image starts its table index at zero
//   (`xor ecx, ecx`, then `mov edx, [ecx + 0x9b8e44]` nine times at 0xC
//   strides, `cmp ecx, 0xc; jl`); this tree bases the same induction variable
//   on the SECOND column instead (`mov ecx, 0x9b8e50`, `mov edx, [ecx - 0xc]`),
//   which re-spells all nine displacements. MEASURED at the same 28 of 41:
//   `FlatButtonDefaultsTable[index + g * 3]`, `*(FlatButtonDefaultsTable + index + g * 3)`, a
//   `const int *const table` local, `index != 3` for the loop test, and a
//   `column` pointer recomputed as `FlatButtonDefaultsTable + index` each iteration. A
//   byte-offset loop (`for (offset = 0; offset < 12; offset += 4)`) reproduces
//   the image's `[ecx + 0x9b8e44]` form but loses more elsewhere: 21 of 41.
//   Walking a SECOND pointer over the table alongside `dest` is far worse,
//   0 of 41 at 46 instructions.
// symbol    ?close@FlatButton@@QAEIXZ
// size      161 bytes
// prototype void (__thiscall ?close@FlatButton@@QAEXXZ)(FlatButton* this)
// callers   134   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006070C0 0x00616780
Return Value: Legacy final-loop residue (this + 0xAEC)
Status: Complete
*/
uint32_t FlatButton::close() {
    time_close_call(&time1_);
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0xA18 / 4] = 0;
    object[0xA14 / 4] = 0;
    object[0xAB8 / 4] = 0xFFFFFFFFU;
    BaseButton::close();

    uint32_t *dest = object + 0xAE0 / 4;
    for (int index = 0; index < 3; ++index) {
        dest[-9] = 0xFFFFFFFFU;
        dest[0] = static_cast<uint32_t>(FlatButtonDefaultsTable[0 * 3 + index]);
        dest[3] = static_cast<uint32_t>(FlatButtonDefaultsTable[1 * 3 + index]);
        dest[6] = static_cast<uint32_t>(FlatButtonDefaultsTable[2 * 3 + index]);
        dest[9] = static_cast<uint32_t>(FlatButtonDefaultsTable[3 * 3 + index]);
        dest[12] = static_cast<uint32_t>(FlatButtonDefaultsTable[4 * 3 + index]);
        dest[15] = static_cast<uint32_t>(FlatButtonDefaultsTable[5 * 3 + index]);
        dest[18] = static_cast<uint32_t>(FlatButtonDefaultsTable[6 * 3 + index]);
        dest[21] = static_cast<uint32_t>(FlatButtonDefaultsTable[7 * 3 + index]);
        dest[24] = static_cast<uint32_t>(FlatButtonDefaultsTable[8 * 3 + index]);
        dest[-3] = 0;
        dest[-6] = 0;
        ++dest;
    }
    // The return value IS the loop residue: `dest` finishes at this + 0xAEC,
    // which is what the image leaves in EAX. Spelling it
    // `reinterpret_cast<uintptr_t>(this) + 0xAECU` instead costs the extra
    // `lea eax, [esi + 0xaec]` the image does not emit.
    return reinterpret_cast<uintptr_t>(dest);
}


/*
Purpose: Destroy a FlatButton by installing its two virtual tables, closing
         the derived stage, then running the complete BaseButton destructor.
// ORIGINAL: 0x00406880 ??1FlatButton@@QAE@XZ 0x00406880-0x004068D8;0x006509A0-0x006509B2 BYTE_EXACT
// LEVER: unlike FlatButton::FlatButton() (the CONSTRUCTOR, capped by the SEH-frame ceiling below), the image's DESTRUCTOR genuinely HAS the SEH frame (`push -1 / push handler / mov eax,fs:[0] / push eax / mov fs:[0],esp`). A real `FlatButton::~FlatButton()` (not the `destroy()` plain-method spelling this marker used to point at via a `// symbol` fact) reproduces it: 0/24 -> 13/24. `destroy()` stays as its own separate method - external call sites (reportif.cpp, scroll.cpp) reach FlatButton objects through it by name, outside this pass's file list, so left alone.
// symbol    ??1FlatButton@@UAE@XZ
// size      106 bytes
// prototype void (__thiscall ??1FlatButton@@QAE@XZ)(FlatButton* this)
// callers   93   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00607040 0x00607DA0
Return Value: Instance pointer in EAX
Status: Complete
*/
FlatButton::~FlatButton() {
    close();
}

FlatButton *FlatButton::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = FlatButtonPrimaryVtable;
    object[0x444 / 4] = FlatButtonBufferVtable;
    close();
    BaseButton::destroy();
    return this;
}



// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00607CF0 ??0FlatButton@@QAE@XZ 0x00607CF0-0x00607D96
// TRIED: THE NOTES BELOW WERE INVISIBLE UNTIL 2026-08-22. They sat AFTER
//   this comment block's closing delimiter, so `decomp.reader` never read
//   them and `frontier.py --untouched` kept offering this body as fresh.
//   They are
//   moved inside verbatim; only the placement changed.
// TRIED: the SEH frame. The image has NO unwind frame here (its flags
//   carry no `frame`, unlike Popup::Popup() at 0x004048A0 which does). This
//   tree's compiled body gets one anyway - `push -1 / push handler /
//   mov eax, fs:[0] / push eax / mov fs:[0], esp` at instruction 0 - plus the
//   four implicit base/member constructions that go with a REAL constructor
//   (0xDC, 0x444, 0xA1C, 0xA4C) where the image's only call is the one to
//   `BaseButton::construct` at 0x00606F30. 62 compiled instructions against
//   the image's 41.
//
//   The construction chain is `BaseButton::construct()`, not a base
//   constructor call: BaseButton()/GraphicWin()/Win() are all empty inline
//   stubs (the "construct method" idiom), and the image's single `call`
//   targets 0x00606F30, which is `?construct@BaseButton@@QAEXXZ`.
//
//   MEASURED 2026-08-21, correcting an earlier version of this note that
//   blamed `Buffer buffer_` / `Heap heap_` / `AutoSound auto_sound_` being
//   held BY VALUE. That is not it: swapping all three (plus BaseButton's own
//   `Time time1_`/`time2_`) for raw `unsigned char[sizeof(T)]` storage the
//   whole way up the chain - GraphicWin, Win, BaseButton at once - does NOT
//   drop the frame. It is still there, byte for byte, with every by-value
//   member gone.
//
//   What actually does it, isolated with a direct `/GX-` recompile of this
//   file (`tools/listing_diff.py 0x00607CF0 --flags '/c /O2 /Gy /GR- /GX-'`):
//   disabling C++ EH removes the frame outright (45 instructions instead of
//   62, first divergence moves from instruction 0 to instruction 1). So the
//   trigger is `/GX` plus a REAL derived-class constructor whose base
//   (`BaseButton`, transitively `Win`) has a destructor the compiler cannot
//   see is trivial - `Win::~Win()` is declared, not empty, and does real
//   external teardown at 0x005EBC90, so it can never BE trivial - combined
//   with a call in the constructor body (`BaseButton::construct()`) that VC6
//   assumes can throw. Declaring that call `__declspec(nothrow)` or
//   `throw()` at its `basebutton.h` declaration does NOT suppress the frame
//   either (measured both ways); VC6 does not appear to trust a caller-side
//   nothrow annotation for this decision.
//
//   `/GX-` WOULD NOT HELP EVEN IF IT WERE REACHABLE, which is a stronger
//   statement than the one this note used to make, and it is measured rather
//   than argued. Added as a per-function flag axis and scored against every
//   other set: this body's best remains `/O1 /GX` at 0.565 similar, while the
//   four `/GX-` variants land at 0.440, 0.465, 0.482 and 0.513. Removing the
//   frame moves every later instruction, so positional agreement gets WORSE
//   even though the instruction count gets closer - 50 against the image's 41
//   under `/GX-`, where `/GX` overshoots. `Scroll::Scroll` (0x006051D0) says
//   the same thing more sharply: `/GX-` compiles 79 instructions against an
//   image of 83 and still scores 0.848, against 0.959 for `/O2 /Ob0 /GX`.
//   Fewer instructions is not a better match. The axis was reverted rather
//   than kept, because it costs 40% more measurement on every gate run and
//   wins nothing.
//
//   `/GX-` is also not reachable from here: it is a whole-translation-unit
//   flag, outside `src/`, and it is not free even in principle.
//   `Buffer::~Buffer()` (0x005D7410, `buffer.h`, an inline
//   `MEASURED virtual ~Buffer()` whose body is a single `close()` call) is
//   currently BYTE_EXACT (20/20 instructions) specifically because the
//   IMAGE's own `??1Buffer@@QAE@XZ` carries this exact SEH prologue; a direct
//   `/GX-` recompile of that same address drops it from 20/20 agreeing to a
//   near-total mismatch (image 20 instructions, compiled 8). So the ORIGINAL
//   project genuinely needs `/GX` elsewhere in this same header's closure,
//   and turning it off to fix this constructor would regress an existing
//   claim.
// TRIED: respelling this body as a `construct()` METHOD - the escape the
//   note above says was "not found". It is found, and it is refused on cost,
//   not on doctrine: by the brief's own test this body IS a method (it
//   constructs no bases, it calls `BaseButton::construct`, and it returns
//   `this` in EAX). But FlatButton is embedded BY VALUE in designwin.h,
//   filewin.h, councwin.h, pickwin.h, diplowin.h, basepop.h, credits.h and
//   basewin.h, and those classes' constructors reach it through the implicit
//   member sequence - filewin.cpp:53 documents its claim as resting on
//   exactly that. Hollowing `FlatButton()` to free the address would silently
//   stop constructing every one of them and take their claims with it. The
//   conversion is a coupled edit across eight headers, not a flatbutton.cpp
//   change.
// TRIED: the table loop here is byte-for-byte the SAME loop as
//   FlatButton::close (0x00607DA0), and the pair of fixes that took THAT one
//   from 0 to 28 of 41 - a walking `dest` pointer plus the nine group reads
//   written out - makes THIS one worse: 44 compiled instructions and 1 of 41
//   agreeing becomes 51 and 0 under `/O1 /GX`, and 62 becomes 69 under
//   `/O2 /GX`. The unwind frame and the four implicit member constructions
//   ahead of the loop have already taken the registers the unrolled form
//   needs, so VC6 spills instead of unrolling cleanly. The two bodies are not
//   under the same codegen heuristic once the frame is there - the same
//   conclusion the TRIED on 0x00607DA0 reached from the other direction.
//   Reverted; this body keeps the rolled `for (group)` loop.
// size      166 bytes
// prototype void (__thiscall ??0FlatButton@@QAE@XZ)(FlatButton* this)
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00606F30
*/

FlatButton::FlatButton() {
    BaseButton::construct();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = FlatButtonPrimaryVtable;
    object[0x444 / 4] = FlatButtonBufferVtable;
    object[0xA18 / 4] = 0;
    object[0xA14 / 4] = 0;
    object[0xAB8 / 4] = 0xFFFFFFFFU;
    for (size_t index = 0; index < 3; ++index) {
        object[(0xABC / 4) + index] = 0xFFFFFFFFU;
        for (size_t group = 0; group < 9; ++group) {
            object[(0xAE0 / 4) + group * 3 + index] =
                static_cast<uint32_t>(FlatButtonDefaultsTable[group * 3 + index]);
        }
        object[(0xAD4 / 4) + index] = 0;
        object[(0xAC8 / 4) + index] = 0;
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GFlatButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004070E0 ??_GFlatButton@@WEEE@AEPAXI@Z 0x004070E0-0x004070EB BYTE_EXACT
// symbol    ??_EFlatButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3FlatButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004068E0` into
//   ??_GFlatButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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

/*
Purpose: Step the receiver back to the subobject ??_GBaseButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00607CE0 ??_GBaseButton@@WEEE@AEPAXI@Z 0x00607CE0-0x00607CEB BYTE_EXACT
// symbol    ??_EBaseButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3BaseButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00607CC0` into
//   ??_GBaseButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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
