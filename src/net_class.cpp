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
#include <mmsystem.h>
#include <cstring>
#include "net_class.h"
#include "original_seam.h"
#include "popup.h"    // pop_caption / pop_caption_value_title, drop_player's asks
#include "stringstruct.h"
#include "hypothesis_layouts.h"
#include "vtable_shim.h"
#include "wave.h"
#include "wave_device.h"
#include "sounddevice.h"
#include "general.h"  // mem_get, for the lobby's address and session buffers
#include "strings.h"
#include "game.h"

// The lobby probe's DirectPlay types and import. The image really does import
// DirectPlayLobbyCreateA from DPLAYX.DLL, so the link input belongs beside the
// only translation unit that names it - a `#pragma comment` here rather than a
// `target_link_libraries` entry, which this file cannot reach.
#include <dplobby.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma comment(lib, "dplayx.lib")
#endif

// 0x697548 - the application GUID the lobby session registers under, read out
// four dwords at a time into session.guidInstance. It holds the same
// {02D1B480-D680-11D0-821E-44454F5454} that control_game passes in as `guid`
// and that game.cpp keeps its own copy of, so it is spelled once more here
// rather than shared: game.cpp's is file-local and this one is too.
static const GUID NetLobbyAppGuid = {
    0x02D1B480, 0xD680, 0x11D0, {0x82, 0x1E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

// ORIGINAL: 0x006339E0 ??1NetFifo@@QAE@XZ 0x006339E0-0x00633A42
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/006339e0.cpp", "image:0x006339E0-0x00633A42", "src/net_class.h: Net::netfifo_ members at +0xE8, +0x10C and +0x130"], "reason": "Bind the existing queue-destructor forwarder to its retained original seed identity before typed ownership recovery. Original calls and the 0x24 embedded queue layout identify this destructor; this binding does not implement or verify it."}
NetFifo::~NetFifo() {
    close();
    DeleteCriticalSection(&lock_);
}

// The wrapped device's own interface, as far as VoiceTx's dispatches reach.
// The tree has no modelled class for this device yet (wave.cpp reaches the
// same object through vtable_slot seams); the one-slot shim exists because a
// FUNCTION-POINTER dispatch in return position compiles a tail-jump where
// the image calls and epilogues - a member virtual call does not. Slot 8 is
// the stop entry ([vtbl + 0x20]); the fillers ahead of it only count offsets.
class VoiceDeviceStop {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual int stop();  // slot 8 ([vtbl + 0x20])
};

/*
Purpose: Begin voice capture: disable the wave device, start the wave-in
         device, arm the Sound-level device through its own vtable slot 0x90,
         and only then light the started bit at 0x54.
// ORIGINAL: 0x004C8EB0 ?start@VoiceTx@@QAEHXZ 0x004C8EB0-0x004C8EF4 BYTE_EXACT
// symbol    ?start@VoiceTx@@UAEHXZ
// size      68 bytes
// prototype int (__thiscall ?start@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51D0 0x004C5B60
// indirect  0x004C8EDE
// LEVER: PROMOTED out of src/recovered/004c8eb0.cpp, which reached the
//        fields as `reinterpret_cast<char *>(this) + 0x54/0x3c` and the
//        device dispatch through a 37-slot local shim. The named-member
//        spelling reproduces the same bytes: flags_54_ is the dword at 0x54
//        (bit 0, and the image TESTS it as a byte and ORs bit 0 as a byte
//        inside a dword store), device_ is Sound's at 0x3C, and the armed
//        dispatch is slot 36 of the device's live vtable - 0x90, the slot
//        this class's own start occupies in VoiceTx's table 0x0066E8C4.
Return Value: 0, or the first failing stage's answer
*/
int VoiceTx::start() {
    if ((flags_54_ & 1) == 0) {
        int result = WaveDeviceGlobal.disable();
        if (result != 0) {
            return result;
        }
        result = WaveInDeviceGlobal->start_record();
        if (result != 0) {
            return result;
        }
        if (device_ != nullptr) {
            typedef int(__fastcall *device_voice_start_fn)(void *);
            result = vtable_slot<device_voice_start_fn>(device_, 0x90)(device_);
            if (result != 0) {
                return result;
            }
        }
        flags_54_ |= 1;
    }
    return 0;
}

/*
Purpose: Stop voice capture in the reverse order of start, and tear the
         started bit down first.
// ORIGINAL: 0x004C8F00 ?stop@VoiceTx@@QAEHXZ 0x004C8F00-0x004C8F3C BYTE_EXACT
// symbol    ?stop@VoiceTx@@UAEHXZ
// size      60 bytes
// prototype int (__thiscall ?stop@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51C0 0x004C5B70
// indirect  0x004C8F33
// LEVER: PROMOTED out of src/recovered/004c8f00.cpp the same way as start
//        above. The started-bit clear is a full DWORD and-cycle
//        (`and ecx, 0xfffffffe`) while start's set is a byte `or al, 1`
//        inside a dword read-modify-write - both fall out of the one
//        `flags_54_` dword with plain `&=` / `|=`.
Return Value: 0, or the failing stage's answer
*/
int VoiceTx::stop() {
    if ((flags_54_ & 1) != 0) {
        int result = WaveInDeviceGlobal->end_record();
        if (result != 0) {
            return result;
        }
        flags_54_ &= ~1u;
        WaveDeviceGlobal.enable();
        if (device_ != nullptr) {
            return reinterpret_cast<VoiceDeviceStop *>(device_)->stop();
        }
    }
    return 0;
}

/*
Purpose: Drop the Sound-level device (vtable slot 0x38) and forget it, so a
         later init re-wraps; the answer is discarded - release always
         answers 0.
// ORIGINAL: 0x004C8EA0 ?release@VoiceTx@@QAEXXZ 0x004C8EA0-0x004C8EAF BYTE_EXACT
// symbol    ?release@VoiceTx@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?release@VoiceTx@@QAEXXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8EA9
// LEVER: the catalogue spells this `QAEXXZ` (void) - withdrawn here. Both
//        epilogues are `xor eax, eax; ret`, the shape that CORRECTED unload's
//        catalogue name to int. The dispatch differs from Sound::release only
//        in the default: the base returns 0x14 when no device is wrapped,
//        this returns 0. The ?voice_tx_release_redirect@@YIHPAX0@Z spelling
//        that held these bytes first was retired with the delegation_thunks
//        block when this member superseded it.
Return Value: 0, always
*/
int VoiceTx::release() {
    if (device_ != nullptr) {
        typedef int(__fastcall *device_release_fn)(void *);
        vtable_slot<device_release_fn>(device_, 0x38)(device_);
    }
    return 0;
}

/*
Purpose: Detach from the Sound-level device, stopping voice first, and hand
         the device to the release hook while the guard dword says the hook
         is live - the same hook and guard ~Sound consults.
// ORIGINAL: 0x004C8F40 ?unload@VoiceTx@@QAEHXZ 0x004C8F40-0x004C8F70 BYTE_EXACT
// symbol    ?unload@VoiceTx@@UAEHXZ
// size      48 bytes
// prototype int (__thiscall ?unload@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8F45 0x004C8F4C 0x004C8F5C
// LEVER: PROMOTED out of src/recovered/004c8f40.cpp. The first two
//        dispatches are VIRTUAL SELF-CALLS of slots 8 and 14 - spelled
//        `stop(); release();`, which is why the class model matters here:
//        the shim spelling could not see that they are this class's own
//        stop and release.
Return Value: 0, always
*/
int VoiceTx::unload() {
    stop();
    release();
    if (WaveDeviceReleaseGuard != 0) {
        (WaveDeviceReleaseSlot())(device_);
    }
    device_ = nullptr;
    return 0;
}

// THE UNRECOVERED VIRTUAL SET. These four bodies keep VoiceTx's vftable
// slots (0x0066E8C4, 33/34/35/37) true without reproducing them: init
// (0x004C8F70, 151 bytes) sits verbatim in src/unrecovered/, and the other
// three are wrapped-device delegations, two of them still NOT_MATCHING
// redirect spellings in delegation_thunks.cpp. Semantic debt until named,
// placeholders, not claims.
int VoiceTx::init(unsigned long, void *, unsigned long) { return 0; }
int VoiceTx::get_next_buffer() { return 0; }
int VoiceTx::return_buffer(struct _MMIOINFO *) { return 0; }
int VoiceTx::get_nbuffers() { return 0; }


/*
Purpose: Build a Net - its embedded VoiceTx, three NetFifo message queues,
         and the JackalVoiceRx-shaped container at +0xB0.
// ORIGINAL: 0x0062D6A0 ??0Net@@QAE@XZ 0x0062D6A0-0x0062D8DC;0x006634A0-0x006634DF
// size      635 bytes
// prototype void (__thiscall ??0Net@@QAE@XZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8CC0 0x006339C0
Return Value: this

Net's primary virtual surface is distinct from its members' virtual bases.
The original 0x0062D822 vfptr store belongs to the compiler, as do the member
construction and exception-cleanup stages. Scalar initialization and shared
no-op target bindings remain unfinished; the primary table bound follows the
original derived-table boundary and slot-5 dispatch documented in Net.
*/
// QueryInterface in 0x0062D9E0 writes this slot with IID_IDirectPlay2A
// (original GUID at 0x006717C0); Net initialization/close clear the same
// four-byte slot. The former NetEnabled literal pointed outside this object.
// RECOVERY_OBJECT: v2 0x009BE600 0x009BE604 ?NetDirectPlay@@3PAUIDirectPlay2@@A 0x00630A00
IDirectPlay2 *NetDirectPlay;

// Original Net construction publishes this at 0x0062D828; do_net,
// check_net and Win::on_key load the same four-byte pointer object.
// RECOVERY_OBJECT: v2 0x009BE608 0x009BE60C ?ActiveNet@@3PAVNet@@A 0x0062D5B0
Net *ActiveNet;

// 0x009BE5F0, the dword just below it: poll_services parks the ServiceStruct
// the enumeration callback fills there for the length of the call, and the
// callback reads it back out - a hand-off, not a second list.
void *ServiceEnumTarget;  // 0x009BE5F0

// The service-provider GUID the enumeration keeps: DirectPlayEnumerateA hands
// each provider's GUID to sub_630bf0, which compares these 16 bytes against it
// and keeps only the match. Read off the image at 0x00697558.
static const GUID KeptServiceProviderGuid = {
    0xD8D29744, 0x208A, 0x11D0, {0xA0, 0x00, 0x9D, 0xBC, 0xB6, 0x67, 0x29, 0x24}};

/*
Purpose: Build a VoiceTx: full volume, centred pan, cleared regions, a
         1000ms default fade, the "Voice Tx" name on the CRT heap, and
         type 7 published through the device slot.
// ORIGINAL: 0x004C8CC0 ??0VoiceTx@@QAE@XZ 0x004C8CC0-0x004C8DAF;0x004C8450-0x004C8457;0x00659FBE-0x00659FD8
// symbol    ??0VoiceTx@@QAE@XZ
// size      239 bytes
// prototype void (__thiscall ??0VoiceTx@@QAE@XZ)(VoiceTx* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x00645460 0x0064558A 0x006465F0
// PROMOTED from src/recovered/units/004c8cc0.cpp. With VoiceTx : public Sound
// the three vtable stages (0x66E444 -> 0x66E3C0 -> 0x66E8C4) are the
// compiler's own base-construction sequence. Sound owns the inherited
// initialization and default fade setup; VoiceTx clears its own flags at
// original 0x004C8D49..0x004C8D5D before allocating the display name.
Return Value: n/a
Status: Complete
*/
VoiceTx::VoiceTx() {
    // The real Sound constructor initializes the inherited device/chain
    // state; do not read an uninitialized device or repeat base construction.
    memset(&flags_54_, 0, sizeof(flags_54_));
    fname_ = ::operator new(0xA);
    strcpy(reinterpret_cast<char *>(fname_), "Voice Tx");
    Sound::set_type(7);
}

// The receiver's no-op insertion/removal hooks have one pointer argument.
// The other embedded list's virtual surface remains unfinished.
int JackalVoiceRx::on_insert(void *) { return 0; }
void JackalVoiceRx::on_remove(void *) { }
void NetUnk72C::unk_slot0() { }
void NetUnk72C::unk_slot1() { }

// The base table's first four entries share ret 8 at 0x00404260; its last
// two share ret at 0x00404280. These are intentional base defaults, not
// missing daemon callback implementations. Net's constructor publishes the
// primary table at 0x0066EB04; AlphaNet's table at 0x0066EACC retains Net's
// player-added/deleted entries. Those original table references establish
// sharing independently of the unrelated catalogue labels at these leaves.
// Each emitted definition requires its own strict proof. This does not prove
// how the old linker shared code, or resolve slot 0's provisional semantics.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_net_event_0@Net@@UAEXHH@Z 0x0062D6A0
void Net::on_net_event_0(int, int) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_added@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_added(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_deleted@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_deleted(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_name_changed@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_name_changed(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_session_lost@Net@@UAEXXZ 0x0062D6A0
void Net::on_session_lost() { }
// RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_host_changed@Net@@UAEXXZ 0x0062D6A0
void Net::on_host_changed() { }

// 0x009BE4E8, two slots above ActiveNet: the hand-off dword on_player_added
// parks its list argument in for the length of the EnumSessions scan. The
// callback reads it back out; nothing else in the image writes the slot.
// The `T *const` spelling folds to the bare immediate the image stores.
static unsigned int *const NetEnumListGuard = (unsigned int *)0x009BE4E8;

// The StringStruct selection-sort pass, defined in stringstruct.cpp (extern
// "C" __fastcall); no header declares it, so this file restates the link
// shape the same way designwin.cpp restates slide_show.
extern "C" void __fastcall sub_5d49d0(StringStruct *self);

/*
Purpose: Run one lobby enumeration for the setup dialog's player-added path:
         guard the handed-in list, scan the session list through DirectPlay's
         EnumSessions (whose callback fills the list through the guard), then
         sort the received list and step its cursor to the last entry.

         The catalogue spells this ?on_player_added@Net@@QAEHPADI@Z, the
         NON-virtual overload - the virtual table slot's void(unsigned long)
         default above is a different, shared body. The parameter names follow
         the body, not the demangler's reading of PADI: the first stack slot
         (the `char *`) is the list the callers build, and the second (the
         `unsigned int`) is the name text pointer, cast at use. Callers push
         the buffer first (0x0063132B, 0x006313A5) and the callee's tail walks
         the first argument as the StringStruct whose fields sit at
         +0x08/+0x0C/+0x10/+0x14.
// ORIGINAL: 0x0062E420 ?on_player_added@Net@@QAEHPADI@Z 0x0062E420-0x0062E536 FILE
// symbol    ?on_player_added@Net@@QAEHPADI@Z
// size      278 bytes
// prototype int (__thiscall ?on_player_added@Net@@QAEHPADI@Z)(Net* this, char*, unsigned int)
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D49D0 0x00644F3A 0x006465F0
// indirect  0x0062E4C4
// Homed from src/unrecovered/0062e420.cpp: the signature correction of wave
// event 142 - the image's own QAEXH identity, an overload beside the virtual
// slot, not its replacement.
Return Value: 7 with no DirectPlay session, 0x10 with no list, 1 when the
         enumeration reported a failure, else 0
Status: Coverage transcription - unclaimed body
*/
int Net::on_player_added(char *list, unsigned int name) {
    if (NetDirectPlay == 0) {
        return 7;
    }
    if (list == 0) {
        return 0x10;
    }

    unsigned char desc[0x50];
    memset(desc, 0, 0x50);
    *reinterpret_cast<unsigned int *>(desc) = 0x50;

    // The session GUID the session was opened with goes into the scan's
    // guidSession - four dwords copied out of the saved pointer.
    const unsigned int *guid =
        reinterpret_cast<const unsigned int *>(guid_6E8_);
    *reinterpret_cast<unsigned int *>(desc + 0x18) = guid[0];
    *reinterpret_cast<unsigned int *>(desc + 0x1C) = guid[1];
    *reinterpret_cast<unsigned int *>(desc + 0x20) = guid[2];
    *reinterpret_cast<unsigned int *>(desc + 0x24) = guid[3];

    // The name text travels as a plain pointer field: an empty (or null)
    // string is stored as no name at all.
    const char *name_text = 0;
    if (name != 0 && *reinterpret_cast<const char *>(name) != 0) {
        name_text = reinterpret_cast<const char *>(name);
    }
    *reinterpret_cast<const char **>(desc + 0x34) = name_text;

    // The enumeration callback (0x00630C30, still its own preserved seed)
    // fills the caller's list through the guard dword.
    *NetEnumListGuard = static_cast<unsigned int>(reinterpret_cast<unsigned long>(list));
    LPDPENUMSESSIONSCALLBACK2 callback = reinterpret_cast<LPDPENUMSESSIONSCALLBACK2>(
        static_cast<unsigned long>(0x00630C30));
    int result = NetDirectPlay->EnumSessions(
        reinterpret_cast<LPDPSESSIONDESC2>(desc), 0, callback, 0, 1);
    *NetEnumListGuard = 0;

    // The received list is sorted (sub_5d49d0, the one-pass selection sort)
    // and its cursor is parked on the last entry.
    sub_5d49d0(reinterpret_cast<StringStruct *>(list));
    if (result != 0) {
        return 1;
    }

    StringStruct *received = reinterpret_cast<StringStruct *>(list);
    if (received->entry_count_ - 1 >= -1) {
        int limit = received->entry_count_;
        received->current_ = received->head_;
        int n = abs(-1);
        if (n <= limit) {
            n = abs(-1);
            if (n > 0) {
                do {
                    n--;
                    received->current_ = received->current_->previous;
                } while (n != 0);
            }
            received->current_position_ = received->entry_count_ - 1;
        }
    }
    return 0;
}

Net::Net() {
    // voice_tx_ (0x58), jackal_voice_rx_ (0xB0), three queues (0xE8..0x14B)
    // and allocation_node_ (0x700) construct implicitly. The node's virtual
    // base at 0x758 is constructed before its list member at 0x72C, whose
    // own allocation base is at 0x750. This preserves the original owner
    // capture order; treating all three as sequential Net members did not.
    // Other initialization and the receiver/list lifecycle remain unfinished.
    ActiveNet = this;
    NetDirectPlay = 0;
    // Original 0x0062D8A3..0x0062D8AC initializes the window's voice inputs.
    voice_hotkey_48_ = 0;
    voice_hotkey_4c_ = 0;
    voice_target_50_ = 0;
    field_54_ = 0;
}

/*
Purpose: Tear down a Net - its embedded container, three NetFifo message
         queues, and its VoiceTx.
// ORIGINAL: 0x004E34D0 ??1Net@@QAE@XZ 0x004E34D0-0x004E365C;0x004E3710-0x004E372B;0x0065C810-0x0065C876
// size      525 bytes
// prototype void (__thiscall ??1Net@@QAE@XZ)(Net* this)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x004C8DB0 0x0062E010 0x006339E0
// indirect  0x004E35D5 0x004E35E2 0x004E3602
// LEVER: call_diff said MORE, 8 calls against the image's 7, and all three
//        causes were real defects rather than encoding ones. (1)
//        `voice_tx_.VoiceTx::~VoiceTx()` was written out in the body of a REAL
//        destructor whose `voice_tx_` is a REAL member, so VC6 emitted the
//        destruction TWICE - once here and once after the body, where the
//        image has it. (2) 0x004E3529 `call 0x402970` is
//        StringStruct::remove_all entered on the list at sub1 - 0x24, and (3)
//        0x004E353A `call 0x401be0` is leaf_recoveries.cpp's sub_401be0 on
//        sub1 - 8. Both went through no-argument `NetVectorTeardownHelper*()`
//        forwarders, which is an indirect call the image does not make AND a
//        receiver it does pass. call_diff is now clean.
// TRIED: 12 of 110 even so, and the ceiling is not spelling. THE IMAGE'S
//            BODY IS COMPILER-GENERATED MEMBER TEARDOWN, and this tree writes
//            it out by hand. The image carries EIGHT EH state stores into
//            [ebp - 4] - 5 before `Net::close`, 6 before remove_all, 4 before
//            sub_401be0, then 3, 2, 1 for the three NetFifos, 0 for the
//            container at 0xB0 and -1 at the end - and VC6 emits those ONLY
//            for members it is destroying itself. The order is exactly
//            reverse declaration order, which is the evidence: the objects in
//            the 0x72C..0x750 region go first, then the fifos at 0x130,
//            0x10C, 0xE8, then the 0xB0 container, then `voice_tx_` at 0x58.
//            `field_E8_[0x6C]` is precisely three 0x24-byte NetFifos, so the
//            three explicit `reinterpret_cast<NetFifo *>(self + N)->~NetFifo()`
//            calls in this body are the compiler's job; so is the 0xB0 walk
//            and the 0x72C list. Declaring those members - and dropping the
//            matching explicit construction from Net::Net, which
//            double_construction.py will then flag - is what this body needs,
//            and it is a layout job rather than a spelling one.
Return Value: n/a
*/
/*
Purpose: Close the network: stop the voice transmitter, reset the timers
         and slot state, and - when a DirectPlay session object exists -
         release it through its interface and drain the player slots.
// ORIGINAL: 0x0062E010 ?close@Net@@QAEXXZ 0x0062E010-0x0062E2CF
// symbol    ?close@Net@@QAEXXZ
// size      736 bytes
// kind      game
// NOT A FULL TRANSCRIPTION, and the old "Status: Complete" was wrong: this is
// the first 0x2C0-byte head of the body - the voice teardown, the five timer
// and slot resets, and the whole DirectPlay guard - and stops before the
// image's critical-section delete, its five CloseHandle calls, the three
// frees, the player-slot clear, the three NetFifo::close calls and the
// 0x6EC memset that follow it. Two defects in what was here are fixed:
// the transmitter dispatch is slot 5 ([vtbl+0x14]), which is VoiceTx::unload
// (stop AND release AND device detach), not stop; and the fourth helper slot
// is 0x0066912C, not 0x006692C - a missing digit that read a pointer out of
// the middle of nowhere.
// BLOCKED ON LAYOUT, not spelling. The image's remaining 400 bytes reach the
// receiver at Net+0xB0 as an object whose FIRST DWORD is a vftable it calls
// through ([this+0xB0] -> call [vtbl+4]), which contradicts the modelled
// JackalVoiceRx (a virtual StringAllocationBase whose first dword is the
// VBTABLE, 0x670DCC). Until that object's real shape is known, that walk - and
// with it the tail of this body - cannot be written without punning this.
// The DirectPlay helper slots are unnamed: 0x66914C is CloseHandle (same slot
// Filemap::create calls for its handles), 0x669170 is DeleteCriticalSection
// (it is called on lock_2C_ under the field_28_ guard that init's
// InitializeCriticalSection sets), and 0x66917C/0x669174/0x669178/0x66912C
// are only identifiable by address.
Return Value: n/a
*/
void Net::close() {
    voice_tx_.unload();

    poll_timeout_E0_ = 0x4E20;
    field_760_ = 0;
    field_764_ = 0;
    poll_time_DC_ = 0;
    field_6DC_ = 0;

    if (NetDirectPlay != 0) {
        // The DirectPlay release sequence through the original's IAT.
        // Unreached until a session opens.
        typedef void(__cdecl *VisitFn)(void *);
        VisitFn visit = *reinterpret_cast<VisitFn *>(0x0066917C);
        visit(&lock_2C_);
        (*reinterpret_cast<void(__cdecl **)(void *)>(0x00669178))(
            events_8_[3]);
        typedef void (OriginalObject::*iface_fn)();
        (ORIGINAL(NetDirectPlay)->*vtable_method<iface_fn>(NetDirectPlay, 0x10))();
        (ORIGINAL(NetDirectPlay)->*vtable_method<iface_fn>(NetDirectPlay, 8))();
        NetDirectPlay = 0;
        direct_play_1c_ = 0;
        (*reinterpret_cast<void(__cdecl **)(void *)>(0x00669174))(&lock_2C_);
        (*reinterpret_cast<void(__cdecl **)(int)>(0x0066912C))(0);
        visit(&lock_2C_);
        (*reinterpret_cast<void(__cdecl **)(void *)>(0x00669174))(&lock_2C_);
        self_20_ = 0;
        buffer_24_ = 0;
        thread_id_4_ = 0;
    }
}

// Retained unresolved entry points, now reached by the owning classes rather
// than NetDaemon manually destroying its bases. No new target is inferred.
void RemoveAllShim::remove_all() {
    typedef void(__fastcall *pending)(RemoveAllShim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00402970))(this, nullptr);
}
class Sub401be0Shim {
 public:
    void run();
};
void Sub401be0Shim::run() {
    typedef void(__fastcall *pending)(Sub401be0Shim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00401BE0))(this, nullptr);
}

Net::~Net() {
    close();
    // The allocation node/list's internal teardown is still unfinished.
    // Preserve its existing operations while recovering member ownership;
    // queues, receiver and VoiceTx remain compiler-owned destruction.
    char *const self = reinterpret_cast<char *>(this);
    reinterpret_cast<RemoveAllShim *>(self + 0x72c)->remove_all();
    *reinterpret_cast<int32_t *>(self + 0x740) = 0;
    reinterpret_cast<Sub401be0Shim *>(self + 0x748)->run();
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0x754);
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0x75c);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
// ORIGINAL: 0x0062ED30 ?poll_players@Net@@QAEHPAUSessionStruct@@@Z 0x0062ED30-0x0062ED38 BYTE_EXACT
// symbol    ?poll_players@Net@@QAEHH@Z
// size      8 bytes
// prototype int (__thiscall ?poll_players@Net@@QAEHPAUSessionStruct@@@Z)(Net* this, SessionStruct*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Net::poll_players(int) {
    return 1;
}


/*
Purpose: Find a player's name by key, or nothing.

             lea edx,[ecx+0x154] / cmp [edx],esi / je found / inc eax
             add edx,0x58 / cmp eax,0x10 / jl loop
       found: cmp eax,0x10 / jne have / xor eax,eax / ret 4
        have: lea edx,[eax+eax*4] / lea eax,[eax+edx*2]
              lea eax,[ecx+eax*8+0x169] / ret 4

         Sixteen entries of 0x58 bytes from 0x154, keyed on each entry's first
         dword. The three `lea`s multiply the index by 5, then by 11, then by
         8 - which is 88, the same 0x58 the search strides by - and the name
         sits at 0x169, twenty-one bytes into the entry rather than at its
         head.

         The miss returns NULL, and it is detected by the counter REACHING
         0x10 rather than by the loop simply ending: `cmp eax,0x10` is
         repeated after the loop for exactly that reason.

         Reached by raw offset, as this file's header requires: the layout is
         not established and this function is not evidence enough to establish
         it - it only shows that something 0x58 bytes wide lives at 0x154.

// ORIGINAL: 0x00631A20 ?get_player_name@Net@@QAEPADK@Z 0x00631A20-0x00631A55 BYTE_EXACT
// size      53 bytes
// prototype int8* (__thiscall ?get_player_name@Net@@QAEPADK@Z)(Net* this, unsigned int)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: a pointer to the name, or nullptr
Status: Complete
*/
char *Net::get_player_name(unsigned long key) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    // A `for`, WITH THE MISS DETECTED AFTER THE LOOP. The image's loop
    // tests only the key match inline (`cmp [edx], esi / je found`); the
    // `index == 0x10` check that decides found-vs-not happens once, AFTER
    // the loop, as its own `cmp eax, 0x10 / jne have`. A loop that returns
    // from inside the loop body folds that second compare away.
    int index;
    for (index = 0; index < 0x10; ++index) {
        uint32_t candidate;
        std::memcpy(&candidate, bytes + 0x154 + index * 0x58,
                    sizeof(candidate));
        if (candidate == key) {
            break;
        }
    }
    if (index == 0x10) {
        return nullptr;
    }
    return reinterpret_cast<char *>(bytes + 0x169 + index * 0x58);
}


