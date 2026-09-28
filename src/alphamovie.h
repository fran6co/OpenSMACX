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
#include "graphicwin.h"

 /*
  * MCIVideo class
  *
 * The MCI video subobject AlphaMovie embeds at +0xA14. Its member extent is
 * only bounded by the accesses recovered so far.
  */
class MCIVideo {
 public:
  MCIVideo();  // ??0MCIVideo@@QAE@XZ  0x005FFD80
  void close();  // ?close@MCIVideo@@QAEXXZ  0x005FFDB0
  int load(char *movie_path, Win *target_window, int x, int y);
  void play();  // ?play@MCIVideo@@QAEXXZ  0x006001E0
  void stop();  // ?stop@MCIVideo@@QAEXXZ  0x00600270
  // 0x00600330, homed into general.cpp: copies the posted frame into the
  // window's own device context. No catalogue name - `blit_frame` is the
  // tree's, recorded on the body's `symbol` fact.
  void blit_frame();
  // ??1MCIVideo@@QAE@XZ at 0x004042C0, a user destructor that calls close();
  // the palette_ member is destroyed by the compiler after the body. The
  // homed definition in alphamovie.cpp is what amovie_project's stack
  // MCIVideo destroys at scope exit.
  ~MCIVideo();

  // Storage the image proves is here: its own methods reach 0x47C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 7 member(s) from the IDA database, 2 named; it starts a member at 0x0, which is where src/ ends.

  uint32_t field_0_;  // 0x0
  // The constructor only zeroes the LOW 16 bits at this offset
  // (`mov word ptr [esi+4], ax`), not the whole dword, so `mciId_` is
  // written through a 16-bit alias there rather than assigned directly -
  // see MCIVideo::MCIVideo() in alphamovie.cpp.
  uint32_t mciId_;  // 0x4
  // MCI_STATUS returns this handle; load, play, and blit_frame pass it to
  // window/GDI APIs. play also preserves the image's low-word MCI callback.
  HWND window_handle_;  // 0x8
  HPALETTE palette_handle_;  // 0xC
  // Sized and pinned: sizeof(Palette) == 0x454 (palette.h), exactly the
  // extent this field already carried as raw bytes. The constructor at
  // 0x005FFD80 placement-constructs a real Palette here (`call 0x5FE2A0`
  // on `this + 0x10`).
  Palette palette_;  // 0x10
  WNDPROC previous_window_proc_;  // 0x464
  // 0x468, stored by load() from its Win* argument and checked by blit_frame.
  Win *target_window_;  // 0x468
  // 0x46C..0x478, named from 0x00600330: BitBlt's destination x, y, width
  // and height in that order, passed straight through as the four
  // destination arguments.
  int blit_x_;       // 0x46C
  int blit_y_;       // 0x470
  int blit_width_;   // 0x474
  int blit_height_;  // 0x478
};

 /*
  * AlphaMovie class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The layout is not established. The constructor's own fields begin at 0xA14,
  * immediately after the base, and where the object ends is unknown, so
  * nothing pins its sizeof and the fields between are not modelled. The
  * `exec` reaches the embedded MCIVideo and GraphicWin base through their
  * source-owned members. Its object extent remains unpinned beyond those
  * observed regions.
  */
// GraphicWin DIRECTLY, not through ConstructedGraphicWin - same reason
// StringBox drops the shim (see stringbox.h): ConstructedGraphicWin's inlined
// implicit constructor stores the shim's own two vftables between the
// GraphicWin base call and the MCIVideo member construction, a store pair
// ??0AlphaMovie does not have. The base's real constructor runs implicitly
// either way, and the shim adds no data and no virtual, so the layout is
// unchanged.
class AlphaMovie : public GraphicWin {
 public:
  void close();
  AlphaMovie();
  // ?exec@AlphaMovie@@QAEHPAD@Z at 0x00404070; implemented in
  // alphamovie.cpp from the compatible seed and routed through its embedded
  // MCIVideo member. amovie_project constructs a real AlphaMovie receiver.
  int exec(char *movie_name);
  // 0x00404310 closes the MCI stream and GraphicWin base in the body;
  // the compiler then destroys the embedded MCIVideo and base naturally.
  ~AlphaMovie();
  void UNK7();
  int UNK2(int a1, int a2, int a3);
  int UNK4(int a1, int a2, int a3, int a4);
  int UNK3(int a1);
  int UNK5(int a1);
  void UNK6(int a1, int a2);
  void UNK6(int menu_id);
  int UNK8(int a1, int a2);

 private:
  // ??0AlphaMovie@@QAE@XZ calls ??0GraphicWin@@QAE@XZ on an unadjusted `this`
  // at 0x0040402F - so GraphicWin is the base - and then builds an MCIVideo
  // at `lea ecx, [esi + 0xa14]` (0x00404034). ??1AlphaMovie mirrors it with
  // `lea edi, [esi + 0xa14]` at 0x00404342. sizeof(GraphicWin) is pinned at
  // 0xA14, so this is the first member after the base.
  //
  // NO SIZE ASSERTION FOLLOWS, deliberately. MCIVideo's storage reaches
  // 0x47C through its height member, with the containing local bracketed by
  // amovie_project's stack frame at 0x47C.
  MCIVideo mciVideo_;  // 0xA14
};

