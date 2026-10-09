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
 * You should have received a copy of the GNU General Public License along
 * with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "original_seam.h"
#include "midi.h"
// The guarded creation/release hooks and their guard dword - the same
// seams the Wave loader and the Sound destructor dispatch through. The
// hook calls themselves go through the product-owned SoundDllProcs
// table (sound.h), NOT wave.h's fixed-address accessors: the image calls
// `dword ptr [0x90DB24]`/`[0x90DB28]` with a HIGHLOW fixup at each site,
// and only a symbol-based spelling emits the DIR32 relocation the checker
// can see - a cast-of-constant folds the address into the disp32 with no
// relocation at all.
#include "wave.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
// init's one-byte clear of the Midi-own byte at 0x54 is a REAL memset call
// in the image (`push 1 / add esi,0x54 / push 0 / push esi / call` at
// 0x004C7CA3); /O2 implies /Oi and the intrinsic would inline it as
// `mov byte ptr [esi],0`. memset is not on stdafx.h's project-wide
// `#pragma function` list, so the forcing is TU-local here - this file's
// only memset - the same idiom textindex.cpp records for memcpy.
#pragma function(memset)
#endif

/*
 * The 32 forwarding methods of Midi, moved here from
 * delegation_thunks.cpp by the same pass that declared their delegate:
 * each is a real member whose delegate dispatch lets VC6 emit the
 * image's `call dword ptr [eax + slot]` itself. 28 measured BYTE_EXACT
 * on arrival (2bbecdbd); the last four were never register allocation
 * but the delegate's same-name overload-group vtable ordering - fixed
 * 2026-09-18 by the two neutral slotNNN names (midi.h's header note
 * carries the measured VC6 behavior and the deck that isolated it).
 *
 * SINCE 2026-09-18 Midi declares its real Sound base (midi.h): the
 * sequencer forwarders read Sound::device_ at 0x3C through a
 * MidiDelegate view - the flat header's `delegate_` member is gone -
 * and the five lifecycle bodies below the forwarder run are the same
 * pass's second half. Every marker, claim and TRIED note carries over
 * unchanged.
 */

