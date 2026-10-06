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
#pragma once
#include "spot.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "stringbox.h"
#include "editbox.h"
#include "checkbox.h"
#include <cstddef>

// One original zero-filled .data object at 0x0090DB98. The second row begins
// at 0x0090DD14; its seven-row suffix is the 0xA64-byte state payload.
// The leading row's tail dword is the owner scanned via 0x0090DD10.
struct NetWinLobbyRow {
  uint8_t prefix_;                  // +0
  uint8_t ready_or_occupied_;       // +1
  uint8_t field_2_;                 // +2
  int8_t faction_or_color_;         // +3
  uint8_t field_4_;                 // +4
  char leader_name_[0x18];         // +5
  char formal_faction_name_[0x28]; // +0x1D
  char noun_faction_[0x18];        // +0x45
  char adjective_faction_[0x80];   // +0x5D
  char leader_title_[0x18];        // +0xDD
  char adjective_leader_[0x80];    // +0xF5
  uint8_t tail_[3];                // +0x175..+0x177
  uint32_t owner_;                  // +178

  // Native 0x483820 reset, rehomed from the former NetWin::UNK5 owner.
  void initialize_empty_slot();
  // Typed callback adapter for the generic strided helper; the existing
  // row initializer remains the sole owner of the row's field writes.
  static void __fastcall initialize_empty_slot_callback(void *element) {
    static_cast<NetWinLobbyRow *>(element)->initialize_empty_slot();
  }
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "lobby row stride");
static_assert(offsetof(NetWinLobbyRow, leader_name_) == 5,
              "lobby leader-name offset");
static_assert(offsetof(NetWinLobbyRow, formal_faction_name_) == 0x1D,
              "lobby formal-name offset");
static_assert(offsetof(NetWinLobbyRow, noun_faction_) == 0x45,
              "lobby faction-noun offset");
static_assert(offsetof(NetWinLobbyRow, adjective_faction_) == 0x5D,
              "lobby faction-adjective offset");
static_assert(offsetof(NetWinLobbyRow, leader_title_) == 0xDD,
              "lobby leader-title offset");
static_assert(offsetof(NetWinLobbyRow, adjective_leader_) == 0xF5,
              "lobby leader-adjective offset");
static_assert(offsetof(NetWinLobbyRow, owner_) == 0x178, "lobby owner slot");
extern NetWinLobbyRow NetWinLobbyRows[8]; // 0x0090DB98
// send_session2_packet copies this whole 0x168-byte block; the first dword
// is the mode tested by the setup dialog.
struct NetWinSessionBlock {
  uint32_t mode_;
  uint8_t state_[0x164];

  // The native missing-player tail addresses lobby row 8, which begins
  // inside this live session object. These views do not create a ninth row.
  void write_row8_prefix(uint8_t value);
  char *row8_leader_name();
  char *row8_noun_faction();
};
static_assert(sizeof(NetWinSessionBlock) == 0x168, "session block extent");
static_assert(offsetof(NetWinLobbyRow, prefix_) < sizeof(NetWinSessionBlock),
              "row-eight prefix is inside session storage");
static_assert(offsetof(NetWinLobbyRow, leader_name_) +
                  sizeof(((NetWinLobbyRow *)0)->leader_name_) <= sizeof(NetWinSessionBlock),
              "row-eight leader text is inside session storage");
static_assert(offsetof(NetWinLobbyRow, noun_faction_) +
                  sizeof(((NetWinLobbyRow *)0)->noun_faction_) <= sizeof(NetWinSessionBlock),
              "row-eight faction text is inside session storage");