/*
Purpose: Give the network layer a slice of time to move voice traffic.
// ORIGINAL: 0x0062D5B0 ?do_net@@YAXXZ 0x0062D5B0-0x0062D5C9 BYTE_EXACT
// size      25 bytes
// callers   12   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// PROMOTED from src/recovered/0062d5b0.cpp, which was byte-exact but never
// compiled - and while it sat there `temp.h` bound this address as a
// `func_msg *const`, so every one of its twelve callers emitted
// `call dword ptr [...]` where the image emits `call rel32`.
Return Value: n/a
Status: Complete
*/
void __cdecl do_net() {
    if (ActiveNet && NetDirectPlay) {
        ActiveNet->process_voice();
    }
}

/*
Purpose: Let the network layer poll, if there is one and it is enabled.
// ORIGINAL: 0x0062D5D0 ?check_net@@YAXXZ 0x0062D5D0-0x0062D5E9 BYTE_EXACT
// size      25 bytes
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// PROMOTED from src/recovered/0062d5d0.cpp. TWO SEPARATE EARLY RETURNS, not
// one `&&`: that is the shape the artifact carried and the shape the image
// has, and `do_net` above genuinely uses the other one.
Return Value: n/a
Status: Complete
*/
void __cdecl check_net() {
    Net *net = ActiveNet;
    if (net == nullptr) {
        return;
    }
    if (NetDirectPlay == 0) {
        return;
    }
    net->check_polling();
}

