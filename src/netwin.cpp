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

#include "alpha.h"     // read_faction
#include "buffer.h"    // the local Buffer load_pcx / get_pixel run on
#include "dialog.h"    // Dialog - the +0xA34 base UNK4 destroys
#include "dialogs.h"   // Dialogs - the pick_* option lists
#include "faction.h"   // Player - the record read_faction fills
#include "game.h"      // ExitTurnLoop, the modal callbacks' stop flag
#include "netdaemon.h" // NetDaemonNet, message_chat - the pump and the chat send
#include "original_seam.h"  // vtable_slot - the unmodelled virtual slots
#include "popmenu.h"   // PopMenu - the pickers' popup and its exec
#include "pulldown.h"  // PullDown - the pickers' second pane
#include "strings.h"   // StringTemp, the pcx path the records name
#include "text.h"      // text_open / text_get / text_item / text_close

// THE SETUP DIALOG, real storage at the image's fixed address - see
// netwin.h. The image reaches it from an unannotated initialiser at
// 0x007AFF0 (`mov ecx, 0x80a6f8 / call 0x481c50`); this build runs the same
// recovered constructor through its own startup instead.
NetWin NetWinDialog;  // 0x0080A6F8

// The pinned PE's .data raw contents end at 0x006A8000, while its virtual
// extent includes 0x0090DB98..0x0090E778: these rows start zero-filled.
// config_players and the network receive path subsequently populate them.
// RECOVERY_OBJECT: v2 0x0090DB98 0x0090E778 ?NetWinLobbyRows@@3PAUNetWinLobbyRow@@A 0x0047B450
NetWinLobbyRow NetWinLobbyRows[8];
// RECOVERY_OBJECT: v2 0x0090E778 0x0090E8E0 ?NetWinSessionData@@3UNetWinSessionBlock@@A 0x0047B450
NetWinSessionBlock NetWinSessionData;
// RECOVERY_OBJECT: v2 0x0090E8E0 0x0090E8F8 ?NetWinSessionOptions@@3UNetWinSessionOptionsState@@A 0x0047B450
NetWinSessionOptionsState NetWinSessionOptions;

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
// setup_faction_data reads the same 0x00691AFC pointer slot as the rules
// loader. AlphaxFileID in alpha.cpp owns it; this TU does not own an alias
// into the old executable's storage.

// Both copies setup_faction_data makes are REAL calls in the image
// (0x00645930), and their size is not a compile-time constant, but /O2
// implies /Oi and VC6 expanded mine to `shr ecx,2 / rep movsd`. This is the
// palette.cpp / path.cpp `memcpy_call` idiom: an out-of-line forwarder the
// compiler will not fold.
static void *memcpy_call(void *dst, const void *src, unsigned len) {
    return memcpy(dst, src, len);
}

// ---- fixed bindings the dialog bodies reach -------------------------------

// 0x0093D4F8, the dword beside the two player-id slots: the lobby-side
// player count. get_player_color bounds a colour search with it and
// modal_callback scans the static faction records with it; no recovered
// body writes it and no catalogue names it.
static int *const NetWinPlayerCount = &NetDaemonNet.player_count_;

// 0x0090DB9B - the static faction-record table the dialog reads directly
// (the allocated copies live in faction_records_): the colour byte of record
// `faction` sits at [faction * 0x17C], the record block the modal pump
// scans starts 0x17A bytes in at 0x0090DD15, and the localized name text of
// each record starts one byte later at 0x0090DD17. Record stride 0x17C.
static char *const NetWinFactionTable =
    reinterpret_cast<char *>(&NetWinLobbyRows[0].faction_or_color_);

// 0x0093D51C - the per-slot records the chat line names its sender from:
// 103 dwords (0x19C) per slot with the DirectPlay id first, indexed by the
// slot number new_chat is handed.
struct NetWinSlotRecord {
    unsigned long id;
    unsigned char tail_[0x19C - 4];
};
static NetWinSlotRecord *const NetWinSlotRecords = (NetWinSlotRecord *)0x0093D51C;


// The two control bodies the dialog's teardown paths reach that are still
// their own preserved seeds in the StringBox / EditBox groups. The same
// shape pick_pass carries below: the forwarder keeps the image's callee
// without claiming the seed, and faults loudly if the body is gone.
static void string_box_close(StringBox *box) {
    typedef void (__fastcall *pending)(StringBox *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00629210))(box,
                                                                      nullptr);
}

static void edit_box_set_text(EditBox *box, char *text) {
    typedef void (__fastcall *pending)(EditBox *, void *, char *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006151E0))(
        box, nullptr, text);
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
    if (text_open(AlphaxFileID, NetWinSectionCustomCount) == 0) {
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
    if (text_open(AlphaxFileID, NetWinSectionFactions) != 0) {
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
    if (text_open(AlphaxFileID, NetWinSectionNewFactions) != 0) {
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
    if (text_open(AlphaxFileID, NetWinSectionCustom) != 0) {
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

/*
Purpose: Resolve the colour byte a faction list draws its player rows with:
         the static table's colour for the handed-in faction, unless the
         allocated record block's own copy claims a different index - the
         first index no earlier faction's name record and no other player's
         colour repeats wins.
// ORIGINAL: 0x0047B380 ?get_player_color@NetWin@@QAEHH@Z 0x0047B380-0x0047B442 FILE
// size      194 bytes
// prototype int (__thiscall ?get_player_color@NetWin@@QAEHH@Z)(NetWin* this, int)
// callers   4   call targets   0
// kind
// flags
// calls     (none)
// Homed from src/unrecovered/0047b380.cpp.
Return Value: the colour byte, or 0xFF for the uncoloured faction, 0 with no
         records loaded
Status: Coverage transcription - unclaimed body
*/
int NetWin::get_player_color(int faction) {
    if (faction_records_ == 0) {
        return 0;
    }
    const NetWinFactionRecord *const records =
        static_cast<const NetWinFactionRecord *>(faction_records_);
    int color = NetWinLobbyRows[faction].faction_or_color_;
    if (color == -1) {
        return 0xFF;
    }
    int taken = 1;
    if (faction > 1) {
        const NetWinLobbyRow *row = &NetWinLobbyRows[1];
        do {
            if (row->faction_or_color_ == color) {
                break;
            }
            taken++;
            ++row;
        } while (taken < faction);
    }
    int index = 0;
    if (taken != faction) {
        index = 0;
        int record_count = records[0].record_count_;
        if (record_count != 0) {
            int bound = *NetWinPlayerCount + 1;
            for (;;) {
                int found = 1;
                if (bound > 1) {
                    const NetWinLobbyRow *row = &NetWinLobbyRows[1];
                    do {
                        if (row->faction_or_color_ == index) {
                            break;
                        }
                        found++;
                        ++row;
                    } while (found < bound);
                }
                if (found == bound) {
                    break;
                }
                index++;
                if (index >= record_count) {
                    break;
                }
            }
        }
    }
    return records[index].color_;
}

/*
Purpose: Paint the three setup-dialog panel sprites: the map's ocean colours
         and the two neighbour panels, each through Buffer::box_sprite with
         the shared InterfaceBoxStyles records (element 3 for the map, 0 for
         the pair beside it).
// ORIGINAL: 0x0047FE80 ?UNK3@NetWin@@QAEXXZ 0x0047FE80-0x0047FEC5 FILE
// size      70 bytes
// prototype void (__thiscall ?UNK3@NetWin@@QAEXXZ)(NetWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E2DD3
// Homed from src/recovered/0047fe80.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body (the preserved seed keeps its
        own BYTE_EXACT claim until this converges)
*/
void NetWin::UNK3() {
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCE4),
                       &InterfaceBoxStyles[3]);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCD4),
                       &InterfaceBoxStyles[0]);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCC4),
                       &InterfaceBoxStyles[0]);
}