/*
Purpose: ?play@Midi@@QAEHXZ - forward 0 argument(s) to slot 0x1c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7760 ?play@Midi@@QAEHXZ 0x004C7760-0x004C7772 BYTE_EXACT
// symbol    ?play@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?play@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::play() {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->play();
    }
    return 0x14;
}
/*
Purpose: ?set_switch_type@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x100 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C77D0 ?set_switch_type@Midi@@QAEHK@Z 0x004C77D0-0x004C77F3 BYTE_EXACT
// symbol    ?set_switch_type@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_switch_type@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C77E0
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::set_switch_type(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_switch_type(a1);
    }
    return 0x13;
}
/*
Purpose: ?add_switch_range@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x108 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7800 ?add_switch_range@Midi@@QAEHKK@Z 0x004C7800-0x004C7827 BYTE_EXACT
// symbol    ?add_switch_range@Midi@@UAEHKK@Z
// size      39 bytes
// prototype int (__thiscall ?add_switch_range@Midi@@QAEHKK@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7814
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::add_switch_range(unsigned long a1, unsigned long a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->add_switch_range(a1, a2);
    }
    return 0x13;
}
/*
Purpose: ?set_nswitch_threads@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x104 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7830 ?set_nswitch_threads@Midi@@QAEHK@Z 0x004C7830-0x004C7853 BYTE_EXACT
// symbol    ?set_nswitch_threads@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_nswitch_threads@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7840
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::set_nswitch_threads(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_nswitch_threads(a1);
    }
    return 0x13;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADKK@Z - forward 3 argument(s) to slot 0xdc of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7860 ?map_patch@Midi@@QAEHPADKK@Z 0x004C7860-0x004C788B SEMANTIC
// symbol    ?map_patch@Midi@@UAEHPADKK@Z
// size      43 bytes
// prototype int (__thiscall ?map_patch@Midi@@QAEHPADKK@Z)(Midi* this, int8*, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7878
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::map_patch(char *a1, unsigned long a2, unsigned long a3) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->slot055(a1, a2, a3);
    }
    return 0x14;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADK@Z - forward 2 argument(s) to slot 0xd8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7890 ?map_patch@Midi@@QAEHPADK@Z 0x004C7890-0x004C78B7 SEMANTIC
// symbol    ?map_patch@Midi@@UAEHPADK@Z
// size      39 bytes
// prototype int (__thiscall ?map_patch@Midi@@QAEHPADK@Z)(Midi* this, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C78A4
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::map_patch(char *a1, unsigned long a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->map_patch(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?clear_patch@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xe0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C78E0 ?clear_patch@Midi@@QAEHK@Z 0x004C78E0-0x004C7903 BYTE_EXACT
// symbol    ?clear_patch@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?clear_patch@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C78F0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::clear_patch(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->clear_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?reset@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7910 ?reset@Midi@@QAEHXZ 0x004C7910-0x004C7925 BYTE_EXACT
// symbol    ?reset@Midi@@UAEHXZ
// size      21 bytes
// prototype int (__thiscall ?reset@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::reset() {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->reset();
    }
    return 0x14;
}
/*
Purpose: ?set_track@Midi@@QAEHPAEK@Z - forward 2 argument(s) to slot 0x84 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7930 ?set_track@Midi@@QAEHPAEK@Z 0x004C7930-0x004C7957 BYTE_EXACT
// symbol    ?set_track@Midi@@UAEHPAEK@Z
// size      39 bytes
// prototype int (__thiscall ?set_track@Midi@@QAEHPAEK@Z)(Midi* this, unsigned int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7944
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_track(uint8_t *a1, unsigned long a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_track(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?mute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7960 ?mute_track@Midi@@QAEHK@Z 0x004C7960-0x004C7983 BYTE_EXACT
// symbol    ?mute_track@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?mute_track@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7970
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::mute_track(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->mute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?unmute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xac of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7990 ?unmute_track@Midi@@QAEHK@Z 0x004C7990-0x004C79B3 BYTE_EXACT
// symbol    ?unmute_track@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?unmute_track@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C79A0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::unmute_track(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->unmute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x94 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79C0 ?set_active_tracks@Midi@@QAEHKK@Z 0x004C79C0-0x004C79E7 SEMANTIC
// symbol    ?set_active_tracks@Midi@@UAEHKK@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_tracks@Midi@@QAEHKK@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C79D4
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_tracks(unsigned long a1, unsigned long a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->slot037(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x90 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79F0 ?set_active_tracks@Midi@@QAEHI@Z 0x004C79F0-0x004C7A13 SEMANTIC
// symbol    ?set_active_tracks@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?set_active_tracks@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A00
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_tracks(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_tracks(a1);
    }
    return 0x14;
}
/*
Purpose: ?play_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x10c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A20 ?play_trackset@Midi@@QAEHI@Z 0x004C7A20-0x004C7A43 BYTE_EXACT
// symbol    ?play_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?play_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A30
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::play_trackset(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->play_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?xpose_trackset@Midi@@QAEHIH@Z - forward 2 argument(s) to slot 0x114 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A50 ?xpose_trackset@Midi@@QAEHIH@Z 0x004C7A50-0x004C7A77 BYTE_EXACT
// symbol    ?xpose_trackset@Midi@@UAEHIH@Z
// size      39 bytes
// prototype int (__thiscall ?xpose_trackset@Midi@@QAEHIH@Z)(Midi* this, unsigned int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A64
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::xpose_trackset(unsigned int a1, int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->xpose_trackset(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?stop_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x110 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A80 ?stop_trackset@Midi@@QAEHI@Z 0x004C7A80-0x004C7AA3 BYTE_EXACT
// symbol    ?stop_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?stop_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A90
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::stop_trackset(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->stop_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z - forward 1 argument(s) to slot 0x98 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7AD0 ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z 0x004C7AD0-0x004C7AF3 BYTE_EXACT
// symbol    ?add_active_trackset@Midi@@UAEHPAUTrackSet@@@Z
// size      35 bytes
// prototype int (__thiscall ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z)(Midi* this, TrackSet*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7AE0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::add_active_trackset(TrackSet *a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->add_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_lo@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B00 ?set_active_range_lo@Midi@@QAEHII@Z 0x004C7B00-0x004C7B27 BYTE_EXACT
// symbol    ?set_active_range_lo@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_range_lo@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B14
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_range_lo(unsigned int a1, unsigned int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_range_lo(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_hi@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B30 ?set_active_range_hi@Midi@@QAEHII@Z 0x004C7B30-0x004C7B57 BYTE_EXACT
// symbol    ?set_active_range_hi@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_range_hi@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B44
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_range_hi(unsigned int a1, unsigned int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_range_hi(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?remove_active_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0xa8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B60 ?remove_active_trackset@Midi@@QAEHI@Z 0x004C7B60-0x004C7B83 BYTE_EXACT
// symbol    ?remove_active_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?remove_active_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B70
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::remove_active_trackset(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->remove_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?get_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x9c of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7B90 ?get_trackset@Midi@@QAEHI@Z 0x004C7B90-0x004C7BB0 BYTE_EXACT
// symbol    ?get_trackset@Midi@@UAEHI@Z
// size      32 bytes
// prototype int (__thiscall ?get_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7BA0
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_trackset(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->get_trackset(a1);
    }
    return 0;
}
/*
Purpose: ?get_ntracks@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7BB0 ?get_ntracks@Midi@@QAEHXZ 0x004C7BB0-0x004C7BC2 BYTE_EXACT
// symbol    ?get_ntracks@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_ntracks@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_ntracks() {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->get_ntracks();
    }
    return 0;
}
/*
Purpose: ?load_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x120 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7BD0 ?load_patch@Midi@@QAEHI@Z 0x004C7BD0-0x004C7BF3 BYTE_EXACT
// symbol    ?load_patch@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?load_patch@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7BE0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::load_patch(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->load_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?unload_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x124 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C00 ?unload_patch@Midi@@QAEHI@Z 0x004C7C00-0x004C7C23 BYTE_EXACT
// symbol    ?unload_patch@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?unload_patch@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7C10
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::unload_patch(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->unload_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_patch@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0x128 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C30 ?set_patch@Midi@@QAEHII@Z 0x004C7C30-0x004C7C57 BYTE_EXACT
// symbol    ?set_patch@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_patch@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7C44
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_patch(unsigned int a1, unsigned int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_patch(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_tempo@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x88 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7D50 ?set_tempo@Midi@@QAEHK@Z 0x004C7D50-0x004C7D73 BYTE_EXACT
// symbol    ?set_tempo@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_tempo@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7D60
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_tempo(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_tempo(a1);
    }
    return 0x14;
}
/*
Purpose: ?get_time@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xd4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7E80 ?get_time@Midi@@QAEHK@Z 0x004C7E80-0x004C7EA0 BYTE_EXACT
// symbol    ?get_time@Midi@@UAEHK@Z
// size      32 bytes
// prototype int (__thiscall ?get_time@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7E90
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_time(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->get_time(a1);
    }
    return 0;
}
/*
Purpose: ?get_control_track@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xf4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EA0 ?get_control_track@Midi@@QAEHXZ 0x004C7EA0-0x004C7EB2 BYTE_EXACT
// symbol    ?get_control_track@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_control_track@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_control_track() {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->get_control_track();
    }
    return 0;
}
/*
Purpose: ?get_total_track_ticks@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xf8 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EC0 ?get_total_track_ticks@Midi@@QAEHK@Z 0x004C7EC0-0x004C7EE0 BYTE_EXACT
// symbol    ?get_total_track_ticks@Midi@@UAEHK@Z
// size      32 bytes
// prototype int (__thiscall ?get_total_track_ticks@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7ED0
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_total_track_ticks(unsigned long a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->get_total_track_ticks(a1);
    }
    return 0;
}
/*
Purpose: ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z - forward 1 argument(s) to slot 0xe4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7EE0 ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z 0x004C7EE0-0x004C7F03 BYTE_EXACT
// symbol    ?set_channel_ctrl@Midi@@UAEHPAUChannelCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z)(Midi* this, ChannelCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7EF0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_channel_ctrl(ChannelCtrl *a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_channel_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z - forward 1 argument(s) to slot 0xe8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F10 ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z 0x004C7F10-0x004C7F33 BYTE_EXACT
// symbol    ?set_program_ctrl@Midi@@UAEHPAUProgramCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z)(Midi* this, ProgramCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7F20
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_program_ctrl(ProgramCtrl *a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_program_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z - forward 1 argument(s) to slot 0xc0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F40 ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z 0x004C7F40-0x004C7F63 BYTE_EXACT
// symbol    ?set_track_ctrl@Midi@@UAEHPAUTrackCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z)(Midi* this, TrackCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7F50
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_track_ctrl(TrackCtrl *a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_track_ctrl(a1);
    }
    return 0x14;
}

// ===== HOMED 2026-09-03 out of the archive. Each is one guarded virtual
// call through the sequencer at 0x3C - Sound::device_, cast to the
// MidiDelegate view - which is what the wrapped object is for.

/*
Purpose: Initialise the MIDI from a filename. The name replaces whatever
         copy the object already holds at 0x4C - the old block returns to
         the game CRT heap, the new one is measured, allocated on that
         same heap and copied in - and the one Midi-own flag byte at 0x54
         is cleared. The second argument is ignored; the status word is
         memset's own answer (the destination pointer).
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C7C60 ?init@Midi@@QAEHPADK@Z 0x004C7C60-0x004C7CB5 BYTE_EXACT
// symbol    ?init@Midi@@UAEHPADK@Z
// size      85 bytes
// prototype int (__thiscall ?init@Midi@@QAEHPADK@Z)(Midi* this, int8*, unsigned int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A 0x006465F0
// LEVER: `return reinterpret_cast<int>(memset(...))` - VC6 rejects a
//        non-void body with no return at all (C4716), and the image's
//        status word IS memset's eax, so the return spells the shared
//        fall-through of both paths. The null store between delete and
//        new is real (the seed measured it): the field is cleared before
//        the new block is measured.
// LEVER: `#pragma function(memset)` (file-local, above) - /O2 implies /Oi,
//        and the intrinsic expands a ONE-byte memset to
//        `mov byte ptr [esi],0`; the image's clear at 0x004C7CA3 is a real
//        call (`push 1 / add esi,0x54 / push 0 / push esi / call _memset /
//        add esp,0xC`). 80 -> 85 bytes, the marker's own size.
// LEVER: the destination is the declared member `field_54_` (midi.h),
//        not a raw this+0x54 cast - this file's one class-debt raw
//        self-access finding. Deck-measured 2026-09-18: the member
//        address compiles to the same bytes as the cast under /Oy-
//        (differing_offsets empty, every neighbor unchanged).
// PROMOTED from src/recovered/units/004c7c60.cpp (seed BYTE_EXACT); homed
//        onto the Sound base 2026-09-18 - the 0x4C slot is Sound::fname_
//        and the cleared byte is the first Midi-own byte past the 0x54
//        Sound sub-object.
// LEVER: the allocation is the DIRECT operator new - ??2@YAPAXI@Z, the
//        image's own callee at 0x0064558A - symmetric with the operator
//        delete above it (??3, 0x006453E0). Measured 2026-09-18: the
//        re-spelling is byte-neutral under /Oy- (differing_offsets still
//        empty) and swaps the emitted reference from the unbound
//        WaveOperatorNew forwarder to ??2, the identity the original
//        relocation names.
Return Value: memset's destination pointer, reinterpreted
Status: Complete
*/
int Midi::init(char *a1, unsigned long a2) {
    if (a1 != 0) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = nullptr;
        fname_ = operator new(strlen(a1) + 1);
        strcpy(static_cast<char *>(fname_), a1);
    }
    return reinterpret_cast<int>(
        memset(&field_54_, 0, 1));
}

