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
  AlphaMovie() { ; }
  ~AlphaMovie() { ; }
  void UNK7();
  int UNK2(int a1, int a2, int a3);
  int UNK4(int a1, int a2, int a3, int a4);
  int UNK5(int a1);
  void UNK6(int a1, int a2);
  void UNK6(int a1);
  int UNK8(int a1, int a2);
};

void __fastcall alpha_movie_unk7_redirect(AlphaMovie *self, void *);
int __fastcall alpha_movie_unk2_redirect(AlphaMovie *self, void *, int a1, int a2, int a3);
int __fastcall alpha_movie_unk4_redirect(AlphaMovie *self, void *, int a1, int a2, int a3, int a4);
int __fastcall alpha_movie_unk5_redirect(AlphaMovie *self, void *, int a1);
void __fastcall alpha_movie_unk6_00404260_redirect(AlphaMovie *self, void *, int a1, int a2);
void __fastcall alpha_movie_unk6_00404270_redirect(AlphaMovie *self, void *, int a1);
int __fastcall alpha_movie_unk8_redirect(AlphaMovie *self, void *, int a1, int a2);
