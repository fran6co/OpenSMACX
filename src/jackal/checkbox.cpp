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
// ORIGINAL: 0x0060FD80 sub_60fd80 0x0060FD80-0x0060FD88;0x0060FD90-0x0060FE40 FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060fd80.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/jackal/checkbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_ECheckBox@@$4PPPPPPPM@A@AEPAXI@Z
// TRIED: signature nullary -> thiscall member(unsigned int flag), returning void*; evidence: ecx read at entry with no stack access, `ret 4` pops one stack dword, `mov eax,ebx` before ret. Body is a scalar deleting destructor for a class with two virtual bases (Dialog, GraphicWin); the leading `sub ecx,[ecx-4]; jmp` is a vtordisp adjustor thunk with no C++ source spelling attempted here, so offsets into the vbase are read via the vbtable at runtime (matching the disassembly) rather than modeled with real virtual inheritance. Shares its tail with 0x0060FED0 (same body, folded).

// ORIGINAL: 0x0060FEB0 sub_60feb0 0x0060FEB0-0x0060FEC1;0x0060FD90-0x0060FE40 FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060feb0.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/jackal/checkbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_ECheckBox@@$4PPPPPLLI@EEE@AEPAXI@Z
// TRIED: known-wall shape (two-span function: adjustor thunk + shared scalar-deleting-dtor body reached via jmp; no C++ source naturally splits into two spans or reproduces the fs:[0] frame). Own __thiscall Obj_60feb0::handler(uint) does the vbase subtraction, the vtable-relocation writes, close()/dtor() calls with the exact `base+off4`/`base+off8` `this` derived from the asm, and the conditional operator delete(base); compiles, MISMATCH at #0 (whole-function register/frame order)

// ORIGINAL: 0x0060FED0 sub_60fed0 0x0060FED0-0x0060FEDE;0x0060FD90-0x0060FE40 FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060fed0.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/jackal/checkbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_ECheckBox@@$4PPPPPPPM@KBI@AEPAXI@Z
// TRIED: signature stdcall(int) -> thiscall member(unsigned int flag), returning void*; same body as 0x0060FD80 (this thunk only differs in the leading adjustor, `sub ecx,[ecx-4]; sub ecx,0xa18` vs `sub ecx,[ecx-4]`), which is why it is COMDAT-folded with it. Vtordisp thunk not modeled; see 0x0060FD80 for the reasoning.

// ORIGINAL: 0x0060FE40 ?on_redraw@thunk1_CheckBox@@QAEXXZ 0x0060FE40-0x0060FE48 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060FE40', '0x0060FE48']]", "mapped constructor ??0CheckBox@@QAE@H@Z directly stores table 0x006705B0", "pinned win slot 0x30 contains original thunk VA 0x0060FE40", "original tail target 0x0060FAB0 is ?on_redraw@CheckBox@@UAEXXZ; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_redraw@CheckBox@@$4PPPPPPPM@A@AEXXZ in ??_7CheckBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_redraw@CheckBox@@$4PPPPPPPM@A@AEXXZ
// kind      game
// ORIGINAL: 0x0060FE50 ?on_mouse_move@thunk1_CheckBox@@QAEXHH@Z 0x0060FE50-0x0060FE58 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060FE50', '0x0060FE58']]", "mapped constructor ??0CheckBox@@QAE@H@Z directly stores table 0x006705B0", "pinned win slot 0x44 contains original thunk VA 0x0060FE50", "original tail target 0x0060FBB0 is ?on_mouse_move@CheckBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_mouse_move@CheckBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7CheckBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_mouse_move@CheckBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060FE70 ?on_left_down@thunk1_CheckBox@@QAEXHH@Z 0x0060FE70-0x0060FE78 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060FE70', '0x0060FE78']]", "mapped constructor ??0CheckBox@@QAE@H@Z directly stores table 0x006705B0", "pinned win slot 0x50 contains original thunk VA 0x0060FE70", "original tail target 0x0060F700 is ?on_left_down@CheckBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_left_down@CheckBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7CheckBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_left_down@CheckBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060FE90 ?on_left_double_click@thunk1_CheckBox@@QAEXHH@Z 0x0060FE90-0x0060FE98 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060FE90', '0x0060FE98']]", "mapped constructor ??0CheckBox@@QAE@H@Z directly stores table 0x006705B0", "pinned win slot 0x70 contains original thunk VA 0x0060FE90", "original tail target 0x0060FA80 is ?on_left_double_click@CheckBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_left_double_click@CheckBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7CheckBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_left_double_click@CheckBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060FEF0 ?on_redraw@thunk3_CheckBox@@QAEXXZ 0x0060FEF0-0x0060FEFE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060FEF0', '0x0060FEFE']]", "mapped constructor ??0CheckBox@@QAE@H@Z directly stores table 0x0067059C", "pinned primary slot 0x8 contains original thunk VA 0x0060FEF0", "original tail target 0x0060FAB0 is ?on_redraw@CheckBox@@UAEXXZ; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_redraw@CheckBox@@$4PPPPPPPM@KBI@AEXXZ in ??_7CheckBox@@6B@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_redraw@CheckBox@@$4PPPPPPPM@KBI@AEXXZ
// kind      game
// ORIGINAL: 0x0060E5F0 sub_60e5f0 0x0060E5F0-0x0060E606 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60e5f0..0x60e606; CRT slot 0x68269c selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_CHECKBOX_SPRITE_1@@3U?$CheckBoxSpriteLifetime@$00@@A, guard ?CheckBoxSpritesCleanupGuard@@3EA, startup _$E21 and cleanup _$E19. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original init to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E8
// ORIGINAL: 0x0060E610 sub_60e610 0x0060E610-0x0060E62F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60e610..0x60e62f; CRT slot 0x68269c selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_CHECKBOX_SPRITE_1@@3U?$CheckBoxSpriteLifetime@$00@@A, guard ?CheckBoxSpritesCleanupGuard@@3EA, startup _$E21 and cleanup _$E19. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original fini to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E6
// ORIGINAL: 0x0060E630 sub_60e630 0x0060E630-0x0060E646 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60e630..0x60e646; CRT slot 0x6826a0 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_CHECKBOX_SPRITE_2@@3U?$CheckBoxSpriteLifetime@$01@@A, guard ?CheckBoxSpritesCleanupGuard@@3EA, startup _$E26 and cleanup _$E24. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original init to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E13
// ORIGINAL: 0x0060E650 sub_60e650 0x0060E650-0x0060E66F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60e650..0x60e66f; CRT slot 0x6826a0 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_CHECKBOX_SPRITE_2@@3U?$CheckBoxSpriteLifetime@$01@@A, guard ?CheckBoxSpritesCleanupGuard@@3EA, startup _$E26 and cleanup _$E24. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original fini to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E11
#include "stdafx.h"
#include "original_seam.h"
#include "checkbox.h"
#include "win.h"  // WinCallbackWindow