/*
Purpose: Remember the MIDI's filename. The previous copy, if any, goes back
         to the game CRT heap first; the new string is measured, allocated
         on that same heap, recorded at 0x4C, and copied in - the field
         already names the block while it is still being filled.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// The staged preamble's `_strcpy`/`_strlen` decls are stale (arity-less,
// and `_strlen`/`_strcpy` as C++ names each pick up a SECOND leading
// underscore from the `extern "C"` __cdecl decoration, landing on
// `__strlen`/`__strcpy` - not the CRT's real `_strlen`/`_strcpy` symbol).
// ORIGINAL: 0x004C7CF0 ?set_fname@Midi@@QAEHPBD@Z 0x004C7CF0-0x004C7D42 BYTE_EXACT
// size      82 bytes
// prototype int (__thiscall ?set_fname@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// LEVER: branch polarity - `if (fname) { work; return 0; } return 0xA;`,
//        the same positive-branch shape Wave::set_fname records
//        (wave.cpp); the guard clause inlines the 0xA return at the top
//        instead of a shared tail at the end.
// PROMOTED from src/recovered/units/004c7cf0.cpp (seed BYTE_EXACT; its
//        `// symbol` PAC fact was the seed's own non-const parameter -
//        the product's `const char *` mangles the catalogued PBD); homed
//        onto the Sound base 2026-09-18 - the 0x4C slot is Sound::fname_.
// LEVER: the allocation is the DIRECT operator new, ??2@YAPAXI@Z at
//        0x0064558A, symmetric with the operator delete above it - the
//        same re-spelling init's marker records, measured with it
//        2026-09-18 (byte-neutral under /Oy-, emitted reference ??2 in
//        place of the unbound WaveOperatorNew forwarder).
Return Value: 0, or 0xA when the name is null
Status: Complete
*/
int Midi::set_fname(const char *fname) {
    if (fname) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = nullptr;
        fname_ = operator new(strlen(fname) + 1);
        strcpy(static_cast<char *>(fname_), fname);
        return 0;
    }
    return 0xA;
}