/*
Purpose: Begin streaming voice over this connection, when net play is live
         and the voice channel is not already starting or running.

// ORIGINAL: 0x0062DF20 ?start_voice@Net@@QAEHK@Z 0x0062DF20-0x0062DF6E BYTE_EXACT
// size      78 bytes
// prototype int (__thiscall ?start_voice@Net@@QAEHK@Z)(Net* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062DF58
//
// PROMOTED out of src/recovered/0062df20.cpp (measured byte-exact there,
// never compiled into the build). The VoiceTx subobject at +0x58 carries the
// dispatch: its vtable slot 36 is the voice "start" entry, and the two flag
// words at +0xd4/+0xd8 are the handle and its state bits - 0x60000000 means
// a start or stop already in flight, 0x20000000 is the voice-started bit
// this body sets once slot 36 has been told to go.
// LEVER: slot 36 is VoiceTx's OWN `?start@VoiceTx@@QAEHXZ` (0x004C8EB0, at
//        its table 0x0066E8C4). The flat VoiceTx model could only spell this
//        dispatch as a local 37-slot VoiceCall shim; the real virtual does
//        it, and the claim held unchanged through the swap.
*/
int Net::start_voice(unsigned long key) {
    if (NetDirectPlay == 0) {
        return 7;
    }
    if ((voice_flags_D8_ & 0x60000000) == 0) {
        field_54_ = 0;
        voice_key_D4_ = key;
        voice_tx_.start();
        voice_flags_D8_ |= 0x20000000;
    }
    return 0;
}

/*
Purpose: Stop the voice stream this connection is carrying, if any, and
         tell the peer the channel closed.

// ORIGINAL: 0x0062DFC0 ?stop_voice@Net@@QAEXXZ 0x0062DFC0-0x0062E005 BYTE_EXACT
// size      69 bytes
// prototype void (__thiscall ?stop_voice@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062F8A0
// indirect  0x0062DFD5
//
// PROMOTED out of src/recovered/units/0062dfc0.cpp. VoiceTx vtable slot 8
// is the voice "stop" entry; `send_packet_type` then announces the close on
// packet type 0x200. 0x0062F8A0 is still unrecovered, so the call goes
// through that member's pending_bodies forwarder - an `E8` to the same
// place the image jumps.
// LEVER: slot 8 is VoiceTx's OWN `?stop@VoiceTx@@QAEHXZ` (0x004C8F00), an
//        override of Sound's stop. Like start_voice, the dispatch held
//        BYTE_EXACT when the VoiceCall shim gave way to `voice_tx_.stop()`.
*/
void Net::stop_voice() {
    if ((voice_flags_D8_ & 0x20000000) != 0) {
        voice_tx_.stop();
        int handle = static_cast<int>(voice_key_D4_);
        uint32_t flags = voice_flags_D8_;
        flags &= 0xDFFFFFFF;
        voice_flags_D8_ = flags;
        send_packet_type(0, 0, handle, 0x200, 0);
    }
}

// Original 0x00630BA1..0x00630BD4 passes this storage and Net+0x6E4's
// capacity to IDirectPlay2::Receive; 0x0062D8F5 sets that capacity to 0x2000.
// Queue readers also copy payload bytes here, and return this buffer's address.
// This bounds the advertised storage independently of the candidate's size;
// the separate global at 0x009BE4BC corroborates, but does not supply, the bound.
// RECOVERY_OBJECT: v2 0x009BC4BC 0x009BE4BC ?NetGetScratch@@3PAEA 0x00630A00
unsigned char NetGetScratch[0x2000];

// 0x0062F390, unrecovered. In-file forwarder, the same shape NetFifo::~NetFifo
// carries above: it keeps process_voice's call an `E8` to the image's own body
// without pulling a second definition into pending_bodies.cpp.
int Net::sub_62f390(void *data, int mode, unsigned long who, int size,
                    int flags) {
    typedef int(__fastcall *pending)(Net *, void *, void *, int, unsigned long,
                                     int, int);
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x0062F390))(
        this, nullptr, data, mode, who, size, flags);
}

// 0x009B2068 - the flag the "ok to drop the hosts?" answer lights: that
// answer returns 1 with this set and no player cut.
static unsigned int *const NetDropAbortFlag = (unsigned int *)0x009B2068;

// 0x009BE5FC, four dwords below ActiveNet: the outgoing field_764_ the host
// change publishes before the new host's id overwrites it.
// RECOVERY_OBJECT: v2 0x009BE5FC 0x009BE600 ?NetHostChangeSlot@@3IA 0x00632350
uint32_t NetHostChangeSlot;

/*
Purpose: Cut one player out of the session by id, or everyone when the caller
         hands 0xFFFFFFFF. The interactive half (reason 0) asks first: the
         dropped side is named through parse_says - both hosts when the timer
         words disagree and the target is not the host slot, the target alone
         otherwise - and a "jackal" popup answers; the dismissal answer can
         instead open NET_ADJUSTTOLERANCE and re-arm the staleness threshold
         to five, ten, twenty or thirty seconds. The cut itself closes the
         player's slot (later records shift down, the top slot's data left
         duplicated behind them), republishes the host id and raises
         on_host_changed when it was the polling target, counts 0x6DC down,
         raises on_player_deleted with the kept name, and unlinkes the
         player's voice-receiver entry - payload deleted, node freed through
         its StringAllocationBase. A reason-0 cut then announces itself with
         a mode-4 0x80-byte packet whose body is the frame slot the image
         passes through unwritten.
// ORIGINAL: 0x00632350 ?drop_player@Net@@QAEHKH@Z 0x00632350-0x00632716 FILE
// size      966 bytes
// prototype int (__thiscall ?drop_player@Net@@QAEHKH@Z)(Net* this, unsigned int, int)
// callers   2   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625EC0 0x00627160 0x00627200 0x0062F8A0 0x00645470
// indirect  0x00632612 0x00632633 0x006326A3 0x006326B1 0x006326CC
// Homed from src/unrecovered/00632350.cpp; this body replaces the in-file
// forwarder that stood here for check_polling.
Return Value: 1 when a player was found (cut, or the abort answer taken), 0
         when the id named nobody
Status: Coverage transcription - unclaimed body
*/
int Net::drop_player(unsigned long key, int reason) {
    int index = 0;
    while (index < 0x10 && players_[index].field_0_ != key) {
        ++index;
    }
    if (index == 0x10) {
        return 0;
    }

    if (reason == 0) {
        if (field_760_ != field_764_ && key != players_[1].field_0_) {
            // The two hosts are named - the grammar slots 0 and 1 - and the
            // plural question asked.
            parse_says(0, get_player_name(players_[0].field_0_), -1, -1);
            parse_says(1, get_player_name(players_[1].field_0_), -1, -1);
            int answer = pop_caption_value_title(
                const_cast<char *>("jackal"),
                const_cast<char *>("NET_OKTODROPHOSTS"), 1, 0, 0);
            if (answer == 0) {
                *NetDropAbortFlag = 1;
                return 1;
            }
            if (answer == 1 || answer > 2) {
                return 0;
            }
            // answer 2: the tolerance popup, then no cut.
            int tolerance = pop_caption(
                const_cast<char *>("jackal"),
                const_cast<char *>("NET_ADJUSTTOLERANCE"), 0);
            switch (tolerance) {
            case 0:
                poll_timeout_E0_ = 5000;
                break;
            case 1:
                poll_timeout_E0_ = 10000;
                break;
            case 2:
                poll_timeout_E0_ = 20000;
                break;
            case 3:
                poll_timeout_E0_ = 30000;
                break;
            default:
                break;
            }
            return 0;
        }
        // Dropping the paired host or client: the caption names which side.
        parse_says(0, get_player_name(key), -1, -1);
        char *caption = field_760_ == field_764_
            ? const_cast<char *>("NET_OKTODROPCLIENT")
            : const_cast<char *>("NET_OKTODROPHOST");
        int answer = pop_caption_value_title(
            const_cast<char *>("jackal"), caption, 1, 0, 0);
        if (answer == 1) {
            return 0;
        }
        if (answer == 2) {
            int tolerance = pop_caption(
                const_cast<char *>("jackal"),
                const_cast<char *>("NET_ADJUSTTOLERANCE"), 0);
            switch (tolerance) {
            case 0:
                poll_timeout_E0_ = 5000;
                break;
            case 1:
                poll_timeout_E0_ = 10000;
                break;
            case 2:
                poll_timeout_E0_ = 20000;
                break;
            case 3:
                poll_timeout_E0_ = 30000;
                break;
            default:
                break;
            }
            return 0;
        }
        // The confirming answer (0) falls through to the cut.
    }

    // The removal. The name rides the frame for the on_player_deleted notice,
    // emptied up front the way the image does when the id has already gone.
    char name[0x100];
    name[0] = 0;
    char *named = get_player_name(key);
    if (named != 0) {
        strcpy(name, named);
        // Close the slot: every later record shifts down one. The image does
        // not clear the top slot, so its data stays duplicated behind.
        for (int moved = index; moved < 0xF; ++moved) {
            players_[moved] = players_[moved + 1];
        }
    }

    if (key == field_764_) {
        NetHostChangeSlot = field_764_;
        field_764_ = players_[0].field_0_;
        on_host_changed();
    }

    --field_6DC_;
    on_player_deleted(name, key);

    // The voice receiver loses this player's entry: walk from its cursor,
    // unlink, hand the departing payload to the first hook, delete it through
    // Sound's own deleting destructor, and free the node through its virtual
    // base - the vbase-adjusted call the image makes.
    if (jackal_voice_rx_.head_ != 0) {
        int scanned = 0;
        while (scanned < jackal_voice_rx_.count_
               && jackal_voice_rx_.cursor_->id_ != key) {
            jackal_voice_rx_.cursor_ = jackal_voice_rx_.cursor_->next_;
            ++scanned;
        }
        if (scanned < jackal_voice_rx_.count_) {
            VoiceReceiverEntry *entry = jackal_voice_rx_.cursor_;
            entry->next_->previous_ = entry->previous_;
            entry->previous_->next_ = entry->next_;
            if (entry == jackal_voice_rx_.head_) {
                jackal_voice_rx_.head_ = entry->next_;
            }
            jackal_voice_rx_.cursor_ = entry->next_;
            Sound *payload = entry->payload_;
            jackal_voice_rx_.on_insert(payload);
            if (payload != 0) {
                delete payload;
            }
            entry->payload_ = 0;
            if (entry != 0) {
                StringAllocationBase *node = entry;
                delete node;
            }
            --jackal_voice_rx_.count_;
        }
        if (jackal_voice_rx_.count_ == 0) {
            jackal_voice_rx_.head_ = 0;
        }
        jackal_voice_rx_.cursor_index_ = jackal_voice_rx_.count_ - 1;
    }

    if (reason == 0) {
        // The frame slot the image passes through unwritten: mode 4, 0x80
        // bytes of it, flagged 1.
        int notice;
        send_packet_type(&notice, 4, 0, 0x80, 1);
    }
    return 1;
}