extern NetWinSessionBlock NetWinSessionData; // 0x0090E778, after the rows
struct NetWinSessionOptionsState {
  uint8_t status_;          // +0
  uint8_t player_count_;    // +1, pick_num_players stores answer - 1
  uint8_t difficulty_;      // +2
  uint8_t time_control_;    // +3
  uint8_t option_4_;        // +4
  uint8_t option_5_;        // +5
  uint8_t planet_size_;     // +6
  uint8_t ocean_coverage_;  // +7
  uint8_t erosion_;         // +8
  uint8_t native_life_;     // +9
  uint8_t cloud_cover_;     // +0xA
  uint8_t reserved_B_;      // +0xB
  uint32_t rule_bits_a_; // +0xC, 0x0090E8EC
  uint32_t rule_bits_b_; // +0x10, 0x0090E8F0
  uint32_t rule_bits_c_; // +0x14, 0x0090E8F4
};
static_assert(sizeof(NetWinSessionOptionsState) == 0x18,
              "session option extent");
static_assert(offsetof(NetWinSessionOptionsState, rule_bits_a_) == 0xC,
              "first session rule word");
extern NetWinSessionOptionsState NetWinSessionOptions; // 0x0090E8E0

// Setup synchronization shared with NetDaemon's added-player callback.
void __cdecl send_all_player_packets(int target);
void __cdecl send_session_packet(int slot, int target);
void __cdecl send_session2_packet(int slot, int target);
void __cdecl send_faction_data(int slot);

 /*
  * MessageFactionData
  *
  * The record setup_faction_data (0x00481FD0) clones whole into the
  * 0x772C block. Only three things about it are proven, all from that one
  * body: the record block it copies starts at 0x10 (`memcpy` from
  * message + 0x10), the block's own LEADING byte is the record type
  * (setup_faction_data writes 7 there when it rebuilds the block itself),
  * and the block's SECOND byte is the record count, read as a byte and
  * floored at 8. Nothing pins its sizeof.
  */
struct MessageFactionData {
  uint8_t field_0_[0x10];  // 0x00..0x0F - nothing recovered reads these
  uint8_t record_type_;    // 0x10
  uint8_t record_count_;   // 0x11
  uint8_t field_12_[1];    // 0x12 - the rest of the cloned block
};

// The heap clone advances by 0x190 bytes per faction. Its first record also
// carries the table type/count; subsequent records use the same prefix for
// their own data. setup_faction_data writes +4, +8 and +9, while
// get_player_color reads the first count and each record's colour byte.
struct NetWinFactionRecord {
  uint8_t record_type_;      // +0; table type in record zero
  uint8_t record_count_;     // +1; table count in record zero
  uint8_t field_2_[2];      // +2..+3
  uint32_t owner_;           // +4
  uint8_t color_;            // +8
  uint8_t active_;           // +9
  char filename_[0x18]; // +0xA..+0x21, text section/file id
  char leader_name_[0x18]; // +0x22..+0x39, copied by setup_faction_data
  char formal_faction_name_[0x28]; // +0x3A..+0x61, copied by setup_faction_data
  char noun_faction_[0x18]; // +0x62..+0x79, copied from Player::noun_faction
  uint8_t data_after_noun_[0x116]; // +0x7A..+0x18F
};
static_assert(sizeof(NetWinFactionRecord) == 0x190,
              "faction record stride");
static_assert(offsetof(NetWinFactionRecord, owner_) == 4,
              "faction owner offset");
static_assert(offsetof(NetWinFactionRecord, color_) == 8,
              "faction colour offset");
static_assert(offsetof(NetWinFactionRecord, filename_) == 0xA,
              "faction filename offset");
static_assert(offsetof(NetWinFactionRecord, leader_name_) == 0x22,
              "faction leader name offset");
static_assert(offsetof(NetWinFactionRecord, formal_faction_name_) == 0x3A,
              "faction formal name offset");
static_assert(offsetof(NetWinFactionRecord, noun_faction_) == 0x62,
              "faction noun offset");
static_assert(sizeof(((NetWinFactionRecord *)0)->noun_faction_) == 0x18,
              "faction noun extent");

