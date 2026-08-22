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
#include "filewin.h"
#include "text.h"
#include "dialog.h"
#include "basebutton.h"

static const uint32_t EditBoxPrimaryVtable = 0x0066A038;
static const uint32_t EditBoxBufferVtable = 0x0066A030;

// The default file-name suffix appended to field_531_ at construction.
static char *const FileWinTextTableDefault = (char *)0x00697118;

/*
Purpose: Compose a FileWin from four member subobjects (two FlatButtons, an
         EditBox, a most-derived ListBox), then clear the window's own
         name-buffer/selection fields and append one text-table entry.
// ORIGINAL: 0x00613850 ??0FileWin@@QAE@XZ 0x00613850-0x006138F9;0x006630E0-0x00663114
// size      221 bytes
// prototype void (__thiscall ??0FileWin@@QAE@XZ)(FileWin* this)
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00607CF0 0x00609DB0 0x00614E50 0x00645470
// TRIED: MEASURED 8/42 agreeing - the SEH prologue itself matches
//            (`push -1/handler; mov eax,fs:[0]; push eax; mov fs:[0],esp`,
//            instructions 0-6 all agree), so this is NOT the CheckBox/
//            EditGroup family's esp-vs-ebp divergence. The first mismatch is
//            register allocation for the four member pointers (`push
//            esi/mov esi,ecx` vs this body's extra `push ebp`), which
//            plain member declarations (relying on FlatButton's own real
//            ctor for implicit construction) did not converge on.
Return Value: n/a
Status: Complete
*/
FileWin::FileWin() {
    // flat_button2_ and flat_button1_ construct through the ordinary
    // implicit member sequence: FlatButton::FlatButton() is a real
    // recovered body (0x00607CF0) and ~FlatButton() stays trivial, so
    // declaring the members is enough to reproduce the image's two calls
    // with no null-pointer guard and no unwanted automatic-destructor
    // side effect either way.
    //
    // edit_box_ stays a `construct()` method call rather than a real
    // constructor, even though ??0EditBox@@QAE@XZ could legally be spelled
    // that way: EditBox's OWN destructor is real (0x00408010, needed as-is
    // by deleting_thunks.cpp's already-BYTE_EXACT scalar_delete_edit_box),
    // so a real edit_box_ member would get an automatic ~EditBox() call
    // here that the image's own destructor never makes. See the note in
    // editbox.h.
    //
    // list_box_'s own default constructor stays a trivial stub - it needs
    // the most-derived flag ListBox cannot get VC6 to synthesise (see the
    // note on CheckBox's constructor) - so its real body is reached
    // explicitly below.
    edit_box_.construct();
    list_box_.construct(1);

    field_208_ = 0;
    field_30C_ = 0;
    field_410_ = 0;
    field_514_ = 0;
    field_531_ = 0;
    strcat(reinterpret_cast<char *>(&field_531_), FileWinTextTableDefault);
    field_63C_ = 0;
    field_33C0_ = 0;
}

