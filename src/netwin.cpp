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
// ORIGINAL: 0x004837C0 ??_GNetWin@@UAEPAXI@Z 0x004837C0-0x004837E1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004837C0-0x004837E1", "Pinned class constructor ??0NetWin@@QAE@XZ at 0x00481C50 stores original class/view table VA 0x66ccec at 0x00481CD6 (mov dword ptr [esi], 0x66ccec); the current genuine owner vftable ??_7NetWin@@6BWin@@@ selects generated ??_GNetWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ENetWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GNetWin@@UAEPAXI@Z at 0x004837C0 calls the source-mapped ??1NetWin@@QAE@XZ at 0x004834E0, then tests byte ptr [ebp + 8], 1; je 0x4837da bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/netwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original NetWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GNetWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x0047AFF0 _$E21 0x0047AFF0-0x0047B006 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682560 selects initializer0x47aff0..0x47b006, registering exit0x47b010..0x47b01a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x80a6f8 bound to?NetWinDialog@@3VNetWin@@A.", "Original startup instruction evidence: [[\"0x47aff0\", \"mov\", \"ecx, 0x80a6f8\"], [\"0x47aff5\", \"call\", \"0x481c50\"], [\"0x47affa\", \"push\", \"0x47b010\"], [\"0x47afff\", \"call\", \"0x645398\"], [\"0x47b004\", \"pop\", \"ecx\"], [\"0x47b005\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x47b010\", \"mov\", \"ecx, 0x80a6f8\"], [\"0x47b015\", \"jmp\", \"0x4834e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/netwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?NetWinDialog@@3VNetWin@@A\", \"??0NetWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?NetWinDialog@@3VNetWin@@A\", \"??1NetWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2cc; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?NetWinDialog@@3VNetWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E8
// ORIGINAL: 0x0047B010 _$E19 0x0047B010-0x0047B01A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682560 selects initializer0x47aff0..0x47b006, registering exit0x47b010..0x47b01a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x80a6f8 bound to?NetWinDialog@@3VNetWin@@A.", "Original startup instruction evidence: [[\"0x47aff0\", \"mov\", \"ecx, 0x80a6f8\"], [\"0x47aff5\", \"call\", \"0x481c50\"], [\"0x47affa\", \"push\", \"0x47b010\"], [\"0x47afff\", \"call\", \"0x645398\"], [\"0x47b004\", \"pop\", \"ecx\"], [\"0x47b005\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x47b010\", \"mov\", \"ecx, 0x80a6f8\"], [\"0x47b015\", \"jmp\", \"0x4834e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/netwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?NetWinDialog@@3VNetWin@@A\", \"??0NetWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?NetWinDialog@@3VNetWin@@A\", \"??1NetWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2cc; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?NetWinDialog@@3VNetWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E6
#include "stdafx.h"
#include "interface_fonts.h"
#include "netwin.h"
#include <cstring>

#include "alpha.h"     // read_faction
#include "buffer.h"    // the local Buffer load_pcx / get_pixel run on
#include "dialog.h"    // CheckBox's virtual Dialog base is at CheckBox-relative +0xA34.
#include "dialogs.h"   // Dialogs - the pick_* option lists
#include "faction.h"   // Player - the record read_faction fills
#include "game.h"      // ExitTurnLoop, the modal callbacks' stop flag
#include "general.h"   // checksum_password, parse_says and shared helpers
#include "netdaemon.h" // NetDaemonNet, message_chat - the pump and the chat send
#include "popmenu.h"   // PopMenu - the pickers' popup and its exec
#include "popup.h"     // Popup - the password modal and typed teardown
#include "pulldown.h"  // PullDown - the pickers' second pane
#include "random.h"    // Random - deterministic lobby faction selection
#include "strings.h"   // StringTemp, the pcx path the records name
#include "text.h"      // text_open / text_get / text_item / text_close

#include "xpops.h"     // x_pop_caption_label for lobby validation prompts
#include <stdlib.h>      // _itoa

// MessageDataRecord and NetDaemon::process_message establish these shared
// packet-prefix positions. F0D leaves +2..3 and +C unwritten; this describes
// only the 16-byte prefix, not the complete allocation or payload extent.
struct NetWinFactionDataPrefix {
    uint16_t opcode_;
    uint8_t unresolved_2_[2];
    int32_t faction_;
    uint32_t timestamp_;
    uint32_t sequence_;
};
static_assert(sizeof(NetWinFactionDataPrefix) == 0x10,
              "faction-data packet prefix size");
static_assert(offsetof(NetWinFactionDataPrefix, opcode_) == 0,
              "faction-data packet opcode offset");
static_assert(offsetof(NetWinFactionDataPrefix, unresolved_2_) == 2,
              "faction-data packet unresolved bytes offset");
static_assert(offsetof(NetWinFactionDataPrefix, faction_) == 4,
              "faction-data packet faction offset");
static_assert(offsetof(NetWinFactionDataPrefix, timestamp_) == 8,
              "faction-data packet timestamp offset");
static_assert(offsetof(NetWinFactionDataPrefix, sequence_) == 0x0C,
              "faction-data packet sequence offset");

// A borrowed byte view writes only the fields initialized by the F0D sender.
// It does not construct the prefix or initialize its unresolved bytes.
class NetWinFactionDataPacketView {
public:
    explicit NetWinFactionDataPacketView(char *bytes) : bytes_(bytes) {}

    void set_opcode(uint16_t value) const {
        std::memcpy(bytes_ + offsetof(NetWinFactionDataPrefix, opcode_), &value,
                    sizeof(value));
    }
    void set_faction(int32_t value) const {
        std::memcpy(bytes_ + offsetof(NetWinFactionDataPrefix, faction_), &value,
                    sizeof(value));
    }
    void set_timestamp(uint32_t value) const {
        std::memcpy(bytes_ + offsetof(NetWinFactionDataPrefix, timestamp_), &value,
                    sizeof(value));
    }
    char *payload() const {
        return bytes_ + sizeof(NetWinFactionDataPrefix);
    }

private:
    char *bytes_;
};
static_assert(sizeof(NetWinFactionDataPacketView) == sizeof(void *),
              "faction-data packet view is a borrowed pointer");

// Compiler-generated non-EH array-constructor iterator for the stack-local
// NetWinConstructedLobbyRow array. The original callback returns its receiver;
// ordinary constructor/array controls reproduce both original bodies.
// Decrement/sign-test count wrap, ECX element callback and lack of unwind
// are preserved by compiler construction, not a handwritten callback loop.
// This binds current physical COFF output, not retained linked selection.
// ORIGINAL: 0x00402EE0 sub_402ee0 0x00402EE0-0x00402F08 FILE BYTE_EXACT
// symbol    ??_H@YGXPAXIHP6EX0@Z@Z
// size      40 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00402EF7
// Historical source mapping: 0x00402EE0 sub_402ee0 -> _sub_402ee0


// These application entry points are implemented in game.cpp and are also
// used by Console.
int __cdecl load_game(int save_slot, int faction_id);
int __cdecl load_map();


// THE SETUP DIALOG, real storage at the image's fixed address - see
// netwin.h. The image reaches it from an unannotated initialiser at
// 0x007AFF0 (`mov ecx, 0x80a6f8 / call 0x481c50`); this build runs the same
// recovered constructor through its own startup instead.
// RECOVERY_OBJECT: v1 0x0080A6F8 ?NetWinDialog@@3VNetWin@@A
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

void NetWinSessionBlock::receive_wire_block(const unsigned char *bytes) {
    std::memcpy(this, bytes, sizeof(*this));
}

void NetWinSessionOptionsState::receive_wire_block(const unsigned char *bytes) {
    std::memcpy(this, bytes, sizeof(*this));
}

void NetWinSessionBlock::write_row8_prefix(uint8_t value) {
    unsigned char *bytes = reinterpret_cast<unsigned char *>(this);
    std::memcpy(bytes + offsetof(NetWinLobbyRow, prefix_), &value, sizeof(value));
}

char *NetWinSessionBlock::row8_leader_name() {
    unsigned char *bytes = reinterpret_cast<unsigned char *>(this);
    return reinterpret_cast<char *>(bytes + offsetof(NetWinLobbyRow, leader_name_));
}

char *NetWinSessionBlock::row8_noun_faction() {
    unsigned char *bytes = reinterpret_cast<unsigned char *>(this);
    return reinterpret_cast<char *>(bytes + offsetof(NetWinLobbyRow, noun_faction_));
}

// Native pick_pass passes these distinct zero-filled input bytes as X_pop_ask
// initial text. Only one-byte start bindings are claimed; capacity and any
// runtime writers remain unknown.
// RECOVERY_OBJECT: v1 0x00811E30 ?NetWinPasswordPromptInitialText@@3DA
char NetWinPasswordPromptInitialText;
// RECOVERY_OBJECT: v1 0x00811E34 ?NetWinPasswordVerifyInitialText@@3DA
char NetWinPasswordVerifyInitialText;

// Pinned original faction-section/path strings; real source storage replaces
// pointers into the old executable. Separate CUSTOMFACTIONS copies stay distinct.
// Original string start0x00686908; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686908 _NetWinSectionCustomCount
static char NetWinSectionCustomCount[] = "CUSTOMFACTIONS";
// Original string start0x00686918; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686918 _NetWinSectionFactions
static char NetWinSectionFactions[] = "FACTIONS";
// Original string start0x00686924; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686924 _NetWinSectionNewFactions
static char NetWinSectionNewFactions[] = "NEWFACTIONS";
// Original string start0x00686930; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686930 _NetWinSectionCustom
static char NetWinSectionCustom[] = "CUSTOMFACTIONS";
// Original string start0x00686940; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686940 _NetWinPcxSuffix
static char NetWinPcxSuffix[] = ".pcx";
// Original string start0x00686948; pinned file-backed bytes and terminating NUL verified.
// RECOVERY_OBJECT: v1 0x00686948 _NetWinDefaultPcx
static char NetWinDefaultPcx[] = "faction.pcx";
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

// 0x0093D4F8 is AlphaNet::player_count_ inside NetDaemonNet. Lobby scans and
// modal callbacks read it at their original sites, including between callbacks.

