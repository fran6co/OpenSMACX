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
// ORIGINAL: 0x0060D060 sub_60d060 0x0060D060-0x0060D076 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60d060..0x60d076; CRT slot 0x682694 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_RADIOBUTTON_SPRITE_1@@3U?$RadioButtonSpriteLifetime@$00@@A, guard ?RadioButtonSpritesCleanupGuard@@3EA, startup _$E21 and cleanup _$E19. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original init to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E21
// ORIGINAL: 0x0060D080 sub_60d080 0x0060D080-0x0060D09F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60d080..0x60d09f; CRT slot 0x682694 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_RADIOBUTTON_SPRITE_1@@3U?$RadioButtonSpriteLifetime@$00@@A, guard ?RadioButtonSpritesCleanupGuard@@3EA, startup _$E21 and cleanup _$E19. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original fini to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E19
// ORIGINAL: 0x0060D0A0 sub_60d0a0 0x0060D0A0-0x0060D0B6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60d0a0..0x60d0b6; CRT slot 0x682698 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_RADIOBUTTON_SPRITE_2@@3U?$RadioButtonSpriteLifetime@$01@@A, guard ?RadioButtonSpritesCleanupGuard@@3EA, startup _$E26 and cleanup _$E24. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original init to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E26
// ORIGINAL: 0x0060D0C0 sub_60d0c0 0x0060D0C0-0x0060D0DF FILE
// RECOVERY_BINDING: v1 {"evidence": ["Complete pinned exclusive span 0x60d0c0..0x60d0df; CRT slot 0x682698 selects startup and its registered cleanup. The corresponding original receiver is separately bound to one offset-zero placed Sprite, with no inferred enclosing allocation.", "Original cleanup reads its independently bound shared byte, tests/sets its own bit BEFORE direct Sprite::close5E3820. Native physical COFF identifies current object ?g_RADIOBUTTON_SPRITE_2@@3U?$RadioButtonSpriteLifetime@$01@@A, guard ?RadioButtonSpritesCleanupGuard@@3EA, startup _$E26 and cleanup _$E24. Actual Sprite destruction is solely close; explicit qualified destruction ends the placed lifetime without duplicate implicit cleanup.", "Chosen lifetime representation, not a recovered original class name. Existing runtime close order and static default-pointer cells remain distinct from guarded registered shutdown. Generated startup can be split among compiler helpers; closure semantics do not establish original startup/unwind bytes, complete cross-TU ordering, retained selection or runtime/game acceptance. Binding banks no claim."], "reason": "Bind original fini to the genuine source-owned guarded Sprite global lifetime."}
// symbol    _$E24
#include "stdafx.h"
#include "original_seam.h"
#include "radiobutton.h"
#include "win.h"  // WinCallbackWindow


// The constructor loads these rebindable pointer cells, not sprite contents.
// init_class passes their original targets to Sprite::extract.
// RECOVERY_OBJECT: v2 0x006970F0 0x006970F4 ?RadioButtonDefault1@@3PAVSprite@@A 0x0060D0E0
Sprite *RadioButtonDefault1 =
    reinterpret_cast<Sprite *>(g_RADIOBUTTON_SPRITE_1.storage_.bytes);
// RECOVERY_OBJECT: v2 0x006970F4 0x006970F8 ?RadioButtonDefault2@@3PAVSprite@@A 0x0060D0E0
Sprite *RadioButtonDefault2 =
    reinterpret_cast<Sprite *>(g_RADIOBUTTON_SPRITE_2.storage_.bytes);

/*
Purpose: Construct the virtual bases through VC6's hidden most-derived flag,
         clear this control's state, and install its default sprite pointers.
// ORIGINAL: 0x0060D0E0 ??0RadioButton@@QAE@H@Z 0x0060D0E0-0x0060D1A2;0x00662F20-0x00662F44 BYTE_EXACT
// symbol    ??0RadioButton@@QAE@XZ
// size      230 bytes
// prototype void (__thiscall ??0RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
Return Value: n/a
Status: Complete
*/