/*
Purpose: Keep the session alive and reap the players that have gone quiet:
         every two seconds send a 0x20 keep-alive packet, then - if the voice
         flag says this connection tracks its peers at all - walk the player
         table and cut whoever has not been heard from inside poll_timeout_E0_
         milliseconds.

         Three shapes of staleness, in the image's order:
           1. the two timer words agree            - sweep all sixteen slots;
           2. the first agrees with player 1's stamp - sweep that one player;
           3. neither                              - look at players 0 and 1,
             and if both are stale, cut every player at once.

         The clock is the game's imported timeGetTime, reached through its
         import slot: the image hoists that load into a callee-saved register
         for the four calls in the first half of the body and falls back to a
         memory-indirect call for the two after the register is spent, so the
         source calls through the slot each time rather than copying it.
// ORIGINAL: 0x006320E0 ?check_polling@Net@@QAEXXZ 0x006320E0-0x00632290
// symbol    ?check_polling@Net@@QAEXXZ
// size      432 bytes
// prototype void (__thiscall ?check_polling@Net@@QAEXXZ)(Net* this)
// callers   4   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062F8A0 0x00632350
// indirect  0x006320FA 0x00632138 0x006321A1 0x006321BF 0x006321F8 0x0063221A 0x00632244 0x0063226E
// LEVER: the keep-alive half is a chain of four guard-and-return steps, and
//        the reap half is one `if (voice_flags_D8_ & 0x40000)` whose three
//        staleness arms each end in `*NetReapBusy = 0` - the shared clear
//        block at 0x632280 is the outer if's own fall-through, which is why
//        four different returns land on it while the sixteen-slot loop's tail
//        writes its own copy inline.
// LEVER: branch 3 walks `stamps[checked * 0x16]` from `&players_->stamp_10_`,
//        NOT `players_[checked].stamp_10_`: the image spills the BASE pointer
//        to [esp+0x10] before the loop and writes `*stamps` through it after
//        the drop, which only a pointer local produces. players_[1] is stored
//        directly beside it - two different base expressions, one value.
// TRIED: calling the slot through the accessor at each site, no local - VC6
//        then never hoists the load into a callee-saved register, gives the
//        function no local slot at all (`push ecx` is missing, so the whole
//        register assignment cascades) and spends ebx on a zero register
//        (`cmp reg,ebx` / `push ebx` where the image has `test` / `push 0`);
//        4 of 135, and compiles 126 instructions against the image's 135.
// TRIED: spelling the four staleness tests against 0 in the inverted form,
//        dropping the pointer local for `players_[checked]`, and hoisting the
//        clock into a `register` local - none of them removes the zero
//        register, and the pointer-local removal brings back a frame pointer
//        (`push ebp / mov ebp,esp`), which the image has none of.
// THE RESIDUAL. MISMATCH 11 of 135, and it is one register-assignment cascade
// plus one dead-store difference, not a transcription error. The image loads
// the clock's import slot into EBP once, AFTER the NetDirectPlay test, and never
// stores it to its home; this tree loads it before the test and VC6 keeps a
// home copy (`mov dword ptr [esp+0x10], ebp`) for a local the image does not
// have. Everything else follows: with EBP spent on the clock the image cannot
// also spend EBX on a zero register, so it tests and pushes immediates where
// this tree compares against and pushes the zero register. Six attempts, and
// the trigger for VC6's hoist-without-home is not in this file's spelling.
Return Value: n/a
*/
void Net::check_polling() {
    DWORD (WINAPI *const now)() = timeGetTime;
    if (NetDirectPlay == 0) {
        return;
    }
    if (now() - poll_time_DC_ <= 2000) {
        return;
    }
    if (*NetKeepAliveBusy != 0) {
        return;
    }
    *NetKeepAliveBusy = 1;
    send_packet_type(0, 0, 0, 0x20, 0);
    poll_time_DC_ = now();
    *NetKeepAliveBusy = 0;
    if (*NetReapBusy != 0) {
        return;
    }
    *NetReapBusy = 1;
    if ((voice_flags_D8_ & 0x40000) != 0) {
        if (field_760_ == field_764_) {
            // Every slot: a live player who is not the one the timer words
            // name and whose stamp is older than the threshold is cut, and a
            // successful cut stamps the time of the cut.
            int count = 0x10;
            do {
                if (players_[0x10 - count].field_0_ != 0
                    && players_[0x10 - count].field_0_ != field_760_
                    && players_[0x10 - count].stamp_10_ != 0
                    && now() - players_[0x10 - count].stamp_10_
                           > poll_timeout_E0_
                    && drop_player(players_[0x10 - count].field_0_, 0) == 0) {
                    players_[0x10 - count].stamp_10_ =
                        now();
                }
                --count;
            } while (count != 0);
            *NetReapBusy = 0;
            return;
        }
        if (field_760_ == players_[1].stamp_10_) {
            if (players_->stamp_10_ == 0) {
                *NetReapBusy = 0;
                return;
            }
            if (now() - players_->stamp_10_
                    <= poll_timeout_E0_) {
                *NetReapBusy = 0;
                return;
            }
            if (drop_player(players_->field_0_, 0) != 0) {
                *NetReapBusy = 0;
                return;
            }
            players_->stamp_10_ = now();
            *NetReapBusy = 0;
            return;
        }
        // Neither timer word names a player this table holds, so both leading
        // slots are stale: cut the lot.
        uint32_t *stamps = &players_->stamp_10_;
        for (int checked = 0; checked < 2; ++checked) {
            if (stamps[checked * 0x16] == 0) {
                return;
            }
            if (now() - stamps[checked * 0x16]
                    < poll_timeout_E0_) {
                return;
            }
        }
        if (drop_player(0xFFFFFFFF, 0) == 0) {
            uint32_t cut = now();
            players_[1].stamp_10_ = cut;
            stamps[0] = cut;
        }
    }
    *NetReapBusy = 0;
}

/*
Purpose: Move one slice of voice traffic: ride the level meter to open and
         close the channel, and when it is open, forward whatever the
         transmitter's capture buffer holds to the session.

             test 0x40000000      -> the channel is closing
               level <  50        -> clear the closing bit
             level > 100          -> set the closing bit and stop
             else                 -> ship the capture buffer

         The two thresholds are UNSIGNED compares in the image (`jae` against
         0x32, `jbe` against 0x64) over a getter the catalogue spells `H`
         (int), so the level is held in an unsigned local rather than compared
         as the getter's own type.
// ORIGINAL: 0x00631A60 ?process_voice@Net@@QAEXXZ 0x00631A60-0x00631B2E BYTE_EXACT
// symbol    ?process_voice@Net@@QAEXXZ
// size      206 bytes
// prototype void (__thiscall ?process_voice@Net@@QAEXXZ)(Net* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD2B0 0x0062F390 0x0062F8A0
// indirect  0x00631A7F 0x00631AAB 0x00631AC8 0x00631AD3 0x00631B0A 0x00631B24
// LEVER (2026-09-03): `voice_sound_call`, a file-local forwarder over
//   autosound.h's MEASURED inline `do_sound`. The helper is `return 0;`, so
//   /Ob1 expanded it to nothing and the body was two instructions short of
//   the image's 78 - the image emits a real `call 0x5FD2B0` as the third
//   instruction, and with the call back the rest of the body's scheduling
//   falls into step (`test edi, eax` from a register, not a memory operand).
//   78 of 78, BYTE_EXACT. Same lever as Win::redraw 0x005F4CC0.
Return Value: n/a
*/
static int __cdecl voice_sound_call() {
    return do_sound();
}

void Net::process_voice() {
    voice_sound_call();
    if ((voice_flags_D8_ & 0x40000000) != 0) {
        unsigned int level = voice_tx_.get_nbuffers();
        if (level < 0x32) {
            voice_flags_D8_ &= ~0x40000000u;
        }
    } else {
        unsigned int level = voice_tx_.get_nbuffers();
        if (level > 100) {
            voice_flags_D8_ |= 0x40000000;
            voice_tx_.stop();
            return;
        }
        // The capture buffer: a dword at 0x18 says where the packet starts and
        // one at 0x20 says where capture has written to.
        uint32_t *packet =
            reinterpret_cast<uint32_t *>(voice_tx_.get_next_buffer());
        if (packet != 0) {
            if (field_54_ != 0) {
                sub_62f390(reinterpret_cast<void *>(packet[6]),
                           static_cast<int>(packet[8] - packet[6]),
                           voice_key_D4_, 0x100, 0);
            } else {
                send_packet_type(reinterpret_cast<void *>(packet[6]),
                                 static_cast<int>(packet[8] - packet[6]),
                                 voice_key_D4_, 0x100, 0);
            }
            voice_tx_.return_buffer(reinterpret_cast<_MMIOINFO *>(packet));
        }
    }
}

// ORIGINAL: 0x0062EE20 sub_62ee20 0x0062EE20-0x0062EE71
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62EE20..62EE71; flags bit2 and field8 on sixteen 58-byte NetPlayer records; shared object9BE4BC.", "git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/recovered/units/0062ee20.cpp"], "reason": "Recover the SDK group enumeration callback against the same source-owned NetPlayerTable used by player enumeration; mark genuine matching NetPlayer records."}
// symbol    ?net_group_player_callback@@YGHKKPBUDPNAME@@KPAX@Z
static int __stdcall net_group_player_callback(unsigned long player_id,
                                               unsigned long,
                                               const DPNAME *, unsigned long,
                                               void *) {
    NetPlayer *const players = NetPlayerTable;
    if (players == 0) return 0;
    for (int index = 0; index < 16; ++index) {
        if (players[index].field_0_ == player_id) {
            if ((players[index].flags_14_ & 2) != 0) {
                players[index].field_8_ = 1;
            }
            return 1;
        }
    }
    return 1;
}

// ORIGINAL: 0x0062ED40 ?create_group@Net@@QAEHPAD@Z 0x0062ED40-0x0062EDA2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62ED40..62EDA2; git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/leaf_recoveries.cpp supplies the former facade mapping and unchanged SDK call contract."], "reason": "Rehome the DirectPlay group constructor from its raw receiver facade to the genuine Net method and SDK IDirectPlay2 interface."}
int Net::create_group(char *group_name) {
    IDirectPlay2 *const direct_play = NetDirectPlay;
    if (direct_play == 0) return 0;
    DPNAME name;
    DPID group = 0;
    memset(&name, 0, sizeof(name));
    name.dwSize = sizeof(name);
    name.lpszShortNameA = group_name;
    const HRESULT result = direct_play->CreateGroup(&group, &name, 0, 0, 0);
    return result == 0 ? static_cast<int>(group) : 0;
}

// Group callback evidence: NetPlayer.field_8_ is the acknowledgement flag
// at +8; field_C_ is the per-peer retry tick at +0xC. Existing exact 0x58
// stride/16-slot extent is independently established by Net and player APIs.
// ORIGINAL: 0x0062EEA0 ?send_group@Net@@QAEHPAXHJH@Z 0x0062EEA0-0x0062F38C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62EEA0..62F38C; retained git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/unrecovered/0062eea0.cpp; genuine NetPlayer/FIFO/DirectPlay owners.", "Native62F038..62F080 initializes only the first peer, then62F14A..62F199 reads all sixteen stack entries. User approved complete active-peer acknowledgement/retry initialization, deliberately differing from original indeterminate-stack paths."], "reason": "Recover group queueing, transmission, acknowledgements, retry and timeout handling using genuine Net state and the SDK; initialize every active peer retry state as an explicitly approved behavior repair."}
int Net::send_group(void *data, int size, long group_id, int flags) {
    int index;
    HANDLE wait_handles[2];
    wait_handles[0] = events_8_[0];
    wait_handles[1] = events_8_[1];

    if (NetDirectPlay == 0) return 0;
    if (size < 0) return 0;
    if (field_6E4_ < static_cast<uint32_t>(size)) return 0;

    check_polling();

    if ((voice_flags_D8_ & 1U) != 0 || (flags & 4) != 0) {
        if (group_id == static_cast<long>(field_760_)) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           field_760_, 0, static_cast<unsigned long>(-1));
            return static_cast<int>(field_E4_);
        }
        if (group_id == 0) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           field_760_, 0, static_cast<unsigned long>(-1));
            if (field_6DC_ < 2) return static_cast<int>(field_E4_);
        }
    }

    const int allocation_size = size + 0x0C;
    unsigned char *const packet =
        static_cast<unsigned char *>(mem_get(allocation_size));
    if (packet == 0) return 0;

    const unsigned int reliable = static_cast<unsigned int>(flags) & 1U;
    uint32_t packet_sequence;
    if (reliable == 0) {
        packet_sequence = 0;
    } else {
        field_E4_ += 1;
        packet_sequence = field_E4_;
    }
    memcpy(packet + 4, &packet_sequence, sizeof packet_sequence);
    if (data != 0) memcpy(packet + 8, data, static_cast<size_t>(size));

    if (reliable == 0 || (voice_flags_D8_ & 0x10000000U) != 0) {
        const uint16_t packet_type = 1;
        memcpy(packet, &packet_type, sizeof packet_type);
        const HRESULT sent = NetDirectPlay->Send(
            field_760_, static_cast<DPID>(group_id), reliable, packet,
            static_cast<DWORD>(allocation_size));
        free(packet);
        return sent == 0 ? static_cast<int>(field_E4_) : 0;
    }

    // Unconfirmed reliable send. The callback marks only group members whose
    // NetPlayer flags include bit 1. NetPlayerTable is shared callback context
    // and remains published exactly as native; it is not a local scratch copy.
    const uint16_t packet_type = 4;
    memcpy(packet, &packet_type, sizeof packet_type);
    for (index = 0; index < 16; ++index) {
        players_[index].field_8_ = 0;
    }
    NetPlayerTable = players_;
    if (NetDirectPlay == 0) {
        free(packet);
        return 0;
    }
    NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), 0,
        net_group_player_callback, 0, 0);

    const unsigned long no_deadline = 0xFFFFFFFFUL;
    unsigned long wait_timeout = no_deadline;
    int first_pending = -1;
    for (index = 0; index < 16; ++index) {
        if (players_[index].field_8_ != 0) {
            first_pending = index;
            break;
        }
    }
    if (first_pending < 0) {
        free(packet);
        return static_cast<int>(field_E4_);
    }

    int pending[16];
    unsigned long retry_delay[16];
    unsigned long base_time[16];
    for (index = 0; index < 16; ++index) {
        pending[index] = 0;
        retry_delay[index] = 0;
        base_time[index] = 0;
    }
    // Deliberate, user-approved repair: the original initializes only the
    // first peer, then reads all sixteen stack entries. Service every selected
    // peer with deterministic acknowledgement/retry state instead.
    for (int peer = 0; peer < 16; ++peer) {
        if (players_[peer].field_8_ != 0) {
            pending[peer] = 1;
            retry_delay[peer] = players_[peer].field_C_;
            if (retry_delay[peer] != no_deadline &&
                (wait_timeout == no_deadline || retry_delay[peer] < wait_timeout)) {
                wait_timeout = retry_delay[peer];
            }
        }
    }

    for (;;) {
        if (NetDirectPlay == 0) {
            free(packet);
            return 0;
        }

        const unsigned long initial_tick = timeGetTime();
        for (index = 0; index < 16; ++index) {
            base_time[index] = initial_tick;
        }

        const HRESULT resent = NetDirectPlay->Send(
            field_760_, static_cast<DPID>(group_id), 0, packet,
            static_cast<DWORD>(allocation_size));
        if (resent != 0) {
            free(packet);
            return 0;
        }

        const unsigned long interval_start = timeGetTime();
        unsigned long elapsed = timeGetTime() - interval_start;
        int any_pending = 0;
        while (elapsed < 20000UL) {
            ResetEvent(wait_handles[1]);
            ResetEvent(wait_handles[0]);
            const DWORD waited = WaitForMultipleObjects(
                2, wait_handles, FALSE, wait_timeout);
            if (waited == WAIT_OBJECT_0 + 1) handle_sys_msg();

            const unsigned long now = timeGetTime();
            any_pending = 0;
            for (index = 0; index < 16; ++index) {
                if (pending[index] == 0) continue;
                NetPlayer &player = players_[index];
                if (player.field_0_ == 0) {
                    pending[index] = 0;
                } else if (player.field_8_ == 0) {
                    const unsigned long tick = timeGetTime();
                    pending[index] = 0;
                    player.field_C_ = (tick + 0x14UL +
                        (player.field_C_ - base_time[index])) >> 1;
                } else {
                    if (retry_delay[index] < now - base_time[index]) {
                        retry_delay[index] = (retry_delay[index] * 3UL) >> 1;
                        if (NetDirectPlay == 0) {
                            free(packet);
                            return 0;
                        }
                        const unsigned long resend_id = player.field_0_;
                        base_time[index] = timeGetTime();
                        const HRESULT retried = NetDirectPlay->Send(
                            field_760_, static_cast<DPID>(resend_id), 0, packet,
                            static_cast<DWORD>(allocation_size));
                        if (retried != 0) {
                            MessageBoxA(0, "Oh no!! Send failed!",
                                        "Message from the Net Underworld", 0);
                            free(packet);
                            return 0;
                        }
                    }
                    any_pending = 1;
                }
            }
            if (any_pending == 0) {
                free(packet);
                return static_cast<int>(field_E4_);
            }
            elapsed = timeGetTime() - interval_start;
        }

        // Timeout notification uses the application's canonical shared
        // StringTemp, with the original nested id lookup and append ordering.
        StringTemp[0] = 0;
        int first_name = 1;
        for (index = 0; index < 16; ++index) {
            if (players_[index].field_8_ != 0) {
                int found_index = -1;
                for (int search = 0; search < 16; ++search) {
                    if (players_[search].field_0_ == players_[index].field_0_) {
                        found_index = search;
                        break;
                    }
                }
                // The native not-found path does not skip this name.  It
                // falls back to the first player's name at Net+0x169.
                char *name = found_index < 0
                    ? players_[0].name_and_tail_
                    : players_[found_index].name_and_tail_;
                strcat(StringTemp, name);
                if (!first_name) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, " ");
                }
                first_name = 0;
            }
        }
        strcat(StringTemp, ".");
        parse_says(0, StringTemp, -1, -1);
        const int answer = pop_caption(const_cast<char *>("jackal"),
                               const_cast<char *>("NET_SENDTIMEDOUT"), 0);
        if (answer == 0) continue;
        if (answer == 1) {
            close();
        } else if (answer == 2) {
            ExitTurnLoop = 1;
        }
        free(packet);
        return 0;
    }
}

