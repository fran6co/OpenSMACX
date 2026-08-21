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

#include "time.h"
#include "vc6_compat.h"

/*
 * EditBox - promoted from src/hypothesis_layouts.h into its own header so
 * DiploWin can embed it by value. Five methods are catalogued (constructor
 * 0x00614E50, close, destructor, deleting destructor, operator delete) and
 * none is recovered yet.
 *
 * Layout evidence is the IDB member table only: 78 four-byte fields between
 * a 0xA14-byte GraphicWin-shaped prefix and a trailing Time, ending at
 * 0xB74. Two independent readings agree on that extent - the table itself,
 * and DiploWin's `editBox` member, which the IDB sizes at exactly 2932.
 * The constructor has not been shown to build GraphicWin on an unadjusted
 * `this`, so the base region is flat storage rather than inheritance, and
 * NOTHING here is pinned: promote to `: GraphicWin` and a static_assert
 * only once derive_class_layout or derive_base_edges produces the evidence.
 */
class EditBox {
 public:
  // 0x00408010 is not recovered: a pending_bodies forwarder, because
  // an empty inline stub emits nothing and the deleting destructor
  // needs a `call rel32`.
  ~EditBox();

  // A `construct` METHOD, NOT A CONSTRUCTOR - the `Win::construct` idiom.
  // The real body (??0EditBox@@QAE@XZ, 0x00614E50) is not promoted yet;
  // FileWin::FileWin placement-news an EditBox member and reaches it by
  // name so that call emits the image's `E8`. Forwarded in
  // pending_bodies.cpp.
  //
  // NOT a real constructor, on purpose, even though ??0EditBox@@QAE@XZ has
  // no most-derived flag and could legally be spelled that way: EditBox's
  // OWN destructor (below) is real and non-trivial (0x00408010, needed
  // as-is by the already-BYTE_EXACT scalar_delete_edit_box in
  // deleting_thunks.cpp), so a real EditBox member gets its automatic
  // destruction from the compiler regardless of what FileWin::~FileWin
  // writes - and the image's FileWin destructor does NOT call 0x00408010
  // at all, it inlines EditBox's teardown through different addresses.
  // Measured: making this a real constructor moved FileWin's own
  // constructor closer but cost the destructor an extra, wrong call.
  void construct();

  // 0x006151E0, a pending_bodies forwarder. `EditGroup::set_text` reached it
  // through a `func_edit_box_set_text` pointer-to-member seam, which emits
  // `call dword ptr [...]` where the image emits `call rel32`.
  void set_text(char *text);

  // 0x00614F30, a pending_bodies forwarder. FileWin::~FileWin calls it
  // directly (inlining EditBox's own teardown rather than going through
  // EditBox's destructor at 0x00408010).
  void close();

 public:
  uint8_t graphicWin_[0xA14];  // 0x0, GraphicWin-shaped base storage

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
  Time time_;  // 0xB4C, the IDB's trailing member; the class ends at 0xB74
};
