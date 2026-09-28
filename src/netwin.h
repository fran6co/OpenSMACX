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
  uint8_t record_[0x174];          // +4..+177, text and settings
  uint32_t owner_;                  // +178
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "lobby row stride");
static_assert(offsetof(NetWinLobbyRow, owner_) == 0x178, "lobby owner slot");
extern NetWinLobbyRow NetWinLobbyRows[8]; // 0x0090DB98
// send_session2_packet copies this whole 0x168-byte block; the first dword
// is the mode tested by the setup dialog.
struct NetWinSessionBlock {
  uint32_t mode_;
  uint8_t state_[0x164];
};
static_assert(sizeof(NetWinSessionBlock) == 0x168, "session block extent");
extern NetWinSessionBlock NetWinSessionData; // 0x0090E778, after the rows
struct NetWinSessionOptionsState {
  uint8_t prefix_[0xC];
  uint32_t rule_bits_a_; // +0xC, 0x0090E8EC
  uint32_t rule_bits_b_; // +0x10, 0x0090E8F0
  uint32_t rule_bits_c_; // +0x14, 0x0090E8F4
};
static_assert(sizeof(NetWinSessionOptionsState) == 0x18,
              "session option extent");
static_assert(offsetof(NetWinSessionOptionsState, rule_bits_a_) == 0xC,
              "first session rule word");
extern NetWinSessionOptionsState NetWinSessionOptions; // 0x0090E8E0

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
  uint8_t data_[0x186];     // +0xA..+0x18F
};
static_assert(sizeof(NetWinFactionRecord) == 0x190,
              "faction record stride");
static_assert(offsetof(NetWinFactionRecord, owner_) == 4,
              "faction owner offset");