/*
Purpose: Poll the embedded NetFifo for a message, when net play is enabled
         and a destination is given.
// ORIGINAL: 0x00630A00 ?get@Net@@QAEHPAKPAK@Z 0x00630A00-0x00630A3A BYTE_EXACT
// symbol    ?get@Net@@QAEHPAK0@Z
// size      58 bytes
// prototype int (__thiscall ?get@Net@@QAEHPAKPAK@Z)(Net* this, unsigned int*, unsigned int*)
// callers   18   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633F70
Return Value: NetGetScratch's address when a message was fetched, else 0
Status: Complete
*/
int Net::get(unsigned long *a, unsigned long *b) {
    if (NetDirectPlay != 0) {
        if (a != 0) {
            int result = queue_10c_.get(NetGetScratch,
                                   reinterpret_cast<unsigned int *>(a), 0,
                                   reinterpret_cast<unsigned int *>(b));
            return result ? reinterpret_cast<int>(NetGetScratch) : 0;
        }
    }
    return 0;
}

// The player-table write seam, 0x009BC4B8 - the dword just below NetGetScratch
// - and the table base it indexes, 0x009BE4BC just above the scratch's end.
// enum_players appends one 0x58-byte NetPlayer record per DirectPlay player
// callback through this pair; init neither reads nor writes either slot in its
// recovered body, so they stay independent bindings, not Net member aliases.
// Source-owned callback selection cells, not pointers into the old image.
// 630C80 loads/reloads both around GetPlayerData; its increment is a DWORD
// at 9BC4B8, immediately before the independently owned NetGetScratch bank.
// RECOVERY_OBJECT: v2 0x009BC4B8 0x009BC4BC ?NetPlayerWriteIndex@@3IA 0x00630C80
unsigned int NetPlayerWriteIndex;
// RECOVERY_OBJECT: v2 0x009BE4BC 0x009BE4C0 ?NetPlayerTable@@3PAVNetPlayer@@A 0x00630C80
NetPlayer *NetPlayerTable;

/*
Purpose: Append one player to the callback-selected table: record the original
         DirectPlay id, read one byte of remote player data through the SDK
         GetPlayerData method (slot +0x50), merge in the connection's voice
         bit, copy the DPNAME long text, and publish the id as the polling
         target when the player is local.
// ORIGINAL: 0x00630C80 ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z 0x00630C80-0x00630D6B FILE
// size      227 bytes
// prototype int (__thiscall ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z)(Net* this, unsigned long, unsigned long, const DPNAME*, unsigned long, void*)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645460
// indirect  0x00630CC5
// Homed from src/unrecovered/00630c80.cpp.
Return Value: 1, always
Status: Coverage transcription - unclaimed body
*/
int Net::enum_players(unsigned long player_id, unsigned long, const DPNAME *name,
                      unsigned long, void *) {
    const unsigned long original_id = player_id;
    NetPlayerTable[NetPlayerWriteIndex].field_0_ = original_id;

    // Native630C8B..CB6 supplies DPGET_REMOTE (zero) as the fifth stack
    // argument including the interface receiver. The one-byte data is written
    // into the player_id argument; the separate DWORD initialized to one is
    // buffer capacity, not the returned flags. Retain the original id for the
    // later polling publication, and reload callback-selected storage.
    DWORD data_size = 1;
    NetDirectPlay->GetPlayerData(original_id, &player_id, &data_size, 0);

    NetPlayerTable[NetPlayerWriteIndex].flags_14_ = static_cast<uint8_t>(player_id);

    if (ActiveNet != 0 && (ActiveNet->voice_flags_D8_ & 2) != 0) {
        NetPlayerTable[NetPlayerWriteIndex].flags_14_ = static_cast<uint8_t>(
            NetPlayerTable[NetPlayerWriteIndex].flags_14_ | 2);
    }

    NetPlayerTable[NetPlayerWriteIndex].name_and_tail_[0] = 0;
    // The image's build is ANSI: the byte string the callback hands in sits
    // in the DPNAME's A-side union slot, and the record's name text is
    // appended straight from it.
    strcat(NetPlayerTable[NetPlayerWriteIndex].name_and_tail_,
           name->lpszLongNameA);

    if ((player_id & 1) != 0 && ActiveNet != 0) {
        ActiveNet->field_764_ = original_id;
    }

    ++NetPlayerWriteIndex;
    return 1;
}

/*
Purpose: Initialise the network session: tear whatever is running down, take
         the 0x2000-byte DirectPlay buffer, the session GUID and the option
         word, ask the embedded voice transmitter to open its 0x1000-byte
         device buffer, clear the sixteen player slots, and register this
         object - its thread, its five events and its queue lock - as live.
// ORIGINAL: 0x0062D8E0 ?init@Net@@QAEHPAU_GUID@@HH@Z 0x0062D8E0-0x0062D9D8 BYTE_EXACT
// size      248 bytes
// prototype int (__thiscall ?init@Net@@QAEHPAU_GUID@@HH@Z)(Net* this, _GUID*, int, int)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062E010
// indirect  0x0062D948 0x0062D983 0x0062D996 0x0062D99F 0x0062D9A8 0x0062D9B1 0x0062D9BA 0x0062D9C3
Return Value: 4 when the buffer cannot be had, else 0
Status: Complete
*/
int Net::init(_GUID *guid, int option_a, int option_b) {
    close();
    field_44_ = option_b;
    field_6E4_ = 0x2000;
    void *buffer = mem_get(0x2000);
    buffer_6E0_ = buffer;
    if (buffer == 0) {
        return 4;
    }
    guid_6E8_ = guid;
    voice_flags_D8_ = option_a;
    field_6D8_ = 0xC8;
    // VoiceTx::init, slot 0x84 of the table at voice_tx_'s front.
    voice_tx_.init(0x1000, 0, 0);
    // The cursor sits one word into each slot - image 0x158, clearing the word
    // behind it first - so it walks field_4_ and reaches field_0_ as [-1].
    uint32_t *word = &players_->field_4_;
    int count = 0x10;
    do {
        word[-1] = 0;
        word[0] = 0;
        word[1] = 0;
        // BUG IN THE ORIGINAL: only the FIRST byte of stamp_10_ is cleared.
        // The field is a dword (check_polling reads and writes all four bytes
        // as a millisecond timestamp), so three stale high bytes survive a
        // re-init and a reused slot can look already-heard-from. Reproduced -
        // a dword store here would not be the image's bytes.
        *reinterpret_cast<uint8_t *>(&word[4]) = 0;
        word[3] = 0;
        word += 0x58 / sizeof(*word);
        --count;
    } while (count != 0);
    self_20_ = this;
    buffer_24_ = buffer_6E0_;
    direct_play_1c_ = NetDirectPlay;
    thread_id_4_ = GetCurrentThreadId();
    events_8_[0] = CreateEventA(0, 0, 0, 0);
    events_8_[1] = CreateEventA(0, 0, 0, 0);
    events_8_[2] = CreateEventA(0, 0, 0, 0);
    events_8_[3] = CreateEventA(0, 0, 0, 0);
    events_8_[4] = CreateEventA(0, 0, 0, 0);
    InitializeCriticalSection(&lock_2C_);
    field_28_ = 1;
    return 0;
}

