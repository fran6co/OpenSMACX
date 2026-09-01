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
#include "radiobutton.h"
#include "vtable_shim.h"
#include "win.h"  // WinCallbackWindow

uint32_t RadioButtonDefault1;  // 0x006970F0
uint32_t RadioButtonDefault2;  // 0x006970F4

/*
Purpose: Compose a RadioButton from its two base-shaped subobjects
         (GraphicWin, Dialog - both constructed only when most-derived),
         install this class's own vtable/vtordisp values into the object's
         vbtable-named slots, and reset the two state fields.

         `a1` is the compiler's most-derived flag in the image
         (??0RadioButton@@QAE@H@Z): nonzero means construct the two bases
         AND set the object's own vbtable pointer, zero means skip both and
         go straight to installing this class's own tables - the same "am I
         the most-derived object" guard ListBox's constructor uses. This
         class cannot let VC6 synthesise that flag (see the class comment in
         radiobutton.h), so it is modelled as an explicit parameter instead.
// ORIGINAL: 0x0060D0E0 ??0RadioButton@@QAE@H@Z 0x0060D0E0-0x0060D1A2;0x00662F20-0x00662F44
// TRIED: MEASURED 3/52 agreeing, same family as CheckBox/EditGroup - no
//            SEH frame at all in this compiled body (`mov eax,0; call $+5`
//            in place of `push -1; push handler`). `dialog_.construct()`
//            is an ORDINARY METHOD CALL, not a placement-new constructor
//            call, so VC6 has no partially-constructed subobject to
//            protect and emits no frame - the same distinction the
//            coordinator's CheckButton::construct() finding names from the
//            other direction. Getting the frame back needs Dialog's OWN
//            constructor to be non-trivial (a real placement-new target)
//            AND GraphicWin's OWN destructor to be non-trivial (what the
//            catch clause would actually unwind to) - both currently kept
//            trivial by design (see the class comment above), and both are
//            real typed members here (dialog_, virtual_base_), so making
//            either non-trivial changes what EVERY CheckBox/EditGroup/
//            RadioButton/SpriteBox object implicitly does at construction,
//            not just this one function. Not attempted: the coupled edit
//            this blocks on is explicitly flagged elsewhere in this file as
//            OPEN and cross-cutting, well beyond one constructor.
// size      230 bytes
// prototype void (__thiscall ??0RadioButton@@QAE@H@Z)(RadioButton* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
Return Value: n/a
Status: Complete
*/
RadioButton::RadioButton(int a1) {
    // THE COMPILER OWNS THE VIRTUAL-BASE SETUP NOW. With
    // `: public virtual GraphicWin, public virtual Dialog` declared, VC6 emits
    // the vbtable pointer store, both vtordisp initialisations, and the unwind
    // that destroys GraphicWin if Dialog's stage throws - all of which this
    // body used to write by hand against a layout composed out of members.
    // What is left is what the compiler cannot do: GraphicWin and Dialog model
    // construction as a `construct()` METHOD rather than a constructor, so the
    // two calls stay, and they are base-qualified rather than member calls.
    // Strip-all direction: the image's five derived-stage stores
    // (0x00669A6C/0x00669A64/0x00669A58 + the two vtordisp refreshes) are
    // gone, exactly as the Dialogs::destroy staging they mirror; the
    // constructor is unclaimed either way.
    if (a1 != 0) {
        new (static_cast<GraphicWin *>(this)) GraphicWin();
        Dialog::construct();
    }

    field_C_ = 0;
    field_10_ = 0;
    field_8_ = RadioButtonDefault2;
    field_4_ = RadioButtonDefault1;
}

/*
Purpose: The complete-object destructor entry, which also conditionally
         frees the allocation: reinstall the base subobjects' own
         vtable/vtordisp values, close the RadioButton, destroy the Dialog
         and GraphicWin subobjects, then free through the executable's
         operator delete only when bit 0 of mode asks. See the note in
         radiobutton.h for why this is a free function rather than a
         destructor.
// ORIGINAL: 0x00406F60 ??1RadioButton@@QAE@H@Z 0x00406F60-0x00406FE2
// symbol    ?radio_button_teardown_redirect@@YIPAVRadioButton@@PAX0I@Z
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
Return Value: the RadioButton pointer (this - 0x18), the original's EAX
Status: Complete
*/
RadioButton *__fastcall radio_button_teardown_redirect(void *adjusted, void *,
                                                        unsigned int mode) {
    char *const self = static_cast<char *>(adjusted) - 0x18;

    // Strip-all direction: the image's five derived-stage re-stores here are
    // gone, as in the constructor.
    reinterpret_cast<RadioButton *>(self)->close();
    reinterpret_cast<Dialog *>(self + 0xA30)->destroy();
    reinterpret_cast<GraphicWin *>(self + 0x18)->~GraphicWin();

    if (mode & 1) {
        operator delete(self);
    }
    return reinterpret_cast<RadioButton *>(self);
}