// The constructor loads these rebindable pointer cells, not sprite contents.
// init_class passes their original targets to Sprite::extract.
// RECOVERY_OBJECT: v2 0x00697104 0x00697108 ?CheckBoxDefault1@@3PAVSprite@@A 0x0060E670
Sprite *CheckBoxDefault1 =
    reinterpret_cast<Sprite *>(g_CHECKBOX_SPRITE_1.storage_.bytes);
// RECOVERY_OBJECT: v2 0x00697108 0x0069710C ?CheckBoxDefault2@@3PAVSprite@@A 0x0060E670
Sprite *CheckBoxDefault2 =
    reinterpret_cast<Sprite *>(g_CHECKBOX_SPRITE_2.storage_.bytes);

/*
Purpose: Construct the virtual bases through VC6's hidden most-derived flag,
         clear this control's state, and install its default sprite pointers.
// ORIGINAL: 0x0060E670 ??0CheckBox@@QAE@H@Z 0x0060E670-0x0060E735;0x00662F60-0x00662F84 BYTE_EXACT
// symbol    ??0CheckBox@@QAE@XZ
// MEASURED (per the coordinator's correction on constructor-vs-flag
//            mangling): a `void construct(int a1)` METHOD spelling of this
//            same body, symbol-mapped to this address, scores WORSE - 0/53
//            against this constructor spelling's 1/53. Neither reaches the
//            frame; the constructor form is kept as the closer of the two.
// size      233 bytes
// prototype void (__thiscall ??0CheckBox@@QAE@H@Z)(CheckBox* this, int)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
// earlier (src/unrecovered/0060e670.cpp) and reached only 0.71 mnemonic
// similarity - the image installs a real fs:[0] frame here but with pure
// esp-relative addressing, no `push ebp; mov ebp,esp`, which no flag set
// this harness tries reproduces. Landed as the closest faithful shape.
Return Value: n/a
Status: Complete
*/
CheckBox::CheckBox() {
    // VC6 owns virtual-base construction and derived table setup under its
    // hidden most-derived flag. The body initializes this class's own state.
    field_4_ = 0;
    key_notification_callback_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;
}
// (The old CheckBox(int a1) spelling: `if (a1 != 0) { new (GraphicWin*)
// GraphicWin(); Dialog::construct(); }` - both hand construction seams.)

/*
Purpose: Tear down a CheckBox: reinstall the base subobjects' own
         vtable/vtordisp values, reset the state fields, and close the
         Dialog and GraphicWin subobjects through the object's own vbtable.
// ORIGINAL: 0x0060E740 ??1CheckBox@@UAE@XZ 0x0060E740-0x0060E7C0
// LEVER: the manual `- 0x1C` came OUT - this is a real override of
//   GraphicWin's virtual destructor now, so VC6 enters it on the GraphicWin
//   subobject and walks back itself, and subtracting again double-counted.
//   2/37 -> 7/37. The note below diagnosed this exactly and could not act on
//   it while the base was a member; it is kept for the diagnosis.
// TRIED: dropping the named `self`/`obj` intermediates and writing
//   `reinterpret_cast<char *>(this)` at each use, which is the lever that
//   works on this class's on_dialog_focus and on_mouse_leave. Here it is
//   WORSE, 7/37 -> 0/37 at the same 47 compiled instructions. The remaining
//   gap is that the image folds the adjustment into every addressing mode
//   (`[edx + ecx - 0x1c]`) where this tree materialises the pointer into esi
//   once and uses `[reg + esi]`; ours also emits the three vtable stores
//   later than the image does.
// TRIED: MEASURED 2/37 agreeing. The image's opening `this` is already
//            the vtordisp-adjusted GraphicWin-subobject pointer -
//            `mov eax,[ecx-0x1c]; lea esi,[ecx-0x1c]` - and this body's
//            `char *const self = this - 0x1C;` opening compiles a different
//            prologue shape (an indirect call appears at instruction 1
//            where the image has none), which VC6 does not choose from a
//            hand-written subtraction the way it does for a genuine
//            vtordisp thunk over real virtual inheritance.
// size      128 bytes
// prototype void (__thiscall ??1CheckBox@@UAE@XZ)(CheckBox* this)
// callers   24   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
// notes     Entered on the ADJUSTED (GraphicWin-subobject) `this`: the image
//           opens `mov eax,[ecx-0x1c]` / `lea esi,[ecx-0x1c]`, exactly the
//           vtordisp adjustor the constructor's own vtordisp slot names.
Return Value: n/a
Status: Complete
*/
CheckBox::~CheckBox() {
    // NO MANUAL ADJUSTMENT. This is a real override of GraphicWin's virtual
    // destructor now, so VC6 enters it on the GraphicWin subobject and walks
    // back to the CheckBox pointer itself; subtracting 0x1C again in source
    // double-counts it. The TRIED note above diagnosed exactly this and could
    // not act on it while the base was a member.
    // Strip-all direction: the image's five derived-stage re-stores here are
    // gone, same as the constructor's. The two base closes are qualified
    // calls - the compiler's virtual-base dispatch.

    field_4_ = 0;
    key_notification_callback_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;

    Dialog::close();
    GraphicWin::close();
}

/*
Purpose: Reset the check box to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
// ORIGINAL: 0x0060E7C0 ?close@CheckBox@@QAEIXZ 0x0060E7C0-0x0060E7F9 BYTE_EXACT
// symbol    ?close@CheckBox@@QAEIXZ
// TRIED: the reconstructed signature `?close@CheckBox@@QAEXXZ`. The census
//   names are RECONSTRUCTIONS - the image is stripped - and this one had the
//   same method carrying two signatures across one hierarchy: Dialog::close and
//   four derived close() returning void, GraphicWin::close and ListBox::close
//   returning uint32_t. The original cannot have had that. Unified on
//   `uint32_t` (the form two bodies were already tuned to, for the EAX
//   residue) and on `virtual`, which is what the vtable slot these classes
//   install by hand actually is - hence `UAEIXZ`.
// LEVER: the vbtable is RE-READ for the second call, exactly as the destructor above it does. One cached `vbtable` local has to survive the first `close()`, so VC6 spends a callee-saved register and a `push edi` the image never makes: 5 of 20. Reading `[esi]` again for the GraphicWin call is 20 of 20. Reading it inline at BOTH sites scores the same 20 of 20, so the first read may stay a named local.
// size      57 bytes
// prototype uint32_t (__thiscall ?close@CheckBox@@QAEIXZ)(CheckBox* this)
// callers   13   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
Return Value: n/a
Status: Complete
*/
uint32_t CheckBox::close() {
    // The base offsets come from the object's own vbtable, read at run time,
    // not from where they sit in a most-derived CheckBox. When one is
    // embedded in a larger class - Dialogs holds a RadioButton at 0x44 - that
    // object's vbtable names different offsets, and hardcoding this class's
    // own sends both calls to the wrong subobject. Doing exactly that passed
    // every unit test here and crashed the game on a null vtable pointer.
    // The base calls go through the runtime vbtable either way - the
    // compiler's dispatch for a virtual-base member call reads it at run
    // time exactly as the old hand walk did, so the embedded case still
    // reaches the right subobjects.
    field_4_ = 0;
    key_notification_callback_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;
    Dialog::close();
    return GraphicWin::close();
}