/*
Purpose: Tear the setup dialog's session state down: drop both faction-record
         blocks, reset the two dialog words at 0x761C/0x7620 to -1, and close
         the embedded controls - the spot list, the message list, the edit
         box, the check box and the front flat button's slot-90 reset.
// ORIGINAL: 0x00481D10 ?close@NetWin@@QAEXXZ 0x00481D10-0x00481D9C FILE
// size      140 bytes
// prototype void (__thiscall ?close@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FA830 0x0060E7C0 0x00614F30 0x00629210 0x00644EF2
// indirect  0x00481D94
// Homed from src/unrecovered/00481d10.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body (the preserved seed keeps its
        own FILE BYTE_EXACT claim until this converges)
*/
void NetWin::close() {
    if (faction_records_ != 0) {
        free(faction_records_);
        faction_records_ = 0;
    }
    if (faction_records_copy_ != 0) {
        free(faction_records_copy_);
        faction_records_copy_ = 0;
    }
    field_761C_ = 0xFFFFFFFF;
    field_7620_ = 0xFFFFFFFF;
    spot_.shutdown();
    string_box_close(&stringBox_);
    editBox_.close();
    checkBox_.close();
    typedef void (__fastcall *flat_button_slot_fn)(void *);
    vtable_slot<flat_button_slot_fn>(&flatButton1_, 0x168)(&flatButton1_);
}

