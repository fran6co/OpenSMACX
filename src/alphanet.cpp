/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x004E29E0 ?do_join_or_create@AlphaNet@@QAEHXZ 0x004E29E0-0x004E2A2F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e29e0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e29e0.cpp"], "reason": "Existing real xpops4args wrapper drives whole create/join operation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?do_join_or_create@AlphaNet@@QAEHXZ
// ORIGINAL: 0x004E2A30 ?create@AlphaNet@@QAEHXZ 0x004E2A30-0x004E2E4B;0x0065C664-0x0065C771 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e2a30.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e2a30.cpp", "Historical seed spelling ?create@AlphaNet@@QAEHXZ; current source mapping ?create@AlphaNet@@QAEHXZ; emitted symbol ?create@AlphaNet@@QAEHXZ."], "reason": "Create flow with typed node/session fields; preserve complete seed EH spans. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// symbol    ?create@AlphaNet@@QAEHXZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: automatic Popup dtor cascade for the SEH frame (LEVER #1); direct pushes for start()/X_pop() arg order re-derived from disasm; join_session/positioning logic transcribed twice (matches two inlined copies in the original). Diverges at instruction #15 (register alloc order for the shared cleanup path), not re-derived further.
// TRIED: mirrors the recovered sibling ?join@AlphaNet@@QAEHXZ (src/unrecovered/004e2e50.cpp) - a stack `Popup popup;` gives the SEH frame/alloca/dtor cascade for free (LEVER #1). Reaches MISMATCH #10 (`lea` vs `push`, a register-allocation detail a few instructions into the body) with this shape. Measured: capturing join_session's result AND the *g_0093d4f0 read into two named locals and branching/returning from BOTH (`int savedResult = *g_0093d4f0; if(joined==0){...return 0;} return savedResult;`) makes the compiler drop the SEH frame entirely (regresses to MISMATCH #0, `push`->`mov` at the very first instruction) even though the ORIGINAL disassembly has this exact duplicated-cleanup shape; inlining the join_session call straight into the `if` condition and reading *g_0093d4f0 only on the return statement (no intermediate locals) keeps the frame. Not chased further - a VC6 /GX codegen threshold this pass's budget does not cover.
// ORIGINAL: 0x004E2E50 ?join@AlphaNet@@QAEHXZ 0x004E2E50-0x004E31D6;0x0065C771-0x0065C7EC FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004e2e50.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Session enumeration/cursor/selection flow; negative popup branch bypasses seek in original and reviewed correction."}
// symbol    ?join@AlphaNet@@QAEHXZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: outer `if (a1 == 0) { big-init-block } else { field_D8_ |= 1 }` (ghidra's polarity, not the naive `!= 0`) plus a duplicated success-tail (field_48_/4C_ writes + return 0) inside the `if (rc >= 0)` arm rather than one shared tail reached via a unified `if (r2 == 0) return 1;` after the join/create if-else - 0.73 -> 0.98 mnemonic similarity. AlphaNet reaches Net::close()/ init() through reinterpret_cast<Net*>(this) (a zero-offset base).
// TRIED: one remaining `test/je` original has on the join() path only (create() path just has `test/jne`) that a single shared `if (r2 != 0)` after the join/create if-else does not reproduce; not chased further.
// ORIGINAL: 0x004E31E0 ?setup@AlphaNet@@QAEHH@Z 0x004E31E0-0x004E32BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004e31e0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Existing actual LobbyAppGuid689218 and provider selection/setup state."}
// symbol    ?setup@AlphaNet@@QAEHH@Z
#include "stdafx.h"
#include "alphanet.h"
#include "spritebox.h"
#include "net_class.h"

#include "stringstruct.h"  // StringAllocationBase, the ledger's virtual base
#include "heap.h"
#include "general.h"  // mem_get, the fallback when the ledger has no Heap


#include "stdafx.h"
#include "alphanet.h"
#include "alphanet_inline.h"
#include "netdaemon.h"
#include "netwin.h"
#include "popup.h"
#include "xpops.h"
#include "main.h"
#include "alpha.h"

#include "text.h"
#include "console.h"
#include "game.h"
#include "general.h"
#include <new>
#include "net_class.h"
#include "heap.h"
#include <cstdlib>
#include <cstring>

