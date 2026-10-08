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
#include "palette.h"
#include "interface_palette.h"
#include "worldwin.h"
#include "basebutton.h"
#include "menu.h"
#include "win.h"
#include "base.h"
#include "map.h"
#include "game.h"
#include "alpha.h"
#include "log.h"
#include "console.h"


void __cdecl fn_005adbd0(ReplayWin *window);

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
// The short localized replay title at 0x00945DFC is a distinct four-byte
// writable object, immediately before ReplayLocalizedPrefix.
// RECOVERY_OBJECT: v2 0x00945DFC 0x00945E00 ?ReplayLocalizedTitle@@3PADA 0x005AD720
char ReplayLocalizedTitle[4];

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005AD720 ?init@ReplayWin@@QAEXXZ 0x005AD720-0x005AD99A
// RECOVERY_BINDING: v1 {"evidence": ["git:c4098269:src/recovered/units/005ad720.cpp", "src/replaywin.h owns GraphicWin, MapWin, FlatButton and Time at the original offsets", "src/console.h ConsoleGlobal cursor history occupies 0x009392B8/0x009392C0/0x00939340"], "reason": "Initialize the real replay window and its owned controls, using Console cursor history and typed shared styles instead of fixed addresses and vtable arithmetic; retain the seed for byte restoration."}
// symbol    ?init@ReplayWin@@QAEXXZ
void ReplayWin::init() {
    field_A14_ = 0;
    reset_base_sites();

    RECT rect = {0, 0, 0x320, 0x258};
    GraphicWin::init(&rect, 0, 0x1000000, 0, 0, 0);
    Win::center();

    const int zoom_x = (((rect.right - rect.left) * 2) /
                        MapLongitudeBounds << 4) / 100 - 0x10;
    const int zoom_y = (((rect.bottom - rect.top) * 2) /
                        MapLatitudeBounds << 4) / 50 - 0x10;
    const int zoom = zoom_x < zoom_y ? zoom_x : zoom_y;

    year_label_[0] = 0;
    strcat(year_label_, Language != 0 ? ReplayLocalizedTitle : "Replay: ");

    rect.left += 3;
    rect.bottom -= 0x17;
    rect.right -= 3;
    rect.bottom -= 3;
    rect.top += 0x1A;
    map_win_.init(&rect, 0, 0x20);
    map_win_.GraphicWin::init(&rect, 0, 0x200020, this, 0, 0);
    map_win_.field_1DD98_ = zoom;
    map_win_.field_1DD70_ = 0x85010080;
    const int cursor = ConsoleGlobal.cursor_history_index_;
    map_win_.field_1DD9C_ = ConsoleGlobal.cursor_history_x_[cursor];
    map_win_.field_1DDA0_ = ConsoleGlobal.cursor_history_y_[cursor];

    rect.top = rect.bottom + 3;
    rect.bottom = rect.top + 0x14;
    flat_button_.BaseButton::init(label_get(0xDB), -1, rect.left, rect.top,
                                  rect.right - rect.left,
                                  rect.bottom - rect.top, this, 0);
    flat_button_.set_text_color(0x41, -1, 1, 1);
    flat_button_.set_border_styles(&InterfaceBoxStyles[27],
                                    &InterfaceBoxStyles[29],
                                    &InterfaceBoxStyles[29]);
    flat_button_.Buffer::set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    flat_button_.redraw();
    map_win_.show(0);
    map_win_.field_1DD70_ |= 0x2000000;

    replay_turn_ = 0;
    event_offset_ = 0;
    timer_.set_modal();
    timer_.start(reinterpret_cast<void (__cdecl *)(int)>(&fn_005adbd0),
                 reinterpret_cast<int>(this), 0x64, 5);
    set_modal(0, 0, 0);
    timer_.release_modal();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005AD9A0 ?close@ReplayWin@@QAEXXZ 0x005AD9A0-0x005AD9E0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005ad9a0.cpp retains the original exact close seed", "src/replaywin.h owns the GraphicWin base and embedded MapWin", "src/mapwin.cpp MapWin::close releases owned_ and clears it"], "reason": "Close both real window subobjects through their owners, then preserve the original post-close safety check on the typed map cache pointer."}
// symbol    ?close@ReplayWin@@QAEXXZ
void ReplayWin::close() {
    GraphicWin::close();
    map_win_.close();
    if (map_win_.owned_ != 0) {
        free(map_win_.owned_);
        map_win_.owned_ = 0;
    }
}

// The original calls save_daemon with the bare autosave name, then reloads
// its .SAV path after the replay window closes. Both image strings are at
// 0x00691408 and 0x00691424 respectively.
static const char ReplaySaveName[] = "Alpha Centauri Autosave 1";
static const char ReplaySaveFile[] = "Alpha Centauri Autosave 1.SAV";

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005ADBF0 ?show_replay@@YAXXZ 0x005ADBF0-0x005ADE7B;0x004210D0-0x004210E7;0x00662240-0x00662355
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005adbf0.cpp retains the SHARED_TAIL seed and construction sequence", "image strings at 0x00691408 and 0x00691424 name the save and reload paths", "src/map.cpp owns WorldClimateSkipTerrainClear at 0x0068F21C", "src/game.cpp owns GameState at 0x009A64C0", "src/replaywin.h models the GraphicWin, MapWin, FlatButton and Time subobjects"], "reason": "Run the replay through a real local ReplayWin lifetime and typed game/climate state instead of a manually patched vtable and fixed executable addresses; retain the seed's unmodeled temporary objects as restoration work."}
// symbol    ?show_replay@@YAXXZ
void __cdecl show_replay() {
    ReplayWin replay;
    WorldClimateSkipTerrainClear = 2;
    save_daemon(const_cast<char *>(ReplaySaveName));
    GameDataStateStorage.fields_.game_state_ |= 0x80;

    replay.init();
    replay.close();

    do_all_draws();
    load_daemon(ReplaySaveFile, 0);
    flush_input();
    WorldClimateSkipTerrainClear = 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005ADF10 ??1ReplayWin@@QAE@XZ 0x005ADF10-0x005AE0A8;0x004210D0-0x004210E7;0x00662360-0x0066244C
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005ADBF0 constructs GraphicWin, MapWin, FlatButton and Time at offsets 0, 0x2320, 0x247A0 and 0x252EC", "image:0x005ADF32 destructs Time at 0x252EC before FlatButton at 0x247A0", "git:d7a40721:src/recovered/units/005adf10.cpp", "src/replaywin.h declares all three complete subobjects"], "reason": "Let C++ destroy ReplayWin's real members in reverse construction order instead of forwarding the complete destructor to original image code."}
// symbol    ??1ReplayWin@@UAE@XZ
ReplayWin::~ReplayWin() {
}

// ORIGINAL: 0x005AD610 ?on_redraw@ReplayWin@@QAEXXZ 0x005AD610-0x005AD71F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:d7a40721:src/recovered/units/005ad610.cpp", "image:0x005AD610-0x005AD71F", "src/replaywin.h map_win_ and year_label_", "src/buffer.h InterfaceBoxStyles[9] and [6]", "src/maininterface.h MainInterfaceGlobal.font1_ at original 0x007CFE8C", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005ad610.cpp"], "reason": "Paint the replay title through real GraphicWin/Buffer bases, shared style and font owners, then dispatch the embedded MapWin's virtual redraw; retain the exact seed for byte restoration. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_redraw@ReplayWin@@UAEXXZ
void ReplayWin::on_redraw() {
    GraphicWin::fill(9);
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT title = *buffer_working_rect();
    canvas->box_sprite(&title, &InterfaceBoxStyles[9]);

    title.left += 4;
    title.right -= 4;
    title.top += 4;
    title.bottom = title.top + 0x14;
    canvas->box_sprite(&title, &InterfaceBoxStyles[6]);

    title.left += 3;
    title.right -= 3;
    title.top += 3;
    title.bottom -= 3;
    canvas->set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
    canvas->set_text_color(TextColourTable[180], TextColourTable[181], 1, 1);
    canvas->write_cent_l(year_label_, &title, strlen(year_label_));

    make_bases();
    map_win_.on_redraw();
}

// ORIGINAL: 0x005AD470 sub_5ad470 0x005AD470-0x005AD4C0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:7ba7935c:src/recovered/units/005ad470.cpp", "src/replaywin.h ReplayWin base_sites_ at 0xA20", "0x005AD560 make_bases consumes the same 512 records", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005ad470.cpp"], "reason": "Own insertion in ReplayWin rather than an isolated raw-offset table class; retain the byte-exact seed for restoration. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
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

// ORIGINAL: 0x005AD500 sub_5ad500 0x005AD500-0x005AD560 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:7ba7935c:src/unrecovered/005ad500.cpp", "src/replaywin.h ReplayWin base_sites_ at 0xA20", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/005ad500.cpp"], "reason": "Recover keyed site invalidation on the real ReplayWin owner rather than a raw-offset table stand-in; retain the exact seed as restoration evidence. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005AD9E0 ?timer_callback@ReplayWin@@QAEXXZ 0x005AD9E0-0x005ADBCC
// RECOVERY_BINDING: v1 {"evidence": ["git:7f24e1c4:src/recovered/units/005ad9e0.cpp", "image:0x005AD9E0-0x005ADBCC", "src/replaywin.h replay_turn_, event_offset_, year_label_ and base_sites_", "src/win.h release_modal slot58 and redraw slot62"], "reason": "Consume typed replay events and update the ReplayWin-owned site table; replace the original-image forwarder without raw self-offsets or vtable shims. The retained seed remains a restoration input, not a product body."}
void ReplayWin::timer_callback() {
    if (replay_turn_ >= GameDataStateStorage.fields_.turn_current_num_) {
        if (replay_turn_ >= GameDataStateStorage.fields_.turn_current_num_ + 50) {
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005ADE80 ?replay_base@@YAXHHHH@Z 0x005ADE80-0x005ADF03
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x005ADE80-0x005ADF03 logs Replay Base and Replay Base Info, packs four arguments and TurnCurrentNum into eight bytes at 0x00950324 plus the byte counter at 0x009469F8, then adds eight", "git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005b3920.cpp clears 0x800 dwords of that buffer; ReplayWin timer_callback reads the same eight-byte records"], "reason": "Own replay-event storage and byte-length accounting in the replay subsystem, replacing fixed-address writes with a typed event record."}
void __cdecl replay_base(int type, int x, int y, int faction_id) {
    log_say("Replay Base", x, y, faction_id);
    log_say("Replay Base Info", GameDataStateStorage.fields_.turn_current_num_, type, ReplayEventBytes);
    ReplayEventRecord event;
    event.type = static_cast<uint8_t>(type);
    event.faction_id = static_cast<uint8_t>(faction_id);
    event.turn = static_cast<int16_t>(GameDataStateStorage.fields_.turn_current_num_);
    event.x = static_cast<int16_t>(x);
    event.y = static_cast<int16_t>(y);
    ReplayEvents[ReplayEventBytes / sizeof(ReplayEventRecord)] = event;
    ReplayEventBytes += sizeof(ReplayEventRecord);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005AD560 ?make_bases@ReplayWin@@QAEXXZ 0x005AD560-0x005AD607
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x005AD560-0x005AD607 clears Map::bit bit zero over MapArea, then walks 512 twelve-byte records at ReplayWin+0xA20", "The original writes Bases with 0x134 stride, storing x/y/faction/population before bit_set, owner_set and reset_territory; src/unrecovered/005ad560.cpp preserves the seed"], "reason": "Rebuild bases from typed replay-owned records into the real Base array and map, without self-offset arithmetic or fixed executable addresses."}
void ReplayWin::make_bases() {
    GameDataStateStorage.fields_.base_current_count_ = 0;
    for (int tile = 0; tile < static_cast<int>(MapArea); ++tile) {
        map_tiles()[tile].bit &= ~1u;
    }
    for (int site_id = 0; site_id < 512; ++site_id) {
        const ReplayBaseSite &site = base_sites_[site_id];
        if (site.x < 0) continue;
        Base &base = Bases[GameDataStateStorage.fields_.base_current_count_];
        base.x = static_cast<int16_t>(site.x);
        base.y = static_cast<int16_t>(site.y);
        base.population_size = 1;
        base.faction_id_current = static_cast<uint8_t>(site.faction_id);
        bit_set(base.x, base.y, 1, 1);
        owner_set(base.x, base.y, base.faction_id_current);
        ++GameDataStateStorage.fields_.base_current_count_;
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
// symbol    ?on_right_double_click@ReplayWin@@UAEXHH@Z
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
// symbol    ?on_mouse_move@ReplayWin@@UAEXHH@Z
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
// symbol    ?on_right_down@ReplayWin@@UAEXHH@Z
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
// symbol    ?on_left_down@ReplayWin@@UAEXHH@Z
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
// symbol    ?on_left_up@ReplayWin@@UAEXHH@Z
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

/*
Purpose: ?on_left_click@ReplayWin@@QAEXHH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x005AD3B0 ?on_left_click@ReplayWin@@QAEXHH@Z 0x005AD3B0-0x005AD3BB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066F8F0, pointer word at +0x4C, identifies original entry 0x005AD3B0", "Prior native snapshot: ?on_left_click_event@ReplayWin@@UAEXHH@Z was emitted for this slot before the source rename. The candidate now names the same Win slot-19 override on_left_click; its current emitted symbol still requires fresh COFF observation."], "reason": "The original own Win table binds this entry as virtual slot 19. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_left_click@ReplayWin@@UAEXHH@Z
// size      11 bytes
// prototype void (__thiscall ?on_left_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005AD3B2
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_left_click(int, int) {
  release_modal();
}

/*
Purpose: ?on_right_click@ReplayWin@@QAEXHH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x005AD3C0 ?on_right_click@ReplayWin@@QAEXHH@Z 0x005AD3C0-0x005AD3CB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066F8F0, pointer word at +0x58, identifies original entry 0x005AD3C0", "Prior native snapshot: ?vslot_22@ReplayWin@@UAEXHH@Z was emitted for this slot before the source rename. The candidate now names the same Win slot-22 override on_right_click; its current emitted symbol still requires fresh COFF observation."], "reason": "The original own Win table binds this entry as virtual slot 22. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_right_click@ReplayWin@@UAEXHH@Z
// size      11 bytes
// prototype void (__thiscall ?on_right_click@ReplayWin@@QAEXHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005AD3C2
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_right_click(int, int) {
  release_modal();
}

/*
Purpose: ?on_button_clicked@ReplayWin@@QAEXH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x005AD430 ?on_button_clicked@ReplayWin@@QAEXH@Z 0x005AD430-0x005AD43B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066F8F0, pointer word at +0xB0, identifies original entry 0x005AD430", "source:src/win.h inherited virtual interface; actual emitted method ?on_button_clicked@ReplayWin@@UAEXH@Z in primary Win table ??_7ReplayWin@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 44. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_button_clicked@ReplayWin@@UAEXH@Z
// size      11 bytes
// prototype void (__thiscall ?on_button_clicked@ReplayWin@@QAEXH@Z)(ReplayWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005AD432
Return Value: n/a
Status: Complete
*/
void ReplayWin::on_button_clicked(int) {
  release_modal();
}

/*
Purpose: ?on_key_click@ReplayWin@@QAEHHH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x005AD440 ?on_key_click@ReplayWin@@QAEHHH@Z 0x005AD440-0x005AD450 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066F8F0, pointer word at +0x64, identifies original entry 0x005AD440", "source:src/win.h inherited virtual interface; actual emitted method ?on_key_click@ReplayWin@@UAEHHH@Z in primary Win table ??_7ReplayWin@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 25. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_key_click@ReplayWin@@UAEHHH@Z
// size      16 bytes
// prototype int (__thiscall ?on_key_click@ReplayWin@@QAEHHH@Z)(ReplayWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005AD442
Return Value: 0x1, a constant
Status: Complete
*/
int ReplayWin::on_key_click(int, int) {
  release_modal();
  return 1;
}
// ORIGINAL: 0x005AE0E0 ??_GReplayWin@@WEEE@AEPAXI@Z 0x005AE0E0-0x005AE0EB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf.", "Pinned vftable slot 0: 0x0066F8E8 -> 0x005AE0E0.", "the ReplayWin Buffer-view table is 0x0066F8E8 at the GraphicWin Buffer subobject+0x444", "The original 11-byte adjustor corrects the earlier mistaken `??3ReplayWin@@SAXPAXI@Z` interpretation: it subtracts 0x444 from ECX and tail-transfers to primary scalar-deleting entry 0x005AE0B0; the corrected `WEEE` signature describes the compiler's adjusting-deleting entry.", "The historical disassembly note records that the primary path returns with ret 4, tests deletion flag bit 0, and invokes scalar operator delete only when that bit is set.", "The historical demangling note identifies adjustment 1092, equal to 0x444; the adjustor does not otherwise access the stack or change the receiver view.", "Source owner: src/replaywin.cpp; generated entry name is compiler-observed as ??_EReplayWin@@WEEE@AEPAXI@Z."], "reason": "Retain the original ReplayWin secondary-Buffer deleting-adjustor identity as a compiler-generated entry owned by the real ReplayWin destructor and GraphicWin hierarchy. The pinned secondary view at 0x0066F8E8, slot 0, targets 0x005AE0E0; this entry adjusts the Buffer receiver by 0x444 and reaches the primary scalar-deleting destructor, whose original path tests flag bit 0 before releasing storage. The current compiler-emitted symbol is recorded from the retained owner COFF, not handwritten."}
// symbol    ??_EReplayWin@@WEEE@AEPAXI@Z
// kind      game

// ORIGINAL: 0x005AE0B0 ??_GReplayWin@@UAEPAXI@Z 0x005AE0B0-0x005AE0D1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical source:src/deleting_thunks.cpp; preserved full original span 0x005AE0B0-0x005AE0D1 and BYTE_EXACT obligation.", "pinned ReplayWin primary table 0x0066F8F0 has slot 0 = 0x005AE0B0; the original primary entry is the exact scalar deleting identity.", "pinned scalar body calls ReplayWin::~ReplayWin at 0x005ADF10, tests flags bit 0, conditionally calls 0x0064557F, returns saved this with ret 4.", "the prior retained owner COFF/PE snapshot emitted ??_GReplayWin@@UAEPAXI@Z with destructor/delete relocations and its ReplayWin table selected that address; this is historical evidence only, not qualification of the current source.", "ReplayWin's source lifetime owns the typed GraphicWin, MapWin, FlatButton and Time subobjects; show_replay creates a real local ReplayWin. No remaining mapped scalar adjustment adapter calls this helper.", "the byte-exact claim is historical and remains a restoration obligation; no current equality or whole-application acceptance is asserted."], "reason": "Bind ReplayWin's original scalar deleting identity to the compiler-generated operation of its genuine virtual destructor; this FILE mapping names generated owner code, not the destructor source text."}
// symbol    ??_GReplayWin@@UAEPAXI@Z
// kind game

/*
Purpose: Fill 1,536 dwords with -1, starting at offset 0xa20.

             lea eax,[ecx+0xa24] / mov ecx,0x200 / or edx,0xffffffff
             mov [eax-4],edx / mov [eax],edx / mov [eax+4],edx
             add eax,0xc / dec ecx / jne / ret

         Three dwords per pass and 0x200 passes, so 1,536 in all - and the
         cursor starts at 0xa24 while the first store is at [eax-4], so the
         region actually begins at 0xa20 and runs to 0x261f. Reading the
         `lea` as the start would put the whole thing four bytes too high.


// ORIGINAL: 0x005AD450 sub_5ad450 0x005AD450-0x005AD470 BYTE_EXACT
// symbol    ?reset_base_sites@ReplayWin@@QAEXXZ
// size      32 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReplayWin::reset_base_sites() {
    for (int index = 0; index < 512; ++index) {
        base_sites_[index].x = -1;
        base_sites_[index].y = -1;
        base_sites_[index].faction_id = -1;
    }
}