/*
Purpose: Probe the DirectPlay lobby for a connection the command line asked
         for, and hand it to the session layer when there is one.

         `/net h` hosts, `/net j` joins, `/net n <name>` names the player and
         `/net tcp <address>` names the host to dial. Everything below the
         option walk is IDirectPlayLobbyA: DirectPlayLobbyCreateA builds the
         lobby, SetConnectionSettings hands it a DPLCONNECTION (an embedded
         DPSESSIONDESC2 plus a DPNAME), GetConnectionSettings reads it back
         sized to DPERR_BUFFERTOOSMALL, and Connect brings up IDirectPlay2A -
         which lands in the 0x9BE600 word every other Net entry point tests as
         the "net play is live" flag.

// ORIGINAL: 0x0062D9E0 ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z 0x0062D9E0-0x0062DF16
// symbol    ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z
// size      1334 bytes
// prototype int (__thiscall ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z)(Net* this, char*, _GUID*, int, int)
// callers   1   call targets   11
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062D8E0 0x0062E010 0x0062E540 0x00644EF2 0x006453E0 0x00645470 0x00645DD0 0x006468D0 0x006474A0 0x00650208
// indirect  0x0062DCAB 0x0062DD24 0x0062DD67 0x0062DDB9 0x0062DDCA 0x0062DE13 0x0062DE46 0x0062DE5B 0x0062DE73 0x0062DE91 0x0062DEBC
//
// The 0x6717C0/0x6717D0/0x6717E0 the image pushes are dxguid's
// IID_IDirectPlay2A / DPAID_INet / DPSPGUID_TCPIP, and 0x697548 is its own
// copy of the application GUID control_game passes in as `guid`. 0x697568 and
// 0x69756c are the literals "tcp" and "0123456789". DirectPlayLobbyCreateA is
// a real DPLAYX.DLL import, so this file carries the
// `#pragma comment(lib, "dplayx.lib")` that names it - the one link input this
// tree cannot reach from CMakeLists.txt.
//
// QueryInterface fills the shared NetDirectPlay pointer with IDirectPlay2A.
// Other Net entry points test that same pointer, rather than a separate flag
// or a read from the original executable's fixed address.
//
// LEVER: the option walk is a `switch`, not an if/else chain - and that is not
//        a style choice. The switch is what makes VC6 promote the case value
//        through `movsx` and emit the image's own `sub eax,0x68 / sub eax,2 /
//        sub eax,4` chain; the if/else spelling gave `mov al,[esi+1] /
//        cmp al,0x68`. The switch ALSO flips VC6's register assignment so
//        `this` lands in EDI instead of ESI, which is what frees ESI for the
//        address buffer and drops the frame back to the image's 0x298.
// LEVER: `player_name` and `net_address` are BOTH 0x100 - the two buffers sit
//        at 0xA8 and 0x1A8 and the frame total is 0x298, so the first guess of
//        0xF0 for the second was wrong by exactly the padding it seemed to
//        explain.
// LEVER: the lobby-address local and the session buffer are TWO variables, not
//        one reused. As one long-lived `address` VC6 gave it a stack slot and
//        every struct offset shifted by 4; split, each fits the register the
//        image uses.
// LEVER: the strings "tcp"/"0123456789", the DP* structs out of <dplobby.h>
//        and the two `strchr(app_name, '/')` calls - the image really does
//        call it twice with the same arguments, the first result discarded.
// TRIED: an if/else chain instead of the switch - `this` moved to ESI, the
//        address buffer to the stack, frame 0x28C, 12 of 432 agreeing.
// TRIED: declaring `host_mode`/`lobby`/`address` before the `init` call - the
//        prologue then zeroed them before the call, where the image zeroes
//        them after; moved after and the first 16 instructions agree.
// TRIED: `char *address` declared without an initialiser and zeroed at the
//        DirectPlayLobbyCreateA call - 16 of 432, worse.
// TRIED: `session`/`name`/`connection` assignment order in all five
//        permutations (name,session,conn / sess,name,conn / conn,sess,name /
//        sess,conn,name / guidInstance first) - 20 of 432 for every one. The
//        interleaved store order the image shows is VC6's scheduler over the
//        hoisted GUID loads, not source order.
// TRIED: dropping the `goto`/`fail:` label and nesting the five late failure
//        conditions - 20 of 432, unchanged. The merge is not caused by the
//        label.
// TRIED: `dwFlags != DPOPEN_JOIN` for the session's status test - the image
//        compares with 2, so it is `!= DPOPEN_CREATE`.
//
// THE RESIDUAL. MISMATCH 20 of 432, and it is two things, not a transcription
// error:
//  1. VC6 merges this body's three BARE `return 0`s into one block at the tail
//     (each is then a 6-byte `jne`), where the image writes all eight
//     epilogues out inline and branches over them with a 2-byte `je`. Probed
//     against four synthetic bodies - a cdecl free function, a __thiscall
//     member with a 0x200 frame, a 4-callee-saved-register member with a
//     switch and eight returns - and VC6 replicated the epilogues in every
//     one, so the trigger is in this body's shape and not in the flag set.
//     Everything after instruction 16 then aligns at a fixed +8 offset.
//  2. The 25 DPSESSIONDESC2/DPNAME/DPLCONNECTION field stores are emitted in a
//     different permutation (every offset otherwise identical). See the TRIED
//     line above: not reachable from assignment order.
Return Value: 1 joined, 2 hosting, 0 on any failure or no lobby request
Status: Homed from src/unrecovered/0062d9e0.cpp. MISMATCH - see THE RESIDUAL.
*/
int Net::check_for_lobby(char *app_name, _GUID *guid, int option_a,
                         int option_b) {
    if (init(guid, option_a, option_b) != 0) {
        return 0;
    }

    int host_mode = 0;
    IDirectPlayLobby *lobby = 0;
    unsigned long address_size;
    IDirectPlay2 *direct_play;
    DPLCONNECTION connection;
    DPNAME name;
    DPSESSIONDESC2 session;
    char net_address[0x100];
    char player_name[0x100];
    char *cursor;
    char *buffer;
    int status;

    if (app_name != 0) {
        field_44_ = option_b;
        if (strchr(app_name, '/') != 0) {
            cursor = strchr(app_name, '/');
            do {
                switch (*++cursor) {
                case 'h':
                    host_mode = 1;
                    break;
                case 'j':
                    host_mode = 0;
                    break;
                case 'n':
                    ++cursor;
                    char *end = strchr(cursor, ' ');
                    player_name[0] = '\0';
                    if (end != 0) {
                        *end = '\0';
                        strcat(player_name, cursor);
                        *end = ' ';
                    } else {
                        strcat(player_name, cursor);
                    }
                    break;
                }
                cursor = strchr(cursor, '/');
            } while (cursor != 0);

            if (host_mode == 0) {
                char *tcp = strstr(app_name, "tcp");
                if (tcp == 0) {
                    goto have_lobby;
                }
                tcp += strcspn(tcp, "0123456789");
                char *end = strchr(tcp, ' ');
                net_address[0] = '\0';
                if (end != 0) {
                    *end = '\0';
                    strcat(net_address, tcp);
                    *end = ' ';
                } else {
                    strcat(net_address, tcp);
                }
            }

            char *address = 0;
            DirectPlayLobbyCreateA(0, &lobby, 0, 0, 0);

            name.dwSize = sizeof(DPNAME);
            name.dwFlags = 0;
            name.lpszShortNameA = player_name;
            name.lpszLongNameA = player_name;

            session.dwSize = sizeof(DPSESSIONDESC2);
            session.dwFlags = DPSESSION_MIGRATEHOST;
            session.guidInstance = NetLobbyAppGuid;
            session.guidApplication = *guid;
            session.dwMaxPlayers = 7;
            session.dwCurrentPlayers = 0;
            session.lpszSessionNameA = 0;
            session.lpszPasswordA = 0;
            session.dwReserved1 = 0;
            session.dwReserved2 = 0;
            session.dwUser1 = option_b;
            session.dwUser2 = 0;
            session.dwUser3 = 0;
            session.dwUser4 = 0;

            connection.dwSize = sizeof(DPLCONNECTION);
            connection.dwFlags = host_mode != 0 ? DPOPEN_CREATE : DPOPEN_JOIN;
            connection.lpSessionDesc = &session;
            connection.lpPlayerName = &name;
            connection.guidSP = DPSPGUID_TCPIP;

            if (host_mode == 0) {
                if (lobby->CreateAddress(DPSPGUID_TCPIP, DPAID_INet,
                                         net_address, strlen(net_address) + 1,
                                         0, &address_size)
                    != DPERR_BUFFERTOOSMALL) {
                    return 0;
                }
                address = static_cast<char *>(mem_get(address_size));
                if (address == 0) {
                    close();
                    return 0;
                }
                if (lobby->CreateAddress(DPSPGUID_TCPIP, DPAID_INet,
                                         net_address, strlen(net_address) + 1,
                                         address, &address_size)
                    != 0) {
                    close();
                    return 0;
                }
                connection.lpAddress = address;
                connection.dwAddressSize = address_size;
            } else {
                connection.lpAddress = 0;
                connection.dwAddressSize = 0;
            }

            lobby->SetConnectionSettings(0, 0, &connection);
            if (address != 0) {
                free(address);
            }
        }
    }

have_lobby:
    if (lobby == 0) {
        if (DirectPlayLobbyCreateA(0, &lobby, 0, 0, 0) != 0) {
            close();
            return 0;
        }
    }

    if (lobby->GetConnectionSettings(0, 0, &address_size)
        != DPERR_BUFFERTOOSMALL) {
        lobby->Release();
        close();
        return 0;
    }

    buffer = static_cast<char *>(mem_get(address_size));
    if (buffer == 0) {
        return 0;
    }

    if (lobby->GetConnectionSettings(0, buffer, &address_size) == 0) {
        status = reinterpret_cast<DPLCONNECTION *>(buffer)->dwFlags
                     != DPOPEN_CREATE
                     ? DPOPEN_CREATE
                     : DPOPEN_JOIN;
        if (reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc != 0) {
            reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc->dwFlags =
                DPSESSION_MIGRATEHOST;
            reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc
                ->dwMaxPlayers = 7;
            if (lobby->SetConnectionSettings(
                    0, 0, reinterpret_cast<DPLCONNECTION *>(buffer)) == 0) {
                if (lobby->Connect(0, &direct_play, 0) == 0) {
                    if (direct_play->QueryInterface(IID_IDirectPlay2A,
                                                    reinterpret_cast<void **>(&NetDirectPlay))
                        == 0) {
                        direct_play_1c_ = NetDirectPlay;
                        direct_play->Release();
                        direct_play = 0;
                        if (status == 1) {
                            voice_flags_D8_ |= 0x10000;
                        } else {
                            voice_flags_D8_ |= 0x20000;
                        }
                        join_session(
                            0,
                            reinterpret_cast<DPLCONNECTION *>(buffer)
                                ->lpPlayerName->lpszShortNameA,
                            0);
                        free(buffer);
                        return status;
                    }
                }
            }
        }
    }

    if (lobby != 0) {
        lobby->Release();
    }
    free(buffer);
    close();
    return 0;
}

/*
Purpose: Zero the three head words and initialise the lock the fifo guards.
// ORIGINAL: 0x006339C0 ??0NetFifo@@QAE@XZ 0x006339C0-0x006339DB BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/units/006339c0.cpp, which reached the CRITICAL_SECTION through `(*reinterpret_cast<InitFn *>(g_00669168))(this + 0xc)` - the import slot spelled as a raw address. A plain `InitializeCriticalSection` call compiles to the image's own `call dword ptr [0x669168]`.
// LEVER: layout-corrected the IDB gave NetFifo 0x10 bytes and four uint32_t members. It is 0x24: ??0Net builds three of them at 0xE8, 0x10C and 0x130, spaced 0x24 apart, and this body starts a CRITICAL_SECTION at 0xC, which is 24 bytes. The fourth uint32_t was the head of that lock.
// symbol    ??0NetFifo@@QAE@XZ
// size      27 bytes
// kind      game
Return Value: `this`, which the image leaves in eax
Status: Complete
*/
NetFifo::NetFifo() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
    InitializeCriticalSection(&lock_);
}

// ---------------------------------------------------------------------------
// THE FIFO QUEUE - the four NetFifo bodies. A queue entry is one 0x18-byte
// allocation: the caller's int at +0, the routing key at +4, the return value
// at +8, a mem_get buffer at +0xC, its length at +0x10 and the intrusive next
// pointer at +0x14. head_ is +0, tail_ +4, count_ +8 and the critical section
// +0xC - the layout hypothesis_layouts.h declares.
//
// 0x00669174 and 0x0066917C are the IAT slots for LeaveCriticalSection and
// EnterCriticalSection, so the bodies below call the real functions; the
// image's `call dword ptr [0x66917c]` is the same import-table indirection.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00633A50 ?close@NetFifo@@QAEXXZ 0x00633A50-0x00633AAB
// TRIED: all four bodies in this block land MISMATCH at instruction 0-1 with
//   the same shape, and it is register allocation, not control flow: the
//   image saves ebx/ebp/esi and dedicates one to the critical-section address
//   (`lea ebp, [esi+0xc]` in the image is `lea ebp, [esi+0xc]` here too, but
//   the image got there AFTER three pushes where this tree needs two). The
//   archive's own transcription of close was 200/229 bytes at best for the
//   sibling get. Naming head_/tail_/count_ and calling the real
//   Enter/LeaveCriticalSection is the faithful form; the missing callee-saved
//   push is not reachable from source form without a dedicated local nobody
//   needs. 2026-09-04.
// size      92 bytes
// prototype void (__thiscall ?close@NetFifo@@QAEXXZ)(NetFifo* this)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
void NetFifo::close() {
  EnterCriticalSection(&lock_);
  while (head_ != 0) {
    NetFifoNode *next = head_->next_;
    if (head_->data_ != 0) {
      free(head_->data_);
    }
    head_->data_ = 0;
    if (head_ != 0) {
      free(head_);
    }
    head_ = next;
  }
  tail_ = 0;
  count_ = 0;
  LeaveCriticalSection(&lock_);
}