/*
Purpose: Tear down a FileWin: close it, then destroy the ListBox, EditBox and
         two FlatButton member subobjects in reverse declaration order,
         restaging each one's vtable/Buffer-vtable pair and its GraphicWin
         base by hand rather than through that member's own destructor.
// ORIGINAL: 0x00407F00 ??1FileWin@@QAE@XZ 0x00407F00-0x00408003;0x00650C80-0x00650CE8
// size      363 bytes
// prototype void (__thiscall ??1FileWin@@QAE@XZ)(FileWin* this)
// callers   2   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x00613900 0x00614F30 0x00616200
// TRIED: MEASURED 14/62 agreeing with named members - better than the
//            preserved artifact's raw-offset form (src/recovered/units/
//            00407f00.cpp, since deleted, reported divergence 1 against a
//            DIFFERENT flag set/scaffold). First mismatch is again register
//            allocation (`sub esp,8` vs `,0xc`; `push esi` vs `push ebx`)
//            starting right after the SEH prologue, which matches through
//            instruction 6.
Return Value: n/a
Status: Complete
*/
FileWin::~FileWin() {
    close();

    list_box_.destroy();
    reinterpret_cast<Dialog *>(reinterpret_cast<char *>(&list_box_) + 0xA60)->destroy();
    graphic_win_destructor_redirect(reinterpret_cast<GraphicWin *>(&list_box_), nullptr);

    reinterpret_cast<uint32_t *>(&edit_box_)[0x000 / 4] = EditBoxPrimaryVtable;
    reinterpret_cast<uint32_t *>(&edit_box_)[0x444 / 4] = EditBoxBufferVtable;
    edit_box_.close();
    edit_box_.time_.~Time();
    graphic_win_destructor_redirect(reinterpret_cast<GraphicWin *>(&edit_box_), nullptr);

    reinterpret_cast<uint32_t *>(&flat_button1_)[0x000 / 4] = FlatButtonPrimaryVtable;
    reinterpret_cast<uint32_t *>(&flat_button1_)[0x444 / 4] = FlatButtonBufferVtable;
    flat_button1_.close();
    flat_button1_.BaseButton::destroy();

    reinterpret_cast<uint32_t *>(&flat_button2_)[0x000 / 4] = FlatButtonPrimaryVtable;
    reinterpret_cast<uint32_t *>(&flat_button2_)[0x444 / 4] = FlatButtonBufferVtable;
    flat_button2_.close();
    flat_button2_.BaseButton::destroy();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00614B80 ?UNK6@FileWin@@QAEXXZ 0x00614B80-0x00614B81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK6() {
}

void __fastcall file_win_unk6_redirect(FileWin *self, void *) {
    self->UNK6();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00614350 ?UNK2@FileWin@@QAEXH@Z 0x00614350-0x00614353 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK2@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK2(int) {
}

void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1) {
    self->UNK2(a1);
}

/*
Purpose: Clear the file window's three name buffers, its selection index and
         one trailing flag.
// ORIGINAL: 0x00614320 ?UNK1@FileWin@@QAEXXZ 0x00614320-0x00614341 BYTE_EXACT
// size      33 bytes
// prototype void (__thiscall ?UNK1@FileWin@@QAEXXZ)(FileWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FileWin::UNK1() {
    field_208_ = 0;
    field_30C_ = 0;
    field_410_ = 0;
    field_531_ = 0;
    field_514_ = 0;
}

void __fastcall file_win_unk1_redirect(FileWin *self, void *) {
    self->UNK1();
}

/*
// ORIGINAL: 0x00614C40 ?on_double_clicked@FileWin@@QAEXH@Z 0x00614C40-0x00614C48 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?on_double_clicked@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006146A0
Status: Complete
*/
void FileWin::on_double_clicked(int a1) {
    UNK4();
}



/*
// ORIGINAL: 0x00614D90 ?init_class@FileWin@@QAAHXZ 0x00614D90-0x00614E2E BYTE_EXACT
// symbol    ?init_class@FileWin@@SAHXZ
// size      158 bytes
// prototype
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005FD550 0x005FD570 0x00644EF2 0x006453E0 0x00645470
//
// Promoted 2026-08-15 from src/unrecovered/00614d90.cpp, where it measured
// BYTE_EXACT as a self-contained FILE unit, to retire its pending_bodies
// forwarder. The FILE unit forced the strlen/strcat calls through `_strlen`/
// `_strcat` externs - one underscore too deep to LINK, a debt FILE-mode
// measurement never sees. The original plainly called the CRT, so this does
// too; the measurement scaffold supplies the `#pragma function` that keeps
// the calls real where the image has real calls (string_routine_pragma).
Status: Complete
*/
static int *const g_006971cc = (int *)0x006971CC;
static int *const g_006971d4 = (int *)0x006971D4;
static int *const g_009b90a8 = (int *)0x009B90A8;

int __cdecl FileWin::init_class() {
    if (*g_009b90a8 != 0) {
        free(reinterpret_cast<void *>(*g_009b90a8));
        *g_009b90a8 = 0;
    }
    if (text_open(reinterpret_cast<char *>(g_006971d4), reinterpret_cast<char *>(g_006971cc)) != 0) {
        return 6;
    }
    char *str = text_get();
    if (str == 0) {
        return 1;
    }
    if (*g_009b90a8 != 0) {
        free(reinterpret_cast<void *>(*g_009b90a8));
        *g_009b90a8 = 0;
    }
    *g_009b90a8 = reinterpret_cast<int>(mem_get(strlen(str) + 1));
    if (*g_009b90a8 == 0) {
        return 4;
    }
    *reinterpret_cast<char *>(*g_009b90a8) = 0;
    strcat(reinterpret_cast<char *>(*g_009b90a8), str);
    return 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         NOT IN listbox.cpp, which defines no ListBox constructor, so
         VC6 emits no vtable and no thunk there. MSVC emits both in
         every TU that CONSTRUCTS the object; filewin.cpp is one, and
         the COMDATs it emits are what the linker folds into these bytes.
// ORIGINAL: 0x0060D000 ?on_dialog_focus@thunk1_ListBox@@QAEXH@Z 0x0060D000-0x0060D008 BYTE_EXACT
// symbol    ?on_dialog_focus@ListBox@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         NOT IN listbox.cpp, which defines no ListBox constructor, so
         VC6 emits no vtable and no thunk there. MSVC emits both in
         every TU that CONSTRUCTS the object; filewin.cpp is one, and
         the COMDATs it emits are what the linker folds into these bytes.
// ORIGINAL: 0x0060CF80 ?on_mouse_leave@thunk1_ListBox@@QAEXHH@Z 0x0060CF80-0x0060CF88 BYTE_EXACT
// symbol    ?on_mouse_leave@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         NOT IN listbox.cpp, which defines no ListBox constructor, so
         VC6 emits no vtable and no thunk there. MSVC emits both in
         every TU that CONSTRUCTS the object; filewin.cpp is one, and
         the COMDATs it emits are what the linker folds into these bytes.
// ORIGINAL: 0x0060D040 ?attach@thunk1_ListBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060D040-0x0060D048 BYTE_EXACT
// symbol    ?attach@ListBox@@$4PPPPPPPM@A@AEHPAXHHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/