// faction.h's per-faction record; fill_player writes the name/title text
// block of one. Opaque here - only the pointer travels.
struct PlayerData;

 /*
  * NetWin class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The layout is not established. The constructor's own fields begin at 0xA14,
  * immediately after the base, and where the object ends is unknown, so
  * nothing pins its sizeof and the fields between are not modelled. The
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
class NetWin : public ConstructedGraphicWin {
 public:
  NetWin();
  // 0x004834E0 explicit cleanup recovered; nested member/base teardown is
  // compiler-owned and no whole-object extent is asserted.
  ~NetWin();
  void UNK1();
  void UNK2();
  void alloc_slots();
  void on_button_clicked(int clicked_id); // 0x0047B450
  void on_left_click(int x, int y) override; // 0x0047B760
  int on_key_click(int a, int b);
  void on_mouse_move(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  // 0x0047B380. Resolves the colour byte a faction list draws its player
  // rows with: the static table's entry for the faction, or the first index
  // no earlier faction's record and no other player's copy claims.
  int get_player_color(int faction);
  // 0x00481D10. Drops the two faction-record blocks, resets the dialog words
  // at 0x761C/0x7620, and closes the embedded controls.
  void close();
  // 0x00483190. The modal pump's callback: drains the daemon's receive queue,
  // pokes the dialog's own slot-12 virtual, and answers whether the wait
  // should end (0) or keep going (1).
  int modal_callback();
  // 0x004832D0. The chat edit's string callback: repaints the message list's
  // text colour from the faction colour of the id handed in.
  static void __cdecl string_callback(int payload, int);
  // 0x00483310. Enter sends the chat line, Escape tears the session dialogs
  // down, everything else goes to the edit box's own key handling.
  int on_key_down(int key);
  // 0x00483420. Formats one chat line - upper-cased sender name, ": ", a
  // space, the text - and hands it to the message list.
  void new_chat(char *message, int slot);
  // 0x0047FE80. Paints the three dialog panel sprites through the Buffer at
  // +0x444.
  void UNK3();
  // 0x004837F0. Destroys the CheckBox view at +0x1C, the Dialog at +0xA34,
  // and the GraphicWin at +0x1C - the CheckBox vbtable's own two displacements.
  void UNK4();
  // 0x00481DA0. Copies one faction record's name/title block into a
  // PlayerData, or just clears +0x45 and re-reads it for slot -1.
  void fill_player(PlayerData *player_data, int slot);
  // 0x00481E90. Seeds the seven base faction records of the static faction
  // table from the 0x59C-stride template records, and resets the trailing
  // eighth/ninth slot words.
  void config_players();
  // 0x00482D90. Completes the chosen multiplayer roster into faction/player
  // state and the existing AlphaNet process-slot records.
  void prepare_game();
  int exec(); // 0x00482490
  // 0x00481AD0. Repaints the dialog: the panel sprites, the centred session
  // title, and the five content groups - draw_custom_rules, draw_player_names,
  // draw_diffs, draw_factions, draw_session_info - then soft_update.
  void on_redraw();
  // 0x0047F960. The custom-rules panel of the redraw.
  void draw_custom_rules();
  // 0x00480370. The player-name rows of the redraw.
  void draw_player_names();
  // 0x00480180. The difficulty rows of the redraw.
  void draw_diffs();
  // 0x0047FED0. The faction rows of the redraw.
  void draw_factions();
  // 0x00480900. The eight session-info rows of the redraw.
  void draw_session_info();
  // 0x00483240. The fixed dialog's twin of modal_callback (the catalogue's
  // QAAHX prefix is public, like every other NetWin member mangle).
  static int __cdecl modal_callback_daemon();

  // Storage the image proves is here: its own methods reach 0x7730.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 206 member(s) from the IDA database, 6 named; it starts a member at 0xA14, which is where src/ ends.

  RECT setup_rects_[50];  // 0xA14..0xD33, native setup/painter rectangle span
  Spot spot_;  // 0xD34
  StringBox stringBox_;  // 0xD40, size == sizeof(StringBox)
  EditBox editBox_;  // 0x38E0, size == sizeof(EditBox)
  CheckBox checkBox_;  // 0x4454, declared CheckBox extent == 0xB28
  FlatButton flatButton1_;  // 0x4F7C
  FlatButton flatButton2_;  // 0x5AC8
  char world_size_labels_[32][0x80];  // 0x6614..0x7613
  uint32_t field_7614_;  // 0x7614
  uint32_t field_7618_;  // 0x7618
  uint32_t field_761C_;  // 0x761C
  uint32_t field_7620_;  // 0x7620
  uint8_t field_7624_[0x104];  // 0x7624
  uint32_t field_7728_;  // 0x7728
  // 0x772C and 0x7730 are the two faction-record blocks setup_faction_data
  // (0x00481FD0) allocates with mem_get and frees again: the second is a
  // memcpy of the first, and every reader in the image re-loads the slot it
  // wants rather than caching it. Renamed from field_772C_/field_7730_ on
  // that body's evidence - they were the only two unnamed dwords the body
  // touched, and both were 4 bytes, so the extent is unchanged.
  void *faction_records_;        // 0x772C
  void *faction_records_copy_;   // 0x7730

  // 0x00481FD0. Rebuilds the two faction-record blocks at 0x772C/0x7730 -
  // one cloned from the network message the caller hands in, one rebuilt
  // from the localized faction text table. `message_data` is what hands the
  // daemon's received record here.
  void setup_faction_data(MessageFactionData *message_faction_data);
  // 0x0047C970. The password prompt the setup dialog ends with; 1 is what
  // process_message's 0xF06 case passes it after is_visible agrees.
  int pick_pass(int payload);
  void pick_player();
  void pick_faction(int slot);
  void pick_diff(int slot);
  void pick_map();
  // The eight setup pickers: one popup each, the answer stored in the ninth
  // slot's byte and announced to the peer in a 0x2F02 packet.
  void pick_num_players();      // 0x0047E010
  void pick_global_diff();      // 0x0047E340
  void pick_time();             // 0x0047E640
  void pick_planet_size();      // 0x0047E930
  void pick_ocean();            // 0x0047EC60
  void pick_erosion();          // 0x0047EF90
  void pick_native();           // 0x0047F2C0
  void pick_clouds();           // 0x0047F5F0
};

// THE MULTIPLAYER SETUP DIALOG - the single NetWin instance in the image, at
// 0x0080A6F8, REAL STORAGE defined in netwin.cpp. process_message
// (0x00534400) dispatches into it by loading that address into ecx for
// setup_faction_data / Win::is_visible / pick_pass, and so do string_callback,
// hang_up, new_game_setup and the rest of the 21 sites in the image that read
// it - `mov ecx, 0x80a6f8` at every one, always unrelated to the receiver the
// enclosing function was handed (see docs/recovery/
// agent-structure-observations.csv, 0x004832D0). It was a raw
// `(NetWin *)0x0080A6F8` binding reading terranx.exe data that is unmapped in
// a standalone build; `->` becomes `.` and the folded `mov ecx, imm32` keeps
// its form with the displacement relocated, exactly as NetDaemonNet does.
extern NetWin NetWinDialog;  // 0x0080A6F8

// THE TWO PLAYER-ID SLOTS the setup dialog reads. No store in .text writes
// either one - the NetWin code at 0x0047B020..0x00482500 reads them 36 and 30
// times and never writes them, so they are filled in from outside this image.
// What names them as ids: setup_faction_data (0x00481FD0) returns early when
// they are equal, and pick_pass (0x0047C970) hands 0x0093D4F0 to
// AlphaNet::pid_2_idx, which walks the eight per-slot records looking for it.
// Transport configuration used by the setup dialog.
extern int NetWinPlayerId;       // 0x0093D4F0, defined in netwin.cpp
extern int NetWinOtherPlayerId;  // 0x0093D4F4, defined in netwin.cpp
