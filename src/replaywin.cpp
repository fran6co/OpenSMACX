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
#include "replaywin.h"
#include "worldwin.h"
#include "basebutton.h"
#include "menu.h"
#include "win.h"
#include "base.h"
#include "map.h"
#include "game.h"
#include "alpha.h"
#include "log.h"

// setup_game clears 0x800 dwords at 0x00950324 and timer_callback reads
// eight-byte records from the same interval. The counter advances in bytes.
// RECOVERY_OBJECT: v2 0x00950324 0x00952324 ?ReplayEvents@@3PAUReplayEventRecord@@A 0x005ADE80
ReplayEventRecord ReplayEvents[1024];
// RECOVERY_OBJECT: v2 0x009469F8 0x009469FC ?ReplayEventBytes@@3HA 0x005ADE80
int ReplayEventBytes;

// The language-specific replay prefix at 0x00945E00 is writable storage
// adjacent to the replay sound object, not an image-text pointer.
// RECOVERY_OBJECT: v2 0x00945E00 0x00945E08 ?ReplayLocalizedPrefix@@3PADA 0x005AD9E0
char ReplayLocalizedPrefix[8];

// ORIGINAL: 0x005AD470 sub_5ad470 0x005AD470-0x005AD4C0
// RECOVERY_BINDING: v1 {"evidence": ["git:7ba7935c:src/recovered/units/005ad470.cpp", "src/replaywin.h ReplayWin base_sites_ at 0xA20", "0x005AD560 make_bases consumes the same 512 records"], "reason": "Own insertion in ReplayWin rather than an isolated raw-offset table class; retain the byte-exact seed for restoration."}
// symbol    ?add_base_site@ReplayWin@@QAEXHHH@Z
void ReplayWin::add_base_site(int x, int y, int faction_id) {
    for (int index = 0; index < 512; ++index) {
        ReplayBaseSite &site = base_sites_[index];
        if (site.x < 0) {
            site.x = x;
            site.y = y;
            site.faction_id = faction_id;
            return;
        }
    }
}

// ORIGINAL: 0x005AD4C0 sub_5ad4c0 0x005AD4C0-0x005AD500
// RECOVERY_BINDING: v1 {"evidence": ["git:7ba7935c:src/leaf_recoveries.cpp", "src/replaywin.h ReplayWin base_sites_ at 0xA20"], "reason": "Recover keyed site update on the ReplayWin owner; the original returns three times the slot index on a hit, and 512 on a miss."}
// symbol    ?update_base_site@ReplayWin@@QAEHHHH@Z
int ReplayWin::update_base_site(int x, int y, int faction_id) {
    for (int index = 0; index < 512; ++index) {
        ReplayBaseSite &site = base_sites_[index];
        if (site.x == x && site.y == y) {
            site.faction_id = faction_id;
            return index * 3;
        }
    }
    return 512;
}

// ORIGINAL: 0x005AD500 sub_5ad500 0x005AD500-0x005AD560
// RECOVERY_BINDING: v1 {"evidence": ["git:7ba7935c:src/unrecovered/005ad500.cpp", "src/replaywin.h ReplayWin base_sites_ at 0xA20"], "reason": "Recover keyed site invalidation on the real ReplayWin owner rather than a raw-offset table stand-in; retain the exact seed as restoration evidence."}
// symbol    ?remove_base_site@ReplayWin@@QAEXHHH@Z
void ReplayWin::remove_base_site(int x, int y, int) {
    for (int index = 0; index < 512; ++index) {
        ReplayBaseSite &site = base_sites_[index];
        if (site.x == x && site.y == y) {
            site.faction_id = -1;
            site.y = -1;
            site.x = -1;
            return;
        }
    }
}

// ORIGINAL: 0x005AD9E0 ?timer_callback@ReplayWin@@QAEXXZ 0x005AD9E0-0x005ADBCC
// RECOVERY_BINDING: v1 {"evidence": ["git:7f24e1c4:src/recovered/units/005ad9e0.cpp", "image:0x005AD9E0-0x005ADBCC", "src/replaywin.h replay_turn_, event_offset_, year_label_ and base_sites_", "src/win.h release_modal slot58 and redraw slot62"], "reason": "Consume typed replay events and update the ReplayWin-owned site table; replace the original-image forwarder without raw self-offsets or vtable shims. The retained seed remains a restoration input, not a product body."}
void ReplayWin::timer_callback() {
    if (replay_turn_ >= TurnCurrentNum) {
        if (replay_turn_ >= TurnCurrentNum + 50) {
            release_modal();
        }
        return;
    }

    int next_turn = replay_turn_ + 1;
    while (event_offset_ < ReplayEventBytes) {
        const ReplayEventRecord event =
            ReplayEvents[event_offset_ / sizeof(ReplayEventRecord)];
        if (event.turn > replay_turn_) {
            next_turn = event.turn;
            break;
        }
        event_offset_ += sizeof(ReplayEventRecord);
        if (event.turn == replay_turn_) {
            switch (event.type) {
            case 0:
                add_base_site(event.x, event.y,
                              static_cast<int8_t>(event.faction_id));
                break;
            case 1:
                update_base_site(event.x, event.y,
                                 static_cast<int8_t>(event.faction_id));
                break;
            case 2:
                remove_base_site(event.x, event.y, 0);
                break;
            }
        }
    }

    year_label_[0] = 0;
    strcat(year_label_, Language != 0 ? ReplayLocalizedPrefix : "Replay: ");
    char year[80];
    _itoa_s(game_year(replay_turn_), year, sizeof(year), 10);
    strcat(year_label_, year);

    make_bases();
    redraw();
    replay_turn_ = next_turn;
}