/*
Purpose: Reset the radio button to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
// ORIGINAL: 0x0060D1B0 ?close@RadioButton@@QAEIXZ 0x0060D1B0-0x0060D1E6 BYTE_EXACT
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
    // The base offsets come from the object's own vbtable, read at run time,
    // not from where they sit in a most-derived RadioButton. When one is
    // embedded in a larger class - Dialogs holds a RadioButton at 0x44 - that
    // object's vbtable names different offsets, and hardcoding this class's
    // own sends both calls to the wrong subobject. Doing exactly that passed
    // every unit test here and crashed the game on a null vtable pointer.
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    field_C_ = 0;
    field_10_ = 0;
    field_8_ = RadioButtonDefault2;
    field_4_ = RadioButtonDefault1;
    reinterpret_cast<Dialog *>(
        self + (*reinterpret_cast<const int32_t *const *>(self))[2])->Dialog::close();
    return reinterpret_cast<GraphicWin *>(
        self + (*reinterpret_cast<const int32_t *const *>(self))[1])->close();
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
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) +
        *reinterpret_cast<int *>(
            *reinterpret_cast<char **>(reinterpret_cast<char *>(this)) + 4)
    )->slot062();
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
    // `this` reaches a vbtable-shaped descriptor with two deltas:
    // entry +8 locates the field, entry +4 the enclosing object's vtable.
    // THE CONSTANT IS 0xF0 NOW - the compiler performs the walk back to the
    // enclosing object as part of the override's entry, so the source states
    // the field's ACTUAL offset there. Note 0x18 + 0xD8 == 0xF0, so the
    // field is at a FIXED +0xF0 in the enclosing object and these classes are
    // subobjects at differing offsets inside it.
    //
    // The arithmetic must go through `char *`. Through `int *` the compiler
    // scales the +8 and emits [eax+0x20].
    //
    // Both parameters are dead; `ret 8` still pops them.
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(this) + 0xF0 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this))) + 8)) = -1;

    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this))) + 4))->slot062();
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
    g_RADIOBUTTON_SPRITE_1.extract(&buf, 0x109, 1, 0x23, 0x20, 0x20, 0);
    g_RADIOBUTTON_SPRITE_2.extract(&buf, 0x109, 0x22, 0x23, 0x20, 0x20, 0);
    return 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to RadioButton and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         RadioButton::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00407090 ?attach@thunk1_RadioButton@@QAEHPAUGraphicWin@@HHH@Z 0x00407090-0x00407098 BYTE_EXACT
// symbol    ?attach@RadioButton@@$4PPPPPPPM@A@AEHPAXHHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

// ====================
Sprite g_RADIOBUTTON_SPRITE_1;
Sprite g_RADIOBUTTON_SPRITE_2;

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
    g_RADIOBUTTON_SPRITE_2.close();
    g_RADIOBUTTON_SPRITE_1.close();
}

// ====================
// Window-event handlers
// ====================

/*
Purpose: A double click on a radio button copies the selected row marker to
         the current-win pointer and notifies the object's stored callback
         with the group's selected id. Entered on the GraphicWin-subobject
         adjusted pointer (the family's thunk convention), so the vbtable
         sits 0x18 below `this` and the callback 8 below; both the marker and
         the selection id are located through that vbtable at run time, since
         this class is used as a subobject of larger windows.
// ORIGINAL: 0x0060E1E0 ?on_left_double_click@RadioButton@@QAEXHH@Z 0x0060E1E0-0x0060E217 BYTE_EXACT
// size      55 bytes
// prototype void (__thiscall ?on_left_double_click@RadioButton@@QAEXHH@Z)(RadioButton* this, int, int)
// callers   1   call targets   1
// kind      game
// calls     0x00609A50
// indirect  0x0060E20D
//
// PROMOTED out of the archived verification unit for this address, whose
// reached the Dialog call through a fabricated two-virtual-base carrier
// class; this file keeps that carrier (RadioCarrier below) because VC6
// computes a virtual-base member receiver straight into ecx, which is where
// the image puts it.
// TRIED: raw vbtable reads through a named `graphicwin_disp` local - the
// store half matched but the Dialog call compiled `lea ecx, [eax + esi -
// 0x18]` where the image has `[ecx + esi - 0x18]` (the displacement local
// landed in eax). 17 of 18. The carrier spelling is 18 of 18.
// LEVER: no cached locals for the callback or the vbtable - the image
// re-reads [this-0x18] before the get_selected_id call and calls [this-8]
// straight out of memory rather than through the register the null check
// used; a cached callback or vbtable compiles 4 bytes smaller and stops
// matching.
Return Value: n/a
Status: Complete
*/
namespace {
// The enclosing-object access the image spells through its own vbtable,
// modelled as the two-virtual-base carrier it reads like: never defined,
// never instantiated, only ever pointed at 0x18 below the handler's entry
// `this` (the shape the archived unit was proved on). VC6
// computes the Dialog receiver straight into ecx and folds the -0x18 into
// the base's own displacement, which is why the pad is 0xc4: 0xc4 - 0x18 is
// the 0xac the image's instruction carries.
class RadioCarrierBase { public:
    char pad[0xc4];
    int marker;
};
class RadioCarrier : public virtual RadioCarrierBase, public virtual Dialog {
};
}  // namespace

void RadioButton::on_left_double_click(int x_coord, int y_coord) {
    typedef void (__cdecl *radio_click_callback)(int);
    char *self = reinterpret_cast<char *>(this);
    WinCallbackWindow = reinterpret_cast<Win *>(
        reinterpret_cast<RadioCarrier *>(self - 0x18)->marker);
    if (*reinterpret_cast<radio_click_callback *>(self - 8) != 0) {
        (*reinterpret_cast<radio_click_callback *>(self - 8))(
            reinterpret_cast<RadioCarrier *>(self - 0x18)
                ->get_selected_id());
    }
}
