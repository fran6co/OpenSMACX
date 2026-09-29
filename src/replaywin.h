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

#include "original_seam.h"
#include "graphicwin.h"
#include "mapwin.h"
#include "flatbutton.h"
#include "time.h"

 /*
  * ReplayWin class
  *
  * Derives from GraphicWin by ordinary single inheritance.
  *
  * This header used to say "no constructor survives in the catalog, so the
  * base cannot be checked". The first clause is still true - there is no
  * ??0ReplayWin@@QAE@XZ in the image, and tools/disasm.py rejects the name -
  * but the conclusion was wrong, and the destructor withdraws it. A
  * destructor's receiver is the object it destroys, and the base subobject is
  * torn down last on an UNADJUSTED this: ??1ReplayWin@@QAE@XZ (0x005ADF10)
  * takes `mov edi, ecx` at 0x005ADF2D, never adjusts edi again, and closes
  * with `mov ecx, edi` at 0x005AE08A / `call ??1GraphicWin@@QAE@XZ` at
  * 0x005AE093. ?close@ReplayWin@@QAEXXZ (0x005AD9A0) agrees independently:
  * `mov esi, ecx` then `call ?close@GraphicWin@@QAEXXZ` at 0x005AD9A3, with
  * ecx still holding this and no lea in between.
  *
  * The base is non-virtual - neither site reads a vbtable to reach it, unlike
  * the MapWin at 0x2320, whose teardown goes through `mov eax, [esi-0x21a6c]`
  * / `[eax+4]` - so it sits at offset 0 and covers [0, 0xA14), which is
  * sizeof(GraphicWin) as this tree declares it. The seven opaque members that
  * spanned exactly those bytes (field_0_ .. field_484_[0x590], ending at
  * 0x484+0x590 == 0xA14) are gone.
  *
  * The 512 replay base sites at 0xA20..0x2220, the MapWin at 0x2320,
  * FlatButton at 0x247A0 and Time at 0x252EC are now typed. The latter
  * ends at 0x25314: show_replay constructs it at EBP-0x38 and the original
  * destructor tears it down before the other two. The earlier 0x252F0
  * size assertion ended inside that Time object and is withdrawn.
  */
struct ReplayBaseSite {
  int x;
  int y;
  int faction_id;
};
static_assert(sizeof(ReplayBaseSite) == 0xC,
              "Replay base site must keep the original twelve-byte stride");

struct ReplayEventRecord {
  uint8_t type;
  uint8_t faction_id;
  int16_t turn;
  int16_t x;
  int16_t y;
};
static_assert(sizeof(ReplayEventRecord) == 8,
              "Replay events must keep the original eight-byte stride");

extern ReplayEventRecord ReplayEvents[1024];
extern int ReplayEventBytes;
void __cdecl replay_base(int type, int x, int y, int faction_id);

class ReplayWin : public GraphicWin {
 public:
  void timer_callback();
  void init();
  void close();
  ReplayWin() { ; }
  ~ReplayWin();
  void on_left_double_click(int, int);
  void on_right_double_click(int, int);
  void on_mouse_move(int, int);
  void on_right_down(int, int);
  void on_left_down(int, int);
  void on_left_up(int, int);
  void on_redraw();
  void add_base_site(int x, int y, int faction_id);
  int update_base_site(int x, int y, int faction_id);
  void remove_base_site(int x, int y, int unused);
  void make_bases();

 private:
  // [0, 0xA14) is the GraphicWin base subobject; the class's own storage
  // starts here, immediately after it.
  uint32_t field_A14_;  // 0xA14
  // RECOVERY_MEMBER: v1 ReplayWin replay_turn_ 0xA18 0x005AD9E0
  int replay_turn_;  // 0xA18
  // RECOVERY_MEMBER: v1 ReplayWin event_offset_ 0xA1C 0x005AD9E0
  int event_offset_;  // byte offset into ReplayEvents
  // 0x005AD598 starts at this+0xA20 and visits exactly 0x200 records with
  // a 0xC stride. The next known member starts at 0x2220.
  // RECOVERY_MEMBER: v1 ReplayWin base_sites_ 0xA20 0x005AD560
  ReplayBaseSite base_sites_[512];  // 0xA20..0x2220
  // RECOVERY_MEMBER: v1 ReplayWin year_label_ 0x2220 0x005AD9E0
  char year_label_[0x100];  // 0x2220..0x2320
  // The original show_replay constructs these three complete subobjects in
  // order at offsets 0x2320, 0x247A0 and 0x252EC. Its destructor tears
  // them down in reverse order. The former opaque tail ended inside Time.
  // RECOVERY_MEMBER: v1 ReplayWin map_win_ 0x2320 0x005ADF10
  MapWin map_win_;
  // RECOVERY_MEMBER: v1 ReplayWin flat_button_ 0x247A0 0x005ADF10
  FlatButton flat_button_;
  // RECOVERY_MEMBER: v1 ReplayWin timer_ 0x252EC 0x005ADF10
  Time timer_;
};
static_assert(sizeof(ReplayWin) == 0x25314,
              "ReplayWin ownership must preserve the original object extent");