/*
Purpose: Load the MIDI from a caller-supplied filename. With no wrapped
         sequencer yet, the creation hook - the slot beside the release
         hook, behind the same guard dword - builds one directly into the
         0x3C field, with the MIDI's own mode 2 (Wave's loaders pass 1);
         the base Sound::load then reads the file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// NOT BYTE_EXACT - MISMATCH, closest reached (mnemonic_similarity 0.9275,
// 32/34 mnemonics in common). The control-flow shape matches the original
// exactly (same branches, same early return, same shared tail calling
// Sound::load), and the ONLY divergence is which callee-saved register
// holds which value: the original assigns esi=this, edi=a1; every source
// form tried here compiles to edi=this, esi=a1 instead - a straight swap,
// not a logic difference. Ruled out (all six produced the identical
// swap): if/else vs. nested-if vs. goto-threaded control flow, naming the
// Sound* cast in its own local, naming a1 in its own local, and addressing
// field_3c_ through a named member vs. a raw reinterpret_cast<char*>(this)
// + 0x3c pointer. This reads as a fixed backend register-allocation
// preference for this shape (which stack param gets which saved register)
// rather than anything source-form can steer - the same class of thing the
// scheduler notes above describe for `lea`/`push` ordering.
// ORIGINAL: 0x004C7D80 ?load@Midi@@QAEHPBD@Z 0x004C7D80-0x004C7DCD
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x4C7DB8 loads filename stack word into EAX; 0x4C7DBB tests it and only a nonzero word bypasses Sound::load at0x4C7DC2.", "Pinned 0x4C7D86 checks receiver device at+3C. Missing device uses the guarded factory with mode2; factory failure remains its returned integer.", "Existing Midi and Sound declarations own this virtual override and device field. The free factory and qualified base call retain their existing ABI; pointer-to-int narrowing is the configured 32-bit target interpretation."], "reason": "Preserve the original pointer-valued integer status on the already-wrapped MIDI path; a null filename must still reach the qualified Sound loader."}
// symbol    ?load@Midi@@UAEHPBD@Z
// size      78 bytes
// prototype int (__thiscall ?load@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C7D8D
// NO BYTE CLAIM: no seed ever measured this body; this is its first
//        product spelling (the task's one genuinely unmeasured body).
//        The Sound::load call is QUALIFIED because Midi::load(const
//        char*) overrides the same virtual - unqualified would recurse.
// Original ABI quirk: with a sequencer already wrapped, the image returns
//        the filename pointer bits as its integer status. A null filename
//        instead reaches Sound::load(nullptr); do not normalize either path
//        to a constant error code. Pointer-to-int narrowing below follows
//        the configured 32-bit VC6 target, not a portable pointer-width claim.
// Historical pre-restoration compiler diagnostic:
//        image hoists `lea eax,[esi+0x3C]` before the null test and caches
//        the argument in edi, so its guard and Sound::load sit at offsets
//        22 and 67 where this spelling puts them at 19 and 62. The
//        creation hook itself lands at the image's own offset 47
//        (measured 2026-09-18) through the SoundDllProcs table - the
//        include note above explains why the fixed-address accessor
//        cannot.
Return Value: filename pointer bits when already wrapped and non-null;
              otherwise 1 with a dead creation hook, the creation error,
              or the base loader's answer
Status: Complete
*/
int Midi::load(const char *a1) {
    int result;
    if (device_ == nullptr) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, a1, 2);
    } else {
        result = static_cast<int>(reinterpret_cast<uintptr_t>(a1));
    }
    if (result != 0) {
        return result;
    }
    return Sound::load(a1);
}

