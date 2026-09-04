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
#include "netwin.h"

#include "alpha.h"    // read_faction
#include "buffer.h"   // the local Buffer load_pcx / get_pixel run on
#include "faction.h"  // Player - the record read_faction fills
#include "strings.h"  // StringTemp, the pcx path the records name
#include "text.h"     // text_open / text_get / text_item / text_close

// THE SETUP DIALOG, real storage at the image's fixed address - see
// netwin.h. The image reaches it from an unannotated initialiser at
// 0x007AFF0 (`mov ecx, 0x80a6f8 / call 0x481c50`); this build runs the same
// recovered constructor through its own startup instead.
NetWin NetWinDialog;  // 0x0080A6F8

// The two player-id slots. Nothing in .text writes either; see netwin.h.
int NetWinPlayerId;       // 0x0093D4F0
int NetWinOtherPlayerId;  // 0x0093D4F4

// The faction text sections setup_faction_data reads, at their image
// addresses. They are `char *const` at namespace scope so VC6 folds each one
// to the immediate the image pushes - a `const char *const` that held the
// literal instead would materialise a pointer slot and cost the load.
static char *const NetWinSectionCustomCount = (char *)0x00686908;  // "CUSTOMFACTIONS"
static char *const NetWinSectionFactions = (char *)0x00686918;     // "FACTIONS"
static char *const NetWinSectionNewFactions = (char *)0x00686924;  // "NEWFACTIONS"
static char *const NetWinSectionCustom = (char *)0x00686930;       // "CUSTOMFACTIONS"
static char *const NetWinPcxSuffix = (char *)0x00686940;           // ".pcx"
static char *const NetWinDefaultPcx = (char *)0x00686948;          // "faction.pcx"
// The text file the sections are read from - a pointer slot, so it is loaded
// through rather than folded.
static char **const NetWinFactionTextFile = (char **)0x00691AFC;

// Both copies setup_faction_data makes are REAL calls in the image
// (0x00645930), and their size is not a compile-time constant, but /O2
// implies /Oi and VC6 expanded mine to `shr ecx,2 / rep movsd`. This is the
// palette.cpp / path.cpp `memcpy_call` idiom: an out-of-line forwarder the
// compiler will not fold.
static void *memcpy_call(void *dst, const void *src, unsigned len) {
    return memcpy(dst, src, len);
}

