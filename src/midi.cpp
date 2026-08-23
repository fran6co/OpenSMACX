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

/*
 * The 32 forwarding methods of Midi, moved here from
 * delegation_thunks.cpp by the same pass that declared their delegate:
 * each is a real member whose `delegate_->f(...)` lets VC6 emit the
 * image's `call dword ptr [eax + slot]` itself. 28 measured BYTE_EXACT
 * on arrival (2bbecdbd); four sit one register-allocation step away.
 */

/*
Purpose: ?play@Midi@@QAEHXZ - forward 0 argument(s) to slot 0x1c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7760 ?play@Midi@@QAEHXZ 0x004C7760-0x004C7772 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->play();
    }
    return 0x14;
}
/*
Purpose: ?set_switch_type@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x100 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C77D0 ?set_switch_type@Midi@@QAEHK@Z 0x004C77D0-0x004C77F3 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_switch_type(a1);
    }
    return 0x13;
}
/*
Purpose: ?add_switch_range@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x108 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7800 ?add_switch_range@Midi@@QAEHKK@Z 0x004C7800-0x004C7827 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->add_switch_range(a1, a2);
    }
    return 0x13;
}
/*
Purpose: ?set_nswitch_threads@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x104 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7830 ?set_nswitch_threads@Midi@@QAEHK@Z 0x004C7830-0x004C7853 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_nswitch_threads(a1);
    }
    return 0x13;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADKK@Z - forward 3 argument(s) to slot 0xdc of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7860 ?map_patch@Midi@@QAEHPADKK@Z 0x004C7860-0x004C788B
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
    if (delegate_ != nullptr) {
        return delegate_->map_patch(a1, a2, a3);
    }
    return 0x14;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADK@Z - forward 2 argument(s) to slot 0xd8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7890 ?map_patch@Midi@@QAEHPADK@Z 0x004C7890-0x004C78B7
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
    if (delegate_ != nullptr) {
        return delegate_->map_patch(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?clear_patch@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xe0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C78E0 ?clear_patch@Midi@@QAEHK@Z 0x004C78E0-0x004C7903 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->clear_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?reset@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7910 ?reset@Midi@@QAEHXZ 0x004C7910-0x004C7925 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->reset();
    }
    return 0x14;
}
/*
Purpose: ?set_track@Midi@@QAEHPAEK@Z - forward 2 argument(s) to slot 0x84 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7930 ?set_track@Midi@@QAEHPAEK@Z 0x004C7930-0x004C7957 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_track(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?mute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7960 ?mute_track@Midi@@QAEHK@Z 0x004C7960-0x004C7983 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->mute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?unmute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xac of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7990 ?unmute_track@Midi@@QAEHK@Z 0x004C7990-0x004C79B3 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->unmute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x94 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79C0 ?set_active_tracks@Midi@@QAEHKK@Z 0x004C79C0-0x004C79E7
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
    if (delegate_ != nullptr) {
        return delegate_->set_active_tracks(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x90 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79F0 ?set_active_tracks@Midi@@QAEHI@Z 0x004C79F0-0x004C7A13
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
    if (delegate_ != nullptr) {
        return delegate_->set_active_tracks(a1);
    }
    return 0x14;
}
/*
Purpose: ?play_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x10c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A20 ?play_trackset@Midi@@QAEHI@Z 0x004C7A20-0x004C7A43 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->play_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?xpose_trackset@Midi@@QAEHIH@Z - forward 2 argument(s) to slot 0x114 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A50 ?xpose_trackset@Midi@@QAEHIH@Z 0x004C7A50-0x004C7A77 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->xpose_trackset(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?stop_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x110 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A80 ?stop_trackset@Midi@@QAEHI@Z 0x004C7A80-0x004C7AA3 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->stop_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z - forward 1 argument(s) to slot 0x98 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7AD0 ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z 0x004C7AD0-0x004C7AF3 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->add_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_lo@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B00 ?set_active_range_lo@Midi@@QAEHII@Z 0x004C7B00-0x004C7B27 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_active_range_lo(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_hi@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B30 ?set_active_range_hi@Midi@@QAEHII@Z 0x004C7B30-0x004C7B57 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_active_range_hi(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?remove_active_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0xa8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B60 ?remove_active_trackset@Midi@@QAEHI@Z 0x004C7B60-0x004C7B83 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->remove_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?get_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x9c of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7B90 ?get_trackset@Midi@@QAEHI@Z 0x004C7B90-0x004C7BB0 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->get_trackset(a1);
    }
    return 0;
}
/*
Purpose: ?get_ntracks@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7BB0 ?get_ntracks@Midi@@QAEHXZ 0x004C7BB0-0x004C7BC2 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->get_ntracks();
    }
    return 0;
}
/*
Purpose: ?load_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x120 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7BD0 ?load_patch@Midi@@QAEHI@Z 0x004C7BD0-0x004C7BF3 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->load_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?unload_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x124 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C00 ?unload_patch@Midi@@QAEHI@Z 0x004C7C00-0x004C7C23 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->unload_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_patch@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0x128 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C30 ?set_patch@Midi@@QAEHII@Z 0x004C7C30-0x004C7C57 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_patch(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_tempo@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x88 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7D50 ?set_tempo@Midi@@QAEHK@Z 0x004C7D50-0x004C7D73 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_tempo(a1);
    }
    return 0x14;
}
/*
Purpose: ?get_time@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xd4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7E80 ?get_time@Midi@@QAEHK@Z 0x004C7E80-0x004C7EA0 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->get_time(a1);
    }
    return 0;
}
/*
Purpose: ?get_control_track@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xf4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EA0 ?get_control_track@Midi@@QAEHXZ 0x004C7EA0-0x004C7EB2 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->get_control_track();
    }
    return 0;
}
/*
Purpose: ?get_total_track_ticks@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xf8 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EC0 ?get_total_track_ticks@Midi@@QAEHK@Z 0x004C7EC0-0x004C7EE0 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->get_total_track_ticks(a1);
    }
    return 0;
}
/*
Purpose: ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z - forward 1 argument(s) to slot 0xe4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7EE0 ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z 0x004C7EE0-0x004C7F03 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_channel_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z - forward 1 argument(s) to slot 0xe8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F10 ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z 0x004C7F10-0x004C7F33 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_program_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z - forward 1 argument(s) to slot 0xc0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F40 ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z 0x004C7F40-0x004C7F63 BYTE_EXACT
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
    if (delegate_ != nullptr) {
        return delegate_->set_track_ctrl(a1);
    }
    return 0x14;
}