/*
 * RECOVERY_HISTORY: v1 address=0x00407030 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=127bea19bd51d4ffaa256021e528250fd06478161070b5cf17c94a0e518b0262
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
 * [ARCHIVE]          RadioButton::on_key_down expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407030 ?on_key_down@thunk1_RadioButton@@QAEHH@Z 0x00407030-0x00407038
 * [ARCHIVE] // symbol    ?adjust_radio_button1_on_key_down@@YIHPAX0H@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype int (__thiscall ?on_key_down@thunk1_RadioButton@@QAEHH@Z)(RadioButton* this, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407030 ?on_key_down@thunk1_RadioButton@@QAEHH@Z 0x00407030-0x00407038 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x00407030 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_key_down@RadioButton@@$4PPPPPPPM@A@AEHH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_key_down@RadioButton@@$4PPPPPPPM@A@AEHH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x004070A0 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=a916e3f64f8676db3c9c3fb87acda341ac5bc95a463b2c4f7c26778110b7d2b3
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from RadioButton's thunk3 subobject to the one
 * [ARCHIVE]          RadioButton::on_redraw expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004070A0 ?on_redraw@thunk3_RadioButton@@QAEXXZ 0x004070A0-0x004070AE BYTE_EXACT
 * [ARCHIVE] // symbol    ?adjust_radio_button3_on_redraw@@YIXPAX0@Z
 * [ARCHIVE] // size      14 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_redraw@thunk3_RadioButton@@QAEXXZ)(RadioButton* this)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] // TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the receiver fold is now
 * [ARCHIVE] //            +0x18 (GraphicWin), so the walk keeps only the Dialog offset 0xA30; the
 * [ARCHIVE] //            net adjustment is the image's `sub ecx, 0xa18` (the family-wide
 * [ARCHIVE] //            Dialog-to-GraphicWin gap 0xA30 - 0x18). The pre-rename spelling also
 * [ARCHIVE] //            pre-subtracted 2584 and emitted `sub ecx, 0x1430` after the rename.
 * [ARCHIVE] //            The compiler now emits its own two-step thunk for this slot; this
 * [ARCHIVE] //            handwritten walk stays the claim host pending coordinated retirement.
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x004070A0 ?on_redraw@thunk3_RadioButton@@QAEXXZ 0x004070A0-0x004070AE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x004070A0 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_redraw@RadioButton@@$4PPPPPPPM@KBI@AEXXZ in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_redraw@RadioButton@@$4PPPPPPPM@KBI@AEXXZ
// kind      game
RadioButton::RadioButton() {
    // VC6 owns virtual-base construction and derived table setup under its
    // hidden most-derived flag. The body initializes this class's own state.

    field_C_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = RadioButtonDefault2;
    unchecked_sprite_ = RadioButtonDefault1;
}

// Genuine destructor phase; virtual-base table staging belongs to VC6.
/*
// ORIGINAL: 0x00406A90 sub_406a90 0x00406A90-0x00406AE3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:360abf0bcf225dda7b384e46e5c26fb7bd2a3bda:src/recovered/units/00406a90.cpp", "pinned:terranx_original:0x406a90", "source:src/jackal/radiobutton.cpp"], "reason": "Original stages RadioButton tables and vtordisps through the+18 virtual-base receiver then calls close. Real out-of-line destructor supplies this non-deleting phase; separate original406F60 owns full scalar deletion and virtual-base destruction. Constructor table correspondence identifies the class, not proximity alone. Preserve all original spans, names, claims and seed discoveries; no new byte grant."}
// symbol    ??1RadioButton@@UAE@XZ
*/
// Historical seed findings: explicit vbtable and vtordisp staging reproduced
// an83-byte fixed-address body; natural hierarchy retains the restoration obligation.
RadioButton::~RadioButton() {
    close();
}

/*
Purpose: Compiler-generated scalar deleting destructor for the out-of-line
         ~RadioButton. Entered on GraphicWin at +0x18, it restores the
         derived tables, closes the control, destroys Dialog and GraphicWin,
         and conditionally deletes the complete object. The catalogue's int
         is the ABI deleting flag, not an explicit C++ destructor parameter.
// ORIGINAL: 0x00406F60 ??1RadioButton@@QAE@H@Z 0x00406F60-0x00406FE2 FILE BYTE_EXACT
// symbol    ??_GRadioButton@@UAEPAXI@Z
// Historical manual-teardown experiment, superseded by real destruction:
// TRIED: MEASURED 3/39 agreeing, first divergence at instruction 2
//            (matches through `push ebp; mov ebp,esp`, then this body loads
//            the vbtable pointer a step earlier than the image's `push
//            esi/edi` pair). No SEH family issue here - the image's own
//            "frame" flag (not an EH one) is an ordinary ebp frame, which
//            this body reproduces the opening of; the mismatch is plain
//            register-allocation order in the vtable-store block.
// size      130 bytes
// prototype void (__thiscall ??1RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x0060D1B0 0x0064557F
Return Value: the complete RadioButton pointer, returned by generated code.
*/