// ORIGINAL: 0x00633AB0 ?add@NetFifo@@QAEHPAXKKHK@Z 0x00633AB0-0x00633B9A
// size      235 bytes
// prototype int (__thiscall ?add@NetFifo@@QAEHPAXKKHK@Z)(NetFifo* this, void*, unsigned long, unsigned long, int, unsigned long)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA30
int NetFifo::add(void *data, unsigned long size, unsigned long key, int type,
                 unsigned long extra) {
  EnterCriticalSection(&lock_);

  NetFifoNode *node = reinterpret_cast<NetFifoNode *>(mem_get(0x18));
  if (head_ == 0) {
    head_ = node;
    if (node == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
  } else {
    NetFifoNode *tail = tail_;
    tail->next_ = node;
    if (tail->next_ == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
  }
  tail_ = node;
  node->next_ = 0;

  node->key_ = key;
  node->caller_value_ = type;
  node->next_ = reinterpret_cast<NetFifoNode *>(extra);

  if (size != 0) {
    node->data_ = mem_get(size);
    if (node->data_ == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
    if (data != 0) {
      memcpy(node->data_, data, size);
    }
    node->data_size_ = size;
  }

  ++count_;
  LeaveCriticalSection(&lock_);
  return 0;
}

// ORIGINAL: 0x00633D90 ?get@NetFifo@@QAEHPAXPAIPAH1@Z 0x00633D90-0x00633E75
// symbol    ?get@NetFifo@@QAEHPAXPAIPAH2@Z
// size      229 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAH1@Z)(NetFifo* this, void*, unsigned int*, int*, int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The queue is walked from the head for a matching key, so this is the
// remove-by-key form; the 0x633F70 overload below pops the head instead.
int NetFifo::get(void *data, unsigned int *key, int *type, int *len) {
  EnterCriticalSection(&lock_);

  NetFifoNode *cur = head_;
  if (cur == 0) {
    LeaveCriticalSection(&lock_);
    return 0;
  }

  NetFifoNode *prev = 0;
  while (cur->key_ != *key) {
    NetFifoNode *next = cur->next_;
    prev = cur;
    if (next == 0) {
      LeaveCriticalSection(&lock_);
      return 0;
    }
    cur = next;
  }

  if (len != 0) {
    *len = static_cast<int>(cur->data_size_);
  }
  if (cur->data_ != 0) {
    if (data != 0) {
      memcpy(data, cur->data_, cur->data_size_);
    }
    free(cur->data_);
    cur->data_ = 0;
  }

  if (prev == 0) {
    head_ = cur->next_;
  } else {
    prev->next_ = cur->next_;
  }

  if (type != 0) {
    *type = cur->caller_value_;
  }
  int result = cur->result_;
  free(cur);
  --count_;
  LeaveCriticalSection(&lock_);
  return result;
}

// ORIGINAL: 0x00633F70 ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z 0x00633F70-0x00634067
// symbol    ?get@NetFifo@@QAEHPAXPAIPAH1@Z
// size      248 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z)(NetFifo* this, void*, unsigned int*, int*, unsigned int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The unsigned-int length pointer repeats the key type (backref 1); the
// keyed-search overload repeats the int* type (backref 2). Keep the original
// catalogue names as hints, distinct from these emitted-symbol bindings.
// The pop-the-head form: whatever is at the front of the queue goes back, and
// the key comes out with it.
int NetFifo::get(void *data, unsigned int *key, int *type,
                 unsigned int *len) {
  EnterCriticalSection(&lock_);

  NetFifoNode *node = head_;
  if (node == 0) {
    LeaveCriticalSection(&lock_);
    return 0;
  }

  if (len != 0) {
    *len = node->data_size_;
  }
  if (node->data_ != 0 && data != 0) {
    memcpy(data, node->data_, node->data_size_);
    if (node->data_ != 0) {
      free(node->data_);
    }
    node->data_ = 0;
  }
  head_ = node->next_;
  if (key != 0) {
    *key = node->key_;
  }
  if (type != 0) {
    *type = node->caller_value_;
  }
  int result = node->result_;
  free(node);
  --count_;
  LeaveCriticalSection(&lock_);
  return result;
}
// ---------------------------------------------------------------------------
// THE DIRECTPLAY GROUP CALLS. All three read the interface pointer out of
// NetDirectPlay - the image's 0x009BE600 .bss, already modelled above - and
// dispatch straight through its vtable, which is what vtable_method is for:
// the call operand stays in memory, so the site keeps the image's single
// `call dword ptr [vtable + N]`. The catalogue says nothing about which of
// the two ids is the group and which is the member, so add_to_group's
// parameters are named for the order the call site sends them in.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x0062EDB0 ?delete_group@Net@@QAEXH@Z 0x0062EDB0-0x0062EDC7 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?delete_group@Net@@QAEXH@Z)(Net* this, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
void Net::delete_group(int group) {
  if (NetDirectPlay != 0) {
    typedef void(__stdcall *slot_fn)(void *, int);
    vtable_method<slot_fn>(NetDirectPlay, 0x20)(NetDirectPlay, group);
  }
}

// ORIGINAL: 0x0062EDD0 ?add_to_group@Net@@QAEHJK@Z 0x0062EDD0-0x0062EDFA BYTE_EXACT
// size      43 bytes
// prototype int (__thiscall ?add_to_group@Net@@QAEHJK@Z)(Net* this, long, unsigned long)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
int Net::add_to_group(long group, unsigned long member) {
  if (NetDirectPlay == 0) {
    return 7;
  }
  typedef int(__stdcall *slot_fn)(void *, long, unsigned long);
  return vtable_method<slot_fn>(NetDirectPlay, 0x0c)(NetDirectPlay, group,
                                                     member) != 0;
}

// ORIGINAL: 0x0062EE00 sub_62ee00 0x0062EE00-0x0062EE1C BYTE_EXACT
// symbol    ?sub_62ee00@@YGHHH@Z
// size      29 bytes
// prototype int (__stdcall sub_62ee00)(int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The catalogue has no name for this one, so the tree keeps the address name
// and the free __stdcall spelling the call sites use. Its return is the
// interface POINTER, not slot 7's own answer - EAX is left holding what it
// loaded, which is what its one caller goes on to test.
int __stdcall sub_62ee00(int first, int second) {
  unsigned long obj = reinterpret_cast<unsigned long>(NetDirectPlay);
  if (obj != 0) {
    typedef int(__stdcall *slot_fn)(void *, int, int);
    obj = static_cast<unsigned long>(
        vtable_method<slot_fn>(NetDirectPlay, 0x1c)(NetDirectPlay, first,
                                                    second));
  }
  return static_cast<int>(obj);
}

// ORIGINAL: 0x0062DF70 ?start_voice@Net@@QAEHJ@Z 0x0062DF70-0x0062DFBE BYTE_EXACT
// size      79 bytes
// prototype int (__thiscall ?start_voice@Net@@QAEHJ@Z)(Net* this, long)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Gates on the interface, then on the two voice-flag bits at +0xD8: out of an
// in-flight capture there is nothing to start. The key is remembered at +0xD4
// so stop_voice can match it, and the started bit is lit only after
// VoiceTx::start comes back - the order the dispatch proves.
int Net::start_voice(long key) {
  if (NetDirectPlay == 0) {
    return 7;
  }
  if ((voice_flags_D8_ & 0x60000000) == 0) {
    field_54_ = 1;
    voice_key_D4_ = static_cast<uint32_t>(key);
    voice_tx_.start();
    voice_flags_D8_ |= 0x20000000;
  }
  return 0;
}

// ORIGINAL: 0x00630BF0 sub_630bf0 0x00630BF0-0x00630C29 BYTE_EXACT
// symbol    ?sub_630bf0@@YGHPBXHHHH@Z
// size      58 bytes
// prototype int (__stdcall sub_630bf0)(const void*, int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// DirectPlayEnumerateA's callback. `ret 0x14` pins five stack dwords and
// stdcall; the catalogue's own head spells them LPGUID/LPSTR/DWORD, which no
// declaration here can carry, so they are the plain pointer/int they are on
// the wire. Only the provider whose GUID matches KeptServiceProviderGuid is
// kept, and its description pointer and id land in the ServiceStruct the
// caller parked in ServiceEnumTarget.
int __stdcall sub_630bf0(const void *guid, int name, int unused_a, int unused_b,
                         int unused_c) {
  if (memcmp(guid, &KeptServiceProviderGuid, 0x10) != 0) {
    ServiceStruct *service =
        reinterpret_cast<ServiceStruct *>(ServiceEnumTarget);
    service->field_1C_ = static_cast<uint32_t>(name);
    service->field_20_ = reinterpret_cast<uint32_t>(guid);
    service->add(0);
  }
  return 1;
}

// ORIGINAL: 0x0062E250 ?poll_services@Net@@QAEHPAUServiceList@@@Z 0x0062E250-0x0062E299 BYTE_EXACT
// size      74 bytes
// prototype int (__thiscall ?poll_services@Net@@QAEHPAUServiceList@@@Z)(Net* this, ServiceList*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Park the list in ServiceEnumTarget for the length of the call - that is
// where sub_630bf0 picks it up - then enumerate. The stack is not cleaned
// after the call, which is what pins DirectPlayEnumerateA as the two-argument
// stdcall the import table carries; the nullary extern the scaffold invented
// could not even be called.
int Net::poll_services(ServiceList *list) {
  if (list == 0) {
    return 0x10;
  }
  if (guid_6E8_ == 0) {
    return 7;
  }
  ServiceEnumTarget = list;
  typedef int(__stdcall *enum_fn)(void *, void *);
  int result = reinterpret_cast<enum_fn>(DirectPlayEnumerateA)(
      reinterpret_cast<void *>(&sub_630bf0), 0);
  ServiceEnumTarget = 0;
  return result != 0;
}

// 0x009BE4C0 - the DPCAPS the service join reads the provider's latency out
// of. GetCaps refuses a zero dwSize, so join_service sizes it to the full
// 0x28 before the call. Real storage beside NetGetScratch's end and the
// player-table pointer above, not a Net member.
static DPCAPS NetServiceCaps;  // 0x009BE4C0

// 0x00697578, the launcher executable the internet service joins: the image
// runs it through its WinExec import at 0x669180 when the kept provider GUID
// matches. A `char *const` at namespace scope so VC6 folds the push.
static char *const NetZoneLauncher = (char *)0x00697578;  // "mplaynow.exe"

// 0x00631BA0, unrecovered (827 bytes): the DirectPlay DPERR interpreter the
// join paths hand their failing HRESULT to - a flat switch over the known
// DPERR_ values ending in the matching in-game error dialog. Still its own
// preserved seed (unrecovered/00631ba0.cpp), so this file carries the same
// forwarder shape sub_62f390 does.
static int sub_631ba0(int error) {
    typedef int(__stdcall *pending)(int);
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x00631BA0))(
        error);
}

/*
Purpose: Join one enumerated service provider: copy the provider GUID into
         the session's 0x6EC slot, and when it is the internet TCP/IP service
         the enumeration kept, just launch the Zone's "mplaynow.exe" - the
         DirectPlay lobby does the rest. Anything else creates the DirectPlay
         object directly, publishes it as the shared interface (the same
         0x9BE600 word every other entry point tests), reads the provider's
         latency, and seeds all sixteen player slots with it - 100 ms when
         the provider does not say.
// ORIGINAL: 0x0062E2A0 ?join_service@Net@@QAEHPAUServiceStruct@@@Z 0x0062E2A0-0x0062E41C FILE
// size      380 bytes
// prototype int (__thiscall ?join_service@Net@@QAEHPAUServiceStruct@@@Z)(Net* this, ServiceStruct*)
// callers   2   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00631BA0 0x00645C70 0x006465F0 0x00650214
// indirect  0x0062E2F2 0x0062E33B 0x0062E353 0x0062E373 0x0062E3A8
// Homed from src/unrecovered/0062e2a0.cpp.
Return Value: 0x10 with no service record, 1 after any reported failure, 1
         when the launcher exited below 21, 0x16 when it did not, else 0
Status: Coverage transcription - unclaimed body
*/
int Net::join_service(ServiceStruct *service) {
    if (service == 0) {
        return 0x10;
    }

    // The provider GUID the enumeration stored at record+8 travels twice:
    // once into the session's own 0x6EC slot, once as the create/QI/compare
    // operand. Four dword copies, not a block move, is what the image does.
    const unsigned int *provider =
        reinterpret_cast<const unsigned int *>(&service->field_8_);
    unsigned int *kept = reinterpret_cast<unsigned int *>(field_6EC_);
    kept[0] = provider[0];
    kept[1] = provider[1];
    kept[2] = provider[2];
    kept[3] = provider[3];

    if (memcmp(provider, &KeptServiceProviderGuid, 0x10) == 0) {
        // The internet service: hand off to the Zone launcher. SW_SHOW, and
        // anything under a successful-process answer reports "failed".
        unsigned int started = WinExec(NetZoneLauncher, SW_SHOW);
        return started < 0x15 ? 1 : 0x16;
    }

    void *direct_play = 0;
    // The DX1 create signature - the image's one DirectPlayCreate import,
    // filled through the out parameter the caller owns.
    HRESULT result = DirectPlayCreate(reinterpret_cast<LPGUID>(
                                          const_cast<unsigned int *>(provider)),
                                      reinterpret_cast<LPDIRECTPLAY *>(
                                          &direct_play),
                                      0);
    if (result != 0) {
        sub_631ba0(result);
        return 1;
    }

    // QueryInterface for the Ansi IDirectPlay2 the rest of Net dispatches
    // through, publish it, and release the creator either way.
    result = static_cast<IDirectPlay2 *>(direct_play)->QueryInterface(
        IID_IDirectPlay2A,
        reinterpret_cast<void **>(&NetDirectPlay));
    direct_play_1c_ = NetDirectPlay;
    if (result != 0) {
        static_cast<IDirectPlay2 *>(direct_play)->Release();
        sub_631ba0(result);
        return 1;
    }
    static_cast<IDirectPlay2 *>(direct_play)->Release();

    memset(&NetServiceCaps, 0, 0x28);
    NetServiceCaps.dwSize = 0x28;
    if (NetDirectPlay->GetCaps(&NetServiceCaps, 1) != 0) {
        return 1;
    }

    voice_flags_D8_ &= 0xEFFFFFFF;

    // The provider's latency estimate (0 when it would not say - the image
    // stores the whole dword, and a zero means the 100 default) lands in
    // every player slot's third word and in the 0x6D4 default beside them.
    unsigned long latency = NetServiceCaps.dwLatency;
    uint32_t *slot = &players_[0].field_C_;
    int count = 0x10;
    if (latency != 0) {
        do {
            *slot = latency;
            slot += 0x58 / sizeof(*slot);
            --count;
        } while (count != 0);
        field_6D4_ = latency;
    } else {
        do {
            *slot = 100;
            slot += 0x58 / sizeof(*slot);
            --count;
        } while (count != 0);
        field_6D4_ = 100;
    }
    return 0;
}

// The modem and serial service-provider address GUIDs the DX5 SDK spells
// DPAID_Modem / DPAID_Serial - dpaddr.h is not in this include set, so the
// two create_session compares join NetLobbyAppGuid and
// KeptServiceProviderGuid as literals read off the image (0x006717F0 /
// 0x00671800). Two-player sessions are forced for both.
static const GUID NetModemServiceGuid = {
    0x44EAA760, 0xCB68, 0x11CF, {0x9C, 0x4E, 0x00, 0xA0, 0xC9, 0x05, 0x42, 0x5E}};
static const GUID NetSerialServiceGuid = {
    0x0F1D6860, 0x88D9, 0x11CF, {0x9C, 0x4E, 0x00, 0xA0, 0xC9, 0x05, 0x42, 0x5E}};

/*
Purpose: Host a named session: close any live one, size it to two players
         when the joined provider was the modem or the serial service, open
         the DPSESSIONDESC2 (MIGRATEHOST, the session GUID from init as the
         application GUID, the option word at 0x44 riding dwUser2), then
         re-read the live description through GetSessionDesc and file its
         name, password, instance GUID, flags and player ceiling in the
         allocation node at 0x700 - the same record the enumeration callback
         fills for received sessions. Leaves the 0x10000 "in a lobby session"
         bit lit.
// ORIGINAL: 0x0062EAA0 ?create_session@Net@@QAEHPADHPAD@Z 0x0062EAA0-0x0062ED2F FILE
// size      655 bytes
// prototype int (__thiscall ?create_session@Net@@QAEHPADHPAD@Z)(Net* this, int8*, int, int8*)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470 0x00645C70 0x006465F0
// indirect  0x0062EB08 0x0062EBD3 0x0062EBF4 0x0062EC20 0x0062EC60
// Homed from src/unrecovered/0062eaa0.cpp.
Return Value: the allocation node's address, or 0 when any stage failed
Status: Coverage transcription - unclaimed body
*/
int Net::create_session(char *name, int max_players, char *password) {
    if (NetDirectPlay == 0 || name == 0 || max_players < 2) {
        return 0;
    }

    if (allocation_node_.session_name_ != 0) {
        free(allocation_node_.session_name_);
        allocation_node_.session_name_ = 0;
        // The replaced session is closed through the interface.
        NetDirectPlay->Close();
    }
    if (allocation_node_.password_ != 0) {
        free(allocation_node_.password_);
        allocation_node_.password_ = 0;
    }

    // The modem and serial providers host exactly two players.
    const unsigned int *provider =
        reinterpret_cast<const unsigned int *>(field_6EC_);
    int seeded_max_players = max_players;
    if (memcmp(provider, &NetModemServiceGuid, 0x10) == 0
        || memcmp(provider, &NetSerialServiceGuid, 0x10) == 0) {
        seeded_max_players = 2;
    }

    DPSESSIONDESC2 desc;
    memset(&desc, 0, 0x50);
    desc.dwSize = 0x50;
    desc.dwFlags = DPSESSION_MIGRATEHOST;
    const unsigned int *session_guid =
        reinterpret_cast<const unsigned int *>(guid_6E8_);
    unsigned int *application =
        reinterpret_cast<unsigned int *>(&desc.guidApplication);
    application[0] = session_guid[0];
    application[1] = session_guid[1];
    application[2] = session_guid[2];
    application[3] = session_guid[3];
    desc.dwMaxPlayers = seeded_max_players;
    desc.lpszSessionNameA = name;
    desc.lpszPasswordA =
        password != 0 && *password != 0 ? password : 0;
    desc.dwUser2 = field_44_;

    if (NetDirectPlay->Open(&desc, DPOPEN_CREATE) != 0) {
        return 0;
    }

    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);

    memset(&NetServiceCaps, 0, 0x28);
    NetServiceCaps.dwSize = 0x28;
    if (NetDirectPlay->GetCaps(&NetServiceCaps, 1) != 0) {
        return 0;
    }

    DPSESSIONDESC2 *live = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    if (live == 0) {
        return 0;
    }
    if (NetDirectPlay->GetSessionDesc(live, &size) != 0) {
        return 0;
    }

    allocation_node_.session_name_ =
        static_cast<char *>(mem_get(strlen(name) + 1));
    *allocation_node_.session_name_ = 0;
    strcpy(allocation_node_.session_name_, name);
    if (password != 0 && *password != 0) {
        allocation_node_.password_ =
            static_cast<char *>(mem_get(strlen(password) + 1));
        *allocation_node_.password_ = 0;
        strcpy(allocation_node_.password_, password);
    }

    const unsigned int *live_guid =
        reinterpret_cast<const unsigned int *>(&live->guidInstance);
    unsigned int *node_guid =
        reinterpret_cast<unsigned int *>(&allocation_node_.instance_guid_);
    node_guid[0] = live_guid[0];
    node_guid[1] = live_guid[1];
    node_guid[2] = live_guid[2];
    node_guid[3] = live_guid[3];
    allocation_node_.session_flags_ = live->dwFlags;
    allocation_node_.max_players_ = max_players;
    allocation_node_.current_players_ = 0;
    free(live);
    voice_flags_D8_ |= 0x10000;
    return reinterpret_cast<int>(&allocation_node_);
}