// Language arrays declare seven bytes; original allocation bounds and lifetimes remain unproved.
// Eight distinct writable original seven-byte script-name instances are
// observed at the respective call operands. These provisional role names
// preserve that storage identity; historical global names and TU are unproved.
// RECOVERY_OBJECT: v1 0x00689238 ?AlphaNetServiceScript@@3PADA
char AlphaNetServiceScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x0068925C ?AlphaNetConnectScript@@3PADA
char AlphaNetConnectScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x00689298 ?AlphaNetCreateScript@@3PADA
char AlphaNetCreateScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x006892B8 ?AlphaNetMissingSessionScript@@3PADA
char AlphaNetMissingSessionScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x006892D8 ?AlphaNetMissingPlayerScript@@3PADA
char AlphaNetMissingPlayerScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x00689318 ?AlphaNetJoinScript@@3PADA
char AlphaNetJoinScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x0068934C ?AlphaNetNoSessionsScript@@3PADA
char AlphaNetNoSessionsScript[7] = "jackal";
// RECOVERY_OBJECT: v1 0x00689368 ?AlphaNetSessionsScript@@3PADA
char AlphaNetSessionsScript[7] = "jackal";

// Superseded seed deleted; seed paths below are historical Git provenance.



/*
Purpose: Convert a multiplayer process ID to its one-based player index.
// ORIGINAL: 0x004E25E0 ?pid_2_idx@AlphaNet@@QAEHK@Z 0x004E25E0-0x004E2607 BYTE_EXACT
// symbol    ?pid_2_idx@AlphaNet@@QAEHI@Z
// size      39 bytes
// prototype int (__thiscall ?pid_2_idx@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   20   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_idx(uint32_t process_id) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].pid_ == process_id) {
            return index;
        }
    }
    return 0;
}

/*
Purpose: Convert a multiplayer process ID to its signed player identity.
// ORIGINAL: 0x004E2610 ?pid_2_who@AlphaNet@@QAEHK@Z 0x004E2610-0x004E2653 BYTE_EXACT
// symbol    ?pid_2_who@AlphaNet@@QAEHI@Z
// size      67 bytes
// prototype int (__thiscall ?pid_2_who@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player identity; process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_who(uint32_t process_id) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].pid_ == process_id) {
            return process_slots_[index].identity_;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its process ID.
// ORIGINAL: 0x004E2660 ?who_2_pid@AlphaNet@@QAEHH@Z 0x004E2660-0x004E26A8 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?who_2_pid@AlphaNet@@QAEHH@Z)(AlphaNet* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Process ID; player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_pid(int identity) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].identity_ == identity) {
            return process_slots_[index].pid_;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its one-based index.
// ORIGINAL: 0x004E26B0 ?who_2_idx@AlphaNet@@QAEXH@Z 0x004E26B0-0x004E26DC BYTE_EXACT
// symbol    ?who_2_idx@AlphaNet@@QAEHH@Z
// size      44 bytes
// prototype void (__thiscall ?who_2_idx@AlphaNet@@QAEXH@Z)(AlphaNet* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_idx(int identity) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].identity_ == identity) {
            return index;
        }
    }
    return 0;
}






/*
Purpose: Clear every player's process slot and hand off to the network close.
// size      40 bytes
// prototype void (__thiscall ?close@AlphaNet@@QAEXXZ)(AlphaNet* this)
// callers   14   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete with a temporary dependency on the network close
*/


/*
Purpose: Build an AlphaNet: the Net base (implicit), the eight process
         slots' {FF,0,0,FF,2} patterns, the ledger, and the trailing Heap -
         all through its typed members.
// ORIGINAL: 0x004E2490 ??0AlphaNet@@QAE@XZ 0x004E2490-0x004E25A5;0x0065C610-0x0065C630
// symbol    ??0AlphaNet@@QAE@XZ
// size      277 bytes
// kind      game
// Player record construction owns the five initial identity/status bytes.
// The ledger and Heap now construct/destruct as ordinary members. The
// image's final [this]=0x66EACC store is now emitted from AlphaNet's real
// overrides. The constructor's remaining source and lifetime remain unfinished.
Return Value: n/a
*/
AlphaNet::AlphaNet() {
    for (int j = 0; j < 8; ++j) {
        process_slots_[j].pid_ = 0;
    }
    player_count_ = 0;
}

// The original closes transport before destroying Heap, ledger, then Net.
// The members/base own those lifetimes; NetDaemon must not repeat them.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00538B60 ??1AlphaNet@@QAE@XZ 0x00538B60-0x00538D0E;0x0065DF70-0x0065E005
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00538b60.cpp: existing identity and complete recorded spans", "Original AlphaNet destruction closes transport, then destroys Heap at +0x148C and ledger at +0x144C before Net; normal members now own this chain."], "reason": "Adopt the retained destructor seed identity for the real AlphaNet owner. Existing shared cleanup spans are preserved, not newly inferred or claimed exact; the native one-entry/new-binding route deliberately does not infer overlapping ownership."}
AlphaNet::~AlphaNet() {
    close();
}

// Intentional no-op overrides, independently present in AlphaNet's original
// table. The preserved original identities belong to these methods, not
// free fastcall functions with invented receiver arguments.
// ORIGINAL: 0x004E32F0 nullsub_20 0x004E32F0-0x004E32F3 BYTE_EXACT
// symbol    ?on_net_event_0@AlphaNet@@UAEXHH@Z
void AlphaNet::on_net_event_0(int, int) { }