// Original 0x0090DB98 begins the eight NetWinLobbyRows (0x17C stride).
// Original 0x0093D51C is NetDaemonNet.process_slots_[0].pid_, not a
// separately allocated slot table: AlphaNet starts at 0x0093CD90, its
// process_slots_ at +0x76C and each slot's pid_ at +0x20.
// The unchecked readiness/owner scans below can leave the eight-row array.
// In the original, index 8's ready-byte access aliases byte 1 of the existing
// mode_ dword at 0x0090E779; its owner dword aliases rule_bits_b_ in the
// existing session-options record at 0x0090E8F0. The scenario text's first
// byte is separately named scenario_text_wire_tail_[0].
// Those observations do not prove a ninth row or source-object adjacency;
// preserving the original abnormal-count effects remains a layout obligation.

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
    if (message_faction_data != 0 && NetDaemonNet.local_player_id() == NetDaemonNet.host_player_id()) {
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
        faction_records_ = static_cast<NetWinFactionRecord *>(mem_get(size));
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

    faction_records_ =
        static_cast<NetWinFactionRecord *>(mem_get(record_size));
    faction_records_copy_ =
        static_cast<NetWinFactionRecord *>(mem_get(record_size));

    // The 7 base factions land first, at the front of the block. The image
    // counts the loop DOWN (`mov [ebp-0x10],7 / dec / jne`), which is why the
    // flag offset climbs in the third clause of the same for.
    if (text_open(AlphaxFileID, NetWinSectionFactions) != 0) {
        return;
    }
    NetWinFactionRecord *records;
    int flag_offset;
    int name_offset;
    int record_index;
    records = faction_records_;
    records[0].record_count_ = static_cast<uint8_t>(record_count);
    records = faction_records_;
    records[0].record_type_ = 7;
    flag_offset = 0;
    for (record_index = 7; record_index != 0;
         --record_index, flag_offset += 0x190) {
        records = faction_records_;
        records[flag_offset / 0x190].active_ = 1;
        text_get();
        records = faction_records_;
        strcpy(records[flag_offset / 0x190].filename_, text_item());
    }

    // The 7 expansion factions follow, one 0x190 stride further on: the flag
    // offset walks 0..0xAF0 while the name offset starts at 0xAF0.
    if (text_open(AlphaxFileID, NetWinSectionNewFactions) != 0) {
        return;
    }
    name_offset = 0xaf0;
    for (flag_offset = 0; flag_offset < 0xaf0; flag_offset += 0x190) {
        records = faction_records_;
        records[flag_offset / 0x190].active_ = 1;
        text_get();
        records = faction_records_;
        strcpy(records[name_offset / 0x190].filename_, text_item());
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
            records = faction_records_;
            records[flag_offset / 0x190].active_ = 1;
            text_get();
            records = faction_records_;
            strcpy(records[name_offset / 0x190].filename_, text_item());
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
        records = faction_records_;
        if (text_open(records[record_index].filename_,
                      records[record_index].filename_) != 0) {
            return;
        }
        records = faction_records_;
        records[record_index].active_ = 1;
        // text_open returned 0 to get here, and the image stores THAT value
        // at +4 as a dword rather than re-deriving it.
        records[record_index].owner_ = 0;

        text_get();
        text_item();
        text_item();
        strncpy(records[record_index].leader_name_, text_item(), 0x18);
        records = faction_records_;
        records[record_index].leader_name_[0x17] = 0;

        StringTemp[0] = 0;
        records = faction_records_;
        strcat(StringTemp, records[record_index].filename_);
        strcat(StringTemp, NetWinPcxSuffix);

        if (buffer.load_pcx(StringTemp, 0, 0xa, 0xec) != 0) {
            buffer.load_pcx(NetWinDefaultPcx, 0, 0xa, 0xec);
        }
        records = faction_records_;
        records[record_index].color_ =
            static_cast<uint8_t>(buffer.get_pixel(4, 0x2f3));

        // read_faction fills the Player from the faction's own file; the
        // strings it read are then copied back into the record at +0x22,
        // +0x3A, +0x62, +0x7A, +0xFA and +0x112, and the leader-gender flag
        // crosses its 0x190-byte row stride.
        strcpy(player.filename, records[record_index].filename_);
        strcpy(player.search_key, records[record_index].filename_);
        read_faction(&player, 0);
        records = faction_records_;
        strcpy(records[record_index].leader_name_, player.name_leader);
        strcpy(records[record_index].formal_faction_name_,
               player.formal_name_faction);
        strcpy(records[record_index].noun_faction_, player.noun_faction);
        strcpy(records[record_index].adjective_faction_,
               player.adj_name_faction);
        strcpy(records[record_index].leader_title_, player.title_leader);

        // The image copies the 0x80-byte adjective at +0x112 and then stores
        // the gender byte at +0x192. Both expressions cross the 0x190 row
        // stride (the latter by two bytes); keep them in the allocation's
        // byte domain rather than inventing a larger record member.
        unsigned char *const allocation_bytes =
            reinterpret_cast<unsigned char *>(records);
        strcpy(reinterpret_cast<char *>(allocation_bytes + offset + 0x112),
               player.adj_leader);
        allocation_bytes[offset + 0x192] =
            static_cast<unsigned char>(player.is_leader_female);
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
            int bound = NetDaemonNet.player_count_ + 1;
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
// ORIGINAL: 0x0047FE80 ?UNK3@NetWin@@QAEXXZ 0x0047FE80-0x0047FEC6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0047fe80.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// size      70 bytes
// prototype void (__thiscall ?UNK3@NetWin@@QAEXXZ)(NetWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E2DD3
// Historical scaffold: git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0047fe80.cpp.
// Its complete 70-byte span ends at FEC6; the prior owner annotation ended
// at FEC5. Retain the full historical extent and restoration obligation.
Return Value: n/a
Status: Coverage transcription; the historical claim now lives on this owner,
        not on a duplicate scaffold. Current exactness remains unproved.
*/
void NetWin::UNK3() {
    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->box_sprite(&setup_rects_[45],
                       &InterfaceBoxStyles[3]);
    buffer->box_sprite(&setup_rects_[44],
                       &InterfaceBoxStyles[0]);
    buffer->box_sprite(&setup_rects_[43],
                       &InterfaceBoxStyles[0]);
}

/*
Purpose: Tear the setup dialog's session state down: drop both faction-record
         blocks, reset the two dialog words at 0x761C/0x7620 to -1, and close
         the embedded controls - the spot list, the message list, the edit
         box, the check box and the front flat button's slot-90 reset.
// ORIGINAL: 0x00481D10 ?close@NetWin@@QAEXXZ 0x00481D10-0x00481D9C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/00481d10.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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
// Native 0x004834E0: explicit resource and control shutdown precedes normal
// compiler-owned teardown of the typed members and GraphicWin base.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004834E0 ??1NetWin@@QAE@XZ 0x004834E0-0x004837B4;0x00656650-0x0065673F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004834e0.cpp", "recovery/tasks/netwin-owner-coverage.json"], "reason": "Recover true destructor with typed automatic member and base teardown, retaining observed explicit frees, state resets and close calls. Native baseline/candidate controls and complete isolated application compile/link passed; no matching or game acceptance claim."}
// symbol    ??1NetWin@@UAE@XZ
NetWin::~NetWin() {
    if (faction_records_ != nullptr) {
        free(faction_records_);
        faction_records_ = nullptr;
    }
    if (faction_records_copy_ != nullptr) {
        free(faction_records_copy_);
        faction_records_copy_ = nullptr;
    }
    field_761C_ = -1;
    field_7620_ = -1;

    spot_.shutdown();
    stringBox_.close();
    editBox_.close();
    checkBox_.close();
    flatButton1_.close();
}

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
    stringBox_.close();
    editBox_.close();
    checkBox_.close();
    flatButton1_.close();
}

/*
Purpose: Copy one faction record's name/title block into the lobby row
         carried by the received packet: six text fields land in its named
         members and the leader-gender byte in tail_[0]. Slot -1 instead
         re-reads just noun_faction_ through the string table, for the row
         the scenario file's own faction entry describes.
// ORIGINAL: 0x00481DA0 ?fill_player@NetWin@@QAEXPAUPlayerData@@H@Z 0x00481DA0-0x00481E8D FILE
// symbol    ?fill_player@NetWin@@QAEXVNetWinLobbyRowView@@H@Z
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
void NetWin::fill_player(NetWinLobbyRowView lobby_row, int slot) {
    if (slot == -1) {
        lobby_row.noun_faction()[0] = 0;
        strcat(lobby_row.noun_faction(),
               label_get(0xFE4 / 4));
        return;
    }
    const NetWinFactionRecord &record = faction_records_[slot];
    strcpy(lobby_row.leader_name(), record.leader_name_);
    strcpy(lobby_row.formal_faction_name(), record.formal_faction_name_);
    strcpy(lobby_row.noun_faction(), record.noun_faction_);
    strcpy(lobby_row.adjective_faction(), record.adjective_faction_);
    strcpy(lobby_row.leader_title(), record.leader_title_);
    const unsigned char *const allocation_bytes =
        reinterpret_cast<const unsigned char *>(faction_records_);
    strcpy(lobby_row.adjective_leader(),
           reinterpret_cast<const char *>(allocation_bytes + slot * 0x190
                                          + 0x112));
    lobby_row.set_tail_0(allocation_bytes[slot * 0x190 + 0x192]);
}

// 0x0090E778 / 0x0090E79C - the two words beside the seven records
// config_players seeds: the eighth slot's dword and its mode byte.
// 0x0090E8E0 - the ninth slot's colour/mode prefix: 0xFF, the colour byte
// read from 0x93D4B0, 2 - with the zeroed pair at +4/+5, the mode bytes 2 at
// +6..+0xA and the 0xFF at +0xB.
// The original byte read at 0x0090E8E3 is options.time_control_ (+3), not a
// separate ninth-row object.

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
    unsigned char color = reinterpret_cast<const unsigned char *>(
        &NetDaemonNet.allocation_node().max_players_)[0];
    NetWinSessionData.mode_ = 0;
    NetWinSessionData.scenario_text_wire_tail_[0] = 0;
    NetWinSessionOptions.status_ = 0xFF;
    NetWinSessionOptions.option_4_ = 0;
    NetWinSessionOptions.option_5_ = 0;
    NetWinSessionOptions.reserved_B_ = 0xFF;
    NetWinSessionOptions.player_count_ = color;
    NetWinSessionOptions.difficulty_ = 2;
    NetWinSessionOptions.planet_size_ = 2;
    NetWinSessionOptions.ocean_coverage_ = 2;
    NetWinSessionOptions.erosion_ = 2;
    NetWinSessionOptions.native_life_ = 2;
    NetWinSessionOptions.cloud_cover_ = 2;

    NetWinLobbyRow *row = &NetWinLobbyRows[1];
    Player *player = &Players[1];
    int row_index = 1;
    do {
        row->owner_ = 0;
        row->prefix_ = 0xFF;
        row->ready_or_occupied_ = 0;
        row->field_2_ = NetWinSessionOptions.difficulty_;
        row->faction_or_color_ = static_cast<int8_t>(0xFF);
        row->field_4_ = 2;
        strcpy(row->leader_name_, player->name_leader);
        strcpy(row->formal_faction_name_, player->formal_name_faction);
        strcpy(row->noun_faction_,
               label_get(0xFE4 / 4));
        strcpy(row->adjective_faction_, player->adj_name_faction);
        strcpy(row->leader_title_, player->title_leader);
        strcpy(row->adjective_leader_, player->adj_leader);
        row->tail_[0] = reinterpret_cast<const uint8_t *>(
            &player->is_leader_female)[0];
        ++row;
        ++player;
        ++row_index;
    } while (row_index < 8);
}

// Private recovered body for the native 0x00482D90 roster finalizer.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00482D90 ?prepare_game@NetWin@@QAEXXZ 0x00482D90-0x0048318E;0x006574FE-0x00657510
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00482d90.cpp", "recovery/tasks/netwin-owner-coverage.json"], "reason": "Recover complete lobby game preparation through real faction records, AlphaNet process-slot ownership, player data and Random lifecycle. Native low-five-bit shifts, signed difficulty and copied status extent preserved; seed and historical obligations retained."}
void NetWin::prepare_game() {
    Random random;
    random.reseed(1000);

    NetWinFactionRecord *const faction_records =
        static_cast<NetWinFactionRecord *>(faction_records_);
    const int faction_count = faction_records[0].record_count_;
    unsigned int used_mask = 0;

    for (int unused_faction = 0; unused_faction < faction_count;
         ++unused_faction) {
        if (faction_records[unused_faction].active_ == 0) {
            used_mask |=
                1u << (static_cast<unsigned int>(unused_faction) & 31u);
        }
    }
    for (int used_seat = 1; used_seat < 8; ++used_seat) {
        const int existing_faction =
            NetWinLobbyRows[used_seat].faction_or_color_;
        if (existing_faction != -1) {
            used_mask |=
                1u << (static_cast<unsigned int>(existing_faction) & 31u);
        }
    }

    int needed = 0;
    for (int needed_faction = 0; needed_faction < faction_count;
         ++needed_faction) {
        if (faction_records[needed_faction].active_ != 0 &&
            (used_mask &
             (1u << (static_cast<unsigned int>(needed_faction) & 31u))) == 0) {
            ++needed;
        }
    }

    unsigned int hidden_mask = 0;
    int remaining = needed;
    for (int assign_seat = 1; assign_seat < 8; ++assign_seat) {
        int8_t &assigned_faction =
            NetWinLobbyRows[assign_seat].faction_or_color_;
        if (assigned_faction == -1) {
            hidden_mask |= 1u << assign_seat;
            unsigned int picked;
            if (remaining == 0) {
                picked = random.get(0, faction_count);
            } else {
                do {
                    picked = random.get(0, faction_count);
                } while ((used_mask & (1u << (picked & 31u))) != 0);
                --remaining;
            }
            assigned_faction = static_cast<int8_t>(picked);
            used_mask |= 1u << (picked & 31u);
        }
    }

    const int mode = NetWinSessionData.mode_;
    if (mode == 0 || mode == 1) {
        for (int player_slot_index = 0; player_slot_index < 8;
             ++player_slot_index) {
            AlphaNetProcessSlot &slot =
                NetDaemonNet.process_slots_[player_slot_index];
            const NetWinLobbyRow &row = NetWinLobbyRows[player_slot_index];
            slot.identity_ = static_cast<int8_t>(row.prefix_);
            slot.field_25_ = row.ready_or_occupied_;
            slot.field_26_ = row.field_2_;
            slot.field_27_ = row.faction_or_color_;
            memcpy(slot.status_bytes_,
                   reinterpret_cast<const uint8_t *>(&row) + 4, 0x172);
            slot.identity_ = static_cast<int8_t>(player_slot_index);
        }
    } else {
        for (int faction_slot_index = 0; faction_slot_index < 8;
             ++faction_slot_index) {
            int chosen_faction =
                NetWinLobbyRows[faction_slot_index].faction_or_color_;
            if (chosen_faction == -1) {
                chosen_faction = 0;
            }
            AlphaNetProcessSlot &slot =
                NetDaemonNet.process_slots_[faction_slot_index];
            const NetWinLobbyRow &row = NetWinLobbyRows[faction_slot_index];
            slot.identity_ = static_cast<int8_t>(row.prefix_);
            slot.field_25_ = row.ready_or_occupied_;
            slot.field_26_ = row.field_2_;
            slot.field_27_ = row.faction_or_color_;
            memcpy(slot.status_bytes_,
                   reinterpret_cast<const uint8_t *>(&row) + 4, 0x172);
            slot.identity_ = static_cast<int8_t>(chosen_faction);
        }
    }

    if (mode == 0 || mode == 1) {
        for (int duplicate_seat = 2; duplicate_seat < 8; ++duplicate_seat) {
            char *const leader_name =
                NetWinLobbyRows[duplicate_seat].leader_name_;
            int duplicate_count = 0;
            for (int earlier_seat = 1; earlier_seat < duplicate_seat;
                 ++earlier_seat) {
                if (strcmp(leader_name,
                           NetWinLobbyRows[earlier_seat].leader_name_) == 0) {
                    ++duplicate_count;
                }
            }
            if (duplicate_count != 0) {
                char suffix[80];
                _itoa(duplicate_count, suffix, 10);
                strcat(leader_name, suffix);
            }
        }

        for (int filename_seat = 1; filename_seat < 8; ++filename_seat) {
            const int filename_faction =
                NetWinLobbyRows[filename_seat].faction_or_color_;
            const char *const faction_file =
                faction_records[filename_faction].filename_;
            strcpy(Players[filename_seat].filename, faction_file);
            strcpy(Players[filename_seat].search_key, faction_file);
            read_faction(filename_seat);
            compute_faction_modifiers(filename_seat);
        }

        for (int player_seat = 1; player_seat < 8; ++player_seat) {
            AlphaNetProcessSlot &slot_record =
                NetDaemonNet.process_slots_[player_seat];
            const int player_faction = slot_record.identity_;
            const NetWinLobbyRow &row = NetWinLobbyRows[player_seat];
            PlayersData[player_faction].diff_level =
                static_cast<int8_t>(slot_record.field_26_);
            if ((hidden_mask & (1u << player_seat)) == 0) {
                strcpy(Players[player_faction].name_leader, row.leader_name_);
                strcpy(Players[player_faction].formal_name_faction,
                       row.formal_faction_name_);
                strcpy(Players[player_faction].noun_faction, row.noun_faction_);
                strcpy(Players[player_faction].adj_faction,
                       row.adjective_faction_);
                strcpy(Players[player_faction].title_leader, row.leader_title_);
                strcpy(Players[player_faction].adj_leader, row.adjective_leader_);
                Players[player_faction].is_leader_female = row.tail_[0];
            }
        }
    } else if (NetDaemonNet.local_player_id() != NetDaemonNet.host_player_id()) {
        for (int art_faction = 1; art_faction < 8; ++art_faction) {
            strcpy(Players[art_faction].filename,
                   faction_records[art_faction].filename_);
            load_faction_art(art_faction);
        }
    }

    load_flags();
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
// symbol    ?on_redraw@NetWin@@UAEXXZ
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
    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->box_sprite(buffer_working_rect(),
                       &InterfaceBoxStyles[3]);

    // The banner rectangle: painted as 0xA14 holds it, then inset by three
    // on every side and painted again - the inset copy is the one the title
    // is centred in.
    RECT banner = setup_rects_[0];
    buffer->box_sprite(&banner, &InterfaceBoxStyles[1]);
    banner.left += 3;
    banner.top += 3;
    banner.right -= 3;
    banner.bottom -= 3;
    buffer->box_sprite(&banner, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           label_get(0x3F0 / 4));
    CharUpperA(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, &banner,
                             strlen(StringTemp));
    }

    draw_custom_rules();
    buffer->box_sprite(&setup_rects_[45],
                       &InterfaceBoxStyles[1]);
    buffer->box_sprite(&setup_rects_[44],
                       &InterfaceBoxStyles[0]);
    buffer->box_sprite(&setup_rects_[43],
                       &InterfaceBoxStyles[0]);
    draw_player_names();
    draw_diffs();
    draw_factions();
    draw_session_info();
    soft_update();
}

// 0x0090E8EC/F0/F4 are the three rule-bit members of the options record.
// 0x0093E8C0 is NetDaemonNet.primary_state_.leading_words_[0] (+0x1B30),
// initialized by NetDaemon's constructor; it is shared with TurnsAssigned,
// not a separate global or the later channels_ array.

// 0x0080A6F0 - the planet-size name count beside the fixed dialog (the
// names occupy world_size_labels_: 32 rows at native 0x80-byte stride.
// Original starts below are distinct objects; stores/accesses establish only
// those starts, not original allocations, complete lifetimes or exact data layout.
// RECOVERY_OBJECT: v1 0x0080A6F0 ?NetWinPlanetSizeLabelCount@@3HA
int NetWinPlanetSizeLabelCount;
// RECOVERY_OBJECT: v1 0x0080A6F4 ?NetWinExecAdjacentState@@3HA
int NetWinExecAdjacentState;
// RECOVERY_OBJECT: v1 0x00811E38 ?NetWinWindowTitleInitialText@@3DA
char NetWinWindowTitleInitialText;


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
    Buffer *buffer = static_cast<Buffer *>(this);
    StringStruct list;

    buffer->box_sprite(&setup_rects_[3],
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = &setup_rects_[2];
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    if (NetDaemonNet.allocation_node().session_name_ != 0) {
        strcat(StringTemp, NetDaemonNet.allocation_node().session_name_);
    }
    CharUpperA(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    buffer->set_clip(title_rect);
    if (*StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }
    buffer->set_clip(buffer_working_rect());

    RECT *panel = &setup_rects_[1];
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
    // body's own starting value. The existing setter directly assigns the
    // same Buffer +0x10 member without additional side effects.
    buffer->set_wrap_width(0x14);

    // ---- session name: {Name: } <global difficulty> ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           label_get(0xFA4 / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    {
        int difficulty = NetWinSessionOptions.difficulty_;
        if (difficulty < 0) {
            difficulty = 0;
        } else if (6 < difficulty) {
            difficulty = 6;
        }
        strcat(StringTemp,
               StringTable.get(Difficulty[difficulty]));
    }
    {
        int written = buffer->write_l(StringTemp, col, row,
                                      strlen(StringTemp));
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
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
           label_get(0x100C / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    strcat(StringTemp,
           StringTable.get(
               TimeControl[static_cast<unsigned char>(NetWinSessionOptions.time_control_)].name));
    {
        int written = buffer->write_l(StringTemp, col, row,
                                      strlen(StringTemp));
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
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
           label_get(0x1078 / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (NetWinSessionData.scenario_text_wire_tail_[0] == 0) {
        strcat(StringTemp,
               label_get(0x107C / 4));
    } else {
        strcat(StringTemp,
               NetWinSessionData.scenario_text_wire_tail_);
    }
    if (NetWinSessionData.mode_ == 1) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               label_get(0x10 / 4));
    } else if (NetWinSessionData.mode_ == 2) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               label_get(0x1018 / 4));
    } else if (NetWinSessionData.mode_ == 3) {
        strcat(StringTemp, " ");
        strcat(StringTemp, "(");
        strcat(StringTemp,
               label_get(0x1014 / 4));
    }
    {
        int bottom = buffer->wrap(StringTemp, col, row,
                                  panel->right - panel->left - 0x1E);
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), BufferLastWrapWriteResult + 3,
                       bottom - buffer->text_line_height());
        }
        spot_.add(0, 0, col, row, panel->right - panel->left, bottom - row);
        row = bottom;
    }

    // ---- opponent difficulty ----
    StringTemp[0] = 0;
    strcat(StringTemp, "{");
    strcat(StringTemp,
           label_get(0xFAC / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, "}");
    strcat(StringTemp, " ");
    if (NetWinSessionData.scenario_text_wire_tail_[0] == 0) {
        int opponent = NetWinSessionOptions.option_4_;
        if (opponent == -1) {
            strcat(StringTemp,
                   label_get(0xFE4 / 4));
        } else {
            if (opponent < 0) {
                opponent = 0;
            } else if (0x1F < opponent) {
                opponent = 0x1F;
            }
            strcat(StringTemp,
                   Labels.strings_ptr[(opponent * 0x80 + 0x6614) / 4]);
        }
    } else {
        char tally[80];
        _itoa(NetWinSessionOptions.option_4_, tally, 10);
        strcat(StringTemp, tally);
        strcat(StringTemp, " ");
        strcat(StringTemp, "X");
        strcat(StringTemp, " ");
        _itoa(NetWinSessionOptions.option_5_, tally, 10);
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
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0 && NetWinSessionData.scenario_text_wire_tail_[0] == 0) {
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
        static const int kTextSlots[4] = {
            0xFB0 / 4, 0xFB4 / 4, 0xFB8 / 4, 0xFBC / 4};
        static const int kSpotIds[4] = {5, 6, 7, 8};
        static const int kTableBases[4] = {0x3D8, 0x3DB, 0x3E1, 0x3E4};
        signed char flag;
        switch (rule) {
        case 0: flag = static_cast<signed char>(NetWinSessionOptions.option_5_); break;
        case 1: flag = static_cast<signed char>(NetWinSessionOptions.planet_size_); break;
        case 2: flag = static_cast<signed char>(NetWinSessionOptions.ocean_coverage_); break;
        default: flag = static_cast<signed char>(NetWinSessionOptions.erosion_); break;
        }

        StringTemp[0] = 0;
        strcat(StringTemp, kLabels[rule]);
        strcat(StringTemp,
               label_get(kTextSlots[rule]));
        strcat(StringTemp, ": ");
        strcat(StringTemp, kLabelEnds[rule]);
        strcat(StringTemp, " ");

        LPSTR text_address;
        if (NetWinSessionData.scenario_text_wire_tail_[0] == 0) {
            if (flag == -1) {
                text_address = Labels.strings_ptr[0xFE4 / 4];
            } else {
                text_address = Labels.strings_ptr[flag + kTableBases[rule]];
            }
        } else {
            text_address = Labels.strings_ptr[0x1000 / 4];
        }
        strcat(StringTemp, StringTable.get(text_address));

        int avail = panel->right - col - 10;
        int bottom;
        if (avail < buffer->text_width(StringTemp)) {
            bottom = buffer->wrap(StringTemp, col, row, avail);
        } else {
            buffer->write_l(StringTemp, col, row, strlen(StringTemp));
            bottom = buffer->text_line_height() + row;
        }
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0 && NetWinSessionData.scenario_text_wire_tail_[0] == 0) {
            Sprite &arrow = g_IFACE_MP_COMBO_ARROW_SPRITES[0];
            arrow.draw(buffer, arrow.transparent_index(), col + 3, row);
            spot_.add(kSpotIds[rule], 0, col, row, 0x1E, bottom - row);
        }
        row = bottom;
    }

    row = buffer->text_line_height() + row;

    if (NetWinSessionData.mode_ == 3 || NetWinSessionData.mode_ == 2) {
        // The unclaimed-faction tally: every record whose claimed id matches
        // nobody in the lobby.
        int connected = 0;
        for (int slot = 8; slot != 0; --slot) {
            unsigned long owned =
                faction_records_[8 - slot].owner_;
            if (owned != 0) {
                int seen = 0;
                if (NetDaemonNet.player_count_ > 0) {
                    unsigned long *scan = reinterpret_cast<unsigned long *>(
                        &NetWinLobbyRows[1].owner_);
                    while (seen < NetDaemonNet.player_count_) {
                        if (*scan == owned) {
                            break;
                        }
                        ++seen;
                        scan += 0x5F;
                    }
                }
                if (seen == NetDaemonNet.player_count_) {
                    ++connected;
                }
            }
        }

        buffer->set_text_color((connected != 0 ? 8 : 0) - 0xF, -1, 1, 1);
        StringTemp[0] = 0;
        strcat(StringTemp, "{");
        strcat(StringTemp,
               label_get(0x113C / 4));
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

    buffer->set_clip(buffer_working_rect());
    soft_update(&setup_rects_[3]);
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
    int pid_idx = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->box_sprite(&setup_rects_[5],
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = &setup_rects_[4];
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[2]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           label_get(0x414 / 4));
    CharUpperA(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }

    NetWinLobbyRow *lobby_row = &NetWinLobbyRows[1];
    AlphaNetProcessSlot *slot_record = NetDaemonNet.process_slots_ + 1;
    RECT *rects = &setup_rects_[6];
    for (int row = 0; row < 7;
         ++row, ++lobby_row, slot_record += 1) {
        if (NetWinSessionData.mode_ == 3 && faction_records_ != 0
            && row >= faction_records_[0].record_type_) {
            break;
        }
        RECT *name_rect = &rects[row * 3];
        RECT *colour_rect = &rects[row * 3 + 3];
        RECT *row_rect = &rects[row * 3 + 6];

        // The ready glyph: element 2 when the record block's leading byte
        // says the slot holds a player.
        int ready = lobby_row->ready_or_occupied_ != 0 ? 2 : 0;
        buffer->box_sprite(row_rect, &InterfaceBoxStyles[48 + ready]);

        if (NetWinSessionData.mode_ == 3 && lobby_row->prefix_ == 0xFF) {
            continue;
        }

        // The lock chip: the chosen faction's own id field against the
        // record's claim dword, only while the eighth word is 2 or 3.
        signed char mode = lobby_row->faction_or_color_;
        int locked;
        if (NetWinSessionData.mode_ == 2 || NetWinSessionData.mode_ == 3) {
            locked = 0;
            if (mode != -1) {
                unsigned long claimed = lobby_row->owner_;
                unsigned long owned = faction_records_[mode].owner_;
                locked = owned != 0 && owned != claimed;
            }
        } else {
            locked = lobby_row->owner_ != 0;
        }
        Sprite &chip = locked ? g_IFACE_LOCK_SPRITES[1]
                              : g_IFACE_LOCK_SPRITES[0];
        chip.draw(buffer, chip.transparent_index(), colour_rect->left + 1,
                  colour_rect->top, 1, 1);

        // The name field: inset, then the row text.
        RECT field = *name_rect;
        field.left += 4;
        field.right -= 4;

        if (NetWinSessionData.mode_ == 0 || NetWinSessionData.mode_ == 1) {
            if (row + 1 == NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id())) {
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
        if (lobby_row->prefix_ == 0xFF) {
            strcat(StringTemp, "Computer");
        } else {
            strcat(StringTemp,
                   NetDaemonNet.get_player_name(slot_record->pid_));
        }
        if (static_cast<uint8_t>(lobby_row->faction_or_color_) != 0xFF) {
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp,
                   lobby_row->leader_name_);
            if (slot_record > NetDaemonNet.process_slots_ + 1) {
                // Count the static records whose name text matches, and
                // append the tally when there is more than one.
                int matches = 0;
                const char *name = lobby_row->leader_name_;
                const NetWinLobbyRow *scan = &NetWinLobbyRows[1];
                for (int seen = row; seen != 0; --seen) {
                    if (strcmp(name, scan->leader_name_) == 0) {
                        ++matches;
                    }
                    ++scan;
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

        ::UnionRect(name_rect, row_rect, &field);

        spot_.add(row, 2, row_rect->left, row_rect->top,
                  row_rect->right - row_rect->left,
                  row_rect->bottom - row_rect->top);
        if (NetWinSessionData.mode_ == 0 || NetWinSessionData.mode_ == 1) {
            spot_.add(row, 2, name_rect->left, name_rect->top,
                      name_rect->right - name_rect->left,
                      name_rect->bottom - name_rect->top);
        }
        spot_.add(row, 5, colour_rect->left, colour_rect->top,
                  colour_rect->right - colour_rect->left,
                  colour_rect->bottom - colour_rect->top);
        soft_update(&field);
    }

    if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
        // Not connected: the left button takes the connect label.
        unsigned char color =
            static_cast<uint8_t>(NetWinLobbyRows[pid_idx].faction_or_color_);
        const LPSTR label = color != 0
            ? Labels.strings_ptr[0x1054 / 4]
            : Labels.strings_ptr[0x1050 / 4];
        flatButton1_.set_name(StringTable.get(label));
        flatButton1_.redraw();
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
// symbol    ?draw_custom_rules@NetWin@@QAEXXZ
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
    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->box_sprite(&setup_rects_[47],
                       &InterfaceBoxStyles[1]);
    RECT *title_rect = &setup_rects_[46];
    buffer->box_sprite(title_rect, &InterfaceBoxStyles[0]);

    StringTemp[0] = 0;
    strcat(StringTemp,
           label_get(0xF9C / 4));
    CharUpperA(StringTemp);
    buffer->set_font(find_font(0xE, 1), 0, 0, 0);
    buffer->set_text_color(0xE1, -1, 1, 1);
    if (StringTemp != 0) {
        buffer->write_cent_l(StringTemp, title_rect,
                             strlen(StringTemp));
    }

    RECT *rules_rect = &setup_rects_[48];
    buffer->box_sprite(rules_rect, &InterfaceBoxStyles[0]);
    buffer->box_sprite(&setup_rects_[49],
                       &InterfaceBoxStyles[0]);

    // The four layout words double as the row rectangle: left/top advance per
    // row while right/bottom stay put, and write_l reads them as a RECT.
    int left = rules_rect->left - 0x14;
    int top = setup_rects_[48].top;
    buffer->set_font(find_font(0xC, 1), 0, 0, 0);
    int right = (rules_rect->right - rules_rect->left) + left - 6;
    int bottom = top + 0x12;
    RECT row_rect;
    row_rect.left = left;
    row_rect.top = top;
    row_rect.right = right;
    row_rect.bottom = bottom;

    // The standing free-technology checkbox, spot id 100.
    int free_tech = (NetWinSessionOptions.rule_bits_c_ & 0x10) == 0 ? 2 : 0;
    Sprite &standing = g_IFACE_CLOSE_X_SPRITES[free_tech];
    standing.draw(buffer, standing.transparent_index(), left, top);
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
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
        unsigned int bits = NetWinSessionOptions.rule_bits_a_;
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
            state = NetWinSessionOptions.rule_bits_b_ & 0x800000;
            break;
        case 17:
            state = NetWinSessionOptions.rule_bits_b_ & 0x1000000;
            break;
        default:
            // Row 15 draws nothing - the separator line in the RULES text.
            line = text_get();
            continue;
        }

        int on = state != 0 ? 2 : 0;
        Sprite &check = g_IFACE_CLOSE_X_SPRITES[on];
        check.draw(buffer, check.transparent_index(), left + 0x19, top);
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
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
    Buffer *buffer = static_cast<Buffer *>(this);
    RECT *rect = &setup_rects_[20];
    NetWinLobbyRow *row = &NetWinLobbyRows[1];
    for (int index = 0; index < 7;
         ++index, ++row, ++rect) {
        if (NetWinSessionData.mode_ == 3 && faction_records_ != 0
            && index >= faction_records_[0].record_type_) {
            break;
        }
        buffer->box_sprite(rect, &InterfaceBoxStyles[0]);
        if (NetWinSessionData.mode_ == 3 && row->prefix_ == 0xFF) {
            continue;
        }

        // The combo-arrow chip: the player's own slot, or - with the two ids
        // still equal - the row whose prefix is the 0xFF default.
        int slot = index + 1;
        if (slot == NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id())
            || (NetDaemonNet.local_player_id() == NetDaemonNet.host_player_id()
                && row->prefix_ == 0xFF)) {
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
        char *name = StringTable.get(Difficulty[row->field_2_]);
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
    Buffer *buffer = static_cast<Buffer *>(this);
    RECT *rect = &setup_rects_[13];
    NetWinLobbyRow *row = &NetWinLobbyRows[1];
    for (int index = 0; index < 7;
         ++index, ++row, ++rect) {
        if (NetWinSessionData.mode_ == 3 && faction_records_ != 0
            && index >= faction_records_[0].record_type_) {
            break;
        }
        buffer->box_sprite(rect, &InterfaceBoxStyles[0]);
        if (NetWinSessionData.mode_ == 3 && row->prefix_ == 0xFF) {
            continue;
        }

        int slot = index + 1;
        int mode = row->faction_or_color_;
        if (slot == NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id())
            || (NetDaemonNet.local_player_id() == NetDaemonNet.host_player_id()
                && row->prefix_ == 0xFF)) {
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
            named = faction_records_[mode].noun_faction_;
        } else {
            named = label_get(0xFE4 / 4);
        }
        strcat(StringTemp, named);
        if (strcmp(named, row->noun_faction_) != 0) {
            strcat(StringTemp, ": ");
            strcat(StringTemp, " ");
            strcat(StringTemp, row->noun_faction_);
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

// The shared tail of the eight scalar pickers runs even on cancellation:
// only the option-byte write is conditional. Native 47E137/47E426 and their
// sibling joins resolve the local slot before copying the live options.
static void pick_send_state() {
    NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    NetWinSessionOptionsPacket packet;
    packet.options_ = NetWinSessionOptions;
    packet.opcode_ = 0x2F02;
    NetDaemonNet.send(&packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
}

// Number/planet pickers and rule clicks load the destination before copying
// (47E147/47EA57/47B9B5), unlike the other six scalar pickers.
static void pick_send_state_with_captured_target() {
    NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    const int target = NetDaemonNet.host_player_id();
    NetWinSessionOptionsPacket packet;
    packet.options_ = NetWinSessionOptions;
    packet.opcode_ = 0x2F02;
    NetDaemonNet.send(&packet, sizeof(packet), target, 1);
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
Purpose: Dispatch a lobby spot click to the map/options/faction/player action
         represented by that spot. Rule toggles update the typed session state
         and send the opcode/options-only 0x2F02 state packet. The original
         leaves the intervening header bytes uninitialized.
// ORIGINAL: 0x0047B760 ?on_left_click@NetWin@@QAEXHH@Z 0x0047B760-0x0047BAF6
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47B9C7/47B9D7/47B9DD: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
// symbol    ?on_left_click@NetWin@@UAEXHH@Z
*/
void NetWin::on_left_click(int x, int y) {
    int spot_position;
    int spot_type;
    spot_.check(x, y, &spot_position, &spot_type);

    switch (spot_type) {
    case 0:
        if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
            return;
        }
        switch (spot_position) {
        case 0: pick_map(); return;
        case 1: pick_num_players(); return;
        case 2: pick_global_diff(); return;
        case 3: pick_time(); return;
        case 4: pick_planet_size(); return;
        case 5: pick_ocean(); return;
        case 6: pick_erosion(); return;
        case 7: pick_native(); return;
        case 8: pick_clouds(); return;
        }
        return;
    case 1:
        if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
            return;
        }
        switch (spot_position) {
        case 0: NetWinSessionOptions.rule_bits_a_ ^= 0x800; break;
        case 1: NetWinSessionOptions.rule_bits_a_ ^= 0x2; break;
        case 2: NetWinSessionOptions.rule_bits_a_ ^= 0x8; break;
        case 3: NetWinSessionOptions.rule_bits_a_ ^= 0x4; break;
        case 4: NetWinSessionOptions.rule_bits_a_ ^= 0x1000; break;
        case 5: NetWinSessionOptions.rule_bits_a_ ^= 0x1; break;
        case 6: NetWinSessionOptions.rule_bits_a_ ^= 0x10; break;
        case 7: NetWinSessionOptions.rule_bits_a_ ^= 0x20; break;
        case 8: NetWinSessionOptions.rule_bits_a_ ^= 0x4000; break;
        case 9: NetWinSessionOptions.rule_bits_a_ ^= 0x200; break;
        case 10: NetWinSessionOptions.rule_bits_a_ ^= 0x40; break;
        case 11: NetWinSessionOptions.rule_bits_a_ ^= 0x100; break;
        case 12: NetWinSessionOptions.rule_bits_a_ ^= 0x2000; break;
        case 13: NetWinSessionOptions.rule_bits_a_ ^= 0x8000; break;
        case 14: NetWinSessionOptions.rule_bits_a_ ^= 0x80; break;
        case 16: NetWinSessionOptions.rule_bits_b_ ^= 0x800000; break;
        case 17: NetWinSessionOptions.rule_bits_b_ ^= 0x1000000; break;
        case 100: NetWinSessionOptions.rule_bits_c_ ^= 0x10; break;
        // Native 47B8BA and selector slot18 join the unchanged-options send.
        default: break;
        }
        pick_send_state_with_captured_target();
        return;
    case 2:
        if (NetDaemonNet.process_slots_[spot_position + 1].pid_ ==
            static_cast<unsigned long>(NetDaemonNet.local_player_id())) {
            pick_player();
        }
        return;
    case 3:
    case 4:
        if (NetDaemonNet.process_slots_[spot_position + 1].pid_ ==
                static_cast<unsigned long>(NetDaemonNet.local_player_id()) ||
            (NetWinLobbyRows[spot_position + 1].prefix_ == 0xFF &&
             NetDaemonNet.local_player_id() == NetDaemonNet.host_player_id())) {
            if (spot_type == 3) {
                pick_faction(spot_position + 1);
            } else {
                pick_diff(spot_position + 1);
            }
        }
        return;
    case 5:
        if ((NetWinSessionData.mode_ == 0 || NetWinSessionData.mode_ == 1) &&
            NetDaemonNet.process_slots_[spot_position + 1].pid_ ==
                static_cast<unsigned long>(NetDaemonNet.local_player_id())) {
            pick_pass(0);
        }
        return;
    }
}