// Seed identity: ?draw_item@CheckBox@@QAEXHHH@Z at 0x0060F030.
// This is the product implementation corresponding to the archived seed.
// The original reads the state argument before selecting the sprite: a
// nonzero second explicit argument selects checked_sprite_, while zero selects
// unchecked_sprite_.  The seed's local geometry value is not that argument.
// ORIGINAL: 0x0060F030 ?draw_item@CheckBox@@QAEXHHH@Z 0x0060F030-0x0060F6FF
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060f030.cpp:1 maps ?draw_item@CheckBox@@QAEXHHH@Z to 0x0060F030–0x0060F6FF; the supplied mapped record classifies this scoped identity as legacy_seed.", "src/jackal/checkbox.cpp:173 defines the complete, unannotated CheckBox::draw_item(int position, int state, int focus_mode); line 250 selects checked_sprite_ when state is nonzero and unchecked_sprite_ otherwise. src/jackal/checkbox.h:157–160 binds those pointer members to offsets 0x10 and 0x14 respectively.", "Fresh original-image SHA-256 equals the pinned 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf. Disassembly confirms the 0x24-byte local allocation and four saved registers, and 0x0060F3DE loads [esp+0x3C], the second explicit argument; its zero branch uses the +0x10 sprite and its nonzero branch uses +0x14.", "Explicit seed_bindings delegation permits this existing identity and editable product TU. This proposal binds only CheckBox; RadioButton binding, the pending finding, and fresh native verification remain outstanding. No code acceptance is asserted."], "reason": "The existing scoped seed identifies CheckBox::draw_item(int,int,int). The unannotated product definition implements that interface using actual bases and members, including the original second-argument sprite selection. Bind this existing identity while preserving its original spans and seed history; verification and discovery resolution remain pending."}
// Deleted seed provenance: git:25c0645e742c23f149e0895c391a9ba79ad744cb:src/unrecovered/0060f030.cpp
void CheckBox::draw_item(int position, int state, int) {
    Win *parent = win_parent_;
    if (parent == 0) {
        return;
    }

    int extra_height = 0;
    Dialog::item_list_.seek_pos(-1);
    for (int i = 0; i < position; ++i) {
        Dialog::item_list_.next_entry();
        if (Dialog::item_list_.current_ != 0 &&
            static_cast<int>(Dialog::item_list_.current_entry()->field_8_) > 1) {
            int lines = Dialog::item_list_.head_ != 0
                ? static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_)
                : 0;
            int height = static_cast<int>(Dialog::field_68_);
            if (height <= Buffer::text_line_height() * lines) {
                lines = Dialog::item_list_.head_ != 0
                    ? static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_)
                    : 0;
                height = Buffer::text_line_height() * lines;
            }
            extra_height += height - static_cast<int>(Dialog::field_68_);
        }
    }

    const int columns = Dialog::field_58_;
    const int column = position / columns;
    const int column_width = Dialog::field_64_;
    const int cell_height = Dialog::field_44_;
    const int x = (column_width + cell_height) * column + Dialog::field_40_;
    const int y = (cell_height >> 1) + extra_height +
                  (position - column * columns) *
                      (Dialog::field_68_ + cell_height);
    if (!Dialog::item_list_.seek_pos(position)) {
        return;
    }

    int text_lines = Dialog::item_list_.head_ != 0
        ? static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_)
        : 0;
    int item_height = static_cast<int>(Dialog::field_68_);
    if (item_height <= Buffer::text_line_height() * text_lines) {
        text_lines = Dialog::item_list_.head_ != 0
            ? static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_)
            : 0;
        item_height = Buffer::text_line_height() * text_lines;
    }

    const int original_x = x;
    const int original_y = y;
    const int original_bottom = y + item_height;

    RECT area;
    area.left = x;
    area.top = y;
    area.right = x + Dialog::field_64_;
    area.bottom = original_bottom;

    const int callback_state = state != 0
        ? 2
        : (Dialog::field_F0_ == static_cast<uint32_t>(position) ? 1 : 0);
    const int callback_result = parent->on_dialog_item_back_draw(
        static_cast<GraphicWin *>(this), Dialog::pos_to_id(position),
        callback_state, &area);
    if (callback_result == 0) {
        static_cast<GraphicWin *>(parent)->copy(
            this, Dialog::field_24_ + original_x,
            Dialog::field_28_ + original_y, original_x, original_y,
            Dialog::field_64_, item_height);

        if (checked_sprite_ != 0 && unchecked_sprite_ != 0) {
            if (state != 0) {
                checked_sprite_->draw(
                    this, checked_sprite_->cTransparentIndex_, original_x,
                    original_y + ((area.bottom - checked_sprite_->iHeight_ -
                                   area.top) >> 1), 1, 1);
            } else {
                unchecked_sprite_->draw(
                    this, unchecked_sprite_->cTransparentIndex_, original_x,
                    original_y + ((area.bottom - unchecked_sprite_->iHeight_ -
                                   area.top) >> 1), 1, 1);
            }
        }
    }

    area.top += Buffer::font1_->descent_;
    area.left += Dialog::field_34_;
    if (Dialog::item_list_.head_ != 0 &&
        Dialog::item_list_.current_->string_payload()->field_8_ == 1) {
        char *text = Dialog::item_list_.current_text();
        char *separator = strchr(text, '|');
        if (separator == 0) {
            text = Dialog::item_list_.current_text();
            if (text != 0) {
                Buffer::write_l(text, &area, strlen(text));
            }
        } else {
            *separator = 0;
            text = Dialog::item_list_.current_text();
            if (text != 0) {
                Buffer::write_l(text, &area, strlen(text));
            }
            *separator = '|';
            ++separator;
            if (separator != 0) {
                Buffer::write_right_l(separator, &area, strlen(separator));
            }
        }
    } else {
        char *text = Dialog::item_list_.current_text();
        char *separator = strchr(text, '|');
        if (separator == 0) {
            text = Dialog::item_list_.current_text();
            Buffer::wrap(text, &area);
        } else {
            *separator = 0;
            text = Dialog::item_list_.current_text();
            Buffer::wrap(text, &area);
            *separator = '|';
            ++separator;
            if (separator != 0) {
                Buffer::write_right_l(separator, &area, strlen(separator));
            }
        }
    }

    if (Dialog::field_B8_ != static_cast<uint32_t>(-1) &&
        state != 0 && Win::is_dialog_focus() != 0) {
        const int text_width = area.right - area.left;
        area.left = Dialog::field_34_;
        area.top = original_y;
        area.right = area.left + text_width;
        area.bottom = original_bottom;
        Buffer::dotted_box(&area, Dialog::field_B8_);
    }
}