// ORIGINAL: 0x004E32C0 nullsub_164 0x004E32C0-0x004E32C3 BYTE_EXACT
// symbol    ?on_player_name_changed@AlphaNet@@UAEXPADK@Z
void AlphaNet::on_player_name_changed(char *, unsigned long) { }

// ORIGINAL: 0x004E32D0 nullsub_165 0x004E32D0-0x004E32D1 BYTE_EXACT
// symbol    ?on_session_lost@AlphaNet@@UAEXXZ
void AlphaNet::on_session_lost() { }

// ORIGINAL: 0x004E32E0 nullsub_166 0x004E32E0-0x004E32E1 BYTE_EXACT
// symbol    ?on_host_changed@AlphaNet@@UAEXXZ
void AlphaNet::on_host_changed() { }

// ORIGINAL: 0x004E25B0 ?close@AlphaNet@@QAEXXZ 0x004E25B0-0x004E25D8 BYTE_EXACT
// kind      game
void AlphaNet::close() {
    // Eight process-ID slots at 0x78C, one per player, stride 0x19C.
    for (size_t slot = 0; slot < 8; ++slot) {
        process_slots_[slot].pid_ = 0;
    }
    player_count_ = 0;
    // The legacy body tail-jumps here with this unchanged, so the network
    // close runs against the same object.
    Net::close();
}
// ALPHANET_CLOSE_END

// ORIGINAL: 0x004E26E0 ?pick_service@AlphaNet@@QAEHPAH@Z 0x004E26E0-0x004E29DF;0x0065C630-0x0065C664
// kind      game
// Historical TRIED record predates separating the complete ServiceList cleanup helper below.
// TRIED: full transcription (guard/save-global, ServiceStruct local, BasePop::start/exec, Dialogs::item list build, join_service) compiles but this address is catalogued SHARED_TAIL - one of its spans is COMDAT-folded with another function, so no body, not even a one-line stub, gets a MISMATCH/BYTE_EXACT verdict
int AlphaNet::pick_service(int *allow_pbem) {
    // The real virtual allocation base captures and clears the incoming heap;
    // ServiceList destruction restores it after provider cleanup.
    ServiceList providers;
    if (allow_pbem != 0) *allow_pbem = 0;
    if (buffer_6E0_ == 0) return 7;
    BasePop *popup = PopupAllocHook();
    if (popup == 0) return 4;
    int result;
    if (poll_services(&providers) != 0 ||
        popup->start(AlphaNetServiceScript, "SELECTSERVICE", -1, 0, 0x42, 0) != 0) {
        result = 1;
    } else {
        providers.seek_pos(-1);
        for (int row = 0; row < providers.entry_count_; ++row) {
            providers.next_entry();
            char *name;
            if (providers.current_ == 0) name = 0;
            else {
                ServiceStruct *payload = providers.head_ == 0 ? 0 : providers.current_->payload_;
                // Original inconsistent-list path dereferences null+4 here.
                name = payload->name_;
            }
            popup->dialogs().Dialogs::item(name, row);
        }
        if (allow_pbem != 0) popup->dialogs().Dialogs::item(label_get(0x1128 / 4), 9999);
        const int selected = popup->exec(0, 0);
        if (selected == -1) result = 1;
        else if (selected == 9999) {
            result = 0;
            if (allow_pbem != 0) *allow_pbem = 1;
        } else {
            // Native selection starts at the head even when the popup's row
            // lies outside the circular list's signed indexing domain.
            if (providers.seek_pos(selected) == 0) providers.seek_pos(0);
            ServiceStruct *payload = providers.head_ == 0 ? 0 : providers.current_->payload_;
            result = join_service(payload);
        }
    }
    delete popup;
    return result;
}

int AlphaNet::do_join_or_create() {
    // The pinned do_join_or_create and setup calls both reference bytes
    // 4E 45 54 43 4F 4E 4E 45 43 54 5F 4A 4F 49 4E 5F 4F 52 5F 43 52 45 41
    // 54 45 00 at 0x689240. This writable-data correspondence does not claim
    // runtime immutability or linker selection.
    // RECOVERY_LITERAL: v1 0x689240 0x68925A ??_C@_0BK@LCOK@NETCONNECT_JOIN_OR_CREATE?$AA@ 0x4E29E0
    const int selected = x_pop_caption_flags(AlphaNetConnectScript, "NETCONNECT_JOIN_OR_CREATE", 0x50, 0);
    if (selected < 0) return 1;
    if (selected == 1) {
        if (join() == 0) return 1;
    } else {
        if (create() == 0) return 1;
    }
    return 0;
}