// ORIGINAL: 0x0047BBF0 ?pick_player@NetWin@@QAEXXZ 0x0047BBF0-0x0047C527;0x00656740-0x006568F9
// RECOVERY_BINDING: v1 {"evidence": ["git:0db4f0dc:src/unrecovered/0047bbf0.cpp", "pinned:0x0047BBF0-0x0047C527 reads Popup/BasePop +0x3100 input-poll state, copies the 0x17c row, selects the source row's faction byte, copies Player fields or ParseStrBuffer strings, and reaches packet construction only when signed slot >= 1", "source:src/netwin.h:NetWinLobbyRow and NetWinSessionData", "source:src/faction.h:Players and Player fields", "source:src/general.h:ParseStrBuffer", "source:src/basepop.h:Popup::input_poll_state and dialogs"], "reason": "Retain the substantive typed lobby-name editor and its complete original primary/exceptional mapping; retire the unannotated generated duplicate, not its unresolved radio/default-faction and seventh gender-field behavior. The seed's synthetic stack frame and manual widget teardown are hypotheses, not additional source owners or proof of native lifetime fidelity. Complete the native custom-name modal through the existing Popup, EditGroup, lobby-row, Player, ParseStrBuffer and typed single-row packet owners. Preserve the signed faction selector, unbounded strcpy operations, one-character case-insensitive gender test, accepted/cancel paths, and native signed slot >= 1 packet condition; retain the full original primary and exceptional spans."}
// TRIED: historical scaffold git:0db4f0dc:src/unrecovered/0047bbf0.cpp mixes an approximate
// stack-frame reconstruction with untyped widget cleanup and custom/default
// radio selection. This source recovers the typed six-field edit and outgoing
// lobby update; its radio/default-faction branch still needs restoration.
// RECOVERY_NOTE: The pinned modal branch, default/player copies, seventh gender
// field, and signed-positive packet condition are now represented below using
// the existing Popup, Player, ParseStrBuffer, and lobby packet owners.
void NetWin::pick_player() {
    // Native 47BC1B constructs the owner even on the saved-mode early exit.
    Popup popup;
    if (NetWinSessionData.mode_ == 2 || NetWinSessionData.mode_ == 3) {
        return;
    }
    const int slot = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());

    // Native scratch starts at EBP-41C, after the 537C-byte Popup, and
    // precedes the first packet at EBP-31C. Recover that 100h accessed
    // window, not an original array/padding declaration or overflow layout.
    char gender_text[0x100];
    strcpy(gender_text,
           label_get((NetWinLobbyRows[slot].tail_[0] == 0 ? 0xB88 : 0xB8C) /
                     4));
    popup.start(PopupStartCaption, "CUSTOMNAMES", -1, 0, 0x200044, 0);
    EditGroup &fields = static_cast<EditGroup &>(popup.dialogs());
    fields.edit(label_get(0x2B8 / 4), NetWinLobbyRows[slot].leader_name_, 0x17);
    fields.edit(label_get(0x2BC / 4), NetWinLobbyRows[slot].formal_faction_name_, 0x27);
    fields.edit(label_get(0x2C0 / 4), NetWinLobbyRows[slot].noun_faction_, 0x17);
    fields.edit(label_get(0x2C4 / 4), NetWinLobbyRows[slot].adjective_faction_, 0x17);
    fields.edit(label_get(0x78C / 4), NetWinLobbyRows[slot].leader_title_, 0x17);
    fields.edit(label_get(0x790 / 4), NetWinLobbyRows[slot].adjective_leader_, 0x17);
    fields.edit(label_get(0x794 / 4), gender_text, 0x17);
    popup.set_loc(0x1000, 0x1000);
    if (popup.exec(0, pick_daemon_callback) < 0) {
        return;
    }

    NetWinLobbyRow updated = NetWinLobbyRows[slot];
    if (popup.input_poll_state() == 1) {
        const int faction = updated.faction_or_color_;
        if (faction == -1) {
            // Native 47C073..47C13E interleaves six fresh lookups/copies.
            strcpy(updated.leader_name_, label_get(0xFE4 / 4));
            strcpy(updated.formal_faction_name_, label_get(0xFE4 / 4));
            strcpy(updated.noun_faction_, label_get(0xFE4 / 4));
            strcpy(updated.adjective_faction_, label_get(0xFE4 / 4));
            strcpy(updated.leader_title_, label_get(0xFE4 / 4));
            strcpy(updated.adjective_leader_, label_get(0xFE4 / 4));
            updated.tail_[0] = 10; // Native 47C146, not a boolean default.
        } else {
            const Player &player = Players[faction];
            strcpy(updated.leader_name_, player.name_leader);
            strcpy(updated.formal_faction_name_, player.formal_name_faction);
            strcpy(updated.noun_faction_, player.noun_faction);
            strcpy(updated.adjective_faction_, player.adj_name_faction);
            strcpy(updated.leader_title_, player.title_leader);
            strcpy(updated.adjective_leader_, player.adj_leader);
            updated.tail_[0] =
                static_cast<uint8_t>(player.is_leader_female);
        }
    } else {
        strcpy(updated.leader_name_, ParseStrBuffer[0].str);
        strcpy(updated.formal_faction_name_, ParseStrBuffer[1].str);
        strcpy(updated.noun_faction_, ParseStrBuffer[2].str);
        strcpy(updated.adjective_faction_, ParseStrBuffer[3].str);
        strcpy(updated.leader_title_, ParseStrBuffer[4].str);
        strcpy(updated.adjective_leader_, ParseStrBuffer[5].str);
        updated.tail_[0] = static_cast<uint8_t>(
            _strnicmp(gender_text, label_get(0xB8C / 4), 1) == 0);
    }

    if (slot < 1) {
        return;
    }

    NetWinLobbySinglePacket packet;
    // The original writes only these header fields; +2 and +0xC remain
    // uninitialized and are sent along with the complete copied row.
    packet.header_.opcode_ = 0x04;
    packet.header_.flags_ = 0x2F;
    packet.header_.slot_ = slot;
    packet.header_.field_8_ = 0;
    packet.row_ = updated;
    NetDaemonNet.Net::send(&packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
}