/*
Purpose: Copy one faction record's name/title block into a PlayerData: the
         six text fields land at +5, +0x1D, +0x45, +0x5D, +0xDD and +0xF5,
         and the leader-gender byte at +0x175. Slot -1 instead re-reads just
         the +0x45 field through the string table, for the record the
         scenario file's own faction entry describes.
// ORIGINAL: 0x00481DA0 ?fill_player@NetWin@@QAEXPAUPlayerData@@H@Z 0x00481DA0-0x00481E87 FILE
// size      237 bytes
// prototype void (__thiscall ?fill_player@NetWin@@QAEXPAUPlayerData@@H@Z)(NetWin* this, PlayerData*, int)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// Homed from src/unrecovered/00481da0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::fill_player(PlayerData *player_data, int slot) {
    if (slot == -1) {
        unsigned char *text =
            reinterpret_cast<unsigned char *>(player_data) + 0x45;
        text[0] = 0;
        strcat(reinterpret_cast<char *>(text),
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)));
        return;
    }
    char *record =
        reinterpret_cast<char *>(faction_records_) + slot * 400;
    unsigned char *target = reinterpret_cast<unsigned char *>(player_data);
    strcpy(reinterpret_cast<char *>(target + 5), record + 0x22);
    strcpy(reinterpret_cast<char *>(target + 0x1D), record + 0x3A);
    strcpy(reinterpret_cast<char *>(target + 0x45), record + 0x62);
    strcpy(reinterpret_cast<char *>(target + 0x5D), record + 0x7A);
    strcpy(reinterpret_cast<char *>(target + 0xDD), record + 0xFA);
    strcpy(reinterpret_cast<char *>(target + 0xF5), record + 0x112);
    target[0x175] = static_cast<unsigned char>(record[0x192]);
}

// 0x0090E778 / 0x0090E79C - the two words beside the seven records
// config_players seeds: the eighth slot's dword and its mode byte.
static unsigned int *const NetWinEighthSlotWord = &NetWinSessionData.mode_;
static unsigned char *const NetWinScenarioText = &NetWinSessionData.state_[0x20];

// 0x0090E8E0 - the ninth slot's colour/mode prefix: 0xFF, the colour byte
// read from 0x93D4B0, 2 - with the zeroed pair at +4/+5, the mode bytes 2 at
// +6..+0xA and the 0xFF at +0xB.
static unsigned char *const NetWinNinthSlot =
    reinterpret_cast<unsigned char *>(&NetWinSessionOptions);

// 0x0093D4B0 - the colour byte beside the player-id slots; the ninth slot's
// prefix is seeded from it.
static unsigned char *const NetWinConfigColor = (unsigned char *)0x0093D4B0;

// 0x00947020 - the default faction records the seven base slots are seeded
// from: one 0x59C-byte Player-shaped record each, the same stride the
// Players array walks. config_players copies the name and leader text out.
static char *const NetWinFactionTemplate = (char *)0x00947020;

/*
Purpose: Seed the seven base faction records of the static faction table.
         Each record's colour/mode prefix is rewritten - 0xFF, 0, 2, 0xFF, 2,
         with the dword at +0x177 cleared - the name, formal-name and leader
         text copied out of the template record at the same index, the title
         re-read through the string table, the +0x174 byte taken from the
         template, and the trailing eighth/ninth slot words reset (the ninth
         slot's prefix carrying the colour byte at 0x93D4B0). The image
         writes the prefix twice per record - the second pass re-stores
         0xFF/0xFF/2/0 over the same bytes - so only the net values land.
// ORIGINAL: 0x00481E90 ?config_players@NetWin@@QAEXXZ 0x00481E90-0x00481FC9 FILE
// size      313 bytes
// prototype void (__thiscall ?config_players@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645460
// Homed from src/recovered/units/00481e90.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::config_players() {
    unsigned char color = *NetWinConfigColor;
    *NetWinEighthSlotWord = 0;
    *NetWinScenarioText = 0;
    NetWinNinthSlot[0] = 0xFF;
    NetWinNinthSlot[4] = 0;
    NetWinNinthSlot[5] = 0;
    NetWinNinthSlot[0xB] = 0xFF;
    NetWinNinthSlot[1] = color;
    NetWinNinthSlot[2] = 2;
    NetWinNinthSlot[6] = 2;
    NetWinNinthSlot[7] = 2;
    NetWinNinthSlot[8] = 2;
    NetWinNinthSlot[9] = 2;
    NetWinNinthSlot[0xA] = 2;

    char *record = NetWinFactionTable + 0x17A;
    char *template_record = NetWinFactionTemplate;
    do {
        *reinterpret_cast<unsigned int *>(record + 0x177) = 0;
        record[-1] = 0xFF;
        record[0] = 0;
        record[1] = 2;
        record[2] = 0xFF;
        record[3] = 2;
        strcpy(record + 4, template_record);
        strcpy(record + 0x1C, template_record + 0x350);
        strcpy(record + 0x44,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)));
        strcpy(record + 0x5C, template_record + 0x31C);
        strcpy(record + 0xDC, template_record + 0x18);
        strcpy(record + 0xF4, template_record + 0x30);
        record[0x174] = template_record[0xC];
        record += 0x17C;
        template_record += 0x59C;
    } while (record < NetWinFactionTable + 0x17A + 7 * 0x17C);
}

/*
Purpose: Repaint the setup dialog: the guard test, the panel sprites (the
         map frame at 0x474, the banner rectangle copied from 0xA14 and then
         inset by three, and the three helper panels at 0xCE4/0xCD4/0xCC4,
         each through its shared InterfaceBoxStyles entry), the centered
         session title upper-cased out of the string table, and the five
         content groups - draw_custom_rules, draw_player_names, draw_diffs,
         draw_factions, draw_session_info - then soft_update.
// ORIGINAL: 0x00481AD0 ?on_redraw@NetWin@@QAEXXZ 0x00481AD0-0x00481C50 FILE
// size      384 bytes
// prototype void (__thiscall ?on_redraw@NetWin@@QAEXXZ)(NetWin* this)
// callers   3   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047F960 0x0047FED0 0x00480180 0x00480370 0x00480900 0x005882F0 0x005D5890 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005F7E90 0x006169A0 0x006453E0 0x00645470
// Homed from src/recovered/units/00481ad0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::on_redraw() {
    if (is_visible() == 0) {
        return;
    }
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0x474),
                       &InterfaceBoxStyles[3]);

    // The banner rectangle: painted as 0xA14 holds it, then inset by three
    // on every side and painted again - the inset copy is the one the title
    // is centred in.
    RECT banner = *reinterpret_cast<RECT *>(&field_A14_);
    buffer->box_sprite(&banner, &InterfaceBoxStyles[1]);
    banner.left += 3;
    banner.top += 3;
    banner.right -= 3;
    banner.bottom -= 3;
    buffer->box_sprite(&banner, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<int *>(Labels.strings_ptr
                                                    + 0x3F0)));
    string_upper_slot()(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, &banner,
                             strlen(StringTemp));
    }

    draw_custom_rules();
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCE4),
                       &InterfaceBoxStyles[1]);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCD4),
                       &InterfaceBoxStyles[0]);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xCC4),
                       &InterfaceBoxStyles[0]);
    draw_player_names();
    draw_diffs();
    draw_factions();
    draw_session_info();
    soft_update();
}

// 0x0096C85C - alpha.cpp's file-local Difficulty[] name-index table (its own
// static binding, not extern): draw_diffs indexes it with the record's mode
// byte and reads the string-table index it holds.
static char **const NetWinDifficultyNames = (char **)0x0096C85C;

// 0x0090E8EC / 0x0090E8F0 - the custom-rules bit fields the eighteen
// draw_custom_rules rows read their checkbox state from (the ability and
// allowed-unit bits); 0x0090E8F4 carries the free-tech bit the standing
// checkbox inverts.
static unsigned int *const NetWinRuleBitsA = &NetWinSessionOptions.rule_bits_a_;
static unsigned int *const NetWinRuleBitsB = &NetWinSessionOptions.rule_bits_b_;
static unsigned int *const NetWinRuleBitsC = &NetWinSessionOptions.rule_bits_c_;

// 0x0093E8C0 - the gate the custom-rules rows test before registering their
// spots; no recovered body writes it and no catalogue names it.
static unsigned int *const NetWinRulesGate =
    &NetDaemonNet.lock_state_[0].dword_[1];

// 0x0078DCA8 - the ready/check panel descriptors the player rows draw their
// slot state from (element [2*ready] of a 0x28-stride BoxSpriteParams
// array); its storage has no RECOVERY_OBJECT binding yet, so the pointer
// names the image's block and the object admission is still the
// coordinator's.
static BoxSpriteParams *const NetWinReadyPanels =
    (BoxSpriteParams *)0x0078DCA8;

// 0x00669328 - the runtime three-rectangle slot StatusWin::redraw also goes
// through; its target is still the original body's to name. Stdcall with
// three rectangle pointers.
static void *const NetWinRectSlot = (void *)0x00669328;

// 0x0093D494 - the char * slot the joined session's name is read from for
// the session-info title (empty until a session names itself).
static char **const NetWinSessionName = (char **)0x0093D494;

// 0x0080A6F0 - the planet-size name count beside the fixed dialog (the
// names themselves ride the 0x80-stride block at 0x6614).
static int *const NetWinPlanetSizeCount = (int *)0x0080A6F0;

// 0x009B39D8 - the x coordinate wrap() finished the wrapped rules line at,
// which the combo-arrow chip of that line is placed beside.
static int *const NetWinWrapX = (int *)0x009B39D8;

/*
Purpose: Paint the eight session-info rows: the panel and title frames, the
         centred session name (or the connected game's), then the labelled
         value rows - map size, rules/scenario, opponent difficulty, the four
         rule toggles - each "<label>: } <value>" with the label braced, the
         combo-arrow chip on the host's rows, and each row handed to the spot
         list. Ends with the connected-players tally for a hosted or joined
         game.
// ORIGINAL: 0x00480900 ?draw_session_info@NetWin@@QAEXXZ 0x00480900-0x00481AC2;0x0065745D-0x00657491 FILE
// size      4598 bytes
// prototype void (__thiscall ?draw_session_info@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401000 0x005882F0 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DC790 0x005DCAB0 0x005DCEA0 0x005DD130 0x005DD730 0x005E2DD3 0x005E4B9A 0x005FA960 0x005FAA90 0x006169A0 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004809CD 0x004819B0 0x004819C2 0x004819DE 0x00481A37 0x00481A49 0x00481A65
// Homed from src/unrecovered/00480900.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::draw_session_info() {
    spot_.kill_type(0);
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    StringStruct list;

    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xA44),
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = reinterpret_cast<RECT *>(self + 0xA34);
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    if (*NetWinSessionName != 0) {
        strcat(StringTemp, *NetWinSessionName);
    }
    string_upper_slot()(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    buffer->set_clip(title_rect);
    if (*StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }
    buffer->set_clip(reinterpret_cast<RECT *>(self + 0x474));

    RECT *panel = reinterpret_cast<RECT *>(self + 0xA24);
    buffer->box_sprite(panel, &InterfaceBoxStyles[0]);
    RECT field;
    field.left = panel->left + 4;
    field.top = panel->top + 4;
    field.right = panel->right - 4;
    field.bottom = panel->bottom - 4;
    buffer->set_clip(&field);

    buffer->set_text_color(0xDF, -1, 1, 1);
    buffer->set_text_color2(0xE2, -1, 1, 1);

    const int col = field.left;
    int row = field.top;
    // The wrapped-text width every write_l measures against; 0x14 is the
    // body's own starting value. Buffer keeps it protected, so the store
    // goes through the object's own layout - a public setter is still owed.
    *reinterpret_cast<uint32_t *>(reinterpret_cast<unsigned char *>(buffer)
                                  + 0x10) = 0x14;

    // ---- session name: {Name: } <global difficulty> ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0xFA4)));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    {
        int difficulty = NetWinNinthSlot[2];
        if (difficulty < 0) {
            difficulty = 0;
        } else if (6 < difficulty) {
            difficulty = 6;
        }
        strcat(StringTemp,
               StringTable.get(
                   reinterpret_cast<int>(NetWinDifficultyNames[difficulty])));
    }
    {
        int written = buffer->write_l(StringTemp, col, row,
                                      strlen(StringTemp));
        if (*NetWinRulesGate != 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), written + 3, row);
        }
        spot_.add(2, 0, col, row, 0x1E, buffer->text_line_height());
        row += buffer->text_line_height();
    }

    // ---- map size: {Size: } <time-control name> ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0x100C)));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    strcat(StringTemp,
           StringTable.get(*reinterpret_cast<int *>(
               &TimeControl[static_cast<unsigned char>(NetWinNinthSlot[3])])));
    {
        int written = buffer->write_l(StringTemp, col, row,
                                      strlen(StringTemp));
        if (*NetWinRulesGate != 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), written + 3, row);
        }
        spot_.add(3, 0, col, row, 0x1E, buffer->text_line_height());
        row += buffer->text_line_height() * 2;
    }

    // ---- rules / scenario: {Rules: } <name or scenario text> ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0x1078)));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (*NetWinScenarioText == 0) {
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0x107C)));
    } else {
        strcat(StringTemp,
               reinterpret_cast<const char *>(NetWinScenarioText));
    }
    if (*NetWinEighthSlotWord == 1) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0x10)));
    } else if (*NetWinEighthSlotWord == 2) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0x1018)));
    } else if (*NetWinEighthSlotWord == 3) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0x1014)));
    }
    {
        int bottom = buffer->wrap(StringTemp, col, row,
                                  panel->right - panel->left - 0x1E);
        if (*NetWinRulesGate != 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), *NetWinWrapX + 3,
                       bottom - buffer->text_line_height());
        }
        spot_.add(0, 0, col, row, panel->right - panel->left, bottom - row);
        row = bottom;
    }

    // ---- opponent difficulty ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0xFAC)));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (*NetWinScenarioText == 0) {
        int opponent = NetWinNinthSlot[4];
        if (opponent == -1) {
            strcat(StringTemp,
                   StringTable.get(
                       *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)));
        } else {
            if (opponent < 0) {
                opponent = 0;
            } else if (0x1F < opponent) {
                opponent = 0x1F;
            }
            strcat(StringTemp,
                   *reinterpret_cast<char **>(
                       reinterpret_cast<char *>(Labels.strings_ptr)
                       + opponent * 0x80 + 0x6614));
        }
    } else {
        char tally[80];
        _itoa(NetWinNinthSlot[4], tally, 10);
        strcat(StringTemp, tally);
        strcat(StringTemp, " ");
        strcat(StringTemp, "X");
        strcat(StringTemp, " ");
        _itoa((*reinterpret_cast<unsigned long *>(&NetWinNinthSlot[4])
                  >> 8) & 0xFF, tally, 10);
        strcat(StringTemp, tally);
    }
    {
        int avail = panel->right - col - 10;
        int bottom;
        if (avail < buffer->text_width(StringTemp)) {
            bottom = buffer->wrap(StringTemp, col, row, avail);
        } else {
            buffer->write_l(StringTemp, col, row, strlen(StringTemp));
            bottom = buffer->text_line_height() + row;
        }
        if (*NetWinRulesGate != 0 && *NetWinScenarioText == 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), col + 3, row);
            spot_.add(4, 0, col, row, 0x1E, bottom - row);
        }
        row = bottom;
    }

    // ---- the four rule toggles ----
    for (int rule = 0; rule < 4; ++rule) {
        static const char *const kLabels[4] = {"{", "{", "{", "{"};
        static const char *const kLabelEnds[4] = {"}", "}", "}", "}"};
        static const int kTextOffsets[4] = {0xFB0, 0xFB4, 0xFB8, 0xFBC};
        static const int kSpotIds[4] = {5, 6, 7, 8};
        static const int kTableBases[4] = {0x3D8, 0x3DB, 0x3E1, 0x3E4};
        signed char flag =
            static_cast<signed char>(NetWinNinthSlot[5 + rule]);

        StringTemp[0] = 0;
        strcat(StringTemp, kLabels[rule]);
        strcat(StringTemp,
               StringTable.get(*reinterpret_cast<int *>(
                   Labels.strings_ptr + kTextOffsets[rule])));
        strcat(StringTemp, ": ");
        strcat(StringTemp, kLabelEnds[rule]);
        strcat(StringTemp, " ");

        int text_id;
        if (*NetWinScenarioText == 0) {
            if (flag == -1) {
                text_id = *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4);
            } else {
                text_id = *reinterpret_cast<int *>(
                    Labels.strings_ptr
                    + (flag + kTableBases[rule]) * 4);
            }
        } else {
            text_id = *reinterpret_cast<int *>(Labels.strings_ptr + 0x1000);
        }
        strcat(StringTemp, StringTable.get(text_id));

        int avail = panel->right - col - 10;
        int bottom;
        if (avail < buffer->text_width(StringTemp)) {
            bottom = buffer->wrap(StringTemp, col, row, avail);
        } else {
            buffer->write_l(StringTemp, col, row, strlen(StringTemp));
            bottom = buffer->text_line_height() + row;
        }
        if (*NetWinRulesGate != 0 && *NetWinScenarioText == 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), col + 3, row);
            spot_.add(kSpotIds[rule], 0, col, row, 0x1E, bottom - row);
        }
        row = bottom;
    }

    row = buffer->text_line_height() + row;

    if (*NetWinEighthSlotWord == 3 || *NetWinEighthSlotWord == 2) {
        // The unclaimed-faction tally: every record whose claimed id matches
        // nobody in the lobby.
        int connected = 0;
        for (int slot = 8; slot != 0; --slot) {
            unsigned long owned = *reinterpret_cast<unsigned long *>(
                static_cast<char *>(faction_records_) + (8 - slot) * 0x190
                + 4);
            if (owned != 0) {
                int seen = 0;
                if (*NetWinPlayerCount > 0) {
                    unsigned long *scan = reinterpret_cast<unsigned long *>(
                        NetWinFactionTable + 0x2F1);
                    while (seen < *NetWinPlayerCount) {
                        if (*scan == owned) {
                            break;
                        }
                        ++seen;
                        scan += 0x5F;
                    }
                }
                if (seen == *NetWinPlayerCount) {
                    ++connected;
                }
            }
        }

        buffer->set_text_color((connected != 0 ? 8 : 0) - 0xF, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp,
               StringTable.get(
                   *reinterpret_cast<int *>(Labels.strings_ptr + 0x113C)));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "}");
        strcat(StringTemp, " ");
        char tally[80];
        _itoa(connected, tally, 10);
        strcat(StringTemp, tally);

        int avail = panel->right - col - 10;
        if (avail < buffer->text_width(StringTemp)) {
            buffer->wrap(StringTemp, col, row, avail);
        } else {
            buffer->write_l(StringTemp, col, row,
                            strlen(StringTemp));
        }
    }

    buffer->set_clip(reinterpret_cast<RECT *>(self + 0x474));
    soft_update(reinterpret_cast<RECT *>(self + 0xA44));
}

/*
Purpose: Paint the player rows: the panel and title frames, the centred
         title, and one row per lobby slot record - the row frame with its
         ready glyph, the lock chip (the lock sprite when the row's chosen
         faction is already claimed elsewhere), the player's name or
         "Computer", the ": <faction>" suffix with a duplicate count, and the
         row, colour and name rectangles handed to the spot list as type-2
         and type-5 icons. Ends by renaming the left flat button for the
         unconnected case.
// ORIGINAL: 0x00480370 ?draw_player_names@NetWin@@QAEXXZ 0x00480370-0x004808F2 FILE
// size      1410 bytes
// prototype void (__thiscall ?draw_player_names@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   21
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047B380 0x004E25E0 0x005882F0 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005E2DD3 0x005E4B4A 0x005E4B9A 0x005FA960 0x005FAA90 0x006074E0 0x006169A0 0x00631A20 0x006453E0 0x00645470 0x00645660 0x0064FC88
// indirect  0x00669328
// Homed from src/recovered/units/00480370.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::draw_player_names() {
    spot_.kill_type(2);
    spot_.kill_type(5);
    int pid_idx = NetDaemonNet.pid_2_idx(NetWinPlayerId);
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xA64),
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = reinterpret_cast<RECT *>(self + 0xA54);
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0x414)));
    string_upper_slot()(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }

    unsigned char *record = reinterpret_cast<unsigned char *>(
        NetWinFactionTable + 0x2F1);
    NetWinSlotRecord *slot_record = NetWinSlotRecords + 1;
    RECT *rects = reinterpret_cast<RECT *>(self + 0xA74);
    for (int row = 0; slot_record < NetWinSlotRecords + 8;
         ++row, record += 0x17C, slot_record += 1) {
        if (*NetWinEighthSlotWord == 3 && faction_records_ != 0
            && row >= static_cast<unsigned char>(
                          static_cast<char *>(faction_records_)[0])) {
            break;
        }
        RECT *name_rect = &rects[row * 3];
        RECT *colour_rect = &rects[row * 3 + 3];
        RECT *row_rect = &rects[row * 3 + 6];

        // The ready glyph: element 2 when the record block's leading byte
        // says the slot holds a player.
        int ready = record[-0x177] != 0 ? 2 : 0;
        buffer->box_sprite(row_rect, &NetWinReadyPanels[ready]);

        if (*NetWinEighthSlotWord == 3 && record[-0x178] == 0xFF) {
            continue;
        }

        // The lock chip: the chosen faction's own id field against the
        // record's claim dword, only while the eighth word is 2 or 3.
        signed char mode = static_cast<signed char>(record[-0x175]);
        int locked;
        if (*NetWinEighthSlotWord == 2 || *NetWinEighthSlotWord == 3) {
            locked = 0;
            if (mode != -1) {
                unsigned long claimed =
                    *reinterpret_cast<unsigned long *>(record);
                unsigned long owned = *reinterpret_cast<unsigned long *>(
                    static_cast<char *>(faction_records_) + mode * 0x190
                    + 4);
                locked = owned != 0 && owned != claimed;
            }
        } else {
            locked = *reinterpret_cast<unsigned long *>(record) != 0;
        }
        Sprite &chip = locked ? g_IFACE_LOCK_SPRITES[1]
                              : g_IFACE_LOCK_SPRITES[0];
        chip.draw(buffer, chip.transparent_index(), colour_rect->left + 1,
                  colour_rect->top, 1, 1);

        // The name field: inset, then the row text.
        RECT field = *name_rect;
        field.left += 4;
        field.right -= 4;

        if (*NetWinEighthSlotWord == 0 || *NetWinEighthSlotWord == 1) {
            if (row + 1 == NetDaemonNet.pid_2_idx(NetWinPlayerId)) {
                Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
                arrow.draw(buffer, arrow.transparent_index(),
                           field.right - 0xA,
                           field.top
                               + (field.bottom - arrow.height() - field.top)
                                     / 2);
            }
        }

        buffer->set_font(find_font(0xE, 1), 0, 0, 0);
        buffer->set_text_color(get_player_color(row + 1), 0, 1, 1);
        buffer->set_clip(&field);

        StringTemp[0] = 0;
        if (record[-0x178] == 0xFF) {
            strcat(StringTemp, "Computer");
        } else {
            strcat(StringTemp,
                   NetDaemonNet.get_player_name(slot_record->id));
        }
        if (record[-0x175] != 0xFF) {
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp,
                   reinterpret_cast<const char *>(record - 0x173));
            if (slot_record > NetWinSlotRecords + 1) {
                // Count the static records whose name text matches, and
                // append the tally when there is more than one.
                int matches = 0;
                const char *name =
                    reinterpret_cast<const char *>(record - 0x173);
                const char *scan =
                    reinterpret_cast<const char *>(NetWinFactionTable + 0x17E);
                for (int seen = row; seen != 0; --seen) {
                    if (strcmp(name, scan) == 0) {
                        ++matches;
                    }
                    scan += 0x17C;
                }
                if (matches != 0) {
                    char tally[0xA];
                    _itoa(matches, tally, 10);
                    strcat(StringTemp, tally);
                }
            }
        }
        if (StringTemp != 0) {
            buffer->write_l(StringTemp, &field, strlen(StringTemp));
        }
        buffer->restore_clip();

        typedef void(__stdcall *rect_slot_fn)(void *, void *, void *);
        reinterpret_cast<rect_slot_fn *>(NetWinRectSlot)[0](
            name_rect, row_rect, &field);

        spot_.add(row, 2, row_rect->left, row_rect->top,
                  row_rect->right - row_rect->left,
                  row_rect->bottom - row_rect->top);
        if (*NetWinEighthSlotWord == 0 || *NetWinEighthSlotWord == 1) {
            spot_.add(row, 2, name_rect->left, name_rect->top,
                      name_rect->right - name_rect->left,
                      name_rect->bottom - name_rect->top);
        }
        spot_.add(row, 5, colour_rect->left, colour_rect->top,
                  colour_rect->right - colour_rect->left,
                  colour_rect->bottom - colour_rect->top);
        soft_update(&field);
    }

    if (*NetWinRulesGate == 0) {
        // Not connected: the left button takes the connect label.
        unsigned char color =
            NetWinFactionTable[pid_idx * 0x17C];
        const char *label = color != 0
            ? reinterpret_cast<const char *>(Labels.strings_ptr
                                             + 0x1054)
            : reinterpret_cast<const char *>(Labels.strings_ptr + 0x1050);
        flatButton1_.set_name(const_cast<char *>(StringTable.get(
            *reinterpret_cast<const int *>(label))));
        typedef void (__fastcall *button_slot_fn)(void *);
        vtable_slot<button_slot_fn>(&flatButton1_, 0xF8)(&flatButton1_);
    }
}

/*
Purpose: Paint the custom-rules panel: the panel and title frames, the
         centred title, and the eighteen RULES-text rows - each a checkbox
         glyph whose state is one ability/allowed bit of the custom-rules
         words (the bit per row in the image's own jump-table order), the
         row's label from the RULES text section, coloured by its state, and
         the row handed to the spot list as a type-1 icon when the gate word
         is set. The standing free-technology checkbox (row id 100, the
         inverted 0x10 bit) is drawn before the text opens; row 15 draws
         nothing; after row 7 the walk jumps to the second column.
// ORIGINAL: 0x0047F960 ?draw_custom_rules@NetWin@@QAEXXZ 0x0047F960-0x0047FE2E FILE
// size      1230 bytes
// prototype void (__thiscall ?draw_custom_rules@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005882F0 0x005BECA0 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005E2DD3 0x005E4B9A 0x005FA960 0x005FAA90 0x005FD530 0x005FD570 0x006169A0 0x006453E0 0x00645470
// Homed from src/recovered/units/0047f960.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::draw_custom_rules() {
    spot_.kill_type(1);
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xD04),
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = reinterpret_cast<RECT *>(self + 0xCF4);
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[0]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get(
               *reinterpret_cast<int *>(Labels.strings_ptr + 0xF9C)));
    string_upper_slot()(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }

    RECT *rules_rect = reinterpret_cast<RECT *>(self + 0xD14);
    buffer->box_sprite(rules_rect, &InterfaceBoxStyles[0]);
    buffer->box_sprite(reinterpret_cast<RECT *>(self + 0xD24),
                       &InterfaceBoxStyles[0]);

    // The four layout words double as the row rectangle: left/top advance per
    // row while right/bottom stay put, and write_l reads them as a RECT.
    int left = rules_rect->left - 0x14;
    int top = *reinterpret_cast<int *>(self + 0xD18);
    buffer->set_font(find_font(0xC, 1), 0, 0, 0);
    int right = (rules_rect->right - rules_rect->left) + left - 6;
    int bottom = top + 0x12;
    RECT row_rect;
    row_rect.left = left;
    row_rect.top = top;
    row_rect.right = right;
    row_rect.bottom = bottom;

    // The standing free-technology checkbox, spot id 100.
    int free_tech = (*NetWinRuleBitsC & 0x10) == 0 ? 2 : 0;
    Sprite &standing = g_IFACE_CLOSE_X_SPRITES[free_tech];
    standing.draw(buffer, standing.transparent_index(), left, top);
    if (*NetWinRulesGate != 0) {
        spot_.add(0x64, 1, left, top, right - left, bottom - top);
    }

    if (text_open(AlphaxFileID, "RULES") != 0) {
        return;
    }
    // The label lines; '#' lines are comments and do not name a row.
    char *line;
    do {
        line = text_get();
    } while (line[0] == '#');

    for (int row = 0; row <= 0x11; ++row) {
        unsigned int state;
        unsigned int bits = *NetWinRuleBitsA;
        switch (row) {
        case 0:
            state = bits & 0x800;
            break;
        case 1:
            state = bits & 0x2;
            break;
        case 2:
            state = bits & 0x8;
            break;
        case 3:
            state = bits & 0x4;
            break;
        case 4:
            state = bits & 0x1000;
            break;
        case 5:
            state = bits & 0x1;
            break;
        case 6:
            state = bits & 0x10;
            break;
        case 7:
            state = bits & 0x20;
            break;
        case 8:
            state = bits & 0x4000;
            break;
        case 9:
            state = bits & 0x200;
            break;
        case 10:
            state = bits & 0x40;
            break;
        case 11:
            state = (bits & 0x100) == 0;
            break;
        case 12:
            state = bits & 0x2000;
            break;
        case 13:
            state = bits & 0x8000;
            break;
        case 14:
            state = bits & 0x80;
            break;
        case 16:
            state = *NetWinRuleBitsB & 0x800000;
            break;
        case 17:
            state = *NetWinRuleBitsB & 0x1000000;
            break;
        default:
            // Row 15 draws nothing - the separator line in the RULES text.
            line = text_get();
            continue;
        }

        int on = state != 0 ? 2 : 0;
        Sprite &check = g_IFACE_CLOSE_X_SPRITES[on];
        check.draw(buffer, check.transparent_index(), left + 0x19, top);
        if (*NetWinRulesGate != 0) {
            spot_.add(row, 1, left + 0x19, top, right - left - 0x19,
                      bottom - top);
        }

        buffer->set_text_color(state != 0 ? 0xDE : 0xE4, -1, 1, 1);
        row_rect.left = left;
        row_rect.top = top;
        row_rect.right = right;
        row_rect.bottom = bottom;
        buffer->write_l(const_cast<char *>(line), &row_rect,
                        static_cast<int>(strlen(line)));

        if (row == 7) {
            // The second column: back to the top, 0x18A to the right.
            left += 0x18A;
            right += 0x18A;
            top -= 0xA0;
            bottom -= 0xA0;
        } else {
            top += 0x14;
            bottom += 0x14;
        }
        line = text_get();
    }
    text_close();
}

/*
Purpose: Paint the difficulty rows: one framed row per base faction record
         (the box sprite, the combo-arrow chip for the player's own slot, the
         difficulty name drawn left-aligned inside an inset field, and the
         row handed to the spot list as a type-4 icon). The icon walk stops
         early while the eighth slot word says 3 and the faction-record
         block's count byte covers fewer rows, and a 0xFF-prefixed record
         keeps its frame but no content.
// ORIGINAL: 0x00480180 ?draw_diffs@NetWin@@QAEXXZ 0x00480180-0x0048036F FILE
// size      495 bytes
// prototype void (__thiscall ?draw_diffs@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047B380 0x004E25E0 0x005882F0 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005E2DD3 0x005E4B9A 0x005FA960 0x005FAA90 0x006169A0 0x006453E0
// Homed from src/recovered/units/00480180.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::draw_diffs() {
    spot_.kill_type(4);
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    RECT *rect = reinterpret_cast<RECT *>(self + 0xB54);
    unsigned char *record =
        reinterpret_cast<unsigned char *>(NetWinFactionTable) + 0x179;
    for (int index = 0; record < reinterpret_cast<unsigned char *>(NetWinFactionTable)
                   + 0x179 + 7 * 0x17C;
         ++index, record += 0x17C, ++rect) {
        if (*NetWinEighthSlotWord == 3 && faction_records_ != 0
            && index >= static_cast<unsigned char>(
                            static_cast<char *>(faction_records_)[0])) {
            break;
        }
        buffer->box_sprite(rect, &InterfaceBoxStyles[0]);
        if (*NetWinEighthSlotWord == 3 && record[0] == 0xFF) {
            continue;
        }

        // The combo-arrow chip: the player's own slot, or - with the two ids
        // still equal - the row whose prefix is the 0xFF default.
        int slot = index + 1;
        if (slot == NetDaemonNet.pid_2_idx(NetWinPlayerId)
            || (NetWinPlayerId == NetWinOtherPlayerId
                && record[0] == 0xFF)) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(),
                       rect->right - 0xA,
                       rect->top
                           + (rect->bottom - arrow.height() - rect->top) / 2);
        }

        buffer->set_font(find_font(0xE, 1), 0, 0, 0);
        buffer->set_text_color(get_player_color(slot), 0, 1, 1);
        RECT field = *rect;
        field.left += 4;
        field.right -= 4;
        buffer->set_clip(&field);
        char *name = StringTable.get(
            reinterpret_cast<int>(NetWinDifficultyNames[record[2]]));
        if (name != 0) {
            buffer->write_l(name, &field, strlen(name));
        }
        buffer->restore_clip();
        spot_.add(index, 4, rect->left, rect->top,
                  rect->right - rect->left, rect->bottom - rect->top);
    }
}

/*
Purpose: Paint the faction rows: the same framed-row walk as draw_diffs one
         panel over - box sprite, chip for the player's own slot, the row
         clipped to a vertically inset field - with the row's text built from
         the allocated faction record the row's mode byte names (its noun at
         +0x62 of the 0x190-stride record) or the string table when the byte
         is -1, and the static table's title appended after ": " whenever it
         differs. Each row lands in the spot list as a type-3 icon.
// ORIGINAL: 0x0047FED0 ?draw_factions@NetWin@@QAEXXZ 0x0047FED0-0x00480173 FILE
// size      675 bytes
// prototype void (__thiscall ?draw_factions@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047B380 0x004E25E0 0x005882F0 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005E2DD3 0x005E4B9A 0x005FA960 0x005FAA90 0x006169A0 0x006453E0 0x00645470 0x00645660
// Homed from src/recovered/units/0047fed0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::draw_factions() {
    spot_.kill_type(3);
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    RECT *rect = reinterpret_cast<RECT *>(self + 0xAE4);
    unsigned char *record =
        reinterpret_cast<unsigned char *>(NetWinFactionTable) + 0x179 + 0x45;
    for (int index = 0;
         record < reinterpret_cast<unsigned char *>(NetWinFactionTable)
                   + 0x179 + 0x45 + 7 * 0x17C;
         ++index, record += 0x17C, ++rect) {
        if (*NetWinEighthSlotWord == 3 && faction_records_ != 0
            && index >= static_cast<unsigned char>(
                            static_cast<char *>(faction_records_)[0])) {
            break;
        }
        buffer->box_sprite(rect, &InterfaceBoxStyles[0]);
        if (*NetWinEighthSlotWord == 3 && record[-0x45] == 0xFF) {
            continue;
        }

        int slot = index + 1;
        int mode = static_cast<signed char>(record[-0x42]);
        if (slot == NetDaemonNet.pid_2_idx(NetWinPlayerId)
            || (NetWinPlayerId == NetWinOtherPlayerId
                && record[-0x45] == 0xFF)) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(),
                       rect->right - 0xA,
                       rect->top
                           + (rect->bottom - arrow.height() - rect->top) / 2);
        }

        buffer->set_font(find_font(0xE, 1), 0, 0, 0);
        buffer->set_text_color(get_player_color(slot), 0, 1, 1);
        RECT field = *rect;
        field.top += 4;
        field.bottom -= 0xB;
        buffer->set_clip(&field);

        StringTemp[0] = 0;
        const char *named;
        if (mode != -1) {
            named = static_cast<char *>(faction_records_) + mode * 0x190
                    + 0x62;
        } else {
            named = StringTable.get(
                *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4));
        }
        strcat(StringTemp, named);
        if (strcmp(named, reinterpret_cast<const char *>(record)) != 0) {
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp, reinterpret_cast<const char *>(record));
        }
        buffer->write_l(StringTemp, &field, strlen(StringTemp));
        buffer->restore_clip();
        spot_.add(index, 3, rect->left, rect->top,
                  rect->right - rect->left, rect->bottom - rect->top);
    }
}

// The exec callback every pick_* hands PopMenu::exec: the image passes its
// own modal_callback_daemon address, reached here through the fixed dialog.
static int __cdecl pick_daemon_callback() {
    return NetWinDialog.modal_callback_daemon();
}

// 0x00589330, unrecovered: fills a popup with the time-control list (the
// TimeControl entries' names off the rules text). Still its own preserved
// seed, so this file carries the forwarder shape the other unrecovered
// callees use.
static void sub_589330(void *popup) {
    typedef void(__cdecl *pending)(void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00589330))(popup);
}

// The shared tail of the eight pick_* dialogs: the mode byte is in, the
// daemon knows the local slot, and the ninth slot's block rides a 0x2F02
// packet to the other player.
static void pick_send_state(unsigned char *mode_byte) {
    NetDaemonNet.pid_2_idx(NetWinPlayerId);
    unsigned char packet[0x2E];
    *reinterpret_cast<unsigned short *>(packet) = 0x2F02;
    memcpy(packet + 0x10, NetWinNinthSlot, 0x18);
    NetDaemonNet.send(packet, 0x28, NetWinOtherPlayerId, 1);
}

// The option pick itself: mouse position to screen coordinates, then the
// popup run modally with the daemon callback; the answer is the chosen item
// index, negative when the pick was dismissed.
static int pick_answer(NetWin &self, PopMenu &pop_menu) {
    int x;
    int y;
    self.get_mouse_pos(&x, &y);
    self.client_to_screen(&x, &y);
    return pop_menu.exec(x, y, pick_daemon_callback);
}

/*
Purpose: The cloud-cover picker: a popup titled from the string table with
         the two cloud options, the answer stored in the ninth slot's cloud
         byte and sent to the peer.
// ORIGINAL: 0x0047F5F0 ?pick_clouds@NetWin@@QAEXXZ 0x0047F5F0-0x0047F91C;0x00657385-0x0065745D FILE
// size      1028 bytes
// prototype void (__thiscall ?pick_clouds@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047f5f0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_clouds() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    int index = 1;
    for (int offset = 0xF84; offset < 0xF90; offset += 4) {
        dialogs.item(StringTable.get(*reinterpret_cast<int *>(
                         Labels.strings_ptr + offset)),
                     index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[0xA] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[0xA]);
    }
}

/*
Purpose: The ocean-coverage picker: the same popup walk with the three ocean
         options; the answer lands in the ninth slot's ocean byte.
// ORIGINAL: 0x0047EC60 ?pick_ocean@NetWin@@QAEXXZ 0x0047EC60-0x0047EF8C;0x006570FD-0x006571D5 FILE
// size      1028 bytes
// prototype void (__thiscall ?pick_ocean@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047ec60.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_ocean() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    int index = 1;
    for (int offset = 0xF60; offset < 0xF6C; offset += 4) {
        dialogs.item(StringTable.get(*reinterpret_cast<int *>(
                         Labels.strings_ptr + offset)),
                     index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[7] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[7]);
    }
}

/*
Purpose: The erosion picker: the same popup walk with the three erosion
         options; the answer lands in the ninth slot's erosion byte.
// ORIGINAL: 0x0047EF90 ?pick_erosion@NetWin@@QAEXXZ 0x0047EF90-0x0047F2BC;0x006571D5-0x006572AD FILE
// size      1028 bytes
// prototype void (__thiscall ?pick_erosion@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047ef90.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_erosion() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    int index = 1;
    for (int offset = 0xF6C; offset < 0xF78; offset += 4) {
        dialogs.item(StringTable.get(*reinterpret_cast<int *>(
                         Labels.strings_ptr + offset)),
                     index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[8] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[8]);
    }
}

/*
Purpose: The native-life picker: the same popup walk with the three fungal
         options; the answer lands in the ninth slot's native byte.
// ORIGINAL: 0x0047F2C0 ?pick_native@NetWin@@QAEXXZ 0x0047F2C0-0x0047F5EC;0x006572AD-0x00657385 FILE
// size      1028 bytes
// prototype void (__thiscall ?pick_native@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047f2c0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_native() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    int index = 1;
    for (int offset = 0xF90; offset < 0xF9C; offset += 4) {
        dialogs.item(StringTable.get(*reinterpret_cast<int *>(
                         Labels.strings_ptr + offset)),
                     index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[9] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[9]);
    }
}

/*
Purpose: The planet-size picker: a popup with the title and one entry per
         name in the 0x80-stride name block at 0x6614, counted by the word at
         0x80A6F0; the answer lands in the ninth slot's planet byte.
// ORIGINAL: 0x0047E930 ?pick_planet_size@NetWin@@QAEXXZ 0x0047E930-0x0047EC60;0x00657025-0x006570FD FILE
// size      1032 bytes
// prototype void (__thiscall ?pick_planet_size@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047e930.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_planet_size() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    int count = *NetWinPlanetSizeCount;
    for (int index = 0; index < count; ++index) {
        dialogs.item(reinterpret_cast<char *>(
                         reinterpret_cast<unsigned char *>(this) + 0x6614
                         + index * 0x80),
                     index + 1);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[6] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[6]);
    }
}

/*
Purpose: The player-count picker: entries numbered max(2, lobby count) to 7,
         the entry itself the answer; the ninth slot's player byte stores
         count - 1.
// ORIGINAL: 0x0047E010 ?pick_num_players@NetWin@@QAEXXZ 0x0047E010-0x0047E33B;0x00656D9D-0x00656E75 FILE
// size      1027 bytes
// prototype void (__thiscall ?pick_num_players@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550 0x0064FC88
// Homed from src/recovered/units/0047e010.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_num_players() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    int first = *NetWinPlayerCount;
    if (first < 2) {
        first = 2;
    }
    for (int count = first; count < 8; ++count) {
        char number[0xA];
        StringTemp[0] = 0;
        _itoa(count, number, 10);
        strcat(StringTemp, number);
        dialogs.item(StringTemp, count);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[1] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[1]);
    }
}

/*
Purpose: The global-difficulty picker: a popup with the title and the six
         difficulty names off alpha's table; the ninth slot's global byte
         stores the answer.
// ORIGINAL: 0x0047E340 ?pick_global_diff@NetWin@@QAEXXZ 0x0047E340-0x0047E640;0x00656E75-0x00656F4D FILE
// size      984 bytes
// prototype void (__thiscall ?pick_global_diff@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   26
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047e340.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_global_diff() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    Dialogs dialogs;
    dialogs.item(StringTable.get(
                     *reinterpret_cast<int *>(Labels.strings_ptr + 0xFE4)),
                 0);
    for (int index = 0; index < 6; ++index) {
        dialogs.item(StringTable.get(reinterpret_cast<int>(
                         NetWinDifficultyNames[index])),
                     index + 1);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[2] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[2]);
    }
}

/*
Purpose: The time-control picker: the popup is filled by the unrecovered
         list builder at 0x589330 (the TimeControl names), then the same
         modal walk; the ninth slot's time byte stores the answer.
// ORIGINAL: 0x0047E640 ?pick_time@NetWin@@QAEXXZ 0x0047E640-0x0047E924;0x00656F4D-0x00657025 FILE
// size      956 bytes
// prototype void (__thiscall ?pick_time@NetWin@@QAEXXZ)(NetWin* this)
// callers   1   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x004066C0 0x00406820 0x00406880 0x00406910 0x004E25E0 0x00588330 0x0059D250 0x0059D3A0 0x005D45B0 0x005D4DD0 0x005E3820 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005FA870 0x00600F00 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00630080 0x00645550
// Homed from src/recovered/units/0047e640.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::pick_time() {
    PopMenu pop_menu;
    PullDown pull_down;
    pop_menu.init();
    sub_589330(&pop_menu);
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinNinthSlot[3] = static_cast<unsigned char>(answer - 1);
        pick_send_state(&NetWinNinthSlot[3]);
    }
}

/*
Purpose: The modal pump's callback: drain the daemon's receive queue (one
         poll, then a second, then the rest), poke the dialog's own slot-12
         virtual so the waiting panel repaints, and - if the readiness scan
         has not already passed - walk the static faction records looking for
         an empty slot and answer whether the wait should end.
// ORIGINAL: 0x00483190 ?modal_callback@NetWin@@QAEHXZ 0x00483190-0x00483233 FILE
// size      164 bytes
// prototype int (__thiscall ?modal_callback@NetWin@@QAEHXZ)(NetWin* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530320
// indirect  0x004831C1
// Homed from src/unrecovered/00483190.cpp.
Return Value: 1 to keep waiting, 0 when the dialog may proceed
Status: Coverage transcription - unclaimed body
*/
int NetWin::modal_callback() {
    if (NetDaemonNet.receive() != 0) {
        if (NetDaemonNet.receive() != 0) {
            do {
            } while (NetDaemonNet.receive() != 0);
        }
        typedef void (__fastcall *self_slot_fn)(void *);
        vtable_slot<self_slot_fn>(this, 0x30)(this);
    }
    int result = 1;
    if (field_7614_ == 0) {
        field_7618_ = 1;
        if (*NetWinPlayerCount >= 1) {
            char *record = NetWinFactionTable + 0x17A;
            int index = 1;
            do {
                if (*record == 0) {
                    field_7618_ = 0;
                }
                index++;
                record += 0x17C;
            } while (index <= *NetWinPlayerCount);
        }
        if (field_7618_ != 0) {
            field_7614_ = 1;
        }
    }
    if (field_7614_ != 0 || ExitTurnLoop != 0) {
        result = 0;
    }
    return result;
}