/*
Purpose: Load the MIDI from its remembered filename. With no wrapped
         sequencer yet, the guarded creation hook builds one directly
         into the 0x3C field, again with mode 2; the base Sound::load
         then reads the file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C7DD0 ?load@Midi@@QAEHXZ 0x004C7DD0-0x004C7E1B BYTE_EXACT
// symbol    ?load@Midi@@UAEHXZ
// size      75 bytes
// prototype int (__thiscall ?load@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C7DFB
// PROMOTED from src/recovered/004c7dd0.cpp (byte-exact emitter match, the
//        only seed for this family that called the base loader directly);
//        homed onto the Sound base 2026-09-18 - the remembered name is
//        Sound::fname_, the wrapped sequencer is Sound::device_, and the
//        qualified `Sound::load` is the direct 0x004C6280 call the image
//        makes (Midi::load() is an overload, not an override, but the
//        qualification keeps the call non-virtual either way).
// LEVER: the creation hook calls `dword ptr [SoundDllProcs+0]`, the
//        table sound.h owns - wave.h's fixed-address accessor emits the
//        same `call [mem]` but folds 0x90DB24 into the disp32 with NO
//        relocation, and the image has a HIGHLOW fixup there. Measured
//        2026-09-18: guard, hook and Sound::load relocations land at the
//        image's own offsets 23, 45 and 60.
Return Value: 0 on success, 8 with no filename, 1 with a dead creation
              hook, or the creation/load error
Status: Complete
*/
int Midi::load() {
    const char *filename = static_cast<const char *>(fname_);
    if (filename != 0) {
        if (device_ == nullptr) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = SoundXOrdinal1CreateSound()(
                &device_, filename, 2);
            if (created != 0) {
                return created;
            }
        }
        return Sound::load(filename);
    }
    return 8;
}