// ORIGINAL: 0x0047C530 ?pick_faction@NetWin@@QAEXH@Z 0x0047C530-0x0047C965;0x006568F9-0x006569C4
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/recovered/units/0047c530.cpp:NetWin::pick_faction", "src/netwin.h:NetWinFactionRecord and NetWinLobbyRow", "src/netwin.cpp:NetWin::pick_pass and NetWinLobbySinglePacket", "Pinned47C819 clears EAX;47C854 stores it to [EBP-1A4], packet+8 relative to sent [EBP-1AC] at47C88A/47C89D. Opcode word/slot/row+10 are written; +2..3/+C..F remain untouched."], "reason": "Recover faction selection through the typed faction records, lobby rows, popup dialog and single-row packet."}
void NetWin::pick_faction(int slot) {
    PopMenu pop_menu;
    pop_menu.init();

    pop_menu.dialogs().item(label_get(0xFE4 / 4), 0);

    const int count = static_cast<NetWinFactionRecord *>(
        faction_records_)[0].record_count_;
    int player_count = count > 0 ? NetDaemonNet.player_count_ : 0;
    for (int faction = 0; faction < count; ++faction) {
        // Native 47C618 reloads the allocation, but not the captured count.
        NetWinFactionRecord *const records =
            static_cast<NetWinFactionRecord *>(faction_records_);
        if (records[faction].active_ == 0) {
            continue;
        }

        bool already_selected = false;
        if (NetWinSessionData.mode_ == 2 || NetWinSessionData.mode_ == 3) {
            // Native 47C656 compares zero with a negative scan bound too.
            already_selected = player_count < 0;
            for (int player = 1; player <= player_count; ++player) {
                if (NetWinLobbyRows[player].faction_or_color_ == faction) {
                    already_selected = true;
                    break;
                }
            }
        }
        if (!already_selected) {
            pop_menu.dialogs().item(records[faction].noun_faction_, faction + 1);
            player_count = NetDaemonNet.player_count_;
        }
    }

    const int answer = pick_answer(*this, pop_menu);
    if (answer < 0) {
        return;
    }

    const int chosen_faction = pop_menu.get_selected_id() - 1;
    const int8_t previous = NetWinLobbyRows[slot].faction_or_color_;
    NetWinLobbyRows[slot].faction_or_color_ =
        static_cast<int8_t>(chosen_faction);
    const int accepted = pick_pass(1);
    NetWinLobbyRows[slot].faction_or_color_ = previous;
    if (accepted == 0) {
        return;
    }

    // Native 47C811 snapshots the restored live row before the signed guard.
    NetWinLobbyRow updated;
    memcpy_call(&updated, &NetWinLobbyRows[slot], 0x17C);
    updated.faction_or_color_ = static_cast<int8_t>(chosen_faction);
    if (slot >= 1) {
        NetWinLobbySinglePacket packet;
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x2F;
        packet.header_.slot_ = slot;
        packet.header_.field_8_ = 0;
        packet.row_ = updated;
        NetDaemonNet.Net::send(&packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
    }
}

// ORIGINAL: 0x0047CF40 ?pick_diff@NetWin@@QAEXH@Z 0x0047CF40-0x0047D2F4;0x00656AED-0x00656BC5
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/recovered/units/0047cf40.cpp:NetWin::pick_diff", "src/netwin.cpp:pick_global_diff and sibling pickers", "src/netwin.cpp:NetWinLobbySinglePacket and NetWin::on_button_clicked", "Pinned47D0C7/47D0D0/47D0D6: opcode word, slot dword and +8 zero; copied row+10 and send18C. +2..3/+C..F are untouched."], "reason": "Recover the per-slot difficulty picker using the established popup objects, difficulty text table, typed lobby row and typed single-row packet."}
void NetWin::pick_diff(int slot) {
    PopMenu pop_menu;
    pop_menu.init();

    for (int index = 0; index < 6; ++index) {
        pop_menu.dialogs().item(StringTable.get(Difficulty[index]), index);
    }

    const int answer = pick_answer(*this, pop_menu);
    NetWinLobbyRow row = NetWinLobbyRows[slot];
    if (answer >= 0) {
        row.field_2_ = static_cast<uint8_t>(answer);
    }

    if (slot >= 1) {
        NetWinLobbySinglePacket packet;
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x2F;
        packet.header_.slot_ = slot;
        packet.header_.field_8_ = 0;
        packet.row_ = row;
        NetDaemonNet.Net::send(&packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
    }
}

// ORIGINAL: 0x0047D300 ?pick_map@NetWin@@QAEXXZ 0x0047D300-0x0047E003;0x00656BC5-0x00656D9D
// RECOVERY_BINDING: v1 {"evidence": ["git:0db4f0dc:src/unrecovered/0047d300.cpp", "source:src/netwin.h", "source:recovery/tests/test_netwin_pick_map_behavior.py", "Pinned47DC27..47DC79: full packet opcode word/slot=-1/+8 zero and A64 row bytes;47DC99..47DCC5: session168 bytes at+10 and opcode word only;47DCDC..47DCEF: leave opcode word only. Unwritten packet bytes remain unspecified."], "reason": "Retain the typed map/game picker with its complete original primary/exceptional mapping. The retired unannotated scaffold omitted the host session population, used an unconstructed scratch Dialogs receiver, and approximated the local widget aggregate. Its useful lifetime observation is that nested Popup/Scroll/FlatButton cleanup belongs to genuine member destruction, not independent manually staged objects; complete native cleanup fidelity remains an obligation."}
// TRIED: the popup and load-map/game dispatch are transcribed; the scenario
// host guard now uses LockEnableMask's live-faction byte. Choice zero still
// lacks the original session-record population and is explicitly unfinished.
void NetWin::pick_map() {
    PopMenu menu;
    Buffer faction_art;
    const uint32_t initial_mode = field_7728_;
    menu.init();
    menu.dialogs().item(label_get(0x107C / 4), 0);
    menu.dialogs().item(label_get(0x0FD8 / 4), 1);
    menu.dialogs().item(label_get(0x0FDC / 4), 2);
    menu.dialogs().item(label_get(0x0FE0 / 4), 3);

    const int choice = pick_answer(*this, menu);
    if (choice < 0) {
        return;
    }
    {
        if (choice == 0) {
            const NetWinFactionRecord *const source_records =
                static_cast<const NetWinFactionRecord *>(faction_records_copy_);
            const int record_count = source_records[0].record_count_;
            const unsigned int record_bytes =
                static_cast<unsigned int>(record_count) * 399u + 0x194u;
            memcpy(faction_records_, faction_records_copy_, record_bytes);

            field_7728_ = 0;
            NetWinSessionOptions.difficulty_ =
                static_cast<uint8_t>(DefaultPrefs.difficulty);
            NetWinSessionOptions.rule_bits_a_ = AlphaIniPrefs.rules;
            NetWinSessionData.scenario_text_wire_tail_[0] = 0;
            NetWinSessionOptions.status_ = 0xFF;

            NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
            NetWinSessionOptionsPacket options_packet;
            options_packet.opcode_ = 0x2F02;
            options_packet.options_ = NetWinSessionOptions;
            NetDaemonNet.Net::send(&options_packet, sizeof(options_packet),
                                   NetDaemonNet.host_player_id(), 1);
        } else {
            GameDataLoadRequest = 1;
            if (choice == 1) {
                if (load_map() != 0) {
                    return;
                }
            } else {
                const int is_host = choice == 2;
                if (load_game(is_host, 0) != 0) {
                    return;
                }
                if (is_host) {
                    const int scenario_factions = bit_count(
                        static_cast<int>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFEu));
                    if (scenario_factions < NetDaemonNet.player_count_) {
                        parse_num(scenario_factions, 0);
                        X_pop("TOOMANYFORSCENARIO", 0);
                        return;
                    }
                }
            }

            GameDataLoadRequest = 0;
            if (field_7728_ != 0) {
                NetWinFactionRecord *const records =
                    static_cast<NetWinFactionRecord *>(faction_records_);
                int faction = 1;
                for (int player_slot = 1; player_slot < 8;
                     ++player_slot, ++faction) {
                    records[faction].owner_ = PlayersData[player_slot].unk_102;
                }

                NetWinSessionOptions.option_4_ = static_cast<uint8_t>(
                    MapLongitudeBounds / 2);
                NetWinSessionOptions.option_5_ =
                    static_cast<uint8_t>(MapLatitudeBounds);
                NetWinSessionData.scenario_text_wire_tail_[0] = 0;

                char *scenario_name = strrchr(
                    loaded_file_path_, '\\');
                if (scenario_name == nullptr) {
                    scenario_name = loaded_file_path_;
                } else {
                    ++scenario_name;
                }
                strcpy(NetWinSessionData.scenario_text_wire_tail_, scenario_name);
            }

            if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
                NetWinSessionOptions.rule_bits_c_ |= 0x10u;
            } else {
                NetWinSessionOptions.rule_bits_c_ &= ~0x10u;
            }
            NetWinSessionOptions.difficulty_ =
                static_cast<uint8_t>(GameDataStateStorage.fields_.diff_level_current_);
            NetWinSessionOptions.status_ =
                static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
            NetWinSessionOptions.rule_bits_a_ = GameDataStateStorage.fields_.game_rules_;

            NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
            NetWinSessionOptionsPacket options_packet;
            options_packet.opcode_ = 0x2F02;
            options_packet.options_ = NetWinSessionOptions;
            NetDaemonNet.Net::send(&options_packet, sizeof(options_packet),
                                   NetDaemonNet.host_player_id(), 1);
        }
    }

    if (field_7728_ != 0 && field_7728_ != 1) {
        NetWinFactionRecord *const records =
            static_cast<NetWinFactionRecord *>(faction_records_);
        records[0].record_count_ = 8;
        records[0].record_type_ = static_cast<uint8_t>(
            bit_count(static_cast<int>((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFEu)));
        records[0].active_ = 0;

        for (int player_slot = 1; player_slot < 8; ++player_slot) {
            NetWinFactionRecord &record = records[player_slot];
            const Player &player = Players[player_slot];
            const uint8_t player_bit =
                static_cast<uint8_t>(1u << player_slot);
            if ((NetWinSessionOptions.status_ & player_bit) == 0) {
                record.active_ = 0;
                continue;
            }

            record.active_ = 1;
            record.owner_ = PlayersData[player_slot].unk_102;
            strcpy(record.filename_, player.filename);
            strcpy(record.leader_name_, player.name_leader);
            strcpy(record.formal_faction_name_, player.formal_name_faction);
            strcpy(record.noun_faction_, player.noun_faction);
            strcpy(record.adjective_faction_, player.adj_name_faction);
            strcpy(record.leader_title_, player.title_leader);
            unsigned char *const allocation_bytes =
                reinterpret_cast<unsigned char *>(records);
            strcpy(reinterpret_cast<char *>(allocation_bytes
                                             + player_slot * 0x190 + 0x112),
                   player.adj_leader);
            records[player_slot + 1].field_2_[0] =
                static_cast<uint8_t>(player.is_leader_female);

            StringTemp[0] = 0;
            strcat(StringTemp, record.filename_);
            strcat(StringTemp, NetWinPcxSuffix);
            if (faction_art.load_pcx(StringTemp, 0, 0x0A, 0xEC) != 0) {
                faction_art.load_pcx(NetWinDefaultPcx, 0, 0x0A, 0xEC);
            }
            record.color_ =
                static_cast<uint8_t>(faction_art.get_pixel(4, 0x2F3));
        }
        send_faction_data(-1);
    } else {
        if (initial_mode != 0 && initial_mode != 1) {
            const NetWinFactionRecord *const source_records =
                static_cast<const NetWinFactionRecord *>(
                    faction_records_copy_);
            const unsigned int record_bytes =
                static_cast<unsigned int>(source_records[0].record_count_) *
                    399u +
                0x194u;
            memcpy(faction_records_, faction_records_copy_, record_bytes);
            send_faction_data(-1);
        }

        char *const default_faction_text =
            label_get(0xFE4 / 4);
        for (int player_slot = 1; player_slot < 8; ++player_slot) {
            NetWinLobbyRow &row = NetWinLobbyRows[player_slot];
            row.faction_or_color_ = -1;
            strcpy(row.leader_name_, default_faction_text);
            strcpy(row.noun_faction_, default_faction_text);
        }
    }

    NetWinLobbyFullPacket assigned_packet;
    for (int packet_row = 0; packet_row < 7; ++packet_row) {
        assigned_packet.rows_[packet_row] = NetWinLobbyRows[packet_row + 1];
    }
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        assigned_packet.header_.opcode_ = 0x04;
        assigned_packet.header_.flags_ = 0x4F;
        assigned_packet.header_.slot_ = -1;
        assigned_packet.header_.field_8_ = 0;
        NetDaemonNet.Net::send(&assigned_packet, sizeof(assigned_packet), 0, 1);
    }

    NetWinSessionData.mode_ = static_cast<uint32_t>(field_7728_);
    NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    NetWinSessionLobbyPacket session_packet;
    session_packet.header_.opcode_ = 0x03;
    session_packet.header_.flags_ = 0x2F;
    session_packet.session_ = NetWinSessionData;
    NetDaemonNet.Net::send(&session_packet, sizeof(session_packet),
                           NetDaemonNet.host_player_id(), 1);

    if (field_7728_ != 0) {
        NetWinLobbyLeavePacket leave_packet;
        leave_packet.header_.opcode_ = 0x0E;
        leave_packet.header_.flags_ = 0x2F;
        NetDaemonNet.Net::send(&leave_packet, sizeof(leave_packet),
                               NetDaemonNet.host_player_id(), 1);
    }
}

