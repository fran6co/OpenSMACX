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
#include "popup.h"
#include "win.h"

 /*
  * TutWin class
  *
  * Derives from Popup. Both ends of the base subobject are measured:
  *
  *   ??0TutWin@@QAE@XZ at 0x004BA6B0 does `mov esi, ecx` and then calls
  *   ??0Popup@@QAE@XZ with no lea in between, so the receiver is the
  *   unadjusted `this`; ??1TutWin@@QAE@XZ at 0x004BE7D0 tears the same chain
  *   down on the same pointer - ?close@Popup@@QAEXXZ, then ?close@BasePop@@QAEXXZ,
  *   then ??1GraphicWin@@QAE@XZ, all on `this+0`, matching Popup : BasePop :
  *   GraphicWin as src/ already declares it.
  *
  *   The base ENDS at 0x537C. That destructor inlines Popup's own teardown:
  *   ?close@Scroll@@QAEXXZ on this+0x3230 - exactly where popup.h puts
  *   `Scroll scroll_` - and the two FlatButtons inside it at +0xAAC and
  *   +0x15F8 (absolute 0x3CDC and 0x4828, the last ending at 0x5374), so the
  *   Scroll really does span [0x3230, 0x3230 + 0x214C) = [0x3230, 0x537C).
  *   From the other side, every field ??0TutWin writes for itself is at
  *   0x537C or above; the only stores it makes below that are the two vptrs
  *   at 0x0 and 0x444, which are GraphicWin's and belong to the base.
  *   sizeof(BasePop) 0x3230 and sizeof(Scroll) 0x214C are both pinned and
  *   both `reached` by verify_member_offsets --pins, so 0x3230 + 0x214C =
  *   0x537C is the base's extent, not an assumption.
  *
  * WITHDRAWN: this comment used to say "Popup's size is not established, so
  * the base cannot be modelled", and held [0, 0x537C) as a slab of field_
  * members instead. The two sentences above are the measurements that
  * withdrew it. It also said "Nothing pins this class's sizeof";
  * `verify_member_offsets.py --class TutWin --pins` prints
  * `reached TutWin 0x53D8 image reaches 0x53D8`, so the static_assert below
  * is reached by the image's own widest access.
  */
class DLLEXPORT TutWin : Popup {
 public:
  void on_move(int a1, int a2);
  TutWin() { ; }
  ~TutWin() { ; }
  void UNK1();
  void UNK3(int a1);
  void do_base(RECT *rect, const char *text, int flag);
  void do_iface(RECT *rect, const char *text, int flag);
  void do_soc(RECT *rect, const char *text, int flag);
  void do_des(RECT *rect, const char *text, int flag);
  void iface_rect(RECT *rect, int *x, int *y);
  void base_rect(RECT *rect, int *x, int *y);
  void soc_rect(RECT *rect, int *x, int *y);
  void des_rect(RECT *rect, int *x, int *y);

 private:
  // [0, 0x537C) is the Popup base subobject; see the note above.
  int32_t field_537C_;
  int32_t field_5380_;
  uint32_t field_5384_;  // 0x5384
  uint32_t field_5388_;  // 0x5388
  uint32_t field_538C_;  // 0x538C
  uint32_t field_5390_;  // 0x5390
  uint32_t field_5394_;  // 0x5394
  uint32_t field_5398_;  // 0x5398
  int32_t field_539C_;
  uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
  int32_t field_53A4_;
  int32_t field_53A8_;
  int32_t field_53AC_;
  uint8_t field_53B0_[0x4];  // 0x53B0
  uint32_t field_53B4_;  // 0x53B4
  int32_t field_53B8_;
  uint32_t field_53BC_;  // 0x53BC
  uint32_t field_53C0_;  // 0x53C0
  int32_t field_53C4_;
  uint32_t field_53C8_;  // 0x53C8
  uint32_t field_53CC_;  // 0x53CC
  uint32_t field_53D0_;  // 0x53D0
  int32_t field_53D4_;
};

static_assert(sizeof(TutWin) == 0x53D8,
              "TutWin layout must match the original executable");

extern uint32_t *TutWinShownFlag;

void __fastcall tut_win_unk1_redirect(TutWin *self, void *);
void __fastcall tut_win_unk3_redirect(TutWin *self, void *, int a1);

// The fixed window iface_rect centres onto, at 0x007AE820.
extern Win *TutWinIfaceWindow;
// The fixed window base_rect centres onto, at 0x006A7628.
extern Win *TutWinBaseWindow;
// The fixed window soc_rect centres onto, at 0x008A6270.
extern Win *TutWinSocWindow;
// The fixed window des_rect centres onto, at 0x0071F2B0.
extern Win *TutWinDesWindow;

void __fastcall tut_win_iface_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_base_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_soc_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_des_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);

// The shared tail of the four do_* helpers: 0x004BDFE0, still original.
typedef int (OriginalObject::*func_tut_win_show)(void *window, const char *text, int x, int y, void *sprite, int flag, int a7, int a8);
extern func_tut_win_show TutWinOriginalShow;

void __fastcall tut_win_do_base_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_iface_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_soc_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_des_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