/*
Purpose: Release the loaded MIDI. While the sequencer at 0x3C is still
         sounding (the base Sound's own is_playing, family slot 0x5C),
         the sequencer's unload (family slot 5) runs alone and its answer
         is the result; once quiet, the same unload runs and the wrapped
         sequencer then goes back through the release hook when that hook
         is live. The sequencer is forgotten either way, the base's detach
         (slot 0x80) runs, and the loaded bit of the flag dword at 0x40
         is cleared.
// ORIGINAL: 0x004C7E20 ?unload@Midi@@QAEHXZ 0x004C7E20-0x004C7E79 BYTE_EXACT
// symbol    ?unload@Midi@@UAEHXZ
// size      89 bytes
// prototype int (__thiscall ?unload@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7E2F 0x004C7E3B 0x004C7E4B 0x004C7E56 0x004C7E66
// LEVER: the real Sound base retires the scaffold's MidiVCall shims - the
//        is_playing/detach dispatches become the compiler's own virtual
//        calls through the vtable the base now owns, and the sequencer's
//        slot-5 unload is MidiDelegate::slot005, promoted void->int here
//        because its answer is this body's result on the busy path.
// LEVER: NO named delegate local. The image holds the result in edi from
//        `xor edi,edi` at entry and re-reads Sound::device_ at each
//        dispatch (`mov ecx,[esi+0x3C]` / `mov edx,[ecx]` after the
//        is_playing call, `mov eax,[esi+0x3C]` before the release hook);
//        a named local steals edi for the pointer and pushes ebx for the
//        result, two instructions the image has not. 96 -> 89 bytes.
// LEVER: the release hook calls `dword ptr [SoundDllProcs+4]`, not
//        wave.h's fixed-address accessor - the accessor folds 0x90DB28
//        into the disp32 with no relocation where the image has a HIGHLOW
//        fixup (see the include note above). Measured 2026-09-18: guard
//        and hook relocations land at the image's own offsets 31 and 45.
// LEVER: own vtable shim (not the scaffolded VCall) so slots 5/23 return int; offset 0x40 field reached by pointer arithmetic, masked &=
// PROMOTED from git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004c7e20.cpp (scaffold BYTE_EXACT with
//        the shim tables); this is the body's first product spelling.
Return Value: the sequencer's unload answer on the busy path, 0 otherwise
Status: Complete
*/
int Midi::unload() {
    int result = 0;
    if (device_ != nullptr) {
        if (is_playing() == 0) {
            static_cast<MidiDelegate *>(device_)->slot005();
            if (WaveDeviceReleaseGuard) {
                SoundXOrdinal2DeleteSound()(device_);
            }
        } else {
            result = static_cast<MidiDelegate *>(device_)->slot005();
        }
        device_ = nullptr;
    }
    detach();
    flags_40_ &= ~1u;
    return result;
}


// ORIGINAL: 0x004C7780 ?set_volume@Midi@@QAEXH@Z 0x004C7780-0x004C77AC BYTE_EXACT
// symbol    ?set_volume@Midi@@UAEXH@Z
void Midi::set_volume(int volume) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        delegate->set_volume(volume);
    }
    volume_ = volume & 0x7f;
}

// ORIGINAL: 0x004C77B0 ?switch_state@Midi@@QAEH_N@Z 0x004C77B0-0x004C77D0 BYTE_EXACT
// symbol    ?switch_state@Midi@@UAEH_N@Z
// NO RETURN on the null path, deliberately: the original leaves eax alone.
int Midi::switch_state(bool on) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->switch_state(on);
    }
}

