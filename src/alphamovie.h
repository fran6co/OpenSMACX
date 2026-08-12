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

 /*
  * MCIVideo class
  *
  * The MCI video subobject AlphaMovie embeds at +0xA14. Declared, not laid
  * out: close() is the only member reached from here and the address is
  * computed by hand at the call site, so nothing needs the size. Not
  * DLLEXPORT - ?close@MCIVideo@@QAEXXZ (0x005FFDB0) is still an original body
  * and dllexport on a class whose members only forward would export a seam
  * that is not this DLL's to publish.
  */
class MCIVideo {
 public:
  void close();  // ?close@MCIVideo@@QAEXXZ  0x005FFDB0

  // Storage the image proves is here: its own methods reach 0x474.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 7 member(s) from the IDA database, 2 named; it starts a member at 0x0, which is where src/ ends.

  uint32_t field_0_;  // 0x0
  uint32_t mciId_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint8_t palette_[0x454];  // 0x10
  uint32_t field_464_;  // 0x464
  uint32_t field_468_;  // 0x468
  uint8_t field_46C_[0x8];  // 0x46C
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
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
class DLLEXPORT AlphaMovie : GraphicWin {
 public:
  void update();
  void close();
  AlphaMovie() { ; }
  ~AlphaMovie() { ; }
  void UNK7();
  int UNK2(int a1, int a2, int a3);
  int UNK4(int a1, int a2, int a3, int a4);
  int UNK3(int a1);
  int UNK5(int a1);
  void UNK6(int a1, int a2);
  void UNK6(int a1);
  int UNK8(int a1, int a2);

 private:
  // ??0AlphaMovie@@QAE@XZ calls ??0GraphicWin@@QAE@XZ on an unadjusted `this`
  // at 0x0040402F - so GraphicWin is the base - and then builds an MCIVideo
  // at `lea ecx, [esi + 0xa14]` (0x00404034). ??1AlphaMovie mirrors it with
  // `lea edi, [esi + 0xa14]` at 0x00404342. sizeof(GraphicWin) is pinned at
  // 0xA14, so this is the first member after the base.
  //
  // NO SIZE ASSERTION FOLLOWS, deliberately. It would have to be
  // 0xA14 + sizeof(MCIVideo), and MCIVideo's own end is only bracketed:
  // ?load@MCIVideo@@QAEHPADPAUWin@@HH@Z writes [esi + 0x470] at 0x005FFFCD,
  // a floor of 0x474, while the IDB claims 0x46C. The upper bound comes from
  // ?amovie_project@@YAXPAD@Z's stack frame and is 0x47C.
  MCIVideo mciVideo_;  // 0xA14
};

void __fastcall alpha_movie_unk7_redirect(AlphaMovie *self, void *);
int __fastcall alpha_movie_unk2_redirect(AlphaMovie *self, void *, int a1, int a2, int a3);
int __fastcall alpha_movie_unk4_redirect(AlphaMovie *self, void *, int a1, int a2, int a3, int a4);
int __fastcall alpha_movie_unk3_redirect(AlphaMovie *self, void *, int a1);
int __fastcall alpha_movie_unk5_redirect(AlphaMovie *self, void *, int a1);
void __fastcall alpha_movie_unk6_00404260_redirect(AlphaMovie *self, void *, int a1, int a2);
void __fastcall alpha_movie_unk6_00404270_redirect(AlphaMovie *self, void *, int a1);
int __fastcall alpha_movie_unk8_redirect(AlphaMovie *self, void *, int a1, int a2);

// MCIVideo::close is still an original body - 141 bytes that shut down the MCI
// device through winmm - so the definition at the end of alphamovie.cpp
// forwards to it. Public, __thiscall, void(void). Rebindable so tests can
// substitute a probe and so the seam can later point at a recovered body.
typedef void (OriginalObject::*func_mci_video_close)();
extern func_mci_video_close MCIVideoOriginalClose;  // 0x005FFDB0
