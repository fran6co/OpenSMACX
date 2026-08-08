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
  * THE LAYOUT IS MEASURED. An earlier version of this header carried no data
  * member and said one would need "its own evidence" - the evidence was
  * already in the image and had simply not been read. Disassembling this
  * class's own methods and following `this` out of ECX:
  *
  *     ?set_iface_mode@SubInterface@@QAEXXZ      0x0045D310  4 bytes at +0x4
  *     ?release_iface_mode@SubInterface@@QAEXXZ  0x0045D380  4 bytes at +0x4
  *     ?delete_iface_mode@SubInterface@@QAEXXZ   0x0045D2E0  touches nothing
  *
  * so the object is AT LEAST 8 bytes, which is independently what the IDB
  * records. Code that indexes a member proves the member is there.
  *
  * +0x0 is NOT proved: no method of this class touches it. It is a dword
  * because the size says so and because +0x4 is dword-aligned, and it is
  * named for its offset rather than given a meaning it has not earned. It is
  * NOT a vtable pointer - every call site reaches these methods with
  * `lea ecx, [esi+0xa14]; call rel32`, a direct call, never through a slot.
  *
  * NOT DLLEXPORT: all three methods are unrecovered, and dllexport on a class
  * demands a definition for every member. They are public, non-virtual
  * __thiscall, void(void) - the mangled names say so - and every call site is
  * a direct `call rel32`, so none of them may be routed through a vtable slot.
  * Two of the three now have FORWARDERS to the original image, which is not a
  * recovery and does not make the class exportable: delete_iface_mode still
  * has no definition, because nothing in the tree calls it.
  */
class SubInterface {
 public:
  void delete_iface_mode();   // 0x0045D2E0  ?delete_iface_mode@SubInterface@@QAEXXZ
  void set_iface_mode();      // 0x0045D310  ?set_iface_mode@SubInterface@@QAEXXZ
  void release_iface_mode();  // 0x0045D380  ?release_iface_mode@SubInterface@@QAEXXZ

 private:
  uint32_t field_0_;  // 0x0  not touched by any method of this class
  uint32_t field_4_;  // 0x4  PROVED: set_iface_mode and release_iface_mode
                      //      both access 4 bytes here
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(SubInterface) == 0x8,
              "SubInterface layout must match the legacy ABI");
#endif

/*
 * Seams for the two methods the tree calls. Both are void(void) __thiscall, so
 * one pointer-to-member type covers both.
 *
 * The DEFINITIONS live at the end of src/reportif.cpp rather than in a
 * subinterface.cpp of their own: this class has no translation unit, and
 * adding one is a CMakeLists change - a shared file. reportif.cpp is the
 * nearest owner that already sees this header, through reportif.h, and
 * already calls release_iface_mode from ReportIf::done.
 *
 * Distinct from statuswin.h's SubInterfaceOriginalReleaseIfaceMode, which
 * names the same address for a different job: that one is the seam StatusWin's
 * reset drives against a global receiver, and the tests rebind it. Sharing one
 * global between the two would make a test that repoints the observer silently
 * repoint every SubInterface::release_iface_mode call in the program.
 */
typedef void (OriginalObject::*func_sub_interface_iface_mode)();
extern func_sub_interface_iface_mode SubInterfaceSetIfaceMode;      // 0x0045D310
extern func_sub_interface_iface_mode SubInterfaceReleaseIfaceMode;  // 0x0045D380
