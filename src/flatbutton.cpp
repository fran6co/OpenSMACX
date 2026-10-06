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
// ORIGINAL: 0x004068E0 ??_GFlatButton@@UAEPAXI@Z 0x004068E0-0x00406901 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004068E0-0x00406901", "Pinned class constructor ??0FlatButton@@QAE@XZ at 0x00607CF0 stores original class/view table VA 0x669754 at 0x00607CFB (mov dword ptr [esi], 0x669754); the current genuine owner vftable ??_7FlatButton@@6BWin@@@ selects generated ??_GFlatButton@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EFlatButton@@UAEPAXI@Z (mode 2).", "Pinned ??_GFlatButton@@UAEPAXI@Z at 0x004068E0 calls the source-mapped ??1FlatButton@@QAE@XZ at 0x00406880, then tests byte ptr [ebp + 8], 1; je 0x4068fa bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/flatbutton.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original FlatButton scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GFlatButton@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "flatbutton.h"
#include "win.h"
#include "palette.h"
#include "temp.h"
#include "sprite.h"
#include <cstring>
extern void __cdecl fn_00607b10(int, BaseButton*);

// The nine-group defaults table read by both the constructor and close():
// runtime defaults that load_interface (0x00455610) publishes into the
// same genuine source storage read by the constructor and close.
// TRIED: an accessor returning the pointer - identical 28/41: cl
// materializes the row base either way (`mov ecx, 0x9b8e50 / mov edx,
// [ecx - 0xc]`) where the image keeps the index in ecx against the
// constant (`xor ecx,ecx / mov edx,[ecx + 0x9b8e44]`).
// Historical integer-array symbol: ?FlatButtonDefaultsTable@@3PAIA; redraw proves borrowed Sprite pointers.
// RECOVERY_OBJECT: v1 0x009B8E44 ?FlatButtonDefaultsTable@@3PAPAVSprite@@A
Sprite *FlatButtonDefaultsTable[27]; // 0x009B8E44, initially zero-filled