int AlphaNet::create() {
    Popup popup;
    prefs_get("Net Session", "---", 0);
    strcpy(ParseStrBuffer[0].str, TextBufferGetPtr);
    prefs_get("Net Name", "---", 0);
    strcpy(ParseStrBuffer[1].str, TextBufferGetPtr);
    for (;;) {
        if (x_pop_caption_value_flags(AlphaNetCreateScript, "NETCONNECT_CREATE", 0x1F, 0x44, 0) == -1) return 0;
        if (ParseStrBuffer[0].str[0] == 0) X_pop(AlphaNetMissingSessionScript, "NETCONNECT_NEED_SESSION", 0);
        else if (ParseStrBuffer[1].str[0] == 0) X_pop(AlphaNetMissingPlayerScript, "NETCONNECT_NEED_PLAYER", 0);
        else break;
    }    prefs_put("Net Session", ParseStrBuffer[0].str);
    prefs_put("Net Name", ParseStrBuffer[1].str);
    // Proposed correction of the existing pointer-return declaration preserves the actual native answer,
    // rather than truncating a host pointer through the current catalogue int.
    NetAllocationNode *session = NetDaemonNet.create_session(ParseStrBuffer[0].str, 7, 0);
    if (session == 0) return 0;
    if (NetDaemonNet.join_session(session, ParseStrBuffer[1].str, 0) == 0) return 0;
    return NetDaemonNet.local_player_id();
}

int AlphaNet::join() {
    Popup popup;
    prefs_get("Net Name", "---", 0);
    strcpy(ParseStrBuffer[0].str, TextBufferGetPtr);
    if (x_pop_caption_value_flags(AlphaNetJoinScript, "NETCONNECT_JOIN", 0x1F, 0x44, 0) == -1) return 0;
    prefs_put("Net Name", ParseStrBuffer[0].str);
    int selected;
    do {
        for (;;) {
            ledger_.remove_all();
            ledger_.current_position_ = 0;
            ledger_.allocator_ = 0;
            NetDaemonNet.enumerate_sessions(&ledger_, 0);
            if (ledger_.entry_count_ != 0) break;
            X_pop(AlphaNetNoSessionsScript, "NETCONNECT_SESSION_NOT_FOUND", 0);
            if (ConsoleGroupAction != 1) return 0;
        }
        popup.start(AlphaNetSessionsScript, "NETCONNECT_SESSIONS", -1, 0, 0x42, nullptr);
        ledger_.seek_pos(-1);
        for (int row = 0; row < ledger_.entry_count_; ++row) {
            ledger_.next_entry();
            char *name;
            if (ledger_.current_ == 0) name = 0;
            else {
                NetAllocationNode *payload = ledger_.head_ == 0 ? 0 : ledger_.current_->payload_;
                name = payload->session_name_;
            }
            popup.dialogs().Dialogs::item(name, row);
        }
        selected = popup.exec(0, 0);
        if (selected < 0) break;
    } while (popup.input_poll_state() == 1);
    // Original 4E3006 branches negative cancellation directly to 4E311C.
    // Seek only in the nonnegative join arm; cancellation must not alter the
    // cursor before removal hooks, or evaluate abs(INT_MIN).
    int result;
    if (selected >= 0) {
        ledger_.seek_pos(selected);
        NetAllocationNode *session = ledger_.head_ == 0 ? 0 : ledger_.current_->payload_;
        result = NetDaemonNet.join_session(session, ParseStrBuffer[0].str, 0);
    } else result = 0;
    ledger_.remove_all();
    ledger_.current_position_ = 0;
    return result;
}

int AlphaNet::setup(int option) {
    for (int slot = 0; slot < 8; ++slot) process_slots_[slot].pid_ = 0;
    player_count_ = 0;
    Net::close();
    IsMultiplayerPBEM = 0;
    if (option == 0) {
        // Original 4E3225 passes game-owned storage 689218, not the
        // value-equal but independent NetLobbyAppGuid at 697548.
        Net::init(const_cast<GUID *>(&LobbyAppGuid), 3, 9);
        const int picked = pick_service(&IsMultiplayerPBEM);
        // 4E323F skips the XOR at4E32B3 and returns the raw picker error.
        if (picked != 0) return picked;
        if (IsMultiplayerPBEM != 0) {
            Net::close();
            return 0;
        }
        const int selected = x_pop_caption_flags(AlphaNetConnectScript, "NETCONNECT_JOIN_OR_CREATE", 0x50, 0);
        if (selected < 0) goto failed;
        if (selected == 1) {
            if (join() == 0) goto failed;
            goto connected;
        }
        if (create() != 0) goto connected;
failed:
        return 1;
    } else voice_flags_D8_ |= 1;
connected:
    voice_hotkey_48_ = 0xDC;
    voice_hotkey_4c_ = 0x100DC;
    return 0;
}
