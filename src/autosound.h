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

class Win;  // the window this record is embedded at offset 0 of
class Sound;

 /*
  * AutoSound - the PROCESS SETTINGS RECORD, embedded at offset 0 of every
  * window (class Win : public AutoSound). 37 words behind one vfptr; the
  * constructor copies them wholesale from AutoSoundDefaults, the mutable
  * 37-field record at [0x009BC080,0x009BC114). Its typed fields follow original
  * global-word order, including the 1,3,4,2 leading permutation. Loader
  * zero-initialization supplies initial values; FX::init optionally resets
  * them later.
  *
 * FIELD MAP (offset = 0x04 + 4*(N-1)). The recovered Win, Scroll, Menu,
 * ListBox and BaseButton paths use numbered values through Sound's nullary
 * vtable slots 23 and 7 (pinned Sound table: is_playing/play; byte offsets
 * 0x5c/0x1c). Win slot 23 has a different two-coordinate contract. These
 * call shapes establish the Sound interface view at those uses, not a
 * concrete subtype, ownership, lifetime, or a semantic setting name.
 *
 * Win uses val_10_/11_/12_ through Sound receiver views. The former
 * FlatButton Sprite interpretation came from an erroneous Scroll setter:
 * native60569A..6056A6 writes FlatButton+AD4/+AD8/+ADC, not these words.
 * These three fields retain the evidenced Sound pointer interface; their
 * complete producer/ownership/lifetime contracts remain unresolved. The
 * Win Sound receiver at +0x50 is val_20_; the ListBox flag is Win's separate
 * +0x98 member. The BaseButton Sound receiver at +0x7C is val_31_; the
 * ListBox notification target is Win's separate +0xC4 parent link. The
 * defaults record uses the same destination types; this establishes no
 * ownership or lifetime for non-null Sound values. Other meanings stay numbered
 * until their writers and use families are recovered. IDB independently
 * numbered the words identically (docs/recovery/idb-members.csv: val1..val37).
  */
class AutoSound {
 public:
  AutoSound();
  // VIRTUAL, so VC6 emits the vfptr at offset 0 - Win's and GraphicWin's
  // too, since Win derives from here (radiobutton.h measured this as the
  // second of the two missing base vtable pointers; Dialog supplied the
  // first).
  //
  // THE BODY IS CLOSE, MEASURED: the image's deleting destructor
  // ??_GAutoSound@@UAEPAXI@Z (36 bytes) restores the vftable, runs close(),
  // then frees conditionally on the caller's flag - which is byte-for-byte
  // what cl emits for a virtual destructor whose body runs close(). The
  // compiler owns ??_G; nothing in the tree names it or the vftable any
  // more.
  virtual ~AutoSound() { close(); }
  // Returns `this`: the image's body opens `mov eax, ecx` and uses EAX as
  // the object base for all 38 stores, which is what a __thiscall that has
  // to leave `this` in EAX does. Declared `void`, VC6 keeps the base in ECX
  // and scores 0 of 77.
  void close();
  void close2();
  void init();
  // ??_GAutoSound@@UAEPAXI@Z as a member: re-installs the vfptr, runs
  // close(), deletes only when the caller's flag says so.
  void *scalar_delete(unsigned int mode);

  // Scroll's pinned paths use this word through Sound slots 23/7. This is an
  // observed interface view, not an ownership or lifetime claim.
  Sound *legacy_sound_33() const { return val_33_; }

  // BaseButton's pinned consumers use val_31_ through Sound slots 23 and 7.
  // This establishes its receiver interface, not its provider or lifetime.
  Sound *legacy_sound_31() const { return val_31_; }
  Sound *legacy_sound_36() const { return val_36_; }
  Sound *legacy_sound_37() const { return val_37_; }
  Sound *legacy_sound_28() const { return val_28_; }
  Sound *legacy_sound_29() const { return val_29_; }
  Sound *legacy_sound_30() const { return val_30_; }
  Sound *event_window() const { return event_window_; }