/*
// ORIGINAL: 0x00481FD0 ?setup_faction_data@NetWin@@QAEXPAUMessageFactionData@@@Z 0x00481FD0-0x00482488;0x006574E9-0x006574FE FILE
// size      1229 bytes
// prototype void (__thiscall ?setup_faction_data@NetWin@@QAEXPAUMessageFactionData@@@Z)(NetWin* this, MessageFactionData*)
// callers   3   call targets   15
// kind      game
// flags     frame;purged_ok
// calls     0x00586090 0x005D4510 0x005D7210 0x005D7410 0x005D7DE0 0x005E2210 0x005FD530 0x005FD550 0x005FD570 0x005FD670 0x00644EF2 0x00645460 0x00645470 0x00645930 0x00645E90
// Homed 2026-09-04 out of src/recovered/units/00481fd0.cpp, which is deleted
// by this: the body is a transcription of the image, not a recovery of it.
//
// CEILING: the local `Buffer buffer` needs unwind-safe destruction, so VC6
// builds the SEH frame the image has (`push -1 / push 0x6574f4 / fs:[0]`)
// and its funclet at 0x006574E9 - the flatbutton.cpp family. The frame turned
// out NOT to be the blocker: this tree's prologue agrees with the image
// through instruction 9, `sub esp,0xb30` included.
//
// LEVER: `memcpy_call`, the palette.cpp / path.cpp idiom. /O2 implies /Oi and
//        VC6 expanded both copies to `shr ecx,2 / rep movsd` where the image
//        calls 0x00645930 twice; the out-of-line forwarder took it 42/323 to
//        51/323.
// TRIED: the register swap that is now the first divergence. The image spills
//        `this` to ESI and uses EDI as its zero register; this tree does the
//        reverse (`push edi / mov edi,ecx`, then `xor esi,esi`), which shifts
//        every subsequent instruction by one. Same callee-saved pair, same
//        count of each, so nothing in the source model names the difference -
//        recorded rather than ground on.
Status: Homed 2026-09-04, MISMATCH 51 of 323 instructions
*/
void NetWin::setup_faction_data(MessageFactionData *message_faction_data) {
    Buffer buffer;

    // The image tests `message != 0` first and only then compares the two
    // player ids - `cmp ebx,edi / je` past the id test - so a null message
    // rebuilds from the text file whatever the ids say.
    if (message_faction_data != 0 && NetWinPlayerId == NetWinOtherPlayerId) {
        return;
    }

    if (faction_records_ != 0) {
        free(faction_records_);
        faction_records_ = 0;
    }

    if (message_faction_data != 0) {
        // Fast path: the sender already has the table; clone it whole. The
        // record count rides byte 0x11 and is clamped to a floor of 8 - the
        // image compares the BYTE (`cmp al,8 / jae`), not a widened int.
        int count =
            message_faction_data->record_count_ < 8
                ? 8
                : static_cast<int>(message_faction_data->record_count_);
        int size = count * 399 + 0x194;
        faction_records_ = mem_get(size);
        memcpy_call(faction_records_, &message_faction_data->record_type_, size);
        return;
    }

    // Slow path: rebuild the table from the localized faction text. The
    // CUSTOMFACTIONS count is discovered by walking until text_item is empty.
    int custom_count = 0;
    if (text_open(*NetWinFactionTextFile, NetWinSectionCustomCount) == 0) {
        text_get();
        if (*text_item() != 0) {
            do {
                custom_count++;
                text_get();
            } while (*text_item() != 0);
        }
        if (custom_count > 10) {
            custom_count = 10;
        } else if (custom_count < 0) {
            custom_count = 0;
        }
    }

    // 0xe base factions plus the custom ones, and never fewer than 8 records.
    int record_count = custom_count + 0xe;
    record_count = record_count < 8 ? 8 : record_count;
    int record_size = record_count * 399 + 0x194;

    faction_records_ = mem_get(record_size);
    faction_records_copy_ = mem_get(record_size);

    // The 7 base factions land first, at the front of the block. The image
    // counts the loop DOWN (`mov [ebp-0x10],7 / dec / jne`), which is why the
    // flag offset climbs in the third clause of the same for.
    if (text_open(*NetWinFactionTextFile, NetWinSectionFactions) != 0) {
        return;
    }
    char *records;
    int flag_offset;
    int name_offset;
    int record_index;
    records = static_cast<char *>(faction_records_);
    records[1] = static_cast<char>(record_count);
    records = static_cast<char *>(faction_records_);
    records[0] = 7;
    flag_offset = 0;
    for (record_index = 7; record_index != 0;
         --record_index, flag_offset += 0x190) {
        records = static_cast<char *>(faction_records_);
        records[flag_offset + 9] = 1;
        text_get();
        records = static_cast<char *>(faction_records_);
        strcpy(records + flag_offset + 0xa, text_item());
    }

    // The 7 expansion factions follow, one 0x190 stride further on: the flag
    // offset walks 0..0xAF0 while the name offset starts at 0xAF0.
    if (text_open(*NetWinFactionTextFile, NetWinSectionNewFactions) != 0) {
        return;
    }
    name_offset = 0xaf0;
    for (flag_offset = 0; flag_offset < 0xaf0; flag_offset += 0x190) {
        records = static_cast<char *>(faction_records_);
        records[flag_offset + 9] = 1;
        text_get();
        records = static_cast<char *>(faction_records_);
        strcpy(records + name_offset + 0xa, text_item());
        name_offset += 0x190;
    }

    // Then however many custom factions the count pass found, at 0x15E0.
    if (text_open(*NetWinFactionTextFile, NetWinSectionCustom) != 0) {
        return;
    }
    if (custom_count > 0) {
        flag_offset = 0;
        name_offset = 0x15e0;
        for (record_index = custom_count; record_index != 0; --record_index) {
            records = static_cast<char *>(faction_records_);
            records[flag_offset + 9] = 1;
            text_get();
            records = static_cast<char *>(faction_records_);
            strcpy(records + name_offset + 0xa, text_item());
            flag_offset += 0x190;
            name_offset += 0x190;
        }
    }

    // Each record is then re-read from its own file - the faction name IS the
    // file id - to pull the leader titles, the noun and the colour pixel.
    Player player;
    int offset;
    for (record_index = 0, offset = 0; record_index < record_count;
         ++record_index, offset += 0x190) {
        records = static_cast<char *>(faction_records_);
        if (text_open(records + offset + 0xa, records + offset + 0xa) != 0) {
            return;
        }
        records = static_cast<char *>(faction_records_);
        records[offset + 9] = 1;
        // text_open returned 0 to get here, and the image stores THAT value
        // at +4 as a dword rather than re-deriving it.
        *reinterpret_cast<int *>(records + offset + 4) = 0;

        text_get();
        text_item();
        text_item();
        strncpy(records + offset + 0x22, text_item(), 0x18);
        records = static_cast<char *>(faction_records_);
        records[offset + 0x39] = 0;

        StringTemp[0] = 0;
        records = static_cast<char *>(faction_records_);
        strcat(StringTemp, records + offset + 0xa);
        strcat(StringTemp, NetWinPcxSuffix);

        if (buffer.load_pcx(StringTemp, 0, 0xa, 0xec) != 0) {
            buffer.load_pcx(NetWinDefaultPcx, 0, 0xa, 0xec);
        }
        records = static_cast<char *>(faction_records_);
        records[offset + 8] = static_cast<char>(buffer.get_pixel(4, 0x2f3));

        // read_faction fills the Player from the faction's own file; the
        // strings it read are then copied back into the record at +0x22,
        // +0x3A, +0x62, +0x7A, +0xFA and +0x112, and the leader-gender flag
        // rides the record's last byte.
        strcpy(player.filename, records + offset + 0xa);
        strcpy(player.search_key, records + offset + 0xa);
        read_faction(&player, 0);
        records = static_cast<char *>(faction_records_);
        strcpy(records + offset + 0x22, player.name_leader);
        strcpy(records + offset + 0x3a, player.formal_name_faction);
        strcpy(records + offset + 0x62, player.noun_faction);
        strcpy(records + offset + 0x7a, player.adj_name_faction);
        strcpy(records + offset + 0xfa, player.title_leader);
        strcpy(records + offset + 0x112, player.adj_leader);
        records[offset + 0x192] = static_cast<char>(player.is_leader_female);
    }

    text_close();
    memcpy_call(faction_records_copy_, faction_records_, record_size);
}

