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
#include "sprite.h"
#include "prodpicker.h"
#include "mapwin.h"
#include "flatbutton.h"
#include "caviar.h"
#include "buffer.h"
#include "font.h"
#include "spot.h"
#include "scroll.h"

#include "original_seam.h"
#include "graphicwin.h"
#include "subinterface.h"

 /*
  * BaseWin class
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
class BaseWin : public ConstructedGraphicWin, public SubInterface {
 public:
  // 0x004165D0, a pending_bodies forwarder: the three click handlers
  // below funnel into it and reached it through a pointer.
  void click(int a1, int a2, int a3, int a4);
  // 0x004160F0, a pending_bodies forwarder.
  void iface_click(int a1, int a2, int a3, int a4);
  // 0x0040C850, a pending_bodies forwarder.
  void draw_supported(int a1);
 public:
  void on_scrolled(int a1, int a2);
  void UNK2(int a1, int a2, int a3, int a4);
  // Two siblings the recovered bodies above reach with a direct `call rel32`
  // - UNK2 calls garrison_click, on_scrolled calls draw_facilities. Both are
  // unrecovered; pending_bodies.cpp forwards them.
  void garrison_click(int vehID, int a2, int a3, int a4);  // 0x0040B140
  void draw_facilities(int a1);                            // 0x0040FCC0
  void show(int a1);
  BaseWin();
  // 0x00420A70 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~BaseWin();
  void close();
  void UNK4();
  void UNK6();
  void UNK7();
  void UNK5(int a1);
  void on_button_toggled(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_scrolled(int a1, int a2);
  static void timer_callback(int a1, int a2);
  void on_left_click(int a1, int a2);
  void on_right_click(int a1, int a2);
  void on_left_double_click(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x40D20.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 293 member(s) from the IDA database, 30 named; it starts a member at 0xA14, which is where src/ ends.

  // The SubInterface is EIGHT bytes, not four, and the ProdPicker after it
  // starts at 0xA1C rather than 0xA18. ??0BaseWin@@QAE@XZ settles both:
  // 0x004084B4 `lea ecx, [esi + 0xa1c]`, 0x004084C1
  // `mov dword ptr [esi + 0xa14], 0x66a6e4` - the SubInterface vftable seven
  // other window constructors store at their own +0xA14 - then
  // `call ??0ProdPicker@@QAE@XZ`. sizeof(ProdPicker) is pinned at 0xA840 and
  // 0xA1C + 0xA840 == 0xB25C, exactly where the next `lea ecx, [esi+0xb25c]`
  // builds graphicWin2_, so the two abut and the `field_B258_` dword this
  // header used to put between them does not exist. The three members total
  // 0xA848 before and after, so nothing past graphicWin2_ moves.
  // The SubInterface is the SECOND BASE, above; MSVC puts it at
  // sizeof(GraphicWin) == 0xA14, which is where the constructor writes its
  // vftable.
  ProdPicker prodPicker_;  // 0xA1C
  GraphicWin graphicWin2_;  // 0xB25C
  Sprite sprites1_[2];  // 0xBC70
  Sprite sprites2_[2];  // 0xBCC8
  Sprite sprites3_[2];  // 0xBD20
  Sprite sprite4_;  // 0xBD78
  Sprite sprite5_;  // 0xBDA4
  Sprite sprite6_;  // 0xBDD0
  Sprite sprites7_[3];  // 0xBDFC
  FlatButton flatButton_;  // 0xBE80
  Sprite sprites8_[42];  // 0xC9CC
  Sprite sprite9_;  // 0xD104
  Sprite sprite10_;  // 0xD130
  Sprite sprite11_;  // 0xD15C
  Sprite sprite12_;  // 0xD188
  Sprite sprite13_;  // 0xD1B4
  Sprite sprite14_;  // 0xD1E0
  Sprite sprite15_;  // 0xD20C
  Buffer buffer1_;  // 0xD238
  FlatButton flatButtons_[22];  // 0xD7C0
  uint32_t field_1D048_;  // 0x1D048
  uint32_t field_1D04C_;  // 0x1D04C
  uint32_t field_1D050_;  // 0x1D050
  uint32_t field_1D054_;  // 0x1D054
  uint32_t field_1D058_;  // 0x1D058
  uint32_t field_1D05C_;  // 0x1D05C
  uint32_t field_1D060_;  // 0x1D060
  uint32_t field_1D064_;  // 0x1D064
  uint32_t field_1D068_;  // 0x1D068
  uint32_t field_1D06C_;  // 0x1D06C
  uint32_t field_1D070_;  // 0x1D070
  uint32_t field_1D074_;  // 0x1D074
  uint32_t field_1D078_;  // 0x1D078
  uint32_t field_1D07C_;  // 0x1D07C
  uint32_t field_1D080_;  // 0x1D080
  uint32_t field_1D084_;  // 0x1D084
  uint32_t field_1D088_;  // 0x1D088
  uint32_t field_1D08C_;  // 0x1D08C
  uint32_t field_1D090_;  // 0x1D090
  uint32_t field_1D094_;  // 0x1D094
  uint32_t field_1D098_;  // 0x1D098
  uint32_t field_1D09C_;  // 0x1D09C
  uint32_t field_1D0A0_;  // 0x1D0A0
  uint32_t field_1D0A4_;  // 0x1D0A4
  uint32_t field_1D0A8_;  // 0x1D0A8
  uint32_t field_1D0AC_;  // 0x1D0AC
  uint32_t field_1D0B0_;  // 0x1D0B0
  uint32_t field_1D0B4_;  // 0x1D0B4
  uint32_t field_1D0B8_;  // 0x1D0B8
  uint32_t field_1D0BC_;  // 0x1D0BC
  uint32_t field_1D0C0_;  // 0x1D0C0
  uint32_t field_1D0C4_;  // 0x1D0C4
  uint32_t field_1D0C8_;  // 0x1D0C8
  uint32_t field_1D0CC_;  // 0x1D0CC
  uint32_t field_1D0D0_;  // 0x1D0D0
  uint32_t field_1D0D4_;  // 0x1D0D4
  uint32_t field_1D0D8_;  // 0x1D0D8
  uint32_t field_1D0DC_;  // 0x1D0DC
  uint32_t field_1D0E0_;  // 0x1D0E0
  uint32_t field_1D0E4_;  // 0x1D0E4
  uint32_t field_1D0E8_;  // 0x1D0E8
  uint32_t field_1D0EC_;  // 0x1D0EC
  uint32_t field_1D0F0_;  // 0x1D0F0
  uint32_t field_1D0F4_;  // 0x1D0F4
  uint32_t field_1D0F8_;  // 0x1D0F8
  uint32_t field_1D0FC_;  // 0x1D0FC
  uint32_t field_1D100_;  // 0x1D100
  uint32_t field_1D104_;  // 0x1D104
  uint32_t field_1D108_;  // 0x1D108
  uint32_t field_1D10C_;  // 0x1D10C
  uint32_t field_1D110_;  // 0x1D110
  uint32_t field_1D114_;  // 0x1D114
  uint32_t field_1D118_;  // 0x1D118
  uint32_t field_1D11C_;  // 0x1D11C
  uint32_t field_1D120_;  // 0x1D120
  uint32_t field_1D124_;  // 0x1D124
  uint32_t field_1D128_;  // 0x1D128
  uint32_t field_1D12C_;  // 0x1D12C
  uint32_t field_1D130_;  // 0x1D130
  uint32_t field_1D134_;  // 0x1D134
  uint32_t field_1D138_;  // 0x1D138
  uint32_t field_1D13C_;  // 0x1D13C
  uint32_t field_1D140_;  // 0x1D140
  uint32_t field_1D144_;  // 0x1D144
  Time time_;  // 0x1D148
  Caviar caviar_;  // 0x1D170
  uint32_t field_1E540_;  // 0x1E540
  uint32_t field_1E544_;  // 0x1E544
  uint32_t field_1E548_;  // 0x1E548
  uint32_t field_1E54C_;  // 0x1E54C
  uint32_t field_1E550_;  // 0x1E550
  uint32_t field_1E554_;  // 0x1E554
  uint32_t field_1E558_;  // 0x1E558
  uint32_t field_1E55C_;  // 0x1E55C
  uint32_t field_1E560_;  // 0x1E560
  uint32_t field_1E564_;  // 0x1E564
  uint32_t field_1E568_;  // 0x1E568
  uint32_t field_1E56C_;  // 0x1E56C
  uint32_t field_1E570_;  // 0x1E570
  uint32_t field_1E574_;  // 0x1E574
  uint32_t field_1E578_;  // 0x1E578
  uint32_t field_1E57C_;  // 0x1E57C
  uint32_t field_1E580_;  // 0x1E580
  uint32_t field_1E584_;  // 0x1E584
  uint32_t field_1E588_;  // 0x1E588
  uint32_t field_1E58C_;  // 0x1E58C
  uint32_t field_1E590_;  // 0x1E590
  uint32_t field_1E594_;  // 0x1E594
  uint32_t field_1E598_;  // 0x1E598
  uint32_t field_1E59C_;  // 0x1E59C
  uint32_t field_1E5A0_;  // 0x1E5A0
  uint32_t field_1E5A4_;  // 0x1E5A4
  uint32_t field_1E5A8_;  // 0x1E5A8
  uint32_t field_1E5AC_;  // 0x1E5AC
  uint32_t field_1E5B0_;  // 0x1E5B0
  uint32_t field_1E5B4_;  // 0x1E5B4
  uint32_t field_1E5B8_;  // 0x1E5B8
  uint32_t field_1E5BC_;  // 0x1E5BC
  uint32_t field_1E5C0_;  // 0x1E5C0
  uint32_t field_1E5C4_;  // 0x1E5C4
  uint32_t field_1E5C8_;  // 0x1E5C8
  uint32_t field_1E5CC_;  // 0x1E5CC
  uint32_t field_1E5D0_;  // 0x1E5D0
  uint32_t field_1E5D4_;  // 0x1E5D4
  uint32_t field_1E5D8_;  // 0x1E5D8
  uint32_t field_1E5DC_;  // 0x1E5DC
  uint32_t field_1E5E0_;  // 0x1E5E0
  uint32_t field_1E5E4_;  // 0x1E5E4
  uint32_t field_1E5E8_;  // 0x1E5E8
  uint32_t field_1E5EC_;  // 0x1E5EC
  uint32_t field_1E5F0_;  // 0x1E5F0
  uint32_t field_1E5F4_;  // 0x1E5F4
  uint32_t field_1E5F8_;  // 0x1E5F8
  uint32_t field_1E5FC_;  // 0x1E5FC
  uint32_t field_1E600_;  // 0x1E600
  uint32_t field_1E604_;  // 0x1E604
  uint32_t field_1E608_;  // 0x1E608
  uint32_t field_1E60C_;  // 0x1E60C
  uint32_t field_1E610_;  // 0x1E610
  uint32_t field_1E614_;  // 0x1E614
  uint32_t field_1E618_;  // 0x1E618
  uint32_t field_1E61C_;  // 0x1E61C
  uint32_t field_1E620_;  // 0x1E620
  uint32_t field_1E624_;  // 0x1E624
  uint32_t field_1E628_;  // 0x1E628
  uint32_t field_1E62C_;  // 0x1E62C
  uint32_t field_1E630_;  // 0x1E630
  uint32_t field_1E634_;  // 0x1E634
  uint32_t field_1E638_;  // 0x1E638
  uint32_t field_1E63C_;  // 0x1E63C
  uint32_t field_1E640_;  // 0x1E640
  uint32_t field_1E644_;  // 0x1E644
  uint32_t field_1E648_;  // 0x1E648
  uint32_t field_1E64C_;  // 0x1E64C
  uint32_t field_1E650_;  // 0x1E650
  uint32_t field_1E654_;  // 0x1E654
  uint32_t field_1E658_;  // 0x1E658
  uint32_t field_1E65C_;  // 0x1E65C
  uint32_t field_1E660_;  // 0x1E660
  // These three are `float`, and they are the third row of a VOX_Matrix that
  // begins at 0x1E64C. ?init@BaseWin@@QAEXXZ passes &field_1E64C_ as the
  // VOX_Matrix* argument of
  // ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z - 0x0041F40F
  // `lea eax, [esi + 0x1e64c]` / `push eax`, then 0x0041F43D `call 0x6182a0` -
  // and VOX_Matrix is `float values[3][3]`, 36 bytes, so the matrix spans
  // 0x1E64C..0x1E66F and its +0x18/+0x1C/+0x20 are exactly these offsets.
  // What types them is that init loads all three with a single-precision x87
  // load, which is not something a `uint32_t` produces: 0x0041F3DC
  // `fld dword ptr [esi + 0x1e664]` / `fchs` / `fst dword ptr [esi + 0x1e664]`,
  // and the same negate-in-place at 0x0041F3EA for 0x1E668 and 0x0041F3FB for
  // 0x1E66C. Each negated value is then multiplied by
  // *(float *)0x0066AA40 == -10000.0f and stored into the three-float local at
  // [ebp-0x38] that is set_camera_direct's VOX_Vect* argument, i.e. the camera
  // position. src/recovered/units/004bf740.cpp
  // (?compute_camera@@YAXPAUVOX_Vect@@PAUVOX_Matrix@@@Z) already carries the
  // same idiom on its own VOX_Matrix out-parameter, negating out+0x18,
  // out+0x1C and out+0x20.
  // They keep `field_` names on purpose: they are three elements of a matrix,
  // not an independent triple, so naming them individually would assert a
  // meaning the arithmetic does not give. Carving 0x1E64C..0x1E66F into one
  // `VOX_Matrix` member - and 0x1E640..0x1E648, the Euler triple {0, 0,
  // 0.785f} that the 0x0041F3D7 `call sub_627D00` turns into that matrix, into
  // one `VOX_Vect` - is the follow-up, not this change.
  // `float` and `uint32_t` are both 4 bytes and 4-aligned, so no offset moves:
  // the VC6 probe reports sizeof(BaseWin) == 0x47838 before and after.
  float field_1E664_;  // 0x1E664
  float field_1E668_;  // 0x1E668
  float field_1E66C_;  // 0x1E66C
  uint32_t field_1E670_;  // 0x1E670
  MapWin mapWin_;  // 0x1E674
  uint32_t field_40AF4_;  // 0x40AF4
  uint32_t field_40AF8_;  // 0x40AF8
  uint32_t field_40AFC_;  // 0x40AFC
  uint32_t field_40B00_;  // 0x40B00
  uint32_t field_40B04_;  // 0x40B04
  uint32_t field_40B08_;  // 0x40B08
  uint32_t focusedBaseID_;  // 0x40B0C
  uint32_t field_40B10_;  // 0x40B10
  uint32_t field_40B14_;  // 0x40B14
  uint32_t field_40B18_;  // 0x40B18
  uint32_t field_40B1C_;  // 0x40B1C
  uint32_t field_40B20_;  // 0x40B20
  uint32_t field_40B24_;  // 0x40B24
  uint32_t field_40B28_;  // 0x40B28
  uint32_t field_40B2C_;  // 0x40B2C
  uint32_t field_40B30_;  // 0x40B30
  uint32_t field_40B34_;  // 0x40B34
  uint32_t field_40B38_;  // 0x40B38
  uint32_t field_40B3C_;  // 0x40B3C
  uint32_t field_40B40_;  // 0x40B40
  uint32_t field_40B44_;  // 0x40B44
  uint32_t field_40B48_;  // 0x40B48
  uint32_t field_40B4C_;  // 0x40B4C
  uint32_t field_40B50_;  // 0x40B50
  uint32_t field_40B54_;  // 0x40B54
  uint32_t field_40B58_;  // 0x40B58
  uint32_t field_40B5C_;  // 0x40B5C
  uint32_t field_40B60_;  // 0x40B60
  uint32_t field_40B64_;  // 0x40B64
  uint32_t field_40B68_;  // 0x40B68
  uint32_t field_40B6C_;  // 0x40B6C
  uint32_t field_40B70_;  // 0x40B70
  uint32_t field_40B74_;  // 0x40B74
  uint32_t field_40B78_;  // 0x40B78
  uint32_t field_40B7C_;  // 0x40B7C
  uint32_t field_40B80_;  // 0x40B80
  uint32_t field_40B84_;  // 0x40B84
  uint32_t field_40B88_;  // 0x40B88
  RECT rect1_;  // 0x40B8C, IDB `rect1`
  uint32_t field_40B9C_;  // 0x40B9C
  uint32_t field_40BA0_;  // 0x40BA0
  uint32_t field_40BA4_;  // 0x40BA4
  uint32_t field_40BA8_;  // 0x40BA8
  RECT rect2_;  // 0x40BAC, IDB `rect2`
  uint32_t field_40BBC_;  // 0x40BBC
  uint32_t field_40BC0_;  // 0x40BC0
  uint32_t field_40BC4_;  // 0x40BC4
  uint32_t field_40BC8_;  // 0x40BC8
  uint32_t field_40BCC_;  // 0x40BCC
  uint32_t field_40BD0_;  // 0x40BD0
  uint32_t field_40BD4_;  // 0x40BD4
  uint32_t field_40BD8_;  // 0x40BD8
  RECT rect3_;  // 0x40BDC, IDB `rect3`
  uint32_t field_40BEC_;  // 0x40BEC
  uint32_t field_40BF0_;  // 0x40BF0
  uint32_t field_40BF4_;  // 0x40BF4
  uint32_t field_40BF8_;  // 0x40BF8
  RECT rect4_;  // 0x40BFC, IDB `rect4`
  uint32_t field_40C0C_;  // 0x40C0C
  uint32_t field_40C10_;  // 0x40C10
  uint32_t field_40C14_;  // 0x40C14
  uint32_t field_40C18_;  // 0x40C18
  uint32_t field_40C1C_;  // 0x40C1C
  uint32_t field_40C20_;  // 0x40C20
  uint32_t field_40C24_;  // 0x40C24
  uint32_t field_40C28_;  // 0x40C28
  uint32_t field_40C2C_;  // 0x40C2C
  uint32_t field_40C30_;  // 0x40C30
  uint32_t field_40C34_;  // 0x40C34
  uint32_t field_40C38_;  // 0x40C38
  uint32_t field_40C3C_;  // 0x40C3C
  uint32_t field_40C40_;  // 0x40C40
  uint32_t field_40C44_;  // 0x40C44
  uint32_t field_40C48_;  // 0x40C48
  uint32_t field_40C4C_;  // 0x40C4C
  uint32_t field_40C50_;  // 0x40C50
  uint32_t field_40C54_;  // 0x40C54
  uint32_t field_40C58_;  // 0x40C58
  uint32_t field_40C5C_;  // 0x40C5C
  uint32_t field_40C60_;  // 0x40C60
  uint32_t field_40C64_;  // 0x40C64
  uint32_t field_40C68_;  // 0x40C68
  uint32_t field_40C6C_;  // 0x40C6C
  uint32_t field_40C70_;  // 0x40C70
  uint32_t field_40C74_;  // 0x40C74
  uint32_t field_40C78_;  // 0x40C78
  uint32_t field_40C7C_;  // 0x40C7C
  uint32_t field_40C80_;  // 0x40C80
  uint32_t field_40C84_;  // 0x40C84
  uint32_t field_40C88_;  // 0x40C88
  uint32_t field_40C8C_;  // 0x40C8C
  uint32_t field_40C90_;  // 0x40C90
  uint32_t field_40C94_;  // 0x40C94
  uint32_t field_40C98_;  // 0x40C98
  uint32_t field_40C9C_;  // 0x40C9C
  uint32_t field_40CA0_;  // 0x40CA0
  uint32_t field_40CA4_;  // 0x40CA4
  uint32_t field_40CA8_;  // 0x40CA8
  uint32_t field_40CAC_;  // 0x40CAC
  uint32_t field_40CB0_;  // 0x40CB0
  uint32_t field_40CB4_;  // 0x40CB4
  uint32_t field_40CB8_;  // 0x40CB8
  uint32_t field_40CBC_;  // 0x40CBC
  uint32_t field_40CC0_;  // 0x40CC0
  uint32_t field_40CC4_;  // 0x40CC4
  uint32_t field_40CC8_;  // 0x40CC8
  uint32_t field_40CCC_;  // 0x40CCC
  uint32_t field_40CD0_;  // 0x40CD0
  uint32_t field_40CD4_;  // 0x40CD4
  uint32_t field_40CD8_;  // 0x40CD8
  uint32_t field_40CDC_;  // 0x40CDC
  uint32_t field_40CE0_;  // 0x40CE0
  uint32_t field_40CE4_;  // 0x40CE4
  uint32_t field_40CE8_;  // 0x40CE8
  uint32_t field_40CEC_;  // 0x40CEC
  uint32_t field_40CF0_;  // 0x40CF0
  uint32_t field_40CF4_;  // 0x40CF4
  uint32_t field_40CF8_;  // 0x40CF8
  uint32_t field_40CFC_;  // 0x40CFC
  uint32_t field_40D00_;  // 0x40D00
  uint32_t field_40D04_;  // 0x40D04
  uint32_t field_40D08_;  // 0x40D08
  uint32_t field_40D0C_;  // 0x40D0C
  uint32_t field_40D10_;  // 0x40D10
  uint32_t field_40D14_;  // 0x40D14
  uint32_t field_40D18_;  // 0x40D18
  RECT rect5_;  // 0x40D1C
  RECT rect6_;  // 0x40D2C
  RECT rect7_;  // 0x40D3C
  uint32_t field_40D4C_;  // 0x40D4C
  uint32_t field_40D50_;  // 0x40D50
  uint32_t field_40D54_;  // 0x40D54
  uint32_t field_40D58_;  // 0x40D58
  uint32_t field_40D5C_;  // 0x40D5C
  uint32_t field_40D60_;  // 0x40D60
  uint32_t field_40D64_;  // 0x40D64
  uint32_t field_40D68_;  // 0x40D68
  RECT rect14_;  // 0x40D6C
  RECT rect15_;  // 0x40D7C
  RECT rect18_;  // 0x40D8C
  RECT rect19_;  // 0x40D9C
  RECT rect8_;  // 0x40DAC
  RECT rect9_;  // 0x40DBC
  RECT rect10_;  // 0x40DCC
  RECT rect11_;  // 0x40DDC
  uint32_t field_40DEC_;  // 0x40DEC
  uint32_t field_40DF0_;  // 0x40DF0
  uint32_t field_40DF4_;  // 0x40DF4
  uint32_t field_40DF8_;  // 0x40DF8
  uint32_t field_40DFC_;  // 0x40DFC

  uint32_t field_40E00_;  // 0x40E00
  uint32_t field_40E04_;  // 0x40E04
  uint32_t field_40E08_;  // 0x40E08
  uint32_t field_40E0C_;  // 0x40E0C
  uint32_t field_40E10_;  // 0x40E10
  uint32_t field_40E14_;  // 0x40E14
  uint32_t field_40E18_;  // 0x40E18
  uint32_t field_40E1C_;  // 0x40E1C
  uint32_t field_40E20_;  // 0x40E20
  uint32_t field_40E24_;  // 0x40E24
  uint32_t field_40E28_;  // 0x40E28
  RECT rect12_;  // 0x40E2C
  RECT rect13_;  // 0x40E3C
  RECT rect16_;  // 0x40E4C
  RECT rect17_;  // 0x40E5C
  uint32_t field_40E6C_;  // 0x40E6C
  uint32_t field_40E70_;  // 0x40E70
  uint32_t field_40E74_;  // 0x40E74
  uint32_t field_40E78_;  // 0x40E78
  RECT rect20_;  // 0x40E7C
  Font font1_;  // 0x40E8C, IDB `font1`, size == sizeof(Font)
  Spot spot1_;  // 0x40EB4, size == sizeof(Spot)
  Spot spot2_;  // 0x40EC0, size == sizeof(Spot)
  Buffer buffer2_;  // 0x40ECC, size == sizeof(Buffer)
  Scroll scroll1_;  // 0x41454, size == sizeof(Scroll)
  Scroll scroll2_;  // 0x435A0, size == sizeof(Scroll)

  // The comment that stood here said the IDB's scroll3 at 0x456EC lay "beyond
  // the proven extent" and would not be declared until a body reached it, and
  // it covered the offset with 0x48 opaque bytes ending at 0x45734. A body
  // does reach it, and the 0x48 was short: tools/verify_member_offsets.py
  // reported `overrun BaseWin+0x45B30 w4: sizeof is 0x45734` against
  // ??1BaseWin@@QAE@XZ. That destructor tears the offset down as a whole
  // Scroll, inlined - 0x00420AB4 `lea edi, [esi + 0x456ec]`, then the two
  // vftable stores `mov dword ptr [edi], 0x669d58` and
  // `mov dword ptr [edi + 0x444], ebx` with ebx = 0x669d50 (the GraphicWin
  // base slot; Scroll derives from GraphicWin, whose subobject vftable sits at
  // +0x444), then `mov ecx, edi` / `call ?close@Scroll@@QAEXXZ`, then
  // ??1FlatButton@@QAE@XZ on edi+0xAAC and edi+0x15F8 - which are exactly
  // flat_button_left_ and flat_button_right_ inside Scroll - and finally
  // ??1GraphicWin@@QAE@XZ on edi itself. The next teardown, at
  // 0x00420B02 `lea edi, [esi + 0x435a0]`, repeats the same sequence on
  // scroll2_, so the shape is the one already declared twice above.
  // ??0BaseWin@@QAE@XZ constructs the matching object at the same offset
  // (`new (self + 0x456ec) Scroll();` in the deleted
  // src/recovered/units/00408490.cpp).
  // 0x456EC + sizeof(Scroll) == 0x47838, which is 8 bytes under the 0x47840
  // upper bound docs/recovery/class-size-bounds.csv derives from the next
  // constructed global, so this is the last member and BaseWin ends here.
  Scroll scroll3_;  // 0x456EC, size == sizeof(Scroll)
};

void __fastcall base_win_close_redirect(BaseWin *self, void *);
void __fastcall base_win_unk4_redirect(BaseWin *self, void *);
void __fastcall base_win_unk6_redirect(BaseWin *self, void *);
void __fastcall base_win_unk7_redirect(BaseWin *self, void *);
void __fastcall base_win_unk5_redirect(BaseWin *self, void *, int a1);
void __fastcall base_win_on_button_toggled_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_down_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_selected_redirect(BaseWin *self, void *, int a1, int a2);

// BaseWin embeds an interface subobject at 0xA14; these four are reached
// through its vtable, so their `this` points there and is adjusted back to
// the BaseWin before dispatching to iface_click, which is not recovered.
typedef void (OriginalObject::*func_base_win_iface_click)(int a1, int a2, int button, int is_double);

// draw_supported is not recovered.
typedef void (OriginalObject::*func_base_win_draw_supported)(int a1);

// The shared click handler these three forward to is not recovered. Unlike
// the iface_click family, these carry no this-adjustment.
typedef void (OriginalObject::*func_base_win_click)(int a1, int a2, int button, int is_double);

void __fastcall base_win_on_iface_left_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_left_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_scrolled_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_left_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_right_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_left_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __cdecl base_win_timer_callback_redirect(int a1, int a2);
void __fastcall base_win_on_mouse_leave_redirect(BaseWin *self, void *, int a1, int a2);

// draw_facilities and garrison_click are declared on the class because the
// recovered on_scrolled and UNK2 reach them with a direct `call rel32`, but
// neither body is recovered - 1788 and 3768 bytes respectively - so the DLL
// has nothing to link the calls against. Both stand in with a forward to the
// original image until the bodies land.
//   ?draw_facilities@BaseWin@@QAEXH@Z    0x0040FCC0  public __thiscall void(int)
//   ?garrison_click@BaseWin@@QAEXHHHH@Z  0x0040B140  public __thiscall
//                                                    void(int, int, int, int)
typedef void (OriginalObject::*func_base_win_draw_facilities)(int a1);

typedef void (OriginalObject::*func_base_win_garrison_click)(int vehID, int a2,
                                                             int a3, int a4);