static_assert(offsetof(NetWinFactionRecord, color_) == 8,
              "faction colour offset");

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
  // 0x004834E0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~NetWin();
  void UNK5();
  void UNK1();
  void UNK2();
  void alloc_slots();
  void on_button_clicked(int clicked_id); // 0x0047B450
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
  void string_callback(int payload, int);
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
  int modal_callback_daemon();

  // Storage the image proves is here: its own methods reach 0x7730.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 206 member(s) from the IDA database, 6 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  uint32_t field_A48_;  // 0xA48
  uint32_t field_A4C_;  // 0xA4C
  uint32_t field_A50_;  // 0xA50
  uint32_t field_A54_;  // 0xA54
  uint32_t field_A58_;  // 0xA58
  uint32_t field_A5C_;  // 0xA5C
  uint32_t field_A60_;  // 0xA60
  uint32_t field_A64_;  // 0xA64
  uint32_t field_A68_;  // 0xA68
  uint32_t field_A6C_;  // 0xA6C
  uint32_t field_A70_;  // 0xA70
  uint32_t field_A74_;  // 0xA74
  uint32_t field_A78_;  // 0xA78
  uint32_t field_A7C_;  // 0xA7C
  uint32_t field_A80_;  // 0xA80
  uint32_t field_A84_;  // 0xA84
  uint32_t field_A88_;  // 0xA88
  uint32_t field_A8C_;  // 0xA8C
  uint32_t field_A90_;  // 0xA90
  uint32_t field_A94_;  // 0xA94
  uint32_t field_A98_;  // 0xA98
  uint32_t field_A9C_;  // 0xA9C
  uint32_t field_AA0_;  // 0xAA0
  uint32_t field_AA4_;  // 0xAA4
  uint32_t field_AA8_;  // 0xAA8
  uint32_t field_AAC_;  // 0xAAC
  uint32_t field_AB0_;  // 0xAB0
  uint32_t field_AB4_;  // 0xAB4
  uint32_t field_AB8_;  // 0xAB8
  uint32_t field_ABC_;  // 0xABC
  uint32_t field_AC0_;  // 0xAC0
  uint32_t field_AC4_;  // 0xAC4
  uint32_t field_AC8_;  // 0xAC8
  uint32_t field_ACC_;  // 0xACC
  uint32_t field_AD0_;  // 0xAD0
  uint32_t field_AD4_;  // 0xAD4
  uint32_t field_AD8_;  // 0xAD8
  uint32_t field_ADC_;  // 0xADC
  uint32_t field_AE0_;  // 0xAE0
  uint32_t field_AE4_;  // 0xAE4
  uint32_t field_AE8_;  // 0xAE8
  uint32_t field_AEC_;  // 0xAEC
  uint32_t field_AF0_;  // 0xAF0
  uint32_t field_AF4_;  // 0xAF4
  uint32_t field_AF8_;  // 0xAF8
  uint32_t field_AFC_;  // 0xAFC
  uint32_t field_B00_;  // 0xB00
  uint32_t field_B04_;  // 0xB04
  uint32_t field_B08_;  // 0xB08
  uint32_t field_B0C_;  // 0xB0C
  uint32_t field_B10_;  // 0xB10
  uint32_t field_B14_;  // 0xB14
  uint32_t field_B18_;  // 0xB18
  uint32_t field_B1C_;  // 0xB1C
  uint32_t field_B20_;  // 0xB20
  uint32_t field_B24_;  // 0xB24
  uint32_t field_B28_;  // 0xB28
  uint32_t field_B2C_;  // 0xB2C
  uint32_t field_B30_;  // 0xB30
  uint32_t field_B34_;  // 0xB34
  uint32_t field_B38_;  // 0xB38
  uint32_t field_B3C_;  // 0xB3C
  uint32_t field_B40_;  // 0xB40
  uint32_t field_B44_;  // 0xB44
  uint32_t field_B48_;  // 0xB48
  uint32_t field_B4C_;  // 0xB4C
  uint32_t field_B50_;  // 0xB50
  uint32_t field_B54_;  // 0xB54
  uint32_t field_B58_;  // 0xB58
  uint32_t field_B5C_;  // 0xB5C
  uint32_t field_B60_;  // 0xB60
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  uint32_t field_B74_;  // 0xB74
  uint32_t field_B78_;  // 0xB78
  uint32_t field_B7C_;  // 0xB7C
  uint32_t field_B80_;  // 0xB80
  uint32_t field_B84_;  // 0xB84
  uint32_t field_B88_;  // 0xB88
  uint32_t field_B8C_;  // 0xB8C
  uint32_t field_B90_;  // 0xB90
  uint32_t field_B94_;  // 0xB94
  uint32_t field_B98_;  // 0xB98
  uint32_t field_B9C_;  // 0xB9C
  uint32_t field_BA0_;  // 0xBA0
  uint32_t field_BA4_;  // 0xBA4
  uint32_t field_BA8_;  // 0xBA8
  uint32_t field_BAC_;  // 0xBAC
  uint32_t field_BB0_;  // 0xBB0
  uint32_t field_BB4_;  // 0xBB4
  uint32_t field_BB8_;  // 0xBB8
  uint32_t field_BBC_;  // 0xBBC
  uint32_t field_BC0_;  // 0xBC0
  uint32_t field_BC4_;  // 0xBC4
  uint32_t field_BC8_;  // 0xBC8
  uint32_t field_BCC_;  // 0xBCC
  uint32_t field_BD0_;  // 0xBD0
  uint32_t field_BD4_;  // 0xBD4
  uint32_t field_BD8_;  // 0xBD8
  uint32_t field_BDC_;  // 0xBDC
  uint32_t field_BE0_;  // 0xBE0
  uint32_t field_BE4_;  // 0xBE4
  uint32_t field_BE8_;  // 0xBE8
  uint32_t field_BEC_;  // 0xBEC
  uint32_t field_BF0_;  // 0xBF0
  uint32_t field_BF4_;  // 0xBF4
  uint32_t field_BF8_;  // 0xBF8
  uint32_t field_BFC_;  // 0xBFC
  uint32_t field_C00_;  // 0xC00
  uint32_t field_C04_;  // 0xC04
  uint32_t field_C08_;  // 0xC08
  uint32_t field_C0C_;  // 0xC0C
  uint32_t field_C10_;  // 0xC10
  uint32_t field_C14_;  // 0xC14
  uint32_t field_C18_;  // 0xC18
  uint32_t field_C1C_;  // 0xC1C
  uint32_t field_C20_;  // 0xC20
  uint32_t field_C24_;  // 0xC24
  uint32_t field_C28_;  // 0xC28
  uint32_t field_C2C_;  // 0xC2C
  uint32_t field_C30_;  // 0xC30
  uint32_t field_C34_;  // 0xC34
  uint32_t field_C38_;  // 0xC38
  uint32_t field_C3C_;  // 0xC3C
  uint32_t field_C40_;  // 0xC40
  uint32_t field_C44_;  // 0xC44
  uint32_t field_C48_;  // 0xC48
  uint32_t field_C4C_;  // 0xC4C
  uint32_t field_C50_;  // 0xC50
  uint32_t field_C54_;  // 0xC54
  uint32_t field_C58_;  // 0xC58
  uint32_t field_C5C_;  // 0xC5C
  uint32_t field_C60_;  // 0xC60
  uint32_t field_C64_;  // 0xC64
  uint32_t field_C68_;  // 0xC68
  uint32_t field_C6C_;  // 0xC6C
  uint32_t field_C70_;  // 0xC70
  uint32_t field_C74_;  // 0xC74
  uint32_t field_C78_;  // 0xC78
  uint32_t field_C7C_;  // 0xC7C
  uint32_t field_C80_;  // 0xC80
  uint32_t field_C84_;  // 0xC84
  uint32_t field_C88_;  // 0xC88
  uint32_t field_C8C_;  // 0xC8C
  uint32_t field_C90_;  // 0xC90
  uint32_t field_C94_;  // 0xC94
  uint32_t field_C98_;  // 0xC98
  uint32_t field_C9C_;  // 0xC9C
  uint32_t field_CA0_;  // 0xCA0
  uint32_t field_CA4_;  // 0xCA4
  uint32_t field_CA8_;  // 0xCA8
  uint32_t field_CAC_;  // 0xCAC
  uint32_t field_CB0_;  // 0xCB0
  uint32_t field_CB4_;  // 0xCB4
  uint32_t field_CB8_;  // 0xCB8
  uint32_t field_CBC_;  // 0xCBC
  uint32_t field_CC0_;  // 0xCC0
  uint32_t field_CC4_;  // 0xCC4
  uint32_t field_CC8_;  // 0xCC8
  uint32_t field_CCC_;  // 0xCCC
  uint32_t field_CD0_;  // 0xCD0
  uint32_t field_CD4_;  // 0xCD4
  uint32_t field_CD8_;  // 0xCD8
  uint32_t field_CDC_;  // 0xCDC
  uint32_t field_CE0_;  // 0xCE0
  uint32_t field_CE4_;  // 0xCE4
  uint32_t field_CE8_;  // 0xCE8
  uint32_t field_CEC_;  // 0xCEC
  uint32_t field_CF0_;  // 0xCF0
  uint32_t field_CF4_;  // 0xCF4
  uint32_t field_CF8_;  // 0xCF8
  uint32_t field_CFC_;  // 0xCFC
  uint32_t field_D00_;  // 0xD00
  uint32_t field_D04_;  // 0xD04
  uint32_t field_D08_;  // 0xD08
  uint32_t field_D0C_;  // 0xD0C
  uint32_t field_D10_;  // 0xD10
  uint32_t field_D14_;  // 0xD14
  uint32_t field_D18_;  // 0xD18
  uint32_t field_D1C_;  // 0xD1C
  uint32_t field_D20_;  // 0xD20
  uint32_t field_D24_;  // 0xD24
  uint32_t field_D28_;  // 0xD28
  uint32_t field_D2C_;  // 0xD2C
  uint32_t field_D30_;  // 0xD30
  Spot spot_;  // 0xD34
  StringBox stringBox_;  // 0xD40, size == sizeof(StringBox)
  EditBox editBox_;  // 0x38E0, size == sizeof(EditBox)
  CheckBox checkBox_;  // 0x4454, declared CheckBox extent == 0xB28
  FlatButton flatButton1_;  // 0x4F7C
  FlatButton flatButton2_;  // 0x5AC8
  uint8_t field_6614_[0x1000];  // 0x6614
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