/*
Purpose: Compiler-generated deleting adjustor through RadioButton virtual
         inheritance; tail-jumps to the scalar deleting destructor at 0x00406F60.
// ORIGINAL: 0x00406F50 ??1thunk1_RadioButton@@QAE@H@Z 0x00406F50-0x00406F58 FILE BYTE_EXACT
// symbol    ??_ERadioButton@@$4PPPPPPPM@A@AEPAXI@Z
// size      8 bytes
// prototype void (__thiscall ??1thunk1_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Compiler-generated deleting adjustor through RadioButton virtual
         inheritance; tail-jumps to the scalar deleting destructor at 0x00406F60.
// ORIGINAL: 0x00407060 ??1thunk2_RadioButton@@QAE@H@Z 0x00407060-0x00407071 FILE BYTE_EXACT
// symbol    ??_ERadioButton@@$4PPPPPLLI@EEE@AEPAXI@Z
// size      17 bytes
// prototype void (__thiscall ??1thunk2_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Compiler-generated deleting adjustor through RadioButton virtual
         inheritance; tail-jumps to the scalar deleting destructor at 0x00406F60.
// ORIGINAL: 0x00407080 ??1thunk3_RadioButton@@QAE@H@Z 0x00407080-0x0040708E FILE BYTE_EXACT
// symbol    ??_ERadioButton@@$4PPPPPPPM@KBI@AEPAXI@Z
// size      14 bytes
// prototype void (__thiscall ??1thunk3_RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Reset the radio button to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
// ORIGINAL: 0x0060D1B0 ?close@RadioButton@@QAEIXZ 0x0060D1B0-0x0060D1E6 BYTE_EXACT
// symbol    ?close@RadioButton@@QAEIXZ
// TRIED: the reconstructed signature `?close@CheckBox@@QAEXXZ`. The census
//   names are RECONSTRUCTIONS - the image is stripped - and this one had the
//   same method carrying two signatures across one hierarchy: Dialog::close and
//   four derived close() returning void, GraphicWin::close and ListBox::close
//   returning uint32_t. The original cannot have had that. Unified on
//   `uint32_t` (the form two bodies were already tuned to, for the EAX
//   residue) and on `virtual`, which is what the vtable slot these classes
//   install by hand actually is - hence `UAEIXZ`.
// LEVER: reread-vbtable the image reads `[esi]` TWICE, once per base call (0x0060D1B5 and 0x0060D1D8). Holding it in a `const int32_t *const vbtable` local made VC6 keep it live across the first call, which costs a callee-saved edi and a push/pop pair the image never emits: 5 of 19 with the cache, BYTE_EXACT 19/19 reading it inline at both uses.
// size      54 bytes
// prototype uint32_t (__thiscall ?close@RadioButton@@QAEIXZ)(RadioButton* this)
// callers   21   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
Return Value: n/a
Status: Complete
*/
uint32_t RadioButton::close() {
    // The base calls go through the runtime vbtable either way - the
    // compiler's own dispatch for a virtual-base member call reads it at run
    // time exactly as the old hand walk did, so the embedded-in-Dialogs case
    // (whose vbtable names different offsets) still reaches the right
    // subobjects.
    field_C_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = RadioButtonDefault2;
    unchecked_sprite_ = RadioButtonDefault1;
    Dialog::close();
    return GraphicWin::close();
}