/*
Purpose: The fixed dialog's twin of modal_callback: the same daemon drain and
         slot-12 dispatch, with the readiness scan and the answer running
         through NetWinDialog's own 0x7614/0x7618 words (the image's fixed
         0x811D0C/0x811D10 slots) instead of the receiver's.
// ORIGINAL: 0x00483240 ?modal_callback_daemon@NetWin@@QAAHXZ 0x00483240-0x004832CF FILE
// symbol    ?modal_callback_daemon@NetWin@@QAEHXZ
// size      143 bytes
// prototype int (__thiscall ?modal_callback_daemon@NetWin@@QAAHXZ)(NetWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00530320
// indirect  0x00483274
// Homed from src/unrecovered/00483240.cpp.
Return Value: 1 to keep waiting, 0 when the dialog may proceed
Status: Coverage transcription - unclaimed body
*/
int NetWin::modal_callback_daemon() {
    if (NetDaemonNet.receive() != 0) {
        if (NetDaemonNet.receive() != 0) {
            do {
            } while (NetDaemonNet.receive() != 0);
        }
        typedef void (__fastcall *dialog_slot_fn)(void *);
        vtable_slot<dialog_slot_fn>(&NetWinDialog, 0x30)(&NetWinDialog);
    }
    if (NetWinDialog.field_7614_ != 0) {
        return 0;
    }
    NetWinDialog.field_7618_ = 1;
    int count = *NetWinPlayerCount;
    if (count >= 1) {
        int all_used = 1;
        char *record = NetWinFactionTable + 0x17A;
        do {
            if (*record == 0) {
                all_used = 0;
            }
            record += 0x17C;
        } while (--count != 0);
        NetWinDialog.field_7618_ = all_used;
        if (all_used == 0) {
            if (ExitTurnLoop != 0) {
                return 0;
            }
            return 1;
        }
    }
    NetWinDialog.field_7614_ = 1;
    return 0;
}