/*
Purpose: The cloud-cover picker: a popup titled from the string table with
         the two cloud options, the answer stored in the ninth slot's cloud
         byte and sent to the peer.
// ORIGINAL: 0x0047F5F0 ?pick_clouds@NetWin@@QAEXXZ 0x0047F5F0-0x0047F91C;0x00657385-0x0065745D FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47F721/47F735/47F73B: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    dialogs.item(label_get(0xFE4 / 4), 0);
    int index = 1;
    for (int slot = 0xF84 / 4; slot < 0xF90 / 4; ++slot) {
        dialogs.item(label_get(slot), index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.cloud_cover_ =
            static_cast<unsigned char>(answer - 1);
    }
    pick_send_state();
}

/*
Purpose: The ocean-coverage picker: the same popup walk with the three ocean
         options; the answer lands in the ninth slot's ocean byte.
// ORIGINAL: 0x0047EC60 ?pick_ocean@NetWin@@QAEXXZ 0x0047EC60-0x0047EF8C;0x006570FD-0x006571D5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47ED91/47EDA5/47EDAB: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    dialogs.item(label_get(0xFE4 / 4), 0);
    int index = 1;
    for (int slot = 0xF60 / 4; slot < 0xF6C / 4; ++slot) {
        dialogs.item(label_get(slot), index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.ocean_coverage_ =
            static_cast<unsigned char>(answer - 1);
    }
    pick_send_state();
}

/*
Purpose: The erosion picker: the same popup walk with the three erosion
         options; the answer lands in the ninth slot's erosion byte.
// ORIGINAL: 0x0047EF90 ?pick_erosion@NetWin@@QAEXXZ 0x0047EF90-0x0047F2BC;0x006571D5-0x006572AD FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47F0C1/47F0D5/47F0DB: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    dialogs.item(label_get(0xFE4 / 4), 0);
    int index = 1;
    for (int slot = 0xF6C / 4; slot < 0xF78 / 4; ++slot) {
        dialogs.item(label_get(slot), index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.erosion_ =
            static_cast<unsigned char>(answer - 1);
    }
    pick_send_state();
}

/*
Purpose: The native-life picker: the same popup walk with the three fungal
         options; the answer lands in the ninth slot's native byte.
// ORIGINAL: 0x0047F2C0 ?pick_native@NetWin@@QAEXXZ 0x0047F2C0-0x0047F5EC;0x006572AD-0x00657385 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47F3F1/47F405/47F40B: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    dialogs.item(label_get(0xFE4 / 4), 0);
    int index = 1;
    for (int slot = 0xF90 / 4; slot < 0xF9C / 4; ++slot) {
        dialogs.item(label_get(slot), index++);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.native_life_ =
            static_cast<unsigned char>(answer - 1);
    }
    pick_send_state();
}

/*
Purpose: The planet-size picker: a popup with the title and one entry per
         name in the 0x80-stride name block at 0x6614, counted by the word at
         0x80A6F0; the answer lands in the ninth slot's planet byte.
// ORIGINAL: 0x0047E930 ?pick_planet_size@NetWin@@QAEXXZ 0x0047E930-0x0047EC60;0x00657025-0x006570FD FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47EA6D/47EA7A/47EA80: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    dialogs.item(label_get(0xFE4 / 4), 0);
    // Native 47E9F4 reloads the bound after each owned-dialog insertion.
    for (int index = 0; index < NetWinPlanetSizeLabelCount; ++index) {
        dialogs.item(world_size_labels_[index], index + 1);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.planet_size_ =
            static_cast<unsigned char>(answer - 1);
    }
    pick_send_state_with_captured_target();
}

static void netwin_set_rect(RECT *rect, int left, int top, int right, int bottom) {
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

// ORIGINAL: 0x00482490 ?exec@NetWin@@QAEHXZ 0x00482490-0x00482D84
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00482490.cpp; original identity and primary482490..482D84 span retained", "Pinned original 0x004824C2 pushes callback 0x00483820, count 7, stride 0x17C and the stack array at EBP-0xAB0 before calling 0x00402EE0; originally the helper calls the callback with each element as ECX. Current source uses a seven-element array of constructed payload wrappers; VC6 supplies its non-EH ??_H iterator and the wrapper constructor returns the receiver, matching the original 0x483820 ABI. 0x00483820 writes offsets 0, 1, 2, 3, 4 and 0x178, matching the constructed row payload. Global wire rows remain passive; this composition does not assert an original class hierarchy.", "recovery/tasks/netwin-owner-coverage.json; revisions1-3 authorize coupled ownership and consumer changes; no matching or runtime acceptance implied."], "reason": "Recover the complete lobby window lifecycle with typed setup rectangles, existing controls, stack-local lobby rows and real receiver-free callbacks."}
// TRIED: full transcription treating `self` as `int *` (matches the original's word-indexed field layout exactly); vtable slot057 retyped to (int,int,int) in this file's VCall, verified against raw disasm; UnionRect/CharUpperA reached via the IAT slot pattern. Reaches MISMATCH #6 'xor' vs 'mov' (register allocation), not chased further given size (2292 bytes).
int NetWin::exec() {
    if (ExitTurnLoop != 0) {
        return 0;
    }

    setup_faction_data(nullptr);
    NetDaemonNet.enable_join();

    struct ExecLobbyPacket {
        NetWinLobbyPacketHeader header_;
        NetWinConstructedLobbyRow rows_[7];
    } lobby_message;
    static_assert(sizeof(ExecLobbyPacket) == 0xA74, "exec lobby message size");
    static_assert(offsetof(ExecLobbyPacket, rows_) == 0x10,
                  "exec lobby payload offset");
    // Ordinary array construction owns the original 0x402EE0 iterator and
    // 0x483820 callback. It runs after enable_join even on the no-send path;
    // process-global rows remain passive storage with no startup constructor.
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        lobby_message.header_.opcode_ = 0x04;
        lobby_message.header_.flags_ = 0x0F;
        lobby_message.header_.slot_ = -1;
        lobby_message.header_.field_8_ = 0;
        // Preserve the original untouched header bytes +2..3 and +C..F.
        // Whole-packet zeroing would deliberately change the wire behavior.
        memcpy(lobby_message.rows_, &NetWinLobbyRows[1], 0xA64);
        lobby_message.header_.flags_ |= 0x40;
        NetDaemonNet.send(&lobby_message, 0xA74, 0, 1);
    }

    while (faction_records_ == nullptr) {
        ::do_task();
    }

    NetDaemonNet.primary_state_.flags_ = 1;
    NetWinExecAdjacentState = 0;
    field_7614_ = 0;
    spot_.init(0x55);

    if (text_open(AlphaxFileID, "WORLDSIZE") == 0) {
        text_get();
        NetWinPlanetSizeLabelCount = text_item_number();
        if (NetWinPlanetSizeLabelCount > 0x20) {
            NetWinPlanetSizeLabelCount = 0x20;
        }
        for (int index = 0; index < NetWinPlanetSizeLabelCount; ++index) {
            char *const label = world_size_labels_[index];
            text_get();
            label[0] = '\0';
            strcat(label, text_item());
            char *const separator = strchr(label, '|');
            if (separator != nullptr) {
                *separator = ' ';
            }
        }
        text_close();
    }

    GraphicWin::init(0, 0, 0x320, 0x258, &NetWinWindowTitleInitialText,
                     0x05000000, nullptr, nullptr, nullptr);
    Win::center();
    field_16C_ = 0x1A;

    // Native setup rectangle span [this+0xA14,this+0xD34): 50 RECTs.
    // Header proposal replaces its existing 200 dwords with setup_rects_[50]
    // and migrates existing field_A14_ painter access to setup_rects_[0].
    RECT *const r = setup_rects_;
    const RECT &work = Buffer::rect2_;
    r[0].left = work.left + 3;
    r[0].top = work.top + 3;
    r[0].right = work.right - 3;
    r[0].bottom = work.top + 0x1D;

    netwin_set_rect(&r[1], 7, 0x3A, 0x134, 0x180);
    netwin_set_rect(&r[2], 7, 0x24, 0x134, 0x38);
    ::UnionRect(&r[3], &r[1], &r[2]);
    r[3].left -= 3;
    r[3].top -= 3;
    r[3].right += 3;
    r[3].bottom += 3;

    netwin_set_rect(&r[4], 0x13D, 0x24, 0x319, 0x38);
    netwin_set_rect(&r[5], 0x13A, 0x21, 0x31C, 0xEB);

    for (int i = 0; i < 7; ++i) {
        const int top = 0x3A + 0x16 * i;
        const int bottom = 0x4E + 0x16 * i;
        netwin_set_rect(&r[6 + i], 0x161, top, 0x213, bottom);
        netwin_set_rect(&r[13 + i], 0x215, top, 0x2A1, bottom);
        netwin_set_rect(&r[20 + i], 0x2A3, top, 0x319, bottom);
        netwin_set_rect(&r[27 + i], 0x14F, top, 0x15F, bottom);
        netwin_set_rect(&r[36 + i], 0x13D, top, 0x14D, bottom);
    }

    netwin_set_rect(&r[34], 0x13D, 0xD4, 0x229, 0xE8);
    netwin_set_rect(&r[35], 0x22C, 0xD4, 0x318, 0xE8);

    netwin_set_rect(&r[43], 0x13D, 0xF1, 0x319, 0x162);
    netwin_set_rect(&r[44], 0x13D, 0x164, 0x319, 0x180);
    ::UnionRect(&r[45], &r[43], &r[44]);
    r[45].left -= 3;
    r[45].top -= 3;
    r[45].right += 3;
    r[45].bottom += 3;

    netwin_set_rect(&r[46], 7, 0x189, 0x319, 0x19D);
    netwin_set_rect(&r[48], 0x1B, 0x19F, 0x18F, 0x251);
    netwin_set_rect(&r[49], 0x1A5, 0x19F, 0x251, 0x319);
    RECT union_scratch;
    ::UnionRect(&union_scratch, &r[46], &r[48]);
    ::UnionRect(&r[47], &union_scratch, &r[49]);
    r[47].left -= 3;
    r[47].top -= 3;
    r[47].right += 3;
    r[47].bottom += 3;

    stringBox_.init(r[43].left + 3, r[43].top + 3,
                    r[43].right - r[43].left - 6,
                    r[43].bottom - r[43].top - 6, this, nullptr, -1);
    // init closes the prior list and clears its callback; install afterward.
    stringBox_.set_write_strings_callback(&NetWin::string_callback);
    Buffer *string_canvas = static_cast<Buffer *>(&stringBox_);
    string_canvas->set_text_color(0xFF, -1, 1, 1);
    stringBox_.set_visible_rows(100);
    stringBox_.show(0);

    editBox_.init(r[44].left, r[44].top,
                  r[44].right - r[44].left,
                  r[44].bottom - r[44].top, this, 0);

    // Existing EditBox members preserve the native field stores; window
    // flags belong to its actual Win base, not the separate CheckBox.
    editBox_.add_window_flags(0x01000100);
    editBox_.field_B1C_ = 9;
    editBox_.field_B30_ = 0;
    Buffer *edit_canvas = static_cast<Buffer *>(&editBox_);
    edit_canvas->set_text_color(0xE1, -1, 1, 1);
    editBox_.field_B20_ = 0x41;
    editBox_.field_B24_ = 0xE1;
    editBox_.field_B14_ = 0x80;
    editBox_.show(0);

    char *button_text = label_get(
        (NetDaemonNet.primary_state_.leading_words_[0] != 0 ? 0x1058 : 0x1050) / 4);
    flatButton1_.init(button_text, 10, r[34].left, r[34].top,
                      r[34].right - r[34].left, r[34].bottom - r[34].top,
                      this, 0);
    Buffer *button_canvas = static_cast<Buffer *>(&flatButton1_);
    button_canvas->set_font(find_font(0xE, 1), 0, 0, 0);
    flatButton1_.redraw();
    flatButton1_.set_text_color(0x41, -1, 1, 1);

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(0x198 / 4));
    if (Language != 2) {
        CharUpperA(StringTemp);
    }
    flatButton2_.init(StringTemp, -2, r[35].left, r[35].top,
                      r[35].right - r[35].left, r[35].bottom - r[35].top,
                      this, 0);
    Buffer *second_button_canvas = static_cast<Buffer *>(&flatButton2_);
    second_button_canvas->set_font(find_font(0xE, 1), 0, 0, 0);
    flatButton2_.redraw();
    flatButton2_.set_text_color(0x41, -1, 1, 1);

    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        NetWinSessionOptions.rule_bits_a_ = AlphaIniPrefs.rules;
        NetWinSessionOptions.difficulty_ = static_cast<uint8_t>(DefaultPrefs.difficulty);
        NetWinSessionOptions.rule_bits_c_ |= 0x10;
        NetWinLobbyRows[1].field_2_ = static_cast<uint8_t>(DefaultPrefs.difficulty);
        NetWinSessionOptions.rule_bits_b_ = AlphaIniPrefs.semaphore;
        NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
        NetWinSessionOptionsPacket packet;
        // Original 0x482C64..0x482C88 writes only the opcode and six options
        // words; reserved header bytes +2..+0xF retain prior stack contents.
        packet.opcode_ = 0x2F02;
        packet.options_ = NetWinSessionOptions;
        NetDaemonNet.send(&packet, 0x28, NetDaemonNet.host_player_id(), 1);
    }

    NetDaemonNet.set_polling(1);
    set_modal(0, &NetWin::modal_callback_daemon, nullptr);
    hide();
    NetDaemonNet.close_game();

    if (field_7614_ > 0) {
        on_redraw();
        on_update();
        prepare_game();
    }
    NetDaemonNet.primary_state_.flags_ = 2;

    if (faction_records_ != nullptr) {
        free(faction_records_);
        faction_records_ = nullptr;
    }
    if (faction_records_copy_ != nullptr) {
        free(faction_records_copy_);
        faction_records_copy_ = nullptr;
    }
    field_761C_ = -1;
    field_7620_ = -1;
    spot_.shutdown();
    stringBox_.close();
    editBox_.close();
    checkBox_.close();
    flatButton1_.close();

    if (ExitTurnLoop != 0) {
        return 0;
    }
    return field_7614_;
}

/*
Purpose: The player-count picker: entries numbered max(2, lobby count) to 7,
         the entry itself the answer; the ninth slot's player byte stores
         count - 1.
// ORIGINAL: 0x0047E010 ?pick_num_players@NetWin@@QAEXXZ 0x0047E010-0x0047E33B;0x00656D9D-0x00656E75 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47E15D/47E16A/47E170: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    int first = NetDaemonNet.player_count_;
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
        // Native 47E11A..47E132 retains the pre-modal lower bound.
        int count = static_cast<int>(static_cast<uint32_t>(answer) +
                                     static_cast<uint32_t>(first));
        if (count < 2) count = 2;
        else if (count > 7) count = 7;
        NetWinSessionOptions.player_count_ = static_cast<unsigned char>(count);
    }
    pick_send_state_with_captured_target();
}

/*
Purpose: The global-difficulty picker: a popup with the title and the six
         difficulty names off alpha's table; the ninth slot's global byte
         stores the answer.
// ORIGINAL: 0x0047E340 ?pick_global_diff@NetWin@@QAEXXZ 0x0047E340-0x0047E640;0x00656E75-0x00656F4D FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47E445/47E459/47E45F: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    Dialogs &dialogs = pop_menu.dialogs();
    for (int index = 0; index < 6; ++index) {
        dialogs.item(StringTable.get(Difficulty[index]), index);
    }
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.difficulty_ =
            static_cast<unsigned char>(answer);
    }
    pick_send_state();
}

/*
Purpose: The time-control picker: the popup is filled by the source-owned
         time-control list builder, then the same
         modal walk; the ninth slot's time byte stores the answer.
// ORIGINAL: 0x0047E640 ?pick_time@NetWin@@QAEXXZ 0x0047E640-0x0047E924;0x00656F4D-0x00657025 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned47E72A/47E73E/47E744: copy six option dwords to packet+10, write opcode word2F02, send28. Shared pick_send_state preserves these writes; header+2..F remains unspecified."], "reason": "Recover the original options packet through genuine NetWin state and typed packet storage."}
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
    pop_menu.init();
    time_controls_dialog(&pop_menu);
    int answer = pick_answer(*this, pop_menu);
    if (answer >= 0) {
        NetWinSessionOptions.time_control_ =
            static_cast<unsigned char>(answer);
    }
    pick_send_state();
}

/*
Purpose: The modal pump's callback: drain the daemon's receive queue (one
         poll, then a second, then the rest), poke the dialog's own slot-12
         virtual so the waiting panel repaints, and - if the readiness scan
         has not already passed - walk the static faction records looking for
         an empty slot and answer whether the wait should end.
// ORIGINAL: 0x00483190 ?modal_callback@NetWin@@QAEHXZ 0x00483190-0x00483234 FILE
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
        on_redraw();
    }
    int result = 1;
    if (field_7614_ == 0) {
        field_7618_ = 1;
        if (NetDaemonNet.player_count_ >= 1) {
            NetWinLobbyRow *record = &NetWinLobbyRows[1];
            int index = 1;
            do {
                if (record->ready_or_occupied_ == 0) {
                    field_7618_ = 0;
                }
                index++;
                ++record;
            } while (index <= NetDaemonNet.player_count_);
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
// symbol    ?modal_callback_daemon@NetWin@@SAHXZ
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
int __cdecl NetWin::modal_callback_daemon() {
    if (NetDaemonNet.receive() != 0) {
        if (NetDaemonNet.receive() != 0) {
            do {
            } while (NetDaemonNet.receive() != 0);
        }
        NetWinDialog.on_redraw();
    }
    if (NetWinDialog.field_7614_ != 0) {
        return 0;
    }
    NetWinDialog.field_7618_ = 1;
    int count = NetDaemonNet.player_count_;
    if (count >= 1) {
        int all_used = 1;
        NetWinLobbyRow *record = &NetWinLobbyRows[1];
        do {
            if (record->ready_or_occupied_ == 0) {
                all_used = 0;
            }
            ++record;
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
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004832D0 ?string_callback@NetWin@@QAAXHH@Z 0x004832D0-0x00483302 FILE BYTE_EXACT
// symbol    ?string_callback@NetWin@@SAXHH@Z
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
void __cdecl NetWin::string_callback(int payload, int) {
    Buffer *canvas = static_cast<Buffer *>(&NetWinDialog.stringBox_);
    canvas->set_text_color(NetWinDialog.get_player_color(payload), 0, 1, 1);
}

/*
Purpose: Route one dialog key press: Enter sends the chat line the edit box
         holds (the daemon maps the sender's process id, the message goes out
         as opcode 0x1E00, the edit box clears), Escape tears the session
         dialogs down the same way close() does and answers 1, everything
         else goes to the edit box's own slot-26 key handling.
// ORIGINAL: 0x00483310 ?on_key_down@NetWin@@QAEHH@Z 0x00483310-0x00483414 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00483310.cpp"], "reason": "Retain the existing source implementation and complete original mapping. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?on_key_down@NetWin@@UAEHH@Z
// Current virtual override emits UAE; retain the catalogued QAE identity above.
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
            return editBox_.on_key_down(key);
        }
        release_modal();
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
        stringBox_.close();
        editBox_.close();
        checkBox_.close();
        flatButton1_.close();
        return 1;
    }
    NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    message_chat(0x1E00, 0,
                 editBox_.text_);
    editBox_.set_text(0);
    return 1;
}

/*
Purpose: Format one chat line and hand it to the message list: the sender's
         name (through the daemon's name table, upper-cased in place), then
         ": ", a space, and the message text.
// ORIGINAL: 0x00483420 ?new_chat@NetWin@@QAEXPADH@Z 0x00483420-0x004834C5 FILE BYTE_EXACT
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
    strcat(text, NetDaemonNet.get_player_name(NetDaemonNet.process_slots_[slot].pid_));
    CharUpperA(text);
    strcat(text, ": ");
    strcat(text, " ");
    strcat(text, message);
    stringBox_.add(text, slot, 1);
}

/*
Purpose: Retain the historical original identity while binding its current
         owner to CheckBox's compiler-emitted complete-object destructor.
         This is generated from NetWin's real CheckBox member lifetime; it is
         not a handwritten NetWin method or a manually staged teardown.
// ORIGINAL: 0x004837F0 ?UNK4@NetWin@@QAEXXZ 0x004837F0-0x00483813 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/004837f0.cpp (historical NetWin catalogue interpretation and preserved seed claim)", "Consumer-emitted ??_DCheckBox@@QAEXXZ calls the CheckBox, Dialog and GraphicWin destructors at receiver+0x1C/+0xA34/+0x1C; pinned original 0x004837F9/0x00483804/0x0048380B target 0x0060E740/0x00608E10/0x005D4DD0 respectively. These are CheckBox-relative receiver adjustments, not NetWin-relative.", "src/netwin.h: NetWin owns the actual CheckBox checkBox_ member at +0x4454; ordinary member destruction emits the CheckBox complete-object helper in this consuming translation unit. No NetWin::UNK4 source callers exist."], "reason": "Keep the original NetWin catalogue identity and seed provenance as historical facts, but bind its current source identity to the genuine compiler-generated CheckBox complete-object destructor emitted for NetWin's real member. Do not hand-author cleanup or attribute the CheckBox-relative receivers to NetWin."}
// symbol    ??_DCheckBox@@QAEXXZ
// size      35 bytes
// prototype void (__thiscall ?UNK4@NetWin@@QAEXXZ)(NetWin* this) [historical catalogue interpretation; not the current owner]
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x0060E740
// Homed from src/recovered/004837f0.cpp.
Return Value: n/a
Status: Compiler-generated CheckBox complete-object destructor, emitted from
        NetWin's CheckBox member lifetime in this consuming translation unit.
*/
// Native 0x0047C970 password flow. Popup and its owned members use ordinary
// automatic lifetime; the native explicit close calls in the archived seed
// are not duplicated over the recovered destructor chain.
// ORIGINAL: 0x0047C970 ?pick_pass@NetWin@@QAEHH@Z 0x0047C970-0x0047CF32;0x006569C4-0x00656AED
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0047c970.cpp", "recovery/tasks/netwin-owner-coverage.json", "Pinned47CEB9/47CED2/47CEDB..47CF03: slot dword, opcode word, explicit +8 zero, row at +10 of size17C; send18C. The opcode does not write +2..3 and +C..F is untouched."], "reason": "Recover full password dialog using genuine Popup lifecycle, input override text and shared parse output, faction validation and native sixteen-byte-header/full-row wire layout. No exactness or game acceptance claim."}
// TRIED: MISMATCH #10 'push' vs 'mov' (framed build). SEH-frame password dialog: local `Popup popup;` is a real automatic object (BasePop fully laid out in the scaffold: embedded FlatButton x2 and Sprite, plus Popup's own `Scroll scroll_` which itself embeds two more FlatButtons at offsets 0xAAC/0x15F8 - matches the trailer thunks in the disassembly exactly), so ctor/dtor calls are emitted by the language rather than hand-modelled. Explicit popup.close()/ popup.scroll_.close() only where the disassembly names them, not at every scope exit. Did not attempt to reproduce the compiler's own SEH funclet layout - accepted as an "omitted as unreachable" divergence per the precedent in dialog.cpp/graphicwin.cpp.
int NetWin::pick_pass(int payload) {
    Popup popup;
    const int slot = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    const bool faction_mode = NetWinSessionData.mode_ == 2 ||
                              NetWinSessionData.mode_ == 3;

    if (!faction_mode && payload != 0) {
        return 1;
    }

    NetWinLobbyRow &row = NetWinLobbyRows[slot];
    const int faction = row.faction_or_color_;
    if (faction == -1) {
        row.owner_ = 0;
        return 1;
    }

    if (faction_mode) {
        const uint32_t expected_hash = static_cast<NetWinFactionRecord *>(
            faction_records_)[faction].owner_;
        // Native 47CB5A..47CB64 bypasses the row read for a zero hash.
        if (expected_hash == 0 || expected_hash == row.owner_) {
            if (payload == 0) {
                X_pop(ScriptPopupFile, "PASSWORDOK", nullptr);
            }
            return 1;
        }
    }

    uint32_t entered_hash = 0;
    for (;;) {
        entered_hash = 0;
        const char *prompt;
        if (faction_mode) {
            NetWinFactionRecord *const records =
                static_cast<NetWinFactionRecord *>(faction_records_);
            parse_says(0, records[faction].noun_faction_, -1, -1);
            prompt = "ENTERPASSWORDFACTION";
        } else {
            prompt = "ENTERPASSWORDNEW";
        }

        if (X_pop_ask(prompt, 0x50, &NetWinPasswordPromptInitialText,
                      nullptr, 0x800040) == -1) {
            return 0;
        }
        const bool have_password = ParseStrBuffer[0].str[0] != 0;
        if (have_password) {
            entered_hash = checksum_password(ParseStrBuffer[0].str);
        }
        if (faction_mode) {
            // Native 47CD43..47CD4C checks the live post-modal allocation.
            NetWinFactionRecord *const records =
                static_cast<NetWinFactionRecord *>(faction_records_);
            if (records[faction].owner_ != entered_hash) {
                X_pop("BADPASSWORD", nullptr);
                return 0;
            }
            row.owner_ = entered_hash;
            return 1;
        }

        if (!have_password) {
            break;
        }

        X_pop_ask("VERIFYPASSWORD", 0x50,
                  &NetWinPasswordVerifyInitialText, nullptr, 0x800000);
        const uint32_t verify_hash = ParseStrBuffer[0].str[0] == 0
            ? 0 : checksum_password(ParseStrBuffer[0].str);
        if (entered_hash == verify_hash) {
            break;
        }
        X_pop("DIFFERENTPASSWORDS", nullptr);
    }

    row.owner_ = entered_hash;
    // Native 47CE7B copies before checking whether the slot may be sent.
    NetWinLobbyRow updated;
    memcpy_call(&updated, &row, 0x17C);
    if (slot >= 1) {
        NetWinLobbySinglePacket packet;
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x2F;
        packet.header_.slot_ = slot;
        packet.header_.field_8_ = 0;
        memcpy(&packet.row_, &updated, 0x17C);
        NetDaemonNet.Net::send(&packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
    }
    return 1;
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
// symbol    ?on_key_click@NetWin@@UAEHHH@Z
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
// symbol    ?on_mouse_move@NetWin@@UAEXHH@Z
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
Purpose: Construct a lobby row, not the GraphicWin base. Native4824C2
         passes483820 to the402EE0 iterator for seven stack-local17C-byte
         rows. Owner178 and the five leading bytes are the only writes;
         remaining row storage stays untouched; EAX returns the receiver.
         Passive global wire rows do not run this constructor.
         Historical catalogue owner
         NetWin::UNK5 is retained as identity, not a receiver interpretation.
// ORIGINAL: 0x00483820 ?UNK5@NetWin@@QAEXXZ 0x00483820-0x0048383C BYTE_EXACT
// symbol    ??0NetWinConstructedLobbyRow@@QAE@XZ
// size      28 bytes
// prototype void (__thiscall ?UNK5@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
NetWinConstructedLobbyRow::NetWinConstructedLobbyRow() {
    payload_.owner_ = 0;
    payload_.prefix_ = 0xFF;
    payload_.ready_or_occupied_ = 0;
    payload_.field_2_ = 0;
    payload_.faction_or_color_ = -1;
    payload_.field_4_ = 2;
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
Purpose: Answer the multiplayer setup dialog's button click. Id -2 hangs up
         the session (store -1 into the dialog's +0x7614 slot) and every id
         but 10 returns; id 10 - the join/start button - validates the lobby
         before broadcasting: the local slot's ready byte is set and every
         peer row probed for a free slot (any free slot, or the
         NOTALLREADY prompt refuses), a running session re-scans its
         faction-record run against the occupied slot rows so a duplicated
         faction raises the PASSWORDSNOTVALIDATED prompt and a 0x2f0e
         leave packet instead, and only then does the click re-mark the
         lobby rows and send either the 0x4f04 full-state packet (0xa74
         bytes including its header) or the single-slot 0x2f04 packet
         (0x18c bytes, when the local slot is at least one).

The original writes only the opcode and each path's selected header/payload
fields. Reserved header bytes and the leave-packet tail remain uninitialized;
do not replace those unspecified bytes with a deterministic zero fill.
*/
// ORIGINAL: 0x0047B450 ?on_button_clicked@NetWin@@QAEXH@Z 0x0047B450-0x0047B746 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The original NetWin constructor installs primary vtable 0x0066CCEC; inherited Win slot 44 at 0x0066CD9C targets 0x0047B450.", "NetDaemon::on_player_deleted directly calls 0x0047B450 with NetWinDialog as receiver and -2 as argument; retired seed git:7eb4d2e7:src/unrecovered/0047b450.cpp records the historical body.", "Pinned47B5FD..47B639/47B664..47B675/47B701..47B738: full and single packets write opcode word, slot and +8 zero then copied rows; leave writes only opcode word. +2..3/+C..F and leave tail are untouched."], "reason": "NetWin owns the lobby button handler as its inherited Win callback and shares the typed lobby rows and session storage with its network senders."}
// symbol    ?on_button_clicked@NetWin@@UAEXH@Z
// TRIED: no SEH frame here (plain `sub esp,0xd9c`, no fs:[0] chain), so this is a direct disassembly transcription rather than a lever candidate. Used one flat byte buffer plus raw `base - N` stores/memcpy matching each `ebp - N` write and each record's 0x17c-byte stride, rather than named locals, since the frame mixes several differently-shaped record buffers with no natural C struct boundary between them.
// size      758 bytes
// prototype void (__thiscall ?on_button_clicked@NetWin@@QAEXH@Z)(NetWin* this, int)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E25E0 0x005BF330 0x00630080 0x00645930
// Return Value: n/a
// Status: Recovered lobby click handling; original seed retired.
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
void NetWin::on_button_clicked(int clicked_id) {
    if (clicked_id == -2) {
        field_7614_ = -1;
        return;
    }
    if (clicked_id != 10) {
        return;
    }

    const int local_slot = NetDaemonNet.pid_2_idx(NetDaemonNet.local_player_id());
    const int player_count = NetDaemonNet.player_count_;

    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        uint8_t &ready = NetWinLobbyRows[local_slot].ready_or_occupied_;
        const int slot_count = player_count + 1;
        int slot = 1;
        ready = 1;
        if (slot_count > 1) {
            do {
                if (NetWinLobbyRows[slot].ready_or_occupied_ == 0) {
                    break;
                }
                ++slot;
            } while (slot < slot_count);
        }
        ready = 0;

        if (slot != slot_count &&
            X_pop(ScriptPopupFile, "NOTALLREADY",
                                0) != 0) {
            return;
        }

        const int transport = NetWinSessionData.mode_;
        if (transport == 2 || transport == 3) {
            const NetWinFactionRecord *const records =
                static_cast<const NetWinFactionRecord *>(faction_records_);
            for (int faction = 1; faction < 8; ++faction) {
                const uint32_t owner = records[faction].owner_;
                if (owner != 0) {
                    const int scan_count = player_count + 1;
                    int scan = 1;
                    if (scan_count > 1) {
                        do {
                            if (NetWinLobbyRows[scan].faction_or_color_ ==
                                faction) {
                                break;
                            }
                            ++scan;
                        } while (scan < scan_count);
                    }
                    if (scan == scan_count ||
                        owner != NetWinLobbyRows[scan].owner_) {
                        X_pop(ScriptPopupFile,
                                            "PASSWORDSNOTVALIDATED", 0);
                        NetWinLobbyLeavePacket packet;
                        packet.header_.opcode_ = 0x0E;
                        packet.header_.flags_ = 0x2F;
                        NetDaemonNet.Net::send(&packet, sizeof(packet),
                                               NetDaemonNet.host_player_id(), 1);
                        return;
                    }
                }
            }
        }

        for (int ready_slot = 1; ready_slot <= player_count; ++ready_slot) {
            NetWinLobbyRows[ready_slot].ready_or_occupied_ = 1;
        }

        if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
            return;
        }
        NetWinLobbyFullPacket packet;
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x4F;
        packet.header_.slot_ = -1;
        packet.header_.field_8_ = 0;
        memcpy(packet.rows_, &NetWinLobbyRows[1], sizeof(packet.rows_));
        NetDaemonNet.Net::send(&packet, sizeof(packet), 0, 1);
    } else {
        NetWinLobbyRow row = NetWinLobbyRows[local_slot];
        row.ready_or_occupied_ = row.ready_or_occupied_ == 0 ? 1 : 0;
        if (local_slot >= 1) {
            NetWinLobbySinglePacket packet;
            packet.header_.opcode_ = 0x04;
            packet.header_.flags_ = 0x2F;
            packet.header_.slot_ = local_slot;
            packet.header_.field_8_ = 0;
            packet.row_ = row;
            NetDaemonNet.Net::send(&packet, sizeof(packet),
                                   NetDaemonNet.host_player_id(), 1);
        }
    }
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

/*
Purpose: Send one faction's data block over the net: make sure the
         MessageFactionData clone exists (asking the dialog to rebuild it),
         then frame the clone with the 0xF0D header and ship it to
         everyone or to the slot a1 names.

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0047B020 ?send_faction_data@@YAXH@Z 0x0047B020-0x0047B0E1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/netwin.h declares this network-dialog synchronization API; src/netwin.cpp owns NetWinDialog, NetWinLobbyRows, the session state, and the player-ID globals; slot ids are NetDaemonNet AlphaNet members.", "The source body preserves the pinned same-player gate, byte count, allocation size, copy extent/order, unfreed frame and broadcast/slot destination branches."], "reason": "Move the complete existing faction-data sender to the owner of its typed dialog, record table and network globals without changing packet writes, send order, leak or historical claim."}
// size      193 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00481FD0 0x005D4510 0x00630080 0x00645930
Return Value: n/a
Status: Coverage transcription - claim pending re-measurement
*/
void __cdecl send_faction_data(int slot_id) {
    if (NetDaemonNet.local_player_id() == NetDaemonNet.host_player_id()) {
        if (NetWinDialog.faction_records_ == 0) {
            NetWinDialog.setup_faction_data(0);
        }
        const uint8_t count = NetWinDialog.faction_records_[0].record_count_;
        const int records = count * 133;
        const uint32_t size = records * 3 + 0x1A4;
        char *frame = static_cast<char *>(mem_get(size));
        NetWinFactionDataPacketView packet(frame);
        packet.set_opcode(0xF0D);
        packet.set_faction(-1);
        packet.set_timestamp(0);
        memcpy(packet.payload(), NetWinDialog.faction_records_,
               records * 3 + 0x194);
        if (slot_id < 0) {
            NetDaemonNet.Net::send(frame, size, 0, 1);
            return;
        }
        NetDaemonNet.Net::send(
            frame, size, NetDaemonNet.process_slots_[slot_id].pid_, 1);
    }
}

/*
Purpose: Broadcast the full-state block to every player: stamp the seven
         per-slot packet headers into one 0xA74-byte frame, copy the
         0xA64-byte state, and send it to everyone (0x4F04) or to the slot
         a1 names (0x1F04) once the turn hand-over allows it.

// ORIGINAL: 0x0047B0F0 ?send_all_player_packets@@YAXH@Z 0x0047B0F0-0x0047B1CE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/netwin.h declares this API and src/netwin.cpp owns the seven-row lobby payload. The destination IDs use NetDaemonNet AlphaNet process slots.", "The gate at 0x0093E8C0 is NetDaemonNet.primary_state_.leading_words_[0] (+0x1B30), the same storage game.cpp names TurnsAssigned."], "reason": "Move the complete state-frame sender to netwin.cpp and use the existing typed turn-handover member; preserve initialization-before-gate, packet copy extent and target branches."}
// LEVER: xor/or order - writing the dword-zero field first in the record loop (matching `xor ebx,ebx` before `or ecx,-1`) reproduced the register setup order; `#pragma function(memcpy)` kept the `call _memcpy` instead of an inlined `rep movsd`.
// size      222 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080
Return Value: n/a
Status: Coverage transcription - claim pending re-measurement
*/
void __cdecl send_all_player_packets(int slot_id) {
    NetWinLobbyFullPacket packet;
    NetWinLobbyRow *row = packet.rows_;
    for (int slot = 7; slot != 0; --slot) {
        row->owner_ = 0;
        row->prefix_ = 0xFF;
        row->ready_or_occupied_ = 0;
        row->field_2_ = 0;
        row->faction_or_color_ = -1;
        row->field_4_ = 2;
        ++row;
    }
    if (slot_id != -1 || NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x0F;
        packet.header_.slot_ = -1;
        packet.header_.field_8_ = 0;
        memcpy(packet.rows_, &NetWinLobbyRows[1], sizeof(packet.rows_));
        if (slot_id < 0) {
            packet.header_.flags_ |= 0x40;
            NetDaemonNet.Net::send(&packet, sizeof(packet), 0, 1);
            return;
        }
        packet.header_.flags_ |= 0x10;
        NetDaemonNet.Net::send(
            &packet, sizeof(packet), NetDaemonNet.process_slots_[slot_id].pid_, 1);
    }
}

/*
Purpose: Broadcast one faction's 0x2F04 data packet to the other player:
         shape the 0x18C-byte message (header word, faction word, zero,
         the five-byte prefix and the 0x17C-byte copied payload - the
         image writes the prefix before the copy overwrites it), and hand
         it to the Net sender sized at one target.

// ORIGINAL: 0x0047B1D0 sub_47b1d0 0x0047B1D0-0x0047B25C FILE
// RECOVERY_BINDING: v1 {"evidence": ["The helper sends through NetDaemonNet to NetDaemonNet.host_player_id() and uses the same 0x17C lobby payload/0x18C message contract as the neighboring NetWin packet senders.", "No direct mapped caller is known; this transfer retains the externally callable identity and does not infer a caller."], "reason": "Move this source-owned packet helper alongside its established NetWin senders; retain prefix-before-copy ordering, positive-faction/null-payload guard and the original unclaimed caller status."}
// symbol    ?sub_47b1d0@@YAXHPAX@Z
// size      140 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080 0x00645930
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void __cdecl sub_47b1d0(int faction_id, void *payload) {
    NetWinLobbySinglePacket packet;
    packet.row_.prefix_ = 0xFF;
    packet.row_.faction_or_color_ = -1;
    packet.row_.owner_ = 0;
    packet.row_.ready_or_occupied_ = 0;
    packet.row_.field_2_ = 0;
    packet.row_.field_4_ = 2;
    if (faction_id > 0 && payload != 0) {
        packet.header_.field_8_ = 0;
        packet.header_.opcode_ = 0x04;
        packet.header_.flags_ = 0x2F;
        packet.header_.slot_ = faction_id;
        memcpy(&packet.row_, payload, sizeof(packet.row_));
        NetDaemonNet.Net::send(
            &packet, sizeof(packet), NetDaemonNet.host_player_id(), 1);
    }
}

/*
Purpose: Broadcast the 24-byte session state words: copy them into the
         0x28-byte frame and send it to everyone (0x4F02), to this
         client's counterpart (0x2F02), or to the slot a2 names (0x1F02).

// ORIGINAL: 0x0047B260 ?send_session_packet@@YAXHH@Z 0x0047B260-0x0047B2DE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/netwin.h declares this API; NetWinSessionOptions, NetDaemonNet.host_player_id() and NetDaemonNet are owned/referenced in netwin.cpp.", "The first argument remains unused as in the current body; negative, zero and positive targets keep their distinct packet opcodes and destinations."], "reason": "Move the complete existing 24-byte session packet sender into netwin.cpp without changing payload, unused parameter, target comparison or header selection."}
// size      126 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080
Return Value: n/a
Status: Coverage transcription - claim pending re-measurement
*/
void __cdecl send_session_packet(int slot_id, int target) {
    NetWinSessionOptionsPacket packet;
    memcpy(&packet.options_, &NetWinSessionOptions, sizeof(packet.options_));
    if (target < 0) {
        packet.opcode_ = 0x4F02;
        NetDaemonNet.Net::send(&packet, sizeof(packet), 0, 1);
        return;
    }
    uint32_t who;
    if (target != 0) {
        who = NetDaemonNet.process_slots_[target].pid_;
        packet.opcode_ = 0x1F02;
    } else {
        who = NetDaemonNet.host_player_id();
        packet.opcode_ = 0x2F02;
    }
    NetDaemonNet.Net::send(&packet, sizeof(packet), who, 1);
}

/*
Purpose: Broadcast the session state blob: copy the 0x168-byte payload,
         then send it addressed to everyone (0x4F03), to this client's
         counterpart (0x2F03), or to the slot a2 names (0x1F03).

// ORIGINAL: 0x0047B2E0 ?send_session2_packet@@YAXHH@Z 0x0047B2E0-0x0047B379 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/netwin.h owns the 0x168-byte NetWinSessionData block and declares this sender; the NetDaemonNet owns AlphaNet process slots.", "The first argument remains unused, and packet bytes/header fields and the target==0 comparison are retained."], "reason": "Move the complete second-session sender into netwin.cpp, preserving its 0x178-byte frame, copy, opcode branches and unused first argument."}
// size      153 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00630080
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void __cdecl send_session2_packet(int slot_id, int target) {
    NetWinSessionBlockPacket packet;
    memcpy(&packet.session_, &NetWinSessionData, sizeof(packet.session_));
    if (target < 0) {
        packet.opcode_ = 0x4F03;
        NetDaemonNet.Net::send(&packet, sizeof(packet), 0, 1);
        return;
    }
    uint32_t who;
    if (target == 0) {
        packet.opcode_ = 0x2F03;
        who = NetDaemonNet.host_player_id();
    } else {
        packet.opcode_ = 0x1F03;
        who = NetDaemonNet.process_slots_[target].pid_;
    }
    NetDaemonNet.Net::send(&packet, sizeof(packet), who, 1);
}

/*
// ORIGINAL: 0x004E2450 sub_4e2450 0x004E2450-0x004E245D BYTE_EXACT
// symbol    _sub_4e2450
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_4e2450() {
    NetWinSessionData.mode_ = 0;
    NetWinSessionData.scenario_text_wire_tail_[0] = 0;
}
