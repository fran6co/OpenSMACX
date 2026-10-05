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
#include "buffer.h"
#include "movie_media.h"

// Movie player. Its Buffer member and timed-text delay are pinned by native
// producers/consumers; the exact object extent is witnessed below. Remaining
// unnamed storage is not a recovered semantic model.
class UV2Player {
 public:
  UV2Player();
  ~UV2Player();
  // Release the native display interfaces before automatic Buffer destruction.
  void close();
  int play(int width, int height);
  // ?exec@UV2Player@@QAEHPADHHH@Z at 0x004BEA50, still an original body
  // (src/unrecovered/004bea50.cpp); the definition in pending_bodies.cpp
  // forwards to it. amovie_project plays the .wve opening through it.
  int exec(char *movie_file, int width, int height, int flags);
  int UNK4();
  static int __cdecl on_key();
  int write_timed(LPSTR text, int x, int y, int length);
  // Post-play MOVLIST text; exec invokes it only when playback was not quit.
  void show_post_movie_text();

  // exec writes the uppercased extension-free movie name here; UNK3 uses
  // it as the text section key. The next independently used field is +104.
  // RECOVERY_MEMBER: v1 UV2Player movie_name_ 0x0 0x004BEA50
  char movie_name_[0x104];  // 0x000..0x104
  uint32_t field_104_;  // 0x104
  IDirectDraw *direct_draw_;  // 0x108
  IDirectDraw2 *direct_draw2_;  // 0x10C
  IDirectDrawSurface2 *primary_surface_;  // 0x110
  IDirectDrawSurface2 *back_buffer_surface_;  // 0x114
  IDirectDrawClipper *clipper_;  // 0x118
  uint32_t field_11C_;  // 0x11C
  uint32_t field_120_;  // 0x120
  MediaDescriptor *media_session_; // 0x124; returned descriptor view
  MediaDescriptor media_; // 0x128..0x8DB; caller-owned descriptor
  Buffer buffer_;  // 0x8DC
  uint32_t write_delay_;  // 0xE64, unsigned elapsed-millisecond threshold
};

// EXACT, not a floor, and the two ends coincide. `?amovie_project@@YAXPAD@Z`
// (0x00403BE0) builds one UV2Player and one GraphicWin in a single frame:
// `lea ecx, [ebp - 0x1f0c]` / `call ??0UV2Player@@QAE@XZ` at 0x00403C05, then
// `lea ecx, [ebp - 0x10a4]` / `call ??0GraphicWin@@QAE@XZ`. Both objects are
// live at once, so the first cannot exceed its slot: 0x1F0C - 0x10A4 == 0xE68.
// The floor comes from the other side - `mov ecx, [ebx + 0xe64]` at
// 0x004BEE08 - and floor meets slot, so the size is settled rather than
// bounded. The frame corroborates itself: the GraphicWin at ebp-0x10A4 with
// its pinned 0xA14 ends exactly where the next local begins.
static_assert(sizeof(UV2Player) == 0xE68,
              "UV2Player layout must match the original executable");

extern UV2Player *UV2PlayerActive;
extern int UV2PlayerQuitRequested;
extern "C" int __cdecl uv2_player_writer_redirect(LPSTR text, int x, int y,
                                                  int length);
