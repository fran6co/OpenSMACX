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

// MCIVideo is JACKAL.LIB's MCI player (src/jackal/mci_video.cpp).
#include "mci_video.h"

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
  virtual int on_key_click(int repeat, int key);
  AlphaMovie();
  // ?exec@AlphaMovie@@QAEHPAD@Z at 0x00404070; implemented in
  // alphamovie.cpp from the compatible seed and routed through its embedded
  // MCIVideo member. amovie_project constructs a real AlphaMovie receiver.
  int exec(char *movie_name);
  // Original primary slot 59 selects this movie continuation policy.
  int on_modal() override;
  // 0x00404310 closes the MCI stream and GraphicWin base in the body;
  // the compiler then destroys the embedded MCIVideo and base naturally.
  ~AlphaMovie();
  void UNK7();
  void UNK1();
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

void __cdecl amovie_project(char *movie);
void __cdecl amovie_project(int project_id);

// MCI message helper names are catalog-neutral; original registration is unresolved.
LRESULT __cdecl sub_5ffe40(HWND, WPARAM virtual_key, int forward_event, int low_payload, int high_payload);
LRESULT __cdecl sub_5ffe80(HWND, int double_click, int x, int y, WPARAM keys);