// ORIGINAL: 0x004C78C0 ?set_base_path@Midi@@QAEHPBD@Z 0x004C78C0-0x004C78DA BYTE_EXACT
// symbol    ?set_base_path@Midi@@UAEHPBD@Z
int Midi::set_base_path(const char *path) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_base_path(path);
    }
}

// ORIGINAL: 0x004C7AB0 ?is_trackset_playing@Midi@@QAE_NI@Z 0x004C7AB0-0x004C7AD0 BYTE_EXACT
// symbol    ?is_trackset_playing@Midi@@UAE_NI@Z
bool Midi::is_trackset_playing(unsigned int trackset) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->is_trackset_playing(trackset);
    }
    return false;
}

// ORIGINAL: 0x004C8050 ?set_play_type@Midi@@QAEHI@Z 0x004C8050-0x004C8072
// symbol    ?set_play_type@Midi@@UAEHI@Z
// Historical TRIED interpretation below is disproven by entry-based decoding:
// the null-delegate branch reaches pop ebp / ret 4 at 0x004C8066/0x004C8067.
// Six padding NOPs follow; mov eax, ecx at 0x004C8070 belongs to the separate
// mapped initializer, and is never reached by this forwarder's null path.
// On that path this method does not write EAX: its result depends on incoming
// register state, not this or a defined zero. The missing C++ return remains an
// unresolved compiler-effects obligation, not proof of faithful reproduction.
// Keep the historical span/claim obligations; this correction does not change
// the original marker or pretend to resolve its overlap with the next entry.
// TRIED: the guarded forwarder is byte-identical for 11 instructions and then
//   the image has six `nop`s and `mov eax, ecx` - on the null delegate path the
//   original returns `this` in eax, not an unset eax, so the artifact's "no
//   return, the original leaves eax untouched" note is wrong about what the
//   caller sees. Spelling that needs `return (int)this;`, a pointer-as-int
//   cast the class-debt ratchet holds at zero. Left at 11 of 18.
// Historical incorrect conclusion: the shipped body leaves eax = `this` on the null path
      //   because ecx still holds it, so callers read a pointer as the int result.
//   Historical reproduction claim is unverified; see the correction above.
int Midi::set_play_type(unsigned int type) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate != nullptr) {
        return delegate->set_play_type(type);
    }
}


// RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?get_game_hwnd@Midi@@UAEHXZ 0x004C75B0
int Midi::get_game_hwnd() { return 0; }

// RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?get_attrib@Midi@@UAEHXZ 0x004C75B0
int Midi::get_attrib() { return 0; }

// ORIGINAL: 0x004C93E0 sub_4c93e0 0x004C93E0-0x004C93E8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Sound table 0x0066E3C0 slot 27 at 0x0066E42C selects 0x00645844, the pure-call path", "pinned:Midi table 0x0066E538 slot 27 at 0x0066E5A4 selects 0x004C93E0", "pinned:Ambience table 0x0066E664 preserves the Midi slot-27 selection at 0x004C93E0", "pinned:0x004C93E0 is mov eax,0x0b; ret 4; the source method has the observed one-dword thiscall stack cleanup and integer result", "source:Midi derives from Sound; Ambience derives from Midi; global Ambience g_AMBIENCE and faction ambience objects require a concrete Midi contract", "history:the complete existing 0x004C93E0 identity, original span, claim and adapter-origin note are transferred from src/field_accessors.cpp; no new claim is granted"], "reason": "Transfer the existing constant-return identity to Midi's real inherited virtual override. Sound's original table is pure; Midi and Ambience tables select this ABI-compatible method. The source API name is the current genuine owner, not a claim about the original undecorated catalog spelling."}
// symbol    ?set_attrib@Midi@@UAEHK@Z
// size      8 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
int Midi::set_attrib(unsigned long) { return 0x0B; }

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// `mov ecx, dword ptr[0x9bc074]`-free entry, but the body reads `[ecx+N]`
// with no matching stack slot, so ecx is `this`, not a stack argument - the
// contract's nullary `__cdecl` is wrong. This is a constructor: three
// vtable installs in sequence, an SEH prologue (`fs:[0]`) with a 0x659f20
// unwind handler that resets the vtable to the first-phase value, and a
// span at 0x4c8450/0x659f20/0x659f28 that is PART of this same function
// (the "spans" list is multi-range). The SEH frame is not modeled here -
// see TRIED.
// ORIGINAL: 0x004C75B0 sub_4c75b0 0x004C75B0-0x004C766D;0x004C8450-0x004C8457;0x00659F20-0x00659F32 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/unrecovered/004c75b0.cpp", "src/midi.h", "src/ambience.h"], "reason": "Original Sound-to-Midi constructor staging and byte54 state identify the genuine Midi owner. Historical seed mapping includes the already shared unwind helper; its overlap remains a restoration limitation, not a newly inferred independent function extent."}
// symbol    ??0Midi@@QAE@XZ
// TRIED: SHARED_TAIL - the 0x4c8450/0x659f20 unwind-handler span is COMDAT-folded with another function's handler, so no tier is reachable regardless of body. Left as a plain field-write transcription (no SEH frame modeled) for the next pass.
Midi::Midi() {
    memset(&field_54_, 0, 1);
    volatile uint8_t &mode = field_54_;
    volatile uint32_t &flags = flags_40_;
    mode |= 1;
    flags |= 8;
}