// ORIGINAL: 0x005ADE80 ?replay_base@@YAXHHHH@Z 0x005ADE80-0x005ADF03
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x005ADE80-0x005ADF03 logs Replay Base and Replay Base Info, packs four arguments and TurnCurrentNum into eight bytes at 0x00950324 plus the byte counter at 0x009469F8, then adds eight", "src/unrecovered/005b3920.cpp clears 0x800 dwords of that buffer; ReplayWin timer_callback reads the same eight-byte records"], "reason": "Own replay-event storage and byte-length accounting in the replay subsystem, replacing fixed-address writes with a typed event record."}
void __cdecl replay_base(int type, int x, int y, int faction_id) {
    log_say("Replay Base", x, y, faction_id);
    log_say("Replay Base Info", TurnCurrentNum, type, ReplayEventBytes);
    ReplayEventRecord event;
    event.type = static_cast<uint8_t>(type);
    event.faction_id = static_cast<uint8_t>(faction_id);
    event.turn = static_cast<int16_t>(TurnCurrentNum);
    event.x = static_cast<int16_t>(x);
    event.y = static_cast<int16_t>(y);
    ReplayEvents[ReplayEventBytes / sizeof(ReplayEventRecord)] = event;
    ReplayEventBytes += sizeof(ReplayEventRecord);
}

// ORIGINAL: 0x005AD560 ?make_bases@ReplayWin@@QAEXXZ 0x005AD560-0x005AD607
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x005AD560-0x005AD607 clears Map::bit bit zero over MapArea, then walks 512 twelve-byte records at ReplayWin+0xA20", "The original writes Bases with 0x134 stride, storing x/y/faction/population before bit_set, owner_set and reset_territory; src/unrecovered/005ad560.cpp preserves the seed"], "reason": "Rebuild bases from typed replay-owned records into the real Base array and map, without self-offset arithmetic or fixed executable addresses."}
void ReplayWin::make_bases() {
    BaseCurrentCount = 0;
    for (int tile = 0; tile < static_cast<int>(MapArea); ++tile) {
        map_tiles()[tile].bit &= ~1u;
    }
    for (int site_id = 0; site_id < 512; ++site_id) {
        const ReplayBaseSite &site = base_sites_[site_id];
        if (site.x < 0) continue;
        Base &base = Bases[BaseCurrentCount];
        base.x = static_cast<int16_t>(site.x);
        base.y = static_cast<int16_t>(site.y);
        base.population_size = 1;
        base.faction_id_current = static_cast<uint8_t>(site.faction_id);
        bit_set(base.x, base.y, 1, 1);
        owner_set(base.x, base.y, base.faction_id_current);
        ++BaseCurrentCount;
    }
    reset_territory();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD3D0 ?on_left_double_click@ReplayWin@@QAEXHH@Z 0x005AD3D0-0x005AD3D3 BYTE_EXACT
// symbol    ?on_left_double_click@ReplayWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_double_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD3E0 ?on_right_double_click@ReplayWin@@QAEXHH@Z 0x005AD3E0-0x005AD3E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_right_double_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD3F0 ?on_mouse_move@ReplayWin@@QAEXHH@Z 0x005AD3F0-0x005AD3F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD400 ?on_right_down@ReplayWin@@QAEXHH@Z 0x005AD400-0x005AD403 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_right_down@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD410 ?on_left_down@ReplayWin@@QAEXHH@Z 0x005AD410-0x005AD413 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_down@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x005AD420 ?on_left_up@ReplayWin@@QAEXHH@Z 0x005AD420-0x005AD423 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_up@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_up(int, int) {
}







/*
// ORIGINAL: 0x005ADBD0 ?timer_callback_daemon@ReplayWin@@QAAXH@Z 0x005ADBD0-0x005ADBE1 BYTE_EXACT
// symbol    ?fn_005adbd0@@YAXPAVReplayWin@@@Z
// size      17 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005AD9E0
Status: Complete
*/
void __cdecl fn_005adbd0(ReplayWin* a1) {
    if (a1) {
        a1->timer_callback();
    }
}
