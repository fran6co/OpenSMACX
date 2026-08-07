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
  * SubInterface class: the interface-mode registry entry.
  *
  * Promoted out of the generated src/hypothesis_layouts.h so that the
  * recovered bodies that already name this type have a real declaration to
  * bind to. Five window classes - BaseWin, CouncWin, Datalink, DiploWin and
  * SocialWin - embed one at +0xA14 and reach it with `lea ecx, [esi + 0xa14]`,
  * and ReportIf reaches its own at `this` unadjusted, so every use in the tree
  * is a reinterpret_cast of an address.
  *
  * NOT a modelled layout, and deliberately carries no data member: nothing
  * recovered dereferences a field, nobody holds one by value, and no member
  * declared here could move a byte in the classes that embed it. The IDB
  * records the object as 0x8 bytes (a vtable slot and one dword); should a
  * class ever need to hold one by value, that is a layout change and needs its
  * own evidence.
  *
  * NOT DLLEXPORT: all three methods are unrecovered, and dllexport on a class
  * demands a definition for every member. They are public, non-virtual
  * __thiscall, void(void) - the mangled names say so - and every call site is
  * a direct `call rel32`, so none of them may be routed through a vtable slot.
  */
class SubInterface {
 public:
  void delete_iface_mode();   // 0x0045D2E0  ?delete_iface_mode@SubInterface@@QAEXXZ
  void set_iface_mode();      // 0x0045D310  ?set_iface_mode@SubInterface@@QAEXXZ
  void release_iface_mode();  // 0x0045D380  ?release_iface_mode@SubInterface@@QAEXXZ
};