// Original4C7CC0: replace bit0 throughout the chained Midi objects.
// ORIGINAL: 0x004C7CC0 sub_4c7cc0 0x004C7CC0-0x004C7CEA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/004c7cc0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?set_mode@Midi@@QAEHH@Z
int Midi::set_mode(int mode) {
    field_54_ ^= (field_54_ ^ static_cast<uint8_t>(mode)) & 1;
    Midi *const child = static_cast<Midi *>(device_);
    if (child) child->set_mode(mode);
    return 0;
}

// ORIGINAL: 0x004C7F70 sub_4c7f70 0x004C7F70-0x004C7F97 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/004c7f70.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7f70@Midi@@UAEHHH@Z
int Midi::sub_4c7f70(int a1, int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) return delegate->slot044(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C7FA0 sub_4c7fa0 0x004C7FA0-0x004C7FC0 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/004c7fa0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7fa0@Midi@@UAE_NI@Z
bool Midi::sub_4c7fa0(unsigned int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) return delegate->slot045(a1);
    return false;
}
// ORIGINAL: 0x004C7FC0 sub_4c7fc0 0x004C7FC0-0x004C7FE7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/004c7fc0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7fc0@Midi@@UAEHHH@Z
int Midi::sub_4c7fc0(int a1, int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) return delegate->slot047(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C7FF0 sub_4c7ff0 0x004C7FF0-0x004C8017 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/004c7ff0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7ff0@Midi@@UAEHHH@Z
int Midi::sub_4c7ff0(int a1, int a2) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) return delegate->slot046(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C8020 sub_4c8020 0x004C8020-0x004C8043 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004c8020.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c8020@Midi@@UAEHH@Z
int Midi::sub_4c8020(int a1) {
    MidiDelegate *const delegate = static_cast<MidiDelegate *>(device_);
    if (delegate) return delegate->slot060(a1);
    return 0x14;
}

/*
Purpose: Destroy an ambience. Its own stage frees the remembered filename
         and releases the device through the guarded hook, clearing both
         fields unconditionally; the compiler-appended ~Sound then repeats
         the same teardown on the now-cleared fields (its frees no-op) and
         unlinks from the global sound chain when the chained bit is set -
         the image carries that Sound stage INLINED, which is why the old
         transcription duplicated it by hand.
// ORIGINAL: 0x004C7670 ??1Ambience@@QAE@XZ 0x004C7670-0x004C7760;0x004C8450-0x004C8457;0x00659F32-0x00659F4C
// symbol    ??1Midi@@UAE@XZ
// size      273 bytes
// prototype void (__thiscall ??1Ambience@@QAE@XZ)(Ambience* this)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C76BE 0x004C76FA
Return Value: n/a
Status: Complete
*/
Midi::~Midi() {
    // THE OWN STAGE ONLY: free the remembered filename and release the
    // device through the guarded hook, clearing both unconditionally. The
    // compiler appends ~Sound, whose stage repeats the same teardown on the
    // now-cleared fields (so its frees no-op) and unlinks from the global
    // sound chain when the chained bit sets - the work the image's ~Ambience
    // carries INLINED. The vtable staging (0x66E538 -> 0x66E3C0 -> 0x66E444)
    // is the compiler's now; AmbienceSoundView - Sound's own member layout
    // under another name - is gone with the inheritance that made it
    // redundant.
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = nullptr;
    if (device_ && WaveDeviceReleaseGuard) {
        (SoundXOrdinal2DeleteSound())(device_);
    }
    device_ = nullptr;
}
