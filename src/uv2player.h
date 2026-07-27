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

 /*
  * UV2Player class
  *
  * Eight methods are catalogued - exec, on_key and four UNKs among them - and
  * only UNK4 is recovered, so NOTHING here pins the layout. There are
  * deliberately no fields, for the same reason as AlphaMenu: UNK4 reads none,
  * and invented storage would be a guess the rest of the tree would inherit.
  */
class DLLEXPORT UV2Player {
 public:
  UV2Player() { ; }
  ~UV2Player() { ; }
  int UNK4();
};

int __fastcall u_v2_player_unk4_redirect(UV2Player *self, void *);