/*
Purpose: The chat list's string callback: the message list repaints one line's
         text colour from the faction colour of the id handed in. The canvas
         is picked by the image's relocated-address idiom - the guard operand
         is this dialog's own stringBox_ address, so the branch survives as
         the neg/sbb/and form the image emits - and the paint target sits
         0x1184 bytes into the dialog.
// ORIGINAL: 0x004832D0 ?string_callback@NetWin@@QAAXHH@Z 0x004832D0-0x00483302 FILE
// symbol    ?string_callback@NetWin@@QAEXHH@Z
// size      50 bytes
// prototype void (__thiscall ?string_callback@NetWin@@QAAXHH@Z)(NetWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047B380 0x005DACB0
// Homed from src/recovered/004832d0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body (the preserved seed keeps its
        own BYTE_EXACT claim until this converges)
*/
void NetWin::string_callback(int payload, int) {
    Buffer *canvas = &NetWinDialog.stringBox_ != 0
        ? reinterpret_cast<Buffer *>(reinterpret_cast<unsigned char *>(
                                          &NetWinDialog) + 0x1184)
        : 0;
    canvas->set_text_color(NetWinDialog.get_player_color(payload), 0, 1, 1);
}

/*
Purpose: Route one dialog key press: Enter sends the chat line the edit box
         holds (the daemon maps the sender's process id, the message goes out
         as opcode 0x1E00, the edit box clears), Escape tears the session
         dialogs down the same way close() does and answers 1, everything
         else goes to the edit box's own slot-26 key handling.
// ORIGINAL: 0x00483310 ?on_key_down@NetWin@@QAEHH@Z 0x00483310-0x00483414 FILE
// size      260 bytes
// prototype int (__thiscall ?on_key_down@NetWin@@QAEHH@Z)(NetWin* this, int)
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E25E0 0x00593190 0x005FA830 0x0060E7C0 0x00614F30 0x006151E0 0x00629210 0x00644EF2
// indirect  0x00483324
// Homed from src/recovered/units/00483310.cpp.
Return Value: 1 for the keys the dialog consumes, else the edit box's answer
Status: Coverage transcription - unclaimed body (the preserved seed keeps its
        own BYTE_EXACT claim until this converges)
*/
int NetWin::on_key_down(int key) {
    if (key != 0xD) {
        if (key != 0x1B) {
            typedef int (__fastcall *edit_box_key_fn)(void *, int);
            return vtable_slot<edit_box_key_fn>(&editBox_, 0x68)(&editBox_,
                                                                 key);
        }
        typedef void (__fastcall *self_slot_fn)(void *);
        vtable_slot<self_slot_fn>(this, 0xE8)(this);
        if (faction_records_ != 0) {
            free(faction_records_);
            faction_records_ = 0;
        }
        if (faction_records_copy_ != 0) {
            free(faction_records_copy_);
            faction_records_copy_ = 0;
        }
        field_761C_ = 0xFFFFFFFF;
        field_7620_ = 0xFFFFFFFF;
        spot_.shutdown();
        string_box_close(&stringBox_);
        editBox_.close();
        checkBox_.close();
        typedef void (__fastcall *flat_button_slot_fn)(void *);
        vtable_slot<flat_button_slot_fn>(&flatButton1_, 0x168)(&flatButton1_);
        return 1;
    }
    NetDaemonNet.pid_2_idx(NetWinPlayerId);
    message_chat(0x1E00, 0,
                 reinterpret_cast<char *>(this) + 0x42F4);
    edit_box_set_text(&editBox_, 0);
    return 1;
}