// 0x0047C970 is still archived in src/unrecovered/0047c970.cpp - a
// SEH-framed password dialog, the largest single body on NetWin's board.
// The declaration in netwin.h is what unblocks its callers, of which
// process_message's 0xF06 case is one; until the body comes across this
// forwards into the original the same way pending_bodies.cpp does, and
// faults loudly rather than pretending to have succeeded.
int NetWin::pick_pass(int payload) {
    typedef int(__fastcall *pending)(NetWin *, void *, int);
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x0047C970))(
        this, nullptr, payload);
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0047F920 ?UNK1@NetWin@@QAEXXZ 0x0047F920-0x0047F921 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0047F930 ?UNK2@NetWin@@QAEXXZ 0x0047F930-0x0047F931 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004834D0 ?alloc_slots@NetWin@@QAEXXZ 0x004834D0-0x004834D1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?alloc_slots@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::alloc_slots() {
}




/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x0047B750 ?on_key_click@NetWin@@QAEHHH@Z 0x0047B750-0x0047B758 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@NetWin@@QAEHHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int NetWin::on_key_click(int, int) {
    return 1;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0047F940 ?on_mouse_move@NetWin@@QAEXHH@Z 0x0047F940-0x0047F943 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@NetWin@@QAEXHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0047F950 ?on_mouse_leave@NetWin@@QAEXHH@Z 0x0047F950-0x0047F953 BYTE_EXACT
// symbol    ?on_mouse_leave@NetWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@NetWin@@QAEXHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::on_mouse_leave(int, int) {
}