 protected:
  // PROTECTED, NOT PRIVATE: Win derives from AutoSound and the homed window
  // bodies read these directly - which is what the image does, since they are
  // one object. `private` modelled a containment that the inheritance edge
  // replaced.
  // The vfptr is EMITTED by the compiler now - see the virtual destructor
  // above. Same four bytes at offset 0, so sizeof(AutoSound) stays 0x98.
  int val_1_;  // 0x0004
  int val_2_;
  Sound *val_3_;
  // Pinned Win::hide 0x5EDCD0 uses Sound slots 23/7 at this offset.
  // RECOVERY_MEMBER: v3 AutoSound val_4_ Sound* 0x10 0x005EDCD0
  Sound *val_4_;
  int val_5_;
  Sound *val_6_;
  Sound *val_7_;
 protected:
  // Win dispatch reads Sound slots 23/7 at +0x20. The old BasePop flag
  // attribution used the wrong receiver: 0x605180/0x6051A0 operate on the
  // embedded Dialog virtual base's separate +0x20 style word.
  // This interface view does not establish ownership or lifetime.
  Sound *val_8_;
  // PROTECTED for the same reason the block above says: Win derives from
  // AutoSound and its homed bodies read these fields. They were reached by
  // raw offset off `reinterpret_cast<char *>(this)` because `private` made
  // the member form uncompilable - the access happened either way, it just
  // could not say what it was touching.
 protected:
  Sound *val_9_;
  // RECOVERY_MEMBER: v3 AutoSound val_10_ Sound* 0x28 0x005EE810
  Sound *val_10_;  // Sound receiver view, not a lifetime/ownership claim.
  // RECOVERY_MEMBER: v3 AutoSound val_11_ Sound* 0x2C 0x005EE8B0
  Sound *val_11_;
  // RECOVERY_MEMBER: v3 AutoSound val_12_ Sound* 0x30 0x005EE950
  Sound *val_12_;
  Sound *val_13_;
  Sound *val_14_;
  Sound *val_15_;
  Sound *val_16_;
  Sound *val_17_;
  Sound *val_18_;
  Sound *val_19_;
  // Win dispatches through Sound slots 23/7 at +0x50.
  // RECOVERY_MEMBER: v3 AutoSound val_20_ Sound* 0x50 0x005F6550
  Sound *val_20_;
  Sound *val_21_;
  Sound *val_22_;
  Sound *val_23_;
  Sound *val_24_;
  Sound *val_25_;
  Sound *val_26_;
  Sound *val_27_;
  Sound *val_28_;
  Sound *val_29_;
  Sound *val_30_;
  // BaseButton's receiver uses Sound slots 23/7; ListBox's parent link is
  // the separate Win member at +0xC4.
  // RECOVERY_MEMBER: v3 AutoSound val_31_ Sound* 0x7C 0x00607A20
  Sound *val_31_;
  int val_32_;
  // Scroll uses Sound slots 23/7 at +0x84.
  // RECOVERY_MEMBER: v3 AutoSound val_33_ Sound* 0x84 0x006069A0
  Sound *val_33_;
  // Menu's original paths use Sound's slot pair 23/7 at this receiver.
  // The historical field label is retained without claiming ownership.
  // RECOVERY_MEMBER: v3 AutoSound event_window_ Sound* 0x88 0x005FB830
  Sound *event_window_;  // 0x88
  // RECOVERY_MEMBER: v3 AutoSound val_35_ Sound* 0x8C 0x005F2830
  Sound *val_35_;
  // ListBox uses Sound slots 23/7 at this offset.
  // RECOVERY_MEMBER: v3 AutoSound val_36_ Sound* 0x90 0x0060A020
  Sound *val_36_;
  // ListBox double-click reads this Sound receiver from its Win parent at
  // +0x94; that parent is followed through Win's separate +0xC4 link.
  // RECOVERY_MEMBER: v3 AutoSound val_37_ Sound* 0x94 0x0060C710
  Sound *val_37_;
};

static_assert(sizeof(AutoSound) == 0x98,
              "AutoSound layout must match the legacy ABI");


// The game CRT operator delete the scalar deleting destructor frees the
// object through; bound here rather than through wave.h so this file's
// link closure stays self-contained.

// Returns `int`, not `void`: the body is `xor eax, eax; ret`, where a void
// function would emit `ret` alone. Corrected in tools/catalogue_corrections.py.

MEASURED inline int __cdecl do_sound() {
    return 0;
}