// 0x0060ED10. Preserve the circular list cursor updates, bitmap-derived
// default cell dimensions, and fixed/automatic geometry branches of the
// original dialog control.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: MISMATCH #101 push/xor - three min/max ternaries rewritten as if/else (operand order matching the original's cmp/jcc) moved the divergence from #78 to #101; further tweaks to the text_height() ternary and the max_width loop's > vs >= made no further difference.
// Virtual-base offsets are read from the vbtable at runtime rather than
// hardcoded, matching the same pattern confirmed on EditGroup::calculate_dimensions
// (0x00612150): vbtable[1] (byte offset 4) is the GraphicWin base offset,
// vbtable[2] (byte offset 8) is the Dialog base offset, and a Buffer member
// sits at GraphicWin_base + 0x444.
// Deleted seed provenance: git:26e5eed1:src/unrecovered/0060ed10.cpp
// ORIGINAL: 0x0060ED10 ?calculate_dimensions@CheckBox@@QAEHXZ 0x0060ED10-0x0060F02B FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060ed10.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60ed10 complete control/data-access review and src/jackal/checkbox.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
int CheckBox::calculate_dimensions() {
    Dialog &dialog = *this;
    Buffer *const buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    if (dialog.item_list_.entry_count_ == 0) {
        return 7;
    }
    buffer->set_text_color(text_color_a_, text_color_b_, text_color_c_, text_color_d_);
    buffer->set_text_color2(text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_);
    buffer->set_text_color3(text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_);
    buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);

    if (dialog.field_34_ == 0 && unchecked_sprite_ != 0 && checked_sprite_ != 0) {
        dialog.field_34_ = unchecked_sprite_->width();
        if (dialog.field_34_ < checked_sprite_->width()) {
            dialog.field_34_ = checked_sprite_->width();
        }
    }
    if (dialog.field_48_ == -1) {
        dialog.field_68_ = 0;
        if (unchecked_sprite_ != 0 && checked_sprite_ != 0) {
            dialog.field_68_ = unchecked_sprite_->height();
            if (dialog.field_68_ < checked_sprite_->height()) {
                dialog.field_68_ = checked_sprite_->height();
            }
        }
    } else {
        dialog.field_68_ = dialog.field_48_;
    }

    dialog.field_58_ = dialog.item_list_.entry_count_;
    int max_text_width = 0;
    for (int i = 0; i < dialog.item_list_.entry_count_; ++i) {
        if (dialog.item_list_.head_ != 0) {
            dialog.item_list_.current_ = dialog.item_list_.current_->next;
            ++dialog.item_list_.current_position_;
            if (dialog.item_list_.current_position_ == dialog.item_list_.entry_count_) {
                dialog.item_list_.current_position_ = 0;
            }
        }
        StringStructEntry *const entry = dialog.item_list_.current_;
        char *text = entry == 0 || entry->payload == 0 ? 0 : entry->string_payload()->text;
        const int width = buffer->text_width(text);
        if (max_text_width < width) {
            max_text_width = width;
        }
    }
    dialog.field_64_ = dialog.field_44_ + dialog.field_34_ + max_text_width;
    const int text_height = buffer->text_height();
    if (dialog.field_68_ <= text_height) {
        dialog.field_68_ = buffer->text_height();
    }

    if ((dialog.field_20_ & 2) != 0 && dialog.field_2C_ != 0) {
        dialog.field_5C_ = dialog.field_2C_;
        dialog.field_64_ = (dialog.field_2C_ - dialog.field_40_ * 2)
                           / dialog.field_4C_ - dialog.field_44_;
    } else {
        int width = dialog.field_64_ * dialog.field_4C_ + dialog.field_40_ * 2;
        const int minimum = dialog.field_2C_ + dialog.field_40_ * 2;
        if (width < minimum) width = minimum;
        dialog.field_5C_ = width;
        dialog.field_64_ = (width - dialog.field_40_ * 2) / dialog.field_4C_
                           - dialog.field_44_;
        dialog.field_5C_ -= dialog.field_44_;
    }
    dialog.field_54_ = dialog.field_4C_;
    if ((dialog.field_20_ & 1) != 0 && dialog.field_30_ != 0) {
        dialog.field_60_ = dialog.field_30_;
        dialog.field_58_ = dialog.field_30_ / dialog.field_68_;
    } else {
        dialog.field_60_ = (dialog.field_68_ + dialog.field_44_) * dialog.field_58_;
    }
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: nothing confirmed byte-exact; MISMATCH #3 'push' vs 'mov' - the vtable-array double-reads (edx/ecx caching across the 0xa34/0x1c constants) get register-allocated differently than the original's edi-kept-live scheme.
// ORIGINAL: 0x0060E800 ?attach@CheckBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060E800-0x0060E97F FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060e800.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60e800 complete control/data-access review and src/jackal/checkbox.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?attach@CheckBox@@UAEHPAVGraphicWin@@HHH@Z
int CheckBox::attach(GraphicWin *parent, int x, int y, int flags) {
    if (parent == 0) return 3;
    if (HandleMain == 0) return 7;
    calculate_dimensions();
    Dialog::field_24_ = x;
    Dialog::field_28_ = y;
    if ((parent->iFlags_ & 0x1000) == 0) flags |= 0x100000;
    const int init_result = GraphicWin::init(
        x, y, Dialog::field_5C_, Dialog::field_60_, 0, flags, parent, 0, 0);
    if (init_result != 0) return init_result;
    Buffer *const buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    buffer->set_text_color(text_color_a_, text_color_b_, text_color_c_, text_color_d_);
    buffer->set_text_color2(text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_);
    buffer->set_text_color3(text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_);
    buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: reusing one `char *b = self - 0x1c` for both the vtable-slot lookup and the null-check-then-adjust pattern collapses two distinct registers (edi and ebp in the original, which hold the same VALUE but are never merged) into one - the original keeps a separate register alive purely for the "cast-to-virtual-base" null check, this candidate CSEs it away. 69/84 mnemonics match, no stack spill (ternary over if/else avoided the local-int spill an earlier if/else form produced).
// ORIGINAL: 0x0060FAB0 ?on_redraw@CheckBox@@QAEXXZ 0x0060FAB0-0x0060FB8D FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060fab0.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60fab0 complete control/data-access review and src/jackal/checkbox.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_redraw@CheckBox@@UAEXXZ
void CheckBox::on_redraw() {
    // Native60FAB0 retains this parent across back-draw and fallback copying.
    Win *const parent = win_parent_;
    if (parent == 0) return;
    WinCallbackWindow = parent;
    const int handled = parent->on_dialog_back_draw(
        static_cast<GraphicWin *>(this));
    if (handled == 0) {
        static_cast<GraphicWin *>(parent)->copy(
            this, Dialog::field_24_, Dialog::field_28_, 0, 0,
            static_cast<int>(dib_.bmiHeader.biWidth),
            -static_cast<int>(dib_.bmiHeader.biHeight));
    }
    for (int position = 0; position < Dialog::item_list_.entry_count_; ++position) {
        const int id = Dialog::pos_to_id(position);
        const int state = get_state_id(id);
        draw_item(position, state,
                  Dialog::field_F0_ == static_cast<uint32_t>(position));
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060F700 ?on_left_down@CheckBox@@QAEXHH@Z 0x0060F700-0x0060F8B0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060f700.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60f700 complete control/data-access review and src/jackal/checkbox.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_left_down@CheckBox@@UAEXHH@Z
void CheckBox::on_left_down(int x, int y) {
    WinCallbackWindow = win_parent_;
    const int columns = Dialog::field_4C_;
    const int rows = Dialog::field_58_;
    const int cell_width = Dialog::field_48_ + Dialog::field_28_;
    const int cell_height = Dialog::field_68_ + Dialog::field_28_;
    const int position = (x / cell_width) * rows + y / cell_height;
    const int id = Dialog::pos_to_id(position);
    Dialog::item_list_.seek_id(id);

    const int previous = static_cast<int>(field_4_);
    flip_state_id(id);
    field_4_ = Dialog::id_to_pos(id);
    if (previous != static_cast<int>(field_4_)) {
        const int old_id = Dialog::pos_to_id(previous);
        draw_item(previous, get_state_id(old_id), 0);
    }

    const int selected = Dialog::selected_position_;
    if (double_click_callback_ != 0) double_click_callback_(selected);
    // The original leaves the list cursor on the matching entry after the
    // callback, since callbacks may mutate the list and the cursor must be
    // re-found against its current contents.
    if (Dialog::item_list_.head_ != 0) {
        Dialog::item_list_.current_ = Dialog::item_list_.head_;
        Dialog::item_list_.current_position_ = 0;
        for (int index = 0; index < Dialog::item_list_.entry_count_; ++index) {
            if (Dialog::item_list_.current_->id == id) break;
            Dialog::item_list_.current_ = Dialog::item_list_.current_->next;
            ++Dialog::item_list_.current_position_;
        }
    }
    draw_item(static_cast<int>(field_4_), get_state_id(id), 1);
    soft_update();
}


// The state word these three share. Every one of them reaches it the same way
// the original does - `mov eax,[ecx]` then `[eax+8]` - so the Dialog offset
// comes from the object's OWN vbtable at run time. Hardcoding this class's own
// 0xA34 would work for a most-derived CheckBox and aim at the wrong subobject
// for an embedded one, which is the defect that passed every unit test here
// and crashed the game on a null vtable pointer. Repeated inline rather than
// factored out, matching close() above, so each body stands alone.

/*
Purpose: Toggle one bit of the state word - clear it when set, set it when
         clear.

             mov eax,[ecx] / mov edx,[eax+8] / mov eax,1
             lea edx,[edx+ecx+0xEC] / mov ecx,[esp+4] / shl eax,cl
             mov ecx,[edx] / test eax,ecx / je set
             not eax / and eax,ecx / mov [edx],eax / ret 4
             set: or ecx,eax / mov [edx],ecx / ret 4

         The shift count is whatever CL holds, so the position is masked to
         five bits by the hardware; `pos & 31` states that rather than relying
         on C++ shift-overflow, which is undefined rather than truncating.
// ORIGINAL: 0x0060EB50 ?UNK1@CheckBox@@QAEXH@Z 0x0060EB50-0x0060EB7D
// size      45 bytes
// prototype void (__thiscall ?UNK1@CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckBox::UNK1(int pos) {
    // The state word is Dialog::selected_position_, reached through the
    // virtual base - the compiler's member access walks the vbtable exactly
    // where the old hand walk read it.
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (selected_position_ & mask) {
        selected_position_ &= ~mask;
    } else {
        selected_position_ |= mask;
    }
}


/*
Purpose: Report whether one bit of the state word is set. Returns the MASKED
         WORD, not a normalised 0 or 1 - the body ends `and eax, esi` with no
         further narrowing, so a caller testing it against 1 would read a
         different answer than the original gives.
// ORIGINAL: 0x0060ECC0 ?UNK2@CheckBox@@QAEHH@Z 0x0060ECC0-0x0060ECE0
// size      32 bytes
// prototype int (__thiscall ?UNK2@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the state word masked to the requested bit
Status: Complete
*/
int CheckBox::UNK2(int pos) {
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    return static_cast<int>(selected_position_ & mask);
}


/*
Purpose: Set or clear one bit of the state word, chosen by the second argument.
         Unlike UNK1 this does not depend on the bit's current value.
// ORIGINAL: 0x0060EC80 ?set_state_pos@CheckBox@@QAEXHH@Z 0x0060EC80-0x0060ECB1
// size      49 bytes
// prototype void (__thiscall ?set_state_pos@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_pos(int pos, int state) {
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (state) {
        selected_position_ |= mask;
    } else {
        selected_position_ &= ~mask;
    }
}


/*
Purpose: Overwrite the whole state word in one store, then repaint through
         the virtual base - unlike UNK1/UNK2/set_state_pos above, which each
         touch a single bit.
// ORIGINAL: 0x0060ECE0 ?set_state_flag@CheckBox@@QAEXJ@Z 0x0060ECE0-0x0060ED03 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?set_state_flag@CheckBox@@QAEXJ@Z)(CheckBox* this, long)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060ECFA
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_flag(long value) {
    // Win slot 62 is a virtual dispatch here. A qualified GraphicWin call
    // would suppress it. Actual virtual bases supply the receiver adjustments
    // for both the member write and the most-derived redraw implementation.
    selected_position_ = static_cast<int32_t>(value);
    redraw();
}

/*
Purpose: Repaint on dialog focus, dispatching through the enclosing object.
// ORIGINAL: 0x0060FB90 ?on_dialog_focus@CheckBox@@UAEXH@Z 0x0060FB90-0x0060FBA7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_dialog_focus@CheckBox@@UAEXH@Z)(CheckBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FB9E
Return Value: n/a
Status: Complete
*/
void CheckBox::on_dialog_focus(int a1) {
    // The repaint, as the plain virtual call it is (win-table slot 62, the
    // redraw default the most-derived may override). a1 is never read;
    // `ret 4` still pops it.
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FEA0 ?on_dialog_focus@thunk1_CheckBox@@QAEXH@Z 0x0060FEA0-0x0060FEA8 BYTE_EXACT
// symbol    ?on_dialog_focus@CheckBox@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
// ORIGINAL: 0x0060FC30 ?on_mouse_leave@CheckBox@@UAEXHH@Z 0x0060FC30-0x0060FC58 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@CheckBox@@UAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FC4F
Return Value: n/a
Status: Complete
*/
void CheckBox::on_mouse_leave(int a1, int a2) {
    // Clear the hover index (Dialog::field_F0_, through the virtual base)
    // and repaint (the redraw virtual) - plain member forms; the
    // compiler's virtual-base machinery does the walking the old hand
    // chains spelled. Both parameters are dead; `ret 8` still pops them.
    field_F0_ = -1;
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FE60 ?on_mouse_leave@thunk1_CheckBox@@QAEXHH@Z 0x0060FE60-0x0060FE68 BYTE_EXACT
// symbol    ?on_mouse_leave@CheckBox@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
// ORIGINAL: 0x0060FC60 ?init_class@CheckBox@@QAAHXZ 0x0060FC60-0x0060FD52;0x00662F84-0x00662F99 BYTE_EXACT
// symbol    ?init_class@CheckBox@@SAHXZ
// size      263 bytes
// prototype int (__cdecl ?init_class@CheckBox@@QAAHXZ)()
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005E39A0
//
// Promoted 2026-08-15 from src/unrecovered/0060fc60.cpp to retire its
// pending_bodies forwarder.
// LEVER: ~Buffer out of line (buffer.h/buffer.cpp, 2026-09-03). Under /O2's
//   /Ob1 an in-class `~Buffer() { close(); }` expands into this body's scope
//   end as vptr store + `close()` + `~Spot()` - 71 instructions and an EBP
//   frame, against the image's 63 and one `call ??1Buffer@@QAE@XZ` with the
//   frameless EH form (`sub esp,0x588`, state at [esp+0x594]). Moving the
//   definition beside ??1Buffer's own marker in buffer.cpp collapsed the
//   teardown to that one call AND dropped the frame: 24/63 -> 63/63. The
//   frame was never a separate lever - it is what the expanded teardown
//   needed a stable pointer for.
Status: Complete
*/
int __cdecl CheckBox::init_class() {
    // A REAL LOCAL, not `char bufMem[sizeof(Buffer)]` with a placement new.
    // The image opens `sub esp, 0x588`, which is sizeof(Buffer) exactly; the
    // array spelling costs four more bytes of frame and an explicit
    // destructor call the compiler would emit itself.
    Buffer buf;
    buf.init(0x20, 0x20, 0, 0);
    int result = buf.load_pcx("jackal.pcx", 0, 10, 0xec);
    if (result != 0) {
        return result;
    }
    g_CHECKBOX_SPRITE_1.sprite().extract(&buf, 0x109, 1, 0x44, 0x20, 0x20, 0);
    g_CHECKBOX_SPRITE_2.sprite().extract(&buf, 0x109, 0x22, 0x44, 0x20, 0x20, 0);
    return 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FEE0 ?attach@thunk1_CheckBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060FEE0-0x0060FEE8 BYTE_EXACT
// symbol    ?attach@CheckBox@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Move the group's selection to the control with `id`, then set or clear
         that control's bit in the shared state word.
// ORIGINAL: 0x0060EB80 ?set_state_id@CheckBox@@QAEXHH@Z 0x0060EB80-0x0060EC0B FILE BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0060eb80.cpp, whose claim proved the ARTIFACT reproduced while the shipped program contained nothing. Kept in the artifact's own spelling for now - it reaches the Dialog subobject through the vbtable by hand (`*(int *)(*(int *)this + 8)`), which is one of the shapes tools/compiler_work.py counts, and unpicking that is a class-model change rather than a promotion.
// symbol    ?set_state_id@CheckBox@@QAEXHH@Z
// size      139 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_id(int id, int value) {
    Dialog::item_list_.seek_id(id);
    const uint32_t mask =
        1U << (static_cast<uint32_t>(Dialog::item_list_.current_position_) & 31U);
    if (value != 0) {
        Dialog::selected_position_ |= mask;
    } else {
        Dialog::selected_position_ &= ~mask;
    }
}

// ORIGINAL: 0x0060EA10 ?flip_state_id@CheckBox@@QAEXH@Z 0x0060EA10-0x0060EB48
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x0060EA10 body searches the item list by id and toggles the bit indexed by the Dialog item-list current position; src/dialog.h exposes item_list_ and selected_position_; src/stringstruct.h documents seek_id cursor updates and return semantics."], "reason": "Recover CheckBox state toggling in the product class: seek_id updates the Dialog item-list cursor and selected_position_ is the shared state-bit word."}
void CheckBox::flip_state_id(int id) {
    Dialog::item_list_.seek_id(id);
    Dialog::selected_position_ ^=
        1U << (static_cast<uint32_t>(Dialog::item_list_.current_position_) & 31U);
}

/*
Purpose: Read the shared state bit of the control with `id`: walk the item list
         to that control, then report `(1 << its ordinal) & state word` -
         nonzero when the box with this id is checked. set_state_id's read-side
         twin, and DipEdit::read_check's per-bit test.
// ORIGINAL: 0x0060EC10 ?get_state_id@CheckBox@@QAEHH@Z 0x0060EC10-0x0060EC79 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0060ec10.cpp, whose claim proved the
//         ARTIFACT reproduced while the shipped program contained nothing, and
//         kept in that artifact's spelling, for the same reason set_state_id
//         above is: it reaches the Dialog subobject through the vbtable by
//         hand. The loop is the artifact's `for (idx = 0; idx < count;
//         idx = idx + 1)` - its note records that the do-while phrasing
//         compiled an extra `cmp;jge` ahead of the back-edge in all 11 tries.
// size      105 bytes
// prototype int (__thiscall ?get_state_id@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: (1 << ordinal) & state word - nonzero when `id`'s box is checked
Status: Complete
*/
int CheckBox::get_state_id(int id) {
    Dialog::item_list_.seek_id(id);
    const uint32_t mask =
        1U << (static_cast<uint32_t>(Dialog::item_list_.current_position_) & 31U);
    return static_cast<int>(Dialog::selected_position_ & mask);
}

// ====================
// Original receivers of Sprite::extract at 0x0060FCFE / 0x0060FD1C.
// RECOVERY_OBJECT: v1 0x009B8F58 ?CheckBoxSpritesCleanupGuard@@3EA
uint8_t CheckBoxSpritesCleanupGuard; // Original separate zero-fill byte9B8F58.
// RECOVERY_OBJECT: v3 0x009B8F60 ?g_CHECKBOX_SPRITE_1@@3VSprite@@A member ?g_CHECKBOX_SPRITE_1@@3U?$CheckBoxSpriteLifetime@$00@@A storage_.bytes
CheckBoxSpriteLifetime<1> g_CHECKBOX_SPRITE_1;
// RECOVERY_OBJECT: v3 0x009B8F90 ?g_CHECKBOX_SPRITE_2@@3VSprite@@A member ?g_CHECKBOX_SPRITE_2@@3U?$CheckBoxSpriteLifetime@$01@@A storage_.bytes
CheckBoxSpriteLifetime<2> g_CHECKBOX_SPRITE_2;

// ===== close_class: once-only teardown of the managed sprites =====

/*
Purpose: ?close_class@CheckBox@@QAAXXZ - run 2 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x0060FD60 ?close_class@CheckBox@@QAAXXZ 0x0060FD60-0x0060FD74 BYTE_EXACT
// symbol    ?teardown_0060fd60@@YAXXZ
// size      20 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_0060fd60() {
    g_CHECKBOX_SPRITE_1.sprite().close();
    g_CHECKBOX_SPRITE_2.sprite().close();
}

/*
Purpose: A double click selects the parent window as the callback window and
         notifies the cached callback with Dialog::selected_position_. The
         original reads this state word directly, not through get_selected_id.
         Its GraphicWin-relative -0x1C vbtable and -0x10 callback accesses
         correspond to the virtual bases and CheckBox's own callback at +0xC;
         the real virtual override lets the compiler supply the adjustments.
// ORIGINAL: 0x0060FA80 ?on_left_double_click@CheckBox@@QAEXHH@Z 0x0060FA80-0x0060FAAD BYTE_EXACT
// symbol    ?on_left_double_click@CheckBox@@UAEXHH@Z
// size      45 bytes
// prototype void (__thiscall ?on_left_double_click@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// indirect  0x0060FAA7
//
// PROMOTED out of the archived verification unit for this address, which proved the body against
// the fixed-address form of the current-win global; this tree's real
// WinCallbackWindow object is the same variable.
Return Value: n/a
Status: Complete
*/
void CheckBox::on_left_double_click(int x_coord, int y_coord) {
    WinCallbackWindow = win_parent_;
    WinNotificationCallback callback = double_click_callback_;
    if (callback != 0) {
        callback(Dialog::selected_position_);
    }
}

/*
Purpose: Add one item to a check box: reject a null label, set or clear this
         item's state bit (the ordinal the new entry is about to take) in the
         shared state word, publish the borrowed label for the insertion hook
         to copy, and append the entry under `id`.

// ORIGINAL: 0x0060E980 ?item@CheckBox@@QAEHPADHH@Z 0x0060E980-0x0060EA0D
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x0060E980 retains name and all original spans/claims", "src/jackal/checkbox.h declares the actual CheckBox::item class member; body uses that existing owner API", "historical source was FILE-mode in git:e9190930285ebdae9121f3ac00645c634df1efea:src/leaf_recoveries.cpp; this transfer places the unchanged C++ member definition in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "the complete source body and existing TRIED/LEVER/DEFERRED history are carried forward unchanged; only annotation mode/location and transitional SCAFFOLD classification change"], "reason": "Transfer the complete CheckBox::item definition to its existing source owner without changing its C++ body. Keep the original FILE spans and historical claim state; BODY is the new per-member measurement scope, not a claim that the old whole-TU recipe is equivalent."}
// size      141 bytes
// prototype int (__thiscall ?item@CheckBox@@QAEHPADHH@Z)(CheckBox* this, int8*, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
// TRIED (forwarder): DipEdit::init adds one item per treaty flag.
Status: Source-owned member body; behavior retained, exactness not measured
*/
int CheckBox::item(char *text, int id, int state) {
    if (text == 0) {
        return 3;
    }
    const uint32_t mask = 1u << (static_cast<uint32_t>(
                                     item_list_.entry_count_) & 31u);
    if (state == 0) {
        selected_position_ &= ~mask;
    } else {
        selected_position_ |= mask;
    }
    item_list_.pending_text_ = text;
    item_list_.field_20_ = 0;
    item_list_.field_24_ = 0;
    return -((item_list_.add(id) != 0) ? 1 : 0) & 4;
}

/*
Purpose: A check box key press: publish the parent window as the callback
         window, repaint the current row unselected, run the arrow/page/space
         navigation over the item list, notify the box's callback with the
         state word, and repaint the new row selected.

// ORIGINAL: 0x0060F8B0 ?on_key_down@CheckBox@@QAEHH@Z 0x0060F8B0-0x0060FA18
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x0060F8B0 retains name and all original spans/claims", "src/jackal/checkbox.h declares the actual CheckBox::on_key_down class member; body uses that existing owner API", "historical source was FILE-mode in git:e9190930285ebdae9121f3ac00645c634df1efea:src/leaf_recoveries.cpp; this transfer places the unchanged C++ member definition in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "the complete source body and existing TRIED/LEVER/DEFERRED history are carried forward unchanged; only annotation mode/location and transitional SCAFFOLD classification change"], "reason": "Transfer the complete CheckBox::on_key_down definition to its existing source owner without changing its C++ body. Keep the original FILE spans and historical claim state; BODY is the new per-member measurement scope, not a claim that the old whole-TU recipe is equivalent."}
// symbol    ?on_key_down@CheckBox@@UAEHH@Z
// size      360 bytes
// prototype int (__thiscall ?on_key_down@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890 0x0060F030
// indirect  0x0060F9D1
// TRIED (seed): the entry convention and field walks - the body is entered on
//         the GraphicWin subobject and reaches CheckBox's own start as
//         `this-0x1c`; the count and column fields ride the runtime vbtable
//         (entry_count_ and field_58_ through the Dialog base) while the bit
//         index (field_4_), the callback (field_8_) and the state word ride
//         direct fields; the real virtual entry lets the compiler supply the
//         adjustments. The seed guessed its switch labels from a range check;
//         the jump tables pinned the real keys: VK_SPACE toggles the row's
//         bit, VK_LEFT/NUMPAD4 and VK_RIGHT/NUMPAD6 page by field_58_ columns,
//         VK_UP/NUMPAD8 and VK_DOWN/NUMPAD2 step one row.
Status: Source-owned member body; behavior retained, exactness not measured
*/
int CheckBox::on_key_down(int key) {
    WinCallbackWindow = win_parent_;
    draw_item(static_cast<int>(field_4_),
              static_cast<int>((1u << (field_4_ & 31u)) & selected_position_),
              0);

    switch (key) {
    case 0x20: {
        const uint32_t mask = 1u << (field_4_ & 31u);
        if ((mask & selected_position_) != 0) {
            selected_position_ &= ~mask;
        } else {
            selected_position_ |= mask;
        }
        break;
    }
    case 0x25:
    case 0x64: {
        int position = static_cast<int>(field_4_) +
                       static_cast<int>(Dialog::field_58_);
        field_4_ = static_cast<uint32_t>(position);
        if (position >= Dialog::item_list_.entry_count_) {
            position -= static_cast<int>(Dialog::field_58_);
            field_4_ = static_cast<uint32_t>(position);
        }
        break;
    }
    case 0x26:
    case 0x68: {
        int position = static_cast<int>(field_4_) - 1;
        if (position <= 0) {
            position = 0;
        }
        field_4_ = static_cast<uint32_t>(position);
        break;
    }
    case 0x27:
    case 0x66: {
        int position = static_cast<int>(field_4_) -
                       static_cast<int>(Dialog::field_58_);
        field_4_ = static_cast<uint32_t>(position);
        if (position < 0) {
            position += static_cast<int>(Dialog::field_58_);
            field_4_ = static_cast<uint32_t>(position);
        }
        break;
    }
    case 0x28:
    case 0x62: {
        int position = static_cast<int>(field_4_) + 1;
        const int last = Dialog::item_list_.entry_count_ - 1;
        if (position >= last) {
            position = last;
        }
        field_4_ = static_cast<uint32_t>(position);
        break;
    }
    default:
        return 0;
    }

    WinNotificationCallback callback = key_notification_callback_;
    if (callback != 0) {
        callback(static_cast<int>(selected_position_));
    }
    draw_item(static_cast<int>(field_4_),
              static_cast<int>((1u << (field_4_ & 31u)) & selected_position_),
              1);
    soft_update();
    return 1;
}

/*
Purpose: A check box mouse move: locate the row under the cursor from the
         Dialog's column geometry, and park that row's ordinal in the hover
         field unless the row is selected, then repaint.

// ORIGINAL: 0x0060FBB0 ?on_mouse_move@CheckBox@@QAEXHH@Z 0x0060FBB0-0x0060FC2B
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x0060FBB0 retains name and all original spans/claims", "src/jackal/checkbox.h declares the actual CheckBox::on_mouse_move class member; body uses that existing owner API", "historical source was FILE-mode in git:e9190930285ebdae9121f3ac00645c634df1efea:src/leaf_recoveries.cpp; this transfer places the unchanged C++ member definition in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "the complete source body and existing TRIED/LEVER/DEFERRED history are carried forward unchanged; only annotation mode/location and transitional SCAFFOLD classification change"], "reason": "Transfer the complete CheckBox::on_mouse_move definition to its existing source owner without changing its C++ body. Keep the original FILE spans and historical claim state; BODY is the new per-member measurement scope, not a claim that the old whole-TU recipe is equivalent."}
// symbol    ?on_mouse_move@CheckBox@@UAEXHH@Z
// size      123 bytes
// prototype void (__thiscall ?on_mouse_move@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0060FC25
// TRIED (forwarder): the entry convention and field walks are the sibling
//         on_key_down's - the seed's outer/base pointers were the runtime
//         vbtable walk the compiler now owns; the hover field is Dialog's
//         field_F0_ and the geometry fields are field_44_/field_58_/
//         field_64_/field_68_, the same quartet draw_item places rows with.
Status: Source-owned member body; behavior retained, exactness not measured
*/
void CheckBox::on_mouse_move(int x_coord, int y_coord) {
    const int cell_width = static_cast<int>(Dialog::field_64_ +
                                            Dialog::field_44_);
    const int cell_height = static_cast<int>(Dialog::field_68_ +
                                             Dialog::field_44_);
    // Native60FBD3..60FBE7 divides each coordinate first, then multiplies
    // the x quotient by the row count. IMUL/ADD retain 32-bit wrapping.
    const uint32_t position = static_cast<uint32_t>(x_coord / cell_width)
                              * Dialog::field_58_
                              + static_cast<uint32_t>(y_coord / cell_height);
    if (((1u << (position & 31u)) & selected_position_) != 0) {
        // Native60FC00 skips redraw when the selected hover is already clear.
        if (Dialog::field_F0_ == 0xFFFFFFFFu) return;
        Dialog::field_F0_ = 0xFFFFFFFFu;
    } else {
        Dialog::field_F0_ = static_cast<uint32_t>(position);
    }
    redraw();
}

// ORIGINAL: 0x0060FE80 ?on_key_down@thunk1_CheckBox@@QAEHH@Z 0x0060FE80-0x0060FE88 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: CheckBox constructor 0x0060E670 installs Win view 0x006705B0; slot +0x68 points to original identity 0x0060FE80", "historical retained build/recovery/builds/build-rosw1myy: build/recovery/builds/build-rosw1myy/CMakeFiles/OpenSMACX.dir/src/checkbox.cpp.obj has matching source-input closure and physical table relocation to ?on_key_down@CheckBox@@$4PPPPPPPM@A@AEHH@Z", "retained COFF ?on_key_down@CheckBox@@$4PPPPPPPM@A@AEHH@Z has the 8-byte vtordisp subtraction shape and tail relocation to real method symbol ?on_key_down@CheckBox@@UAEHH@Z", "the exact legacy kind fact `game` is copied verbatim from the source envelope and parser-read back; it is existing catalogue classification, not a newly granted kind", "this evidence establishes a current owner-TU/compiler-thunk correspondence only; no fresh build, linked retained-table selection, body-byte match, or whole-game acceptance is claimed"], "reason": "The CheckBox Win-view constructor-installed table selects this original receiver adjustor at byte offset 0x68; the retained owner COFF physically emits the corresponding compiler vtordisp thunk and its relocation tails to CheckBox::on_key_down. This FILE marker remains with the actual class owner TU; it does not prove final linked table selection or full acceptance."}
// kind      game
// symbol    ?on_key_down@CheckBox@@$4PPPPPPPM@A@AEHH@Z
