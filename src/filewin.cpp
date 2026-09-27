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
#include "sprite.h"
#include "text.h"
#include "dialog.h"
#include "basebutton.h"

// The default file-name suffix appended to field_531_ at construction - REAL
// STORAGE seeded from the image's own .data bytes (image_data.py, 0x00697118:
// 2e 00 00 00 - a "." entry in the dword-slotted text table that continues
// with the second ".", "LISTBOX" and "jackal.pcx" further on). It was
// `static char *const FileWinTextTableDefault = (char *)0x00697118`, pointing
// at the image's mapped-in data; the array carries the bytes instead. strcat
// reads up to the first NUL either way, and the `push offset` folds to the
// same instruction, the displacement relocated.
static const char FileWinTextTableDefault[4] = {'.', 0, 0, 0};  // 0x00697118

// The table's NEXT entry (0x0069711C), also "." in the shipped image:
// close() resets the name buffer to it, exactly as the constructor resets
// the buffer to the first. Same real-storage argument as above.
static const char FileWinTextTableDefaultNext[4] = {'.', 0, 0, 0};  // 0x0069711C

// The dialog's three fixed sprites, 0x30 apart (0x009B9018/0x009B9048/
// 0x009B9078). Their individual roles are not established - the dialog's
// own bring-up loads them and close() reaches Sprite::close (0x005E3820)
// on each, in this order. Named bindings rather than array indexing:
// sizeof(Sprite) is not pinned to the 0x30 stride.
static Sprite *const FileWinFixedSprite0 = reinterpret_cast<Sprite *>(0x009B9018);
static Sprite *const FileWinFixedSprite1 = reinterpret_cast<Sprite *>(0x009B9048);
static Sprite *const FileWinFixedSprite2 = reinterpret_cast<Sprite *>(0x009B9078);

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
    // The declared members construct once, in declaration order. ListBox's
    // most-derived flag is a compiler ABI argument, not a second construction.
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
         two FlatButton members in reverse declaration order. C++ owns the
         member/base destruction and partial-construction unwind.
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
}

/*
Purpose: Close the file dialog: close the two button strips, the edit box
         and the list (each a virtual or member close on its subobject),
         reset the window's own name/selection fields and append the text
         table's second '.' entry, close the dialog's three fixed sprites,
         drop the selection word, and destroy the dialog's own window
         handle if one was created.

// ORIGINAL: 0x00613900 ?close@FileWin@@QAEXXZ 0x00613900-0x006139AC FILE
// size      172 bytes
// prototype void (__thiscall ?close@FileWin@@QAEXXZ)(FileWin* this)
// callers   5   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820 0x00609F20 0x00614F30 0x00645470
// indirect  0x00613910 0x00613922 0x0061399D
// TRIED (seed): DestroyWindowFn via *g_0066930c (established lever from
//         00635750.cpp/00635450.cpp) compiles clean; mnemonic sim 0.95,
//         one pop/pop vs add-esp reorder right after the strcat call (#23)
//         not chased further. The two indirect sites are the FlatButton
//         slot-90 closes (BaseButton's virtual close, basebutton.h); the
//         third is user32!DestroyWindow through its import slot.
Status: Coverage transcription - unclaimed body
*/
void FileWin::close() {
    flat_button2_.close();
    flat_button1_.close();
    edit_box_.close();
    list_box_.close();
    field_208_ = 0;
    field_30C_ = 0;
    field_410_ = 0;
    field_514_ = 0;
    field_531_ = 0;
    strcat(reinterpret_cast<char *>(&field_531_), FileWinTextTableDefaultNext);
    FileWinFixedSprite0->close();
    FileWinFixedSprite1->close();
    FileWinFixedSprite2->close();
    field_63C_ = 0;
    if (field_33C0_ != 0) {
        DestroyWindow(reinterpret_cast<HWND>(field_33C0_));
        field_33C0_ = 0;
    }
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
// 0x006971CC and 0x006971D4 - the image stores both names INLINE in .rdata
// ("FILEWIN", then "jackal"), so the tree stores them inline too. The raw
// fixed-address bindings read unmapped memory in the standalone build, which
// is where the boot's `return 6` out of jackal_init_real came from.
static const char kFileWinSection[] = "FILEWIN";  // 0x006971CC
static const char kFileWinTextName[] = "jackal";  // 0x006971D4
// 0x009B90A8 - the loaded file's text buffer: init_class stages text_get()'s
// string into it through mem_get, and close_class frees it once. THE OBJECT,
// not the `static int *const s_filewin_text_ptr = (int *)0x009B90A8` binding:
// the slot itself is real storage now, a pointer-sized object seeded zero
// like the image's, and every `*s_filewin_text_ptr` read/write through the
// binding becomes a read/write of the object at the same folded addresses.
static char *s_filewin_text_ptr = nullptr;  // 0x009B90A8

int __cdecl FileWin::init_class() {
    if (s_filewin_text_ptr != nullptr) {
        free(s_filewin_text_ptr);
        s_filewin_text_ptr = nullptr;
    }
    if (text_open(kFileWinTextName, kFileWinSection) != 0) {
        return 6;
    }
    char *str = text_get();
    if (str == 0) {
        return 1;
    }
    if (s_filewin_text_ptr != nullptr) {
        free(s_filewin_text_ptr);
        s_filewin_text_ptr = nullptr;
    }
    s_filewin_text_ptr = static_cast<char *>(mem_get(strlen(str) + 1));
    if (s_filewin_text_ptr == nullptr) {
        return 4;
    }
    *s_filewin_text_ptr = 0;
    strcat(s_filewin_text_ptr, str);
    return 0;
}

/*
Purpose: Free the file text the init_class staged, once. Reads no state of
         its own - the object is irrelevant to the body - and is called by
         jackal_close with no receiver set up at all.
// ORIGINAL: 0x00614E30 ?close_class@FileWin@@QAAXXZ 0x00614E30-0x00614E4D BYTE_EXACT
// symbol    ?close_class@FileWin@@QAAXXZ
// size      29 bytes
// callers   1   call targets   1
// kind      game
// calls     0x00644EF2
//
// PROMOTED out of the archived verification unit for this address. The free is the same
// CRT callee init_class above reaches with the same `free` spelling; the
// freed pointer is read once and the null test jumps to the plain return.
// jackal_close still reaches this through the free forwarder
// filewin_close_class (src/pending_bodies.cpp), which now forwards to the
// member - the image's call there sets up no receiver, and a member call
// cannot spell that.
Return Value: n/a
Status: Complete
*/
void FileWin::close_class() {
    char *value = s_filewin_text_ptr;
    if (value) {
        free(value);
        s_filewin_text_ptr = nullptr;
    }
}