// Seed identity: ?draw_item@RadioButton@@QAEXHH@Z at 0x0060D750.
// The state argument selects the checked/unchecked sprite and also controls
// the focus rectangle, as shown by the original post-draw branch.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: full register-level transliteration (RD macro + eax/ebx/ecx/edx/edi/ebp locals, goto for shared tails); compiles and matches control flow through the vbtable-offset chases, StringStruct/Dialog/Buffer/Sprite/Win member calls, but MISMATCH #5 (prologue register order) not chased further.
// ORIGINAL: 0x0060D750 ?draw_item@RadioButton@@QAEXHH@Z 0x0060D750-0x0060DE08
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060d750.cpp:1 maps ?draw_item@RadioButton@@QAEXHH@Z to 0x0060D750–0x0060DE08. The supplied record identifies it as an existing scoped legacy_seed.", "src/jackal/radiobutton.cpp:159–280 contains the unannotated RadioButton::draw_item(int position, int state) definition using actual GraphicWin/Dialog bases, item-list members and Buffer/Sprite calls. Lines 237–242 require both sprite pointers and select checked_sprite_ for nonzero state, unchecked_sprite_ otherwise.", "Fresh original-image SHA-256 matches pinned 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf. Original 0x0060DAE8 and 0x0060DAF3 load this+8 and this+4; 0x0060DAFE reads [esp+0x3C], the second explicit argument after 0x24 locals and four saved registers. The nonzero branch draws this+8 and the zero branch draws this+4, consistent with src/jackal/radiobutton.h:89–92.", "Fresh source SHA-256 and definition line were obtained read-only. RadioButton is included in this finding and task scope, and src/jackal/radiobutton.cpp is editable under the supplied authorization. Native binding publication, discovery reassessment and compiler verification remain outstanding; no code is accepted."], "reason": "Bind the existing scoped RadioButton seed to its unannotated product definition under explicit seed_bindings delegation. The signature and state-dependent Sprite member selection have original evidence. Preserve seed spans/history, task baseline and pending discovery; binding does not establish correctness or acceptance."}
void RadioButton::draw_item(int position, int state) {
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

// 0x0060D380. The dialog list cursor is intentionally advanced in place, as
// in the original: each entry caches the number of wrapped text lines in its
// payload while the row/column dimensions are calculated.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: full Ghidra-derived arithmetic transliteration (vtbl[1]/vtbl[2] base offsets, Buffer receiver fixed up) compiles and matches logic; MISMATCH #1 is prologue register-save order, not chased.
// ORIGINAL: 0x0060D380 ?calculate_dimensions@RadioButton@@QAEHXZ 0x0060D380-0x0060D745 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060d380.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60d380 complete control/data-access review and src/jackal/radiobutton.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
int RadioButton::calculate_dimensions() {
    Dialog &dialog = *this;
    GraphicWin &graphic = *this;
    Buffer *const buffer = static_cast<Buffer *>(&graphic);
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

    dialog.field_58_ = dialog.field_50_;
    if (dialog.field_58_ > dialog.item_list_.entry_count_) {
        dialog.field_58_ = dialog.item_list_.entry_count_;
    }
    if (dialog.field_58_ < dialog.item_list_.entry_count_) {
        const int columns = dialog.item_list_.entry_count_ / dialog.field_58_ + 1;
        dialog.field_4C_ = columns;
        dialog.field_58_ = dialog.item_list_.entry_count_ / columns;
        if (columns * dialog.field_58_ < dialog.item_list_.entry_count_) {
            ++dialog.field_58_;
        }
    }

    int max_text_width = 0;
    int max_wrap_width = 100000;
    if ((dialog.field_20_ & 2) != 0 && dialog.field_2C_ != 0) {
        max_wrap_width = (dialog.field_2C_ / dialog.field_4C_
                          - dialog.field_40_ * 2) - dialog.field_34_;
    }
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
        if (entry != 0 && entry->string_payload() != 0) {
            entry->string_payload()->field_8_ = width / max_wrap_width + 1;
        }
    }
    dialog.field_64_ = dialog.field_44_ + dialog.field_34_ + max_text_width;
    const int text_height = buffer->text_height();
    if (dialog.field_68_ <= text_height) {
        dialog.field_68_ = buffer->text_height();
    }

    if ((dialog.field_20_ & 2) == 0 || dialog.field_2C_ == 0) {
        int width = dialog.field_64_ * dialog.field_4C_ + dialog.field_40_ * 2;
        const int minimum = dialog.field_2C_ + dialog.field_40_ * 2;
        if (width < minimum) width = minimum;
        dialog.field_5C_ = width;
        dialog.field_64_ = (width - dialog.field_40_ * 2) / dialog.field_4C_
                           - dialog.field_44_;
        dialog.field_5C_ -= dialog.field_44_;
    } else {
        dialog.field_5C_ = dialog.field_2C_;
        dialog.field_64_ = (dialog.field_2C_ - dialog.field_40_ * 2)
                           / dialog.field_4C_ - dialog.field_44_;
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
// TRIED: none confirmed byte-exact; MISMATCH #26 'push' vs 'mov', same vtable-array-double-read shape as the sibling CheckBox::attach (0x0060E800). GraphicWin::init's scaffold return type is void (the mangled name spells X), so the caller cannot propagate its failure code the way the CheckBox::attach byte pattern implies it should - RadioButton::attach DOES check calculate_dimensions()'s return here, unlike CheckBox's.
// ORIGINAL: 0x0060D1F0 ?attach@RadioButton@@QAEHPAUGraphicWin@@HHH@Z 0x0060D1F0-0x0060D377 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060d1f0.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60d1f0 complete control/data-access review and src/jackal/radiobutton.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?attach@RadioButton@@UAEHPAVGraphicWin@@HHH@Z
int RadioButton::attach(GraphicWin *parent, int x, int y, int flags) {
    if (parent == 0) return 3;
    if (HandleMain == 0) return 7;
    const int dimensions = calculate_dimensions();
    if (dimensions != 0) return dimensions;
    Dialog::field_24_ = x;
    Dialog::field_28_ = y;
    if ((parent->iFlags_ & 0x1000) == 0) flags |= 0x100000;
    GraphicWin::init(x, y, Dialog::field_5C_, Dialog::field_60_, 0,
                     flags, parent, 0, 0);
    Buffer *const buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    buffer->set_text_color(text_color_a_, text_color_b_, text_color_c_, text_color_d_);
    buffer->set_text_color2(text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_);
    buffer->set_text_color3(text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_);
    buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: 200 vs 222 bytes; the many named int locals (offsets, args) push an extra callee-saved register before the first field read, where the original moves `this` into edi first. Did not try collapsing to fewer locals given budget.
// `[edi-0x18]` at entry (edi=ecx=`this`) is a virtual-base recovery: this
// class is entered with `this` already adjusted, and `this-0x18` is the
// real object start whose OWN vtable+4 gives a further byte offset used
// to reach members past that. The pattern repeats at vtbl+8 for a second
// virtual base. slot010 needs its own shim (int argument, int return);
// the emitted VCall's slots are void().
// Deleted seed provenance: git:26e5eed1:src/unrecovered/0060e220.cpp
// ORIGINAL: 0x0060E220 ?on_redraw@RadioButton@@QAEXXZ 0x0060E220-0x0060E2FE FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0060e220.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60e220 complete control/data-access review and src/jackal/radiobutton.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_redraw@RadioButton@@UAEXXZ
void RadioButton::on_redraw() {
    WinCallbackWindow = win_parent_;
    if (win_parent_ == 0) return;
    const int handled = win_parent_->on_dialog_back_draw(
        static_cast<GraphicWin *>(this));
    if (handled == 0) {
        static_cast<GraphicWin *>(win_parent_)->copy(
            this, Dialog::field_24_, Dialog::field_28_, 0, 0,
            static_cast<int>(dib_.bmiHeader.biWidth),
            -static_cast<int>(dib_.bmiHeader.biHeight));
    }
    for (int position = 0; position < Dialog::item_list_.entry_count_; ++position) {
        draw_item(position, Dialog::selected_position_ == position ? 1 : 0);
    }
    soft_update();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060DE10 ?on_left_down@RadioButton@@QAEXHH@Z 0x0060DE10-0x0060E01B FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060de10.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60de10 complete control/data-access review and src/jackal/radiobutton.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_left_down@RadioButton@@UAEXHH@Z
void RadioButton::on_left_down(int x, int y) {
    WinCallbackWindow = win_parent_;
    Dialog::item_list_.seek_pos(-1);
    int remaining_y = y;
    int first = 1;
    while (remaining_y > 0) {
        if (!first && Dialog::item_list_.current_position_ ==
                          Dialog::item_list_.entry_count_ - 1) {
            return;
        }
        first = 0;
        Dialog::item_list_.next_entry();
        const int lines = Dialog::item_list_.current_ == 0 ||
                          Dialog::item_list_.current_->payload == 0
            ? 0 : static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_);
        int item_height = Buffer::text_line_height() * lines;
        const int minimum = static_cast<int>(Dialog::field_2C_ + Dialog::field_50_);
        if (minimum > item_height) item_height = minimum;
        remaining_y -= item_height;
    }
    const int position = (x / static_cast<int>(Dialog::field_4C_ + Dialog::field_2C_)) *
                         static_cast<int>(Dialog::field_40_) +
                         Dialog::item_list_.current_position_;

    const int previous = Dialog::selected_position_;
    if (position == previous) {
        if ((Dialog::field_20_ & 4) != 0 && win_parent_ != 0) {
            win_parent_->on_button_clicked(-1);
        }
        return;
    }

    draw_item(previous, 0);
    Dialog::selected_position_ = position;
    if (double_click_callback_ != 0) double_click_callback_(position);
    draw_item(position, 1);
    Dialog::field_F0_ = static_cast<uint32_t>(-1);
    soft_update();
    if ((Dialog::field_20_ & 4) != 0 && win_parent_ != 0) {
        win_parent_->on_button_clicked(-1);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060E320 ?on_mouse_move@RadioButton@@QAEXHH@Z 0x0060E320-0x0060E494 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060e320.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x60e320 complete control/data-access review and src/jackal/radiobutton.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_mouse_move@RadioButton@@UAEXHH@Z
void RadioButton::on_mouse_move(int x, int y) {
    Dialog::item_list_.seek_pos(-1);
    int remaining_y = y;
    int first = 1;
    while (remaining_y > 0) {
        if (!first && Dialog::item_list_.current_position_ ==
                          Dialog::item_list_.entry_count_ - 1) {
            break;
        }
        first = 0;
        Dialog::item_list_.next_entry();
        const int lines = Dialog::item_list_.current_ == 0 ||
                          Dialog::item_list_.current_->payload == 0
            ? 0 : static_cast<int>(Dialog::item_list_.current_->string_payload()->field_8_);
        int item_height = Buffer::text_line_height() * lines;
        const int minimum = static_cast<int>(Dialog::field_2C_ + Dialog::field_50_);
        if (minimum > item_height) item_height = minimum;
        remaining_y -= item_height;
    }
    const int position = (x / static_cast<int>(Dialog::field_4C_ + Dialog::field_2C_)) *
                         static_cast<int>(Dialog::field_40_) +
                         Dialog::item_list_.current_position_;
    if (position == Dialog::selected_position_ && Dialog::field_F0_ ==
        static_cast<uint32_t>(-1)) return;
    if (Dialog::field_F0_ != static_cast<uint32_t>(position)) {
        Dialog::field_F0_ = static_cast<uint32_t>(position);
        redraw();
    }
}


/*
// ORIGINAL: 0x0060E300 ?on_dialog_focus@RadioButton@@QAEXH@Z 0x0060E300-0x0060E317 BYTE_EXACT
// symbol    ?on_dialog_focus@RadioButton@@UAEXH@Z
// size      23 bytes
// prototype void (__thiscall ?on_dialog_focus@RadioButton@@QAEXH@Z)(RadioButton* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060E30E
Status: Complete
*/
void RadioButton::on_dialog_focus(int a1) {
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to RadioButton and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         RadioButton::on_dialog_focus overrides a virtual through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00407050 ?on_dialog_focus@thunk1_RadioButton@@QAEXH@Z 0x00407050-0x00407058 BYTE_EXACT
// symbol    ?on_dialog_focus@RadioButton@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_RadioButton@@QAEXH@Z)(RadioButton* this, int)
// kind      game
Status: Complete
*/

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
// ORIGINAL: 0x0060E4A0 ?on_mouse_leave@RadioButton@@QAEXHH@Z 0x0060E4A0-0x0060E4C8 BYTE_EXACT
// symbol    ?on_mouse_leave@RadioButton@@UAEXHH@Z
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060E4BF
Return Value: n/a
Status: Complete
*/
void RadioButton::on_mouse_leave(int a1, int a2) {
    // Clear the hover index (Dialog::field_F0_, through the virtual base)
    // and repaint (the redraw virtual) - plain member forms; the
    // compiler's virtual-base machinery does the walking the old hand
    // chains spelled. Both parameters are dead; `ret 8` still pops them.
    field_F0_ = -1;
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to RadioButton and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         RadioButton::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00407010 ?on_mouse_leave@thunk1_RadioButton@@QAEXHH@Z 0x00407010-0x00407018 BYTE_EXACT
// symbol    ?on_mouse_leave@RadioButton@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
// ORIGINAL: 0x0060E4D0 ?init_class@RadioButton@@QAAHXZ 0x0060E4D0-0x0060E5C2;0x00662F44-0x00662F59 BYTE_EXACT
// symbol    ?init_class@RadioButton@@SAHXZ
// size      263 bytes
// prototype int (__cdecl ?init_class@RadioButton@@QAAHXZ)()
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005E39A0
//
// Promoted 2026-08-15 from src/unrecovered/0060e4d0.cpp to retire its
// pending_bodies forwarder. Sibling of CheckBox::init_class; same
// placement-new Buffer shape and the same SEH-funclet gap (see that body's
// TRIED).
// LEVER: ~Buffer out of line, as in CheckBox::init_class above - 24/63 ->
//   63/63 on moving the definition from buffer.h into buffer.cpp, and the
//   same collapse of the EBP frame into the image's frameless EH form.
Status: Complete
*/
int __cdecl RadioButton::init_class() {
    // A REAL LOCAL. The note that stood here kept the placement-new form to
    // preserve the image's SEH frame, and that reasoning was right about the
    // frame and wrong about what produces it: a plain `Buffer buf;` is a real
    // local with a non-trivial destructor too, so VC6 emits the same frame -
    // and it also gets `sub esp, 0x588`, sizeof(Buffer) exactly, where the
    // char array costs four more bytes. The CheckBox sibling this note called
    // "the same ceiling" is BYTE_EXACT with this spelling.
    Buffer buf;
    buf.init(0x20, 0x20, 0, 0);
    int result = buf.load_pcx("jackal.pcx", 0, 10, 0xec);
    if (result != 0) {
        return result;
    }
    g_RADIOBUTTON_SPRITE_1.sprite().extract(&buf, 0x109, 1, 0x23, 0x20, 0x20, 0);
    g_RADIOBUTTON_SPRITE_2.sprite().extract(&buf, 0x109, 0x22, 0x23, 0x20, 0x20, 0);
    return 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to RadioButton and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         RadioButton::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00407090 ?attach@thunk1_RadioButton@@QAEHPAUGraphicWin@@HHH@Z 0x00407090-0x00407098 BYTE_EXACT
// symbol    ?attach@RadioButton@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

// ====================
// Original receivers of Sprite::extract at 0x0060E56E / 0x0060E58C.
// RECOVERY_OBJECT: v1 0x009B8EF0 ?RadioButtonSpritesCleanupGuard@@3EA
uint8_t RadioButtonSpritesCleanupGuard; // Original separate zero-fill byte9B8EF0.
// RECOVERY_OBJECT: v3 0x009B8EF8 ?g_RADIOBUTTON_SPRITE_1@@3VSprite@@A member ?g_RADIOBUTTON_SPRITE_1@@3U?$RadioButtonSpriteLifetime@$00@@A storage_.bytes
RadioButtonSpriteLifetime<1> g_RADIOBUTTON_SPRITE_1;
// RECOVERY_OBJECT: v3 0x009B8F28 ?g_RADIOBUTTON_SPRITE_2@@3VSprite@@A member ?g_RADIOBUTTON_SPRITE_2@@3U?$RadioButtonSpriteLifetime@$01@@A storage_.bytes
RadioButtonSpriteLifetime<2> g_RADIOBUTTON_SPRITE_2;

// ===== close_class: once-only teardown of the managed sprites =====

/*
Purpose: ?close_class@RadioButton@@QAAXXZ - run 2 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x0060E5D0 ?close_class@RadioButton@@QAAXXZ 0x0060E5D0-0x0060E5E4 BYTE_EXACT
// symbol    ?teardown_0060e5d0@@YAXXZ
// size      20 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_0060e5d0() {
    g_RADIOBUTTON_SPRITE_2.sprite().close();
    g_RADIOBUTTON_SPRITE_1.sprite().close();
}

// ====================
// Window-event handlers
// ====================

/*
Purpose: A double click selects the parent window as the callback window and
         notifies the stored callback with the group's selected id. The image
         enters through the GraphicWin virtual-base receiver: its -0x18
         vbtable access reaches Win::win_parent_ and the virtual Dialog base;
         its -8 access is RadioButton's own callback at complete offset 0x10.
         The real virtual override lets the compiler supply these adjustments.
// ORIGINAL: 0x0060E1E0 ?on_left_double_click@RadioButton@@QAEXHH@Z 0x0060E1E0-0x0060E217 BYTE_EXACT
// symbol    ?on_left_double_click@RadioButton@@UAEXHH@Z
// size      55 bytes
// prototype void (__thiscall ?on_left_double_click@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   1
// kind      game
// calls     0x00609A50
// indirect  0x0060E20D
//
// The notification is reloaded after get_selected_id, unlike CheckBox's
// cached callback. Both the null test and call use the declared callback
// member; the original virtual-base adjustment is compiler-owned.
Return Value: n/a
Status: Complete
*/
void RadioButton::on_left_double_click(int x_coord, int y_coord) {
    WinCallbackWindow = win_parent_;
    if (double_click_callback_ != 0) {
        double_click_callback_(Dialog::get_selected_id());
    }
}

/*
Purpose: A radio button key press: publish the parent window as the callback
         window, move the selection with the arrows and page keys over the
         item list, drop the hover index, and repaint the previous and the
         new selection.

// ORIGINAL: 0x0060E020 ?on_key_down@RadioButton@@QAEHH@Z 0x0060E020-0x0060E183
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x0060E020 retains name and all original spans/claims", "src/jackal/radiobutton.h declares the actual RadioButton::on_key_down class member; body uses that existing owner API", "historical source was FILE-mode in git:e9190930285ebdae9121f3ac00645c634df1efea:src/leaf_recoveries.cpp; this transfer places the unchanged C++ member definition in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "the complete source body and existing TRIED/LEVER/DEFERRED history are carried forward unchanged; only annotation mode/location and transitional SCAFFOLD classification change"], "reason": "Transfer the complete RadioButton::on_key_down definition to its existing source owner without changing its C++ body. Keep the original FILE spans and historical claim state; BODY is the new per-member measurement scope, not a claim that the old whole-TU recipe is equivalent."}
// symbol    ?on_key_down@RadioButton@@UAEHH@Z
// size      355 bytes
// prototype int (__thiscall ?on_key_down@RadioButton@@QAEHH@Z)(RadioButton* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890 0x0060D750
// indirect  0x0060E145
// TRIED (seed): the entry convention is the CheckBox sibling's (entered on
//         the GraphicWin subobject, runtime vbtable for the Dialog fields);
//         the seed guessed its switch labels from a range check, and the
//         jump tables pinned the real keys: VK_LEFT/NUMPAD4 pages forward by
//         field_58_ columns, VK_UP/NUMPAD8 steps back one, VK_RIGHT/NUMPAD6
//         pages back, VK_DOWN/NUMPAD2 steps forward one.
Status: Source-owned member body; behavior retained, exactness not measured
*/
int RadioButton::on_key_down(int key) {
    WinCallbackWindow = win_parent_;
    const int previous = static_cast<int>(selected_position_);

    switch (key) {
    case 0x25:
    case 0x64: {
        int position = previous + static_cast<int>(Dialog::field_58_);
        selected_position_ = static_cast<uint32_t>(position);
        if (position >= Dialog::item_list_.entry_count_) {
            position -= static_cast<int>(Dialog::field_58_);
            selected_position_ = static_cast<uint32_t>(position);
        }
        break;
    }
    case 0x26:
    case 0x68: {
        int position = previous - 1;
        selected_position_ = static_cast<uint32_t>(position);
        if (position <= 0) {
            selected_position_ = 0;
        }
        break;
    }
    case 0x27:
    case 0x66: {
        int position = previous - static_cast<int>(Dialog::field_58_);
        selected_position_ = static_cast<uint32_t>(position);
        if (position < 0) {
            position += static_cast<int>(Dialog::field_58_);
            selected_position_ = static_cast<uint32_t>(position);
        }
        break;
    }
    case 0x28:
    case 0x62: {
        int position = previous + 1;
        selected_position_ = static_cast<uint32_t>(position);
        const int last = Dialog::item_list_.entry_count_ - 1;
        if (position >= last) {
            selected_position_ = static_cast<uint32_t>(last);
        }
        break;
    }
    default:
        return 0;
    }

    Dialog::field_F0_ = 0xFFFFFFFFu;
    draw_item(previous, 0);
    if (double_click_callback_ != 0) {
        double_click_callback_(static_cast<int>(selected_position_));
    }
    draw_item(static_cast<int>(selected_position_), 1);
    soft_update();
    return 1;
}

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00406FF0 ?on_redraw@thunk1_RadioButton@@QAEXXZ 0x00406FF0-0x00406FF8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0RadioButton@@QAE@H@Z at 0x0060D0E0 at 0x0060D138 installs table 0x00669A6C through the thunk1 vbtable-derived view installs table 0x00669A6C; the pinned slot at table+0x30 contains 0x00406FF0. Original thunk receiver adjustment and tail target were checked against the pinned image: sub ecx,[ecx-4]; jmp 0x0060E220.", "Current genuine owner is src/jackal/radiobutton.cpp; retained owner-object vftable ??_7RadioButton@@6BWin@@@ has an absolute relocation at +0x30 to ?on_redraw@RadioButton@@$4PPPPPPPM@A@AEXXZ, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_redraw@RadioButton@@UAEXXZ method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_redraw@RadioButton@@$4PPPPPPPM@A@AEXXZ
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_RadioButton@@QAEXXZ)(RadioButton* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): with Win slot12 renamed
//            on_redraw, RadioButton::on_redraw overrides that slot through GraphicWin as
//            well as Dialog slot2, so the qualified call's receiver fold moved from
//            the Dialog offset 0xA30 to the GraphicWin offset 0x18. The walk now
//            pre-subtracts 0x18 to cancel it, leaving the image's bare vtordisp thunk;
//            the old 0xA30 walk emitted an extra `sub ecx, 0xa18` here. The compiler
//            now emits its own vtordisp thunk for this slot; this handwritten walk
//            stays the claim host pending coordinated retirement onto it.
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407000 ?on_mouse_move@thunk1_RadioButton@@QAEXHH@Z 0x00407000-0x00407008 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0RadioButton@@QAE@H@Z at 0x0060D0E0 at 0x0060D138 installs table 0x00669A6C through the thunk1 vbtable-derived view installs table 0x00669A6C; the pinned slot at table+0x44 contains 0x00407000. Original thunk receiver adjustment and tail target were checked against the pinned image: sub ecx,[ecx-4]; jmp 0x0060E320.", "Current genuine owner is src/jackal/radiobutton.cpp; retained owner-object vftable ??_7RadioButton@@6BWin@@@ has an absolute relocation at +0x44 to ?on_mouse_move@RadioButton@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_mouse_move@RadioButton@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_mouse_move@RadioButton@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x00407020 ?on_left_down@thunk1_RadioButton@@QAEXHH@Z 0x00407020-0x00407028 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0RadioButton@@QAE@H@Z at 0x0060D0E0 at 0x0060D138 installs table 0x00669A6C through the thunk1 vbtable-derived view installs table 0x00669A6C; the pinned slot at table+0x50 contains 0x00407020. Original thunk receiver adjustment and tail target were checked against the pinned image: sub ecx,[ecx-4]; jmp 0x0060DE10.", "Current genuine owner is src/jackal/radiobutton.cpp; retained owner-object vftable ??_7RadioButton@@6BWin@@@ has an absolute relocation at +0x50 to ?on_left_down@RadioButton@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_left_down@RadioButton@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_left_down@RadioButton@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from RadioButton's thunk1 subobject to the one
         RadioButton::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407040 ?on_left_double_click@thunk1_RadioButton@@QAEXHH@Z 0x00407040-0x00407048 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor ??0RadioButton@@QAE@H@Z at 0x0060D0E0 at 0x0060D138 installs table 0x00669A6C through the thunk1 vbtable-derived view installs table 0x00669A6C; the pinned slot at table+0x70 contains 0x00407040. Original thunk receiver adjustment and tail target were checked against the pinned image: sub ecx,[ecx-4]; jmp 0x0060E1E0.", "Current genuine owner is src/jackal/radiobutton.cpp; retained owner-object vftable ??_7RadioButton@@6BWin@@@ has an absolute relocation at +0x70 to ?on_left_double_click@RadioButton@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_left_double_click@RadioButton@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_left_double_click@RadioButton@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
