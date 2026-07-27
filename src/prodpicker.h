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
  * ProdPicker class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; ProdPicker's own extent is not established and nothing pins its
  * sizeof.
  *
  * Most methods below ignore their arguments and return a constant, which is
  * why they could be replaced ahead of that mapping. close reaches one real
  * field: because GraphicWin is pinned at exactly 0xA14, the dword it clears
  * at 0xA14 is ProdPicker's own first field, which is exact rather than a
  * guess. Nothing past it is established.
  */
class DLLEXPORT ProdPicker : GraphicWin {
 public:
  ProdPicker() { ; }
  ~ProdPicker() { ; }
  int UNK1(int a1);
  void on_mouse_move(int a1, int a2);
  void UNK3(int a1);
  void close();
  void on_redraw_nc(int, int);

 private:
  int32_t field_A14_;   // first field past the pinned GraphicWin base
};

int __fastcall prod_picker_unk1_redirect(ProdPicker *self, void *, int a1);
void __fastcall prod_picker_on_mouse_move_redirect(ProdPicker *self, void *, int a1, int a2);
void __fastcall prod_picker_unk3_redirect(ProdPicker *self, void *, int a1);
void __fastcall prod_picker_close_redirect(ProdPicker *self, void *);
void __fastcall prod_picker_on_redraw_nc_redirect(
    ProdPicker *self, void *, int a1, int a2);
