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
  * bind to. EIGHT window classes - BaseWin, CouncWin, Datalink, DesignWin,
  * DiploPop, DiploWin, MainInterface and SocialWin - embed one at +0xA14 and
  * reach it with `lea ecx, [esi + 0xa14]`, and BattleWin and ReportIf hold
  * theirs at +0. The count is not a guess: the dword 0x0066A6E4 occurs
  * exactly ten times in .text, every one of them the immediate of a
  * `mov dword ptr [...], 0x66a6e4` inside a constructor.
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
  * +0x0 IS THE VFTABLE POINTER, and the paragraph that used to stand here
  * denying it was wrong. No method of this class touches it, which is what
  * misled the earlier reading - but ten constructors WRITE it, all with the
  * same table: `mov dword ptr [esi + 0xa14], 0x66a6e4` in BaseWin
  * (0x004084C1), CouncWin (0x00428651), Datalink (0x00428FF0), DesignWin
  * (0x004344C2), DiploPop (0x0043F021), DiploWin (0x00444FF1), MainInterface
  * (0x0045EF53) and SocialWin (0x004AEA11), and `mov dword ptr [esi],
  * 0x66a6e4` in BattleWin (0x00422EE6) and ReportIf (0x004AD1A7). The table
  * at 0x0066A6E4 is nineteen consecutive `__purecall` slots - an abstract
  * interface's vftable - and the same table landing at +0xA14 in eight
  * classes and at +0 in two is what identifies it as SubInterface's rather
  * than any one of theirs.
  *
  * It stays OPAQUE STORAGE rather than becoming a `virtual` declaration: the
  * three methods are reached by direct `call rel32`, never through a slot, so
  * a C++ vtable here would add a pointer the original installs by hand and
  * change every call site.
  *
  * IS THIS A BASE RATHER THAN A MEMBER? Probably, and the offsets are what
  * says so. In the eight window classes it sits at 0xA14, which is exactly
  * `sizeof(GraphicWin)` - precisely where MSVC puts a SECOND base under
  * multiple inheritance, `class BaseWin : GraphicWin, SubInterface`. In
  * BattleWin and ReportIf it sits at 0, where a sole base goes. And a class
  * whose vftable is nineteen consecutive `__purecall` slots is an ABSTRACT
  * interface, which cannot be held by value as a member at all.
  *
  * It is not DECLARED that way here, for two reasons that are about this
  * tree rather than about the original. `tools/class_layouts.py` refuses
  * multiple inheritance outright rather than guessing at the interleave, so
  * declaring it would push eight window classes out of the layout extractor
  * and back to opaque shells - the opposite of what naming them is for. And
  * nothing yet depends on the distinction: a member declared first after the
  * base lands at the same offset, so no offset in this tree moves either way.
  * The reading is recorded here so that whoever needs the distinction - a
  * body that converts one of these to a `SubInterface *`, or a virtual
  * dispatch through the interface - starts from the evidence rather than
  * rediscovering it.
  *
  * All three methods are unrecovered. They are public, non-virtual
  * __thiscall, void(void) - the mangled names say so - and every call site is
  * a direct `call rel32`, so none of them may be routed through a vtable slot.
  * Two of the three now have FORWARDERS to the original image, which is not a
  * recovery: delete_iface_mode still
  * has no definition, because nothing in the tree calls it.
  */
class SubInterface {
 public:
  void delete_iface_mode();   // 0x0045D2E0  ?delete_iface_mode@SubInterface@@QAEXXZ
  void set_iface_mode();      // 0x0045D310  ?set_iface_mode@SubInterface@@QAEXXZ
  void release_iface_mode();  // 0x0045D380  ?release_iface_mode@SubInterface@@QAEXXZ

 protected:
  uint32_t vtable_;  // 0x0  the vftable at 0x0066A6E4, installed by hand by
                     //      ten constructors; opaque so no C++ vtable is
                     //      generated. No method of this class reads it.
                     //      Protected, not private: the derived constructors
                     //      that install it write it directly.
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