/*
Purpose: Clear the dword at 0x178 and write a five-byte pattern over the start
         of the object.

             mov [eax+0x178],ecx   ; ecx is zero
             mov byte [eax],0xFF   / [eax+1],0 / [eax+2],0
             mov byte [eax+3],0xFF / [eax+4],2

         THE FIRST FIVE BYTES ARE INSIDE THE GraphicWin BASE, and that was
         checked rather than assumed. ??0NetWin@@QAE@XZ does `mov esi,ecx` then
         calls GraphicWin's constructor with ecx UNADJUSTED, which by the
         base-versus-member rule in AGENTS.md makes GraphicWin a base at offset
         zero - its Spot and StringBox members follow at 0xD34 and 0xD40, on
         `this` plus an offset. The modelled inheritance is right.

         So this really does write over the object's first five bytes, which
         the model attributes to Win's leading AutoSound. 0xFF,00,00,0xFF then
         2 reads like a colour and a mode rather than a pointer, and that
         appearance is what made this look like a modelling error; it is not.
         The behaviour is transcribed as the original has it, byte for byte and
         in the original's order - 0x178 first - rather than corrected to what
         it seems like it ought to be.
// ORIGINAL: 0x00483820 ?UNK5@NetWin@@QAEXXZ 0x00483820-0x0048383C
// size      28 bytes
// prototype void (__thiscall ?UNK5@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK5() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<uint32_t *>(self + 0x178) = 0;
    self[0] = 0xFF;
    self[1] = 0;
    self[2] = 0;
    self[3] = 0xFF;
    self[4] = 2;
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
// ORIGINAL: 0x00481C50 ??0NetWin@@QAE@XZ 0x00481C50-0x00481D08;0x00657491-0x006574E9
// size      272 bytes
// prototype void (__thiscall ??0NetWin@@QAE@XZ)(NetWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FA860 0x00607CF0 0x0060E670 0x00614E50 0x00629110
*/

// The six subobjects (GraphicWin base, then spot_/stringBox_/editBox_/
// checkBox_/flatButton1_/flatButton2_, in declaration order) construct via
// the implicit base+member-init sequence; 0x005D4CF0 is `?construct@
// GraphicWin@@QAEXXZ`, a construct() method (see basebutton.cpp's own
// `?construct@BaseButton@@QAEXXZ`), which the trivial `GraphicWin() { ; }`
// base ctor cannot reach implicitly, so it is called explicitly here.
//
// TRIED: byte-exact is not reachable from this marker alone.
// StringBox()/CheckBox() are still trivial stubs (0x00629110/0x0060E670
// unfilled) and EditBox() has no declared constructor at all - none of the
// three are among this pass's seven targets, so the image's calls to them
// cannot be reproduced from here. CheckBox's own call also pushes an extra
// `1` (a virtually-inherited GraphicWin's hidden most-derived flag, added
// automatically by the compiler once CheckBox::CheckBox() takes it - not
// reachable while checkBox_'s constructor is the current no-arg stub).
NetWin::NetWin() {
    // The two vtable stores the old hand body spelled at [this] (0x0066CCEC)
    // and [this+0x444] (0x0066CCE4) are the compiler's now: NetWin is
    // most-derived through GraphicWin's Win/Buffer base pair, and its real
    // constructor installs both vptrs itself. The two scalar clears below are
    // the body's own work - 0x772C and 0x7730, the faction-record blocks.
    faction_records_ = 0;
    faction_records_copy_ = 0;
}

/*
Purpose: Step the receiver back to the subobject ??_GNetWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x00483840 ??_GNetWin@@WEEE@AEPAXI@Z 0x00483840-0x0048384B BYTE_EXACT
// symbol    ??_ENetWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3NetWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004837C0` into
//   ??_GNetWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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