/*
Purpose: Format one chat line and hand it to the message list: the sender's
         name (through the daemon's name table, upper-cased in place), then
         ": ", a space, and the message text.
// ORIGINAL: 0x00483420 ?new_chat@NetWin@@QAEXPADH@Z 0x00483420-0x004834C4 FILE
// size      165 bytes
// prototype void (__thiscall ?new_chat@NetWin@@QAEXPADH@Z)(NetWin* this, char*, int)
// callers   2   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00593190 0x00629710 0x00631A20 0x00645470 0x0066931C
// Homed from src/unrecovered/00483420.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void NetWin::new_chat(char *message, int slot) {
    char text[0x100];
    text[0] = 0;
    strcat(text, NetDaemonNet.get_player_name(NetWinSlotRecords[slot].id));
    string_upper_slot()(text);
    strcat(text, ": ");
    strcat(text, " ");
    strcat(text, message);
    stringBox_.add(text, slot, 1);
}

/*
Purpose: Destroy the dialog's CheckBox view - the CheckBox subobject whose
         vbtable puts its GraphicWin base at +0x1C and its Dialog base at
         +0xA34 - then the Dialog and the GraphicWin, in that order.
// ORIGINAL: 0x004837F0 ?UNK4@NetWin@@QAEXXZ 0x004837F0-0x00483813 FILE
// size      35 bytes
// prototype void (__thiscall ?UNK4@NetWin@@QAEXXZ)(NetWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x0060E740
// Homed from src/recovered/004837f0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body (the preserved seed keeps its
        own BYTE_EXACT claim until this converges)
*/
void NetWin::UNK4() {
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    reinterpret_cast<CheckBox *>(self + 0x1C)->~CheckBox();
    reinterpret_cast<Dialog *>(self + 0xA34)->~Dialog();
    reinterpret_cast<GraphicWin *>(self + 0x1C)->~GraphicWin();
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