// ORIGINAL: 0x00630A40 sub_630a40 0x00630A40-0x00630A7D BYTE_EXACT
// symbol    ?sub_630a40@Net@@QAEHHH@Z
// size      62 bytes
// prototype int (__thiscall sub_630a40)(Net* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Reads one message off the 0x10C queue: NetGetScratch is the buffer, `first`
// receives the routing key and `second` the caller's int. The answer is the
// BUFFER's address when the read landed and zero when it did not - the
// image's `neg/sbb/and eax, 0x9bc4bc` tail, which is why the guard has to be
// an `extern` object and not a folded literal.
int Net::sub_630a40(int first, int second) {
  if (NetDirectPlay == 0) {
    return 0;
  }
  if (first == 0) {
    return 0;
  }
  int got = queue_10c_.get(NetGetScratch,
                            reinterpret_cast<unsigned int *>(first),
                            reinterpret_cast<int *>(0),
                            reinterpret_cast<int *>(second));
  return got != 0 ? reinterpret_cast<int>(NetGetScratch) : 0;
}

// ORIGINAL: 0x00630980 sub_630980 0x00630980-0x006309BA BYTE_EXACT
// symbol    ?sub_630980@Net@@QAEHHH@Z
// size      59 bytes
// prototype int (__thiscall sub_630980)(Net* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// The sender's twin of sub_630a40 above: hand the message to the 0x10C queue's
// own reader and answer the buffer's address on success.
// LEVER: the two guards SHARE one exit here (`je 0x6309b5` twice, one
//   xor/ret tail), which is the chained `||` shape - unlike sub_630a40 above,
//   whose image gives each guard its own inline `ret 8` and so wants two
//   separate `if`s. Measured both ways: chained 20/20 BYTE_EXACT, separate
//   guards 2/19 with the branch polarity flipped. 2026-09-04.
int Net::sub_630980(int first, int second) {
  if (NetDirectPlay == 0 || first == 0) {
    return 0;
  }
  int got = queue_10c_.sub_634200(NetGetScratch, first, 0, second);
  return got != 0 ? reinterpret_cast<int>(NetGetScratch) : 0;
}

// ORIGINAL: 0x006309C0 sub_6309c0 0x006309C0-0x006309FD BYTE_EXACT
// symbol    ?sub_6309c0@Net@@QAEHHH@Z
// size      61 bytes
// prototype int (__thiscall sub_6309c0)(Net* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// LEVER: the mirror image of sub_630980 above, and the guards are the one
//   thing that differs - HERE the image gives each guard its own inline
//   `xor/ret 8`, so the two tests stay SEPARATE; chaining them shared one
//   tail and flipped the polarity (2/19). Same body, opposite branch shape,
//   on two functions 64 bytes apart. Measured both ways, 2026-09-04.
int Net::sub_6309c0(int first, int second) {
  if (NetDirectPlay == 0) {
    return 0;
  }
  if (first == 0) {
    return 0;
  }
  int got = queue_10c_.sub_6340c0(NetGetScratch, first, 0, second);
  return got != 0 ? reinterpret_cast<int>(NetGetScratch) : 0;
}

// ORIGINAL: 0x00632300 ?set_polling@Net@@QAEXH@Z 0x00632300-0x0063234F BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?set_polling@Net@@QAEHH@Z)(Net* this, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Sixteen per-player polling stamps at 0x164 - players_[i].stamp_10_, 0x58
// apart - drop to zero; the polling bit in the 0xD8 flag word follows the
// argument.
void Net::set_polling(int flag) {
  for (int i = 0; i < 0x10; ++i) {
    players_[i].stamp_10_ = 0;
  }
  if (flag != 0) {
    voice_flags_D8_ |= 0x40000;
  } else {
    voice_flags_D8_ &= 0xFFFBFFFF;
  }
}

// ---------------------------------------------------------------------------
// THE SESSION LOCK TOGGLES. close_game and enable_join are one body twice:
// only the DPSESSION_JOINDISABLED bit they store differs. Both gate on the
// 0x10000 bit of the 0xD8 flag word - the "joined a lobby session" state
// check_for_lobby lights - because the round-trip is a lobby-session call.
// ---------------------------------------------------------------------------

/*
Purpose: Close the joined session to further joins: read the session
         description back through the interface, light DPSESSION_JOINDISABLED
         in its flags dword, and store it again.
// ORIGINAL: 0x00631B30 ?close_game@Net@@QAEXXZ 0x00631B30-0x00631B9A FILE
// size      106 bytes
// prototype void (__thiscall ?close_game@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// indirect  0x00631B51 0x00631B71 0x00631B87
// Homed from src/recovered/units/00631b30.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void Net::close_game() {
    if (NetDirectPlay == 0 || (voice_flags_D8_ & 0x10000) == 0) {
        return;
    }
    // Size pass, then the real read: GetSessionDesc(NULL, &size) answers the
    // needed byte count without touching a buffer.
    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);
    DPSESSIONDESC2 *desc = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    NetDirectPlay->GetSessionDesc(desc, &size);
    // The flags dword at desc+4: the whole dword is read back, bit 5 set, and
    // the dword stored - a byte `or` inside a dword read-modify-write.
    desc->dwFlags |= DPSESSION_JOINDISABLED;
    NetDirectPlay->SetSessionDesc(desc, 0);
    if (desc != 0) {
        free(desc);
    }
}

/*
Purpose: Reopen the joined session to joins - close_game's mirror, clearing
         DPSESSION_JOINDISABLED by the same read-modify-store round trip.
// ORIGINAL: 0x00632070 ?enable_join@Net@@QAEXXZ 0x00632070-0x006320DA FILE BYTE_EXACT
// size      106 bytes
// prototype void (__thiscall ?enable_join@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// Homed from src/recovered/units/00632070.cpp, retired 2026-09-27 by
// record-then-retire: this body was fresh-measured 42/42 BYTE_EXACT in this
// TU and the claim now banks here.
Return Value: n/a
Status: Complete
*/
void Net::enable_join() {
    if (NetDirectPlay == 0 || (voice_flags_D8_ & 0x10000) == 0) {
        return;
    }
    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);
    DPSESSIONDESC2 *desc = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    NetDirectPlay->GetSessionDesc(desc, &size);
    // Bit 5 of the low byte, exactly like close_game sets it - `and al, 0xdf`.
    desc->dwFlags &= ~DPSESSION_JOINDISABLED;
    NetDirectPlay->SetSessionDesc(desc, 0);
    if (desc != 0) {
        free(desc);
    }
}

// 0x009BE4EC - the DPNAME record the 3-case's GetPlayerName fills; its
// lpszLongNameA slot at +0xC is the text that case forwards. Fixed storage
// beside NetServiceCaps, not a Net member.
static unsigned char *const NetPlayerNameRecord =
    (unsigned char *)0x009BE4EC;

// 0x00632CC0, unrecovered (234 bytes). Both handle_sys_msg call sites pass
// this receiver - the image sets ecx to Net - and answer through the three
// stack arguments the catalogued `ret 0xc` cleans up. In-file forwarder, the
// same shape drop_player's carried.
int Net::sub_632cc0(char *text, int id, unsigned char flag) {
    typedef int(__fastcall *pending)(Net *, void *, char *, int, unsigned char);
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x00632CC0))(
        this, nullptr, text, id, flag);
}

/*
Purpose: Drain the system-message queue: one 0x28-byte record at a time off
         the third fifo, each either a DPSYS_* envelope the image keys on the
         queue's routing key or a peer packet keyed on its first byte.

           key != 0 - a routed peer message:
             byte 0 bit 0x80 set            -> cut the named player;
             byte 0 bit 0x10 set            -> refresh the join list, one
                                               0x28-byte entry per record;
           key == 0 - a system envelope, by the type byte:
             3    both timer words agree and the payload says so
                  -> re-read the player's name and refresh one join-list entry;
             5    -> cut the named player (reason 1);
             0x31 -> the session was lost: raise it and close the connection;
             0x102 -> stamp the named player's slot byte from the message;
             0x103 -> the player's name changed (virtual slot 3).

         The receive scratch doubles as the message body: the type byte at 0,
         two dwords at 4 and 8, a name pointer at 0xC whose storage ALSO
         fronts the 0x28-stride join-list entries, and text pointers at 0x10
         and 0x18.
// ORIGINAL: 0x00632DB0 ?handle_sys_msg@Net@@QAEXXZ 0x00632DB0-0x00632F97 FILE
// size      487 bytes
// prototype void (__thiscall ?handle_sys_msg@Net@@QAEXXZ)(Net* this)
// callers   7   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062E010 0x00632350 0x00632CC0 0x00633F70
// indirect  0x00632EDB 0x00632F0C 0x00632F34
// Homed from src/recovered/units/00632db0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void Net::handle_sys_msg() {
    if (NetDirectPlay == 0) {
        return;
    }
    unsigned int key;
    // The pop-the-head read: the routing key comes out with the message, and
    // the scratch receives the body either way.
    while (queue_130_.get(NetGetScratch, &key,
                          reinterpret_cast<int *>(0),
                          reinterpret_cast<unsigned int *>(0)) != 0) {
        unsigned char *packet = NetGetScratch;
        unsigned long word_4 =
            *reinterpret_cast<unsigned long *>(packet + 4);
        unsigned long id = *reinterpret_cast<unsigned long *>(packet + 8);
        if (key != 0) {
            unsigned char flags = packet[0];
            if ((flags & 0x80) != 0) {
                drop_player(id, 1);
            } else if ((flags & 0x10) != 0) {
                // The join-list refresh: one 0x28-byte entry per count byte,
                // walked in place over the storage the name pointer fronts.
                const unsigned char *entry = packet + 0xC;
                unsigned char count =
                    static_cast<unsigned char>(id);
                for (unsigned char seen = 0; seen < count; ++seen) {
                    sub_632cc0(
                        reinterpret_cast<char *>(
                            const_cast<unsigned char *>(entry + 4)),
                        *reinterpret_cast<const unsigned long *>(entry),
                        entry[0x24]);
                    entry += 0x28;
                }
            }
            continue;
        }
        switch (packet[0]) {
        case 3:
            // Refresh one player's name: only while the two timer words name
            // the same target and the payload word agrees to re-read it.
            if (field_760_ == field_764_ && word_4 == 1) {
                unsigned long size = 0x100;
                NetDirectPlay->GetPlayerName(id, NetPlayerNameRecord, &size);
                unsigned char *text =
                    *reinterpret_cast<unsigned char **>(packet + 0x10);
                int first = text != 0 ? text[0] : 0;
                sub_632cc0(
                    *reinterpret_cast<char **>(NetPlayerNameRecord + 0xC), id,
                    static_cast<unsigned char>(first));
            }
            break;
        case 5:
            if (word_4 == 1) {
                drop_player(id, 1);
            }
            break;
        case 0x31:
            // The host moved on or the session died: raise it, then close.
            on_session_lost();
            close();
            break;
        case 0x102: {
            // Stamp the named player's slot byte from the message's own text.
            int index = 0;
            while (index < 0x10 && players_[index].field_0_ != id) {
                ++index;
            }
            if (index == 0x10) {
                break;
            }
            unsigned char *text =
                *reinterpret_cast<unsigned char **>(packet + 0xC);
            if (text == 0) {
                break;
            }
            players_[index].flags_14_ = text[0];
            break;
        }
        case 0x103:
            on_player_name_changed(
                *reinterpret_cast<char **>(packet + 0x18), id);
            break;
        default:
            break;
        }
    }
}