/*
Purpose: Close the primary Time member, reset FlatButton orientation state,
         close the BaseButton base, and reload all nine three-entry tables.
The historical forwarding notes below predate the separate timer lifecycle TU;
this definition now calls the actual Time member.
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
// symbol    ?close@FlatButton@@UAEXXZ
// size      161 bytes
// prototype void (__thiscall ?close@FlatButton@@QAEXXZ)(FlatButton* this)
// callers   134   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006070C0 0x00616780
Return Value: None; the old pointer-valued return reproduced incidental EAX.
The LEVER/TRIED notes above describe historical spelling experiments, not a
license to walk unrelated members or invent a result. Slot90 is genuine virtual
cleanup; load_interface owns the shared default-table producer.
*/
void FlatButton::close() {
    time1_.close();
    field_A18_ = 0;
    field_A14_ = 0;
    field_AB8_ = 0xFFFFFFFFU;
    BaseButton::close();
    for (int column = 0; column < 3; ++column) {
        field_ABC_[column] = 0xFFFFFFFFU;
        for (int group = 0; group < 9; ++group) {
            config[group * 3 + column] =
                FlatButtonDefaultsTable[group * 3 + column];
        }
        field_AD4_[column] = nullptr;
        border_styles_[column] = 0;
    }
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
// MOVED IN-CLASS 2026-09-02 (body: src/flatbutton.h) so ??1DipEdit can expand
// its five FlatButton teardowns the way the image does; this class's own
// COMDAT keeps being emitted here and still measures 24/24 against
// 0x00406880.

FlatButton *FlatButton::destroy() {
    FlatButton::close();
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
    // BaseButton has already constructed its GraphicWin and Time members.
    // Reconstructing it here would also overwrite the FlatButton table stage.
    field_A18_ = 0;
    field_A14_ = 0;
    field_AB8_ = 0xFFFFFFFFU;
    for (size_t index = 0; index < 3; ++index) {
        field_ABC_[index] = 0xFFFFFFFFU;
        for (size_t group = 0; group < 9; ++group) {
            config[group * 3 + index] =
                FlatButtonDefaultsTable[group * 3 + index];
        }
        field_AD4_[index] = 0;
        border_styles_[index] = 0;
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

// ??_GBaseButton@@WEEE@AEPAXI@Z (0x00607CE0) moved to basebutton.cpp,
// 2026-09-03: this unit emits no BaseButton symbol at all, so the claim
// measured UNRESOLVED from here. ??_EBaseButton is defined by basebutton.obj
// alone - that is where ~BaseButton lives - and read from there it is the
// image's own 11 bytes. A comparison compiles a TRANSLATION UNIT, so the
// claim has to live where the COMDAT is emitted. The full TRIED evidence
// that named this move is folded into the marker's new home.

// ORIGINAL: 0x00608500 ?on_mouse_move@FlatButton@@QAEXHH@Z 0x00608500-0x00608657
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/recovered/units/00608500.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x00608500; physical VC6 COFF emitted symbol ?on_mouse_move@FlatButton@@UAEXHH@Z."]}
// symbol    ?on_mouse_move@FlatButton@@UAEXHH@Z
void FlatButton::on_mouse_move(int x, int y) {
    if (WinPointerOwner1 == this || WinPointerOwner2 == this) {
        BaseButton::on_mouse_move(x, y);
        return;
    }
    const RECT &buffer_rect = this->Buffer::rect1_;
    const RECT hit_rect = {
        0, 0,
        buffer_rect.right - buffer_rect.left,
        buffer_rect.bottom - buffer_rect.top
    };
    if (in_box(x, y, &hit_rect) == 0) {
        if (field_A74_ == 1) {
            time1_.stop();
            if (bubble_text_ != 0) Win::clear_bubble_text();
            field_A74_ = 0;
            if (win_parent_ != 0) {
                win_parent_->vslot_40(static_cast<int>(field_A78_), 0);
            }
            redraw();
        }
    } else if (field_A74_ == 0) {
        if (WinButtonHoverHook != 0) WinButtonHoverHook();
        field_A74_ = 1;
        if (win_parent_ != 0) {
            win_parent_->vslot_40(static_cast<int>(field_A78_), 1);
        }
        if (bubble_text_ != 0) {
            time1_.init(
                reinterpret_cast<void (__cdecl *)(int, int)>(&fn_00607b10),
                0, static_cast<int>(reinterpret_cast<unsigned long>(this)),
                1000, 5);
            time1_.pulse();
        }
        redraw();
    }
}

// ORIGINAL: 0x00607E50 ?on_redraw@FlatButton@@QAEXXZ 0x00607E50-0x006084F9
// RECOVERY_BINDING: v1 {"evidence": ["task:pulldown-flatbutton-renderer-and-sprite-ownership", "historical seed:src/unrecovered/00607e50.cpp", "Pinned original00607E50; full native instructions, current class and producer/consumer closure."], "reason": "Recover the complete redraw in its genuine virtual C++ owner using real source-owned members, callbacks, sprites and Buffer operations; pointer configuration is borrowed, not owned."}
// symbol    ?on_redraw@FlatButton@@UAEXXZ
// TRIED: faithful port of the 4-way fill/box_sprite/8-tile/bevel-box draw dispatch (indexed by the clamped 0/1/2 state, using the FlatButton pseudo-arrays at 0xAD4/AC8/B10/B1C/B28/B34/AE0/AEC/AF8/B04/B40 via raw offset casts) plus the trailing text layout; verified Buffer.width_/ height_ land at 0x4C4/0x4C8 (buffer_ embedded at 0x444, matching Win's real size) and Sprite.iWidth_/iHeight_/cTransparentIndex_ against the disassembly's +0x18/+0x1c/+8. The tail's Ghidra "unaff_EBX"/"unaff_EBP" are mistraced [esp+0x14]/[esp+0x1c] RECT slots, not live registers; rebuilt from raw asm as rectLeft/rectRight instead. The final two-string fallback clip-rect arithmetic (stack-slot reuse interacting with a push) was approximated rather than traced exactly. Compiles and matches through instr #14; not chased further given the size.
void FlatButton::on_redraw() {
  Win *const parent = parent_window();
  WinCallbackWindow = parent;
  if (HandleMain == 0 || parent == 0) {
    return;
  }

  Buffer *const buffer = static_cast<Buffer *>(this);
  buffer->sync_to_palette(PaletteActive);
  const RECT working = buffer->current_clip();
  int rect_left = working.left;
  int rect_top = working.top;
  int rect_right = working.right;
  int rect_bottom = working.bottom;
  fill(WinFillColour);

  int state = static_cast<int>(field_AB8_);
  if (state < 0 || state > 2) {
    if (field_A9C_ == 0 && field_A18_ == 0) {
      state = field_A74_ != 0 ? 1 : 0;
    } else {
      state = 2;
    }
  }

  int drawn_count = 0;
  Sprite *const fill_sprite = field_AD4_[state];
  if (fill_sprite != 0) {
    const int x = (buffer->pixel_width() - fill_sprite->width()) / 2;
    const int y = -((fill_sprite->height() + buffer->pixel_height()) / 2);
    fill_sprite->draw(buffer, fill_sprite->transparent_index(), x, y, 1, 1);
  } else if (border_styles_[state] != 0) {
    RECT box = {rect_left, rect_top, rect_right, rect_bottom};
    buffer->box_sprite(&box, border_styles_[state]);
  } else {
    Sprite *const center = config[12 + state];
    if (center != 0) {
      drawn_count = 1;
      center->draw(buffer, center->transparent_index(), 0, 0, 1, 1);

      Sprite *const right = config[15 + state];
      if (right != 0) {
        drawn_count = 2;
        right->draw(buffer, right->transparent_index(),
                    buffer->pixel_width() - right->width(), 0, 1, 1);
      }

      Sprite *const bottom = config[18 + state];
      if (bottom != 0) {
        ++drawn_count;
        bottom->draw(buffer, bottom->transparent_index(), 0,
                     -(bottom->height() + buffer->pixel_height()), 1, 1);
      }

      Sprite *const bottom_right =
          config[21 + state];
      if (bottom_right != 0) {
        ++drawn_count;
        bottom_right->draw(buffer, bottom_right->transparent_index(),
                           buffer->pixel_width() - bottom_right->width(),
                           -(bottom_right->height() + buffer->pixel_height()),
                           1, 1);
      }

      Sprite *const top_left_tile = config[state];
      if (top_left_tile != 0 && bottom != 0) {
        const int center_height = center->height();
        ++drawn_count;
        buffer->tile(top_left_tile, 0, 0, 0, center_height,
                     top_left_tile->width(),
                     -(bottom->height() + buffer->pixel_height()
                       + center_height));
      }

      Sprite *const bottom_left_tile =
          config[3 + state];
      if (bottom_left_tile != 0 && right != 0 && bottom_right != 0) {
        const int right_height = right->height();
        const int tile_width = bottom_left_tile->width();
        ++drawn_count;
        buffer->tile(bottom_left_tile, 0, 0,
                     buffer->pixel_width() - tile_width, right_height,
                     tile_width,
                     -(bottom_right->height() + buffer->pixel_height()
                       + right_height));
      }

      Sprite *const top_right_tile =
          config[6 + state];
      if (top_right_tile != 0 && right != 0) {
        const int center_width = center->width();
        ++drawn_count;
        buffer->tile(top_right_tile, 0, 0, center_width, 0,
                     (buffer->pixel_width() - right->width()) - center_width,
                     top_right_tile->height());
      }

      Sprite *const bottom_right_mid_tile =
          config[9 + state];
      if (bottom_right_mid_tile != 0 && bottom != 0 && bottom_right != 0) {
        const int bottom_width = bottom->width();
        const int tile_height = bottom_right_mid_tile->height();
        ++drawn_count;
        buffer->tile(bottom_right_mid_tile, 0, 0, bottom_width,
                     -(buffer->pixel_height() + tile_height),
                     (buffer->pixel_width() - bottom_right->width())
                         - bottom_width,
                     tile_height);
      }

      Sprite *const full_center_tile =
          config[24 + state];
      if (full_center_tile != 0 && drawn_count == 8) {
        Sprite *const bottom_left =
            config[3 + state];
        const int tile_height = top_right_tile->height();
        const int tile_left = bottom_left->width();
        buffer->tile(full_center_tile, 0, 0, tile_left, tile_height,
                     (buffer->pixel_width()
                      - top_left_tile->width()) - tile_left,
                     -(bottom_right_mid_tile->height()
                       + buffer->pixel_height() + tile_height));
      }
    } else {
      int highlight_color = static_cast<int>(field_A94_);
      drawn_count = 0;
      RECT bevel = {rect_left, rect_top, rect_right, rect_bottom};
      if (highlight_color != -1) {
        buffer->box(&bevel, highlight_color, highlight_color);
        ++bevel.left;
        --bevel.right;
        ++bevel.top;
        --bevel.bottom;
        if (!parent->has_dialog_focus_flag() && is_dialog_focus() != 0) {
          highlight_color = static_cast<int>(field_A94_);
          buffer->box(&bevel, highlight_color, highlight_color);
          ++bevel.left;
          --bevel.right;
          ++bevel.top;
          --bevel.bottom;
          drawn_count = 1;
        }
      }

      if (field_A9C_ == 0) {
        while (drawn_count < static_cast<int>(bevel_thickness_)) {
          buffer->box(&bevel, static_cast<int>(bevel_upper_),
                      static_cast<int>(bevel_lower_));
          ++bevel.left;
          --bevel.right;
          ++bevel.top;
          --bevel.bottom;
          ++drawn_count;
        }
      } else {
        while (drawn_count < static_cast<int>(bevel_thickness_)) {
          buffer->box(&bevel, static_cast<int>(bevel_lower_),
                      static_cast<int>(bevel_upper_));
          ++bevel.left;
          --bevel.right;
          ++bevel.top;
          --bevel.bottom;
          ++drawn_count;
        }
        bevel.left += static_cast<int>(bevel_thickness_);
        bevel.top += static_cast<int>(bevel_thickness_);
        bevel.right += static_cast<int>(bevel_thickness_);
        bevel.bottom += static_cast<int>(bevel_thickness_);
      }
      rect_left = bevel.left;
      rect_top = bevel.top;
      rect_right = bevel.right;
      rect_bottom = bevel.bottom;
    }
  }

  if (name_ != 0) {
    const int text_color = static_cast<int>(field_ABC_[state]);
    if (text_color >= 0) {
      set_text_color(text_color, -1, 1, 1);
    }

    if (parent->vslot_05(reinterpret_cast<int>(this),
                         reinterpret_cast<int>(name_),
                         static_cast<int>(field_A78_)) == 0) {
      const int text_width = rect_right - rect_left - 6;
      const int wrapped_height = buffer->wrap_height(name_, text_width);
      const int line_height = buffer->text_line_height();
      const int text_height = buffer->text_height();
      const int vertical_shift = (wrapped_height - line_height) + text_height;
      const int y = ((rect_bottom - rect_top) - vertical_shift) / 2 + rect_top;

      if (y >= 0 && (static_cast<int>(field_18_) > 1
                     || buffer->text_width(name_) <= text_width)) {
        buffer->wrap_cent(name_, rect_left + 3, y, text_width);
      } else {
        const int ellipsis_width = buffer->text_width("...");
        // Native fallback reuses the rectangle rather than the wrap baseline:
        // inset both sides, reserve the dots, then translate that rectangle
        // by its own width for the second write.
        RECT clipped_text = {rect_left + 3, rect_top,
                             rect_right - 3 - ellipsis_width, rect_bottom};
        buffer->set_clip(&clipped_text);
        buffer->write_l(name_, &clipped_text,
                        static_cast<int>(strlen(name_)));
        buffer->restore_clip();

        const int clipped_width = clipped_text.right - clipped_text.left;
        RECT ellipsis = {clipped_text.right, clipped_text.top,
                         clipped_text.right + clipped_width,
                         clipped_text.bottom};
        buffer->write_l("...", &ellipsis, 3);
      }
    }
  }
}
