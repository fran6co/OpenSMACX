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
  * Sound class
  *
  * Layout not established; its methods reach as far as 0x90, so the
  * object is at least that large. The only thing pinned here is the vtable
  * pointer at offset zero, which fade dispatches through; it is held as
  * opaque storage so no C++ vtable is generated that could disagree with the
  * original's. The rest is an object for the canary to seed, not a modelled
  * layout.
  */
class Sound {
 public:
  Sound() { ; }
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E3C0).
  // The compiler now owns the vftable these slots name; Sound's storage no
  // longer carries a hand-rolled table. Slot 1 and the three FX-region slots
  // (24/25/26/30 in the image's table) are unrecovered bodies - placeholder
  // definitions in sound.cpp, semantic debt until named.
  virtual int set_fade(unsigned long fade);        // slot 0 (0x4C6580)
  virtual void unk_slot1();                        // slot 1 (0x4482F0, unrecovered)
  virtual ~Sound();                                // slot 2 (0x4C92D0 ??_GSound)
  virtual int UNK1(int a1);                        // slot 3 (0x4C6430)
  virtual int load(const char *fname);             // slot 4 (0x4C6280)
  virtual int unload();                            // slot 5 (0x4C6440)
  virtual int play(unsigned int effect);           // slot 6 (0x4C64A0)
  virtual int play();                              // slot 7 (0x4C6480)
  virtual int stop();                              // slot 8 (0x4C64D0)
  virtual void fade(unsigned long a1);             // slot 9 (0x4C6600)
  virtual int fade();                              // slot 10 (0x4C65E0)
  virtual void fade_in(unsigned int a1);           // slot 11 (0x4C6660)
  virtual int fade_in();                           // slot 12 (0x4C6620)
  virtual void ramp(int a1, int a2, unsigned int a3);  // slot 13 (0x4C6640)
  virtual int release();                           // slot 14 (0x4C64F0)
  virtual int get_game_hwnd();                     // slot 15 (pure in image; Wave implements)
  virtual void set_volume(int volume);             // slot 16 (0x4C6510)
  virtual void set_pan(int pan);                   // slot 17 (0x4C66B0)
  virtual void set_loop_state(long loop_state);    // slot 18 (0x4C6540)
  virtual void set_delay(unsigned int delay);      // slot 19 (0x4C6560)
  virtual void set_type(unsigned int type);        // slot 20 (0x4C61E0)
  virtual int set_fade_in(unsigned int fade_in);   // slot 21 (0x4C65B0)
  virtual int is_looping();                        // slot 22 (0x4C6690)
  virtual int is_playing();                        // slot 23 (0x4C64C0)
  virtual void unk_slot24();                       // slot 24 (0x448380, unrecovered)
  virtual void unk_slot25();                       // slot 25 (0x448310, unrecovered)
  virtual void unk_slot26();                       // slot 26 (0x448320, unrecovered)
  virtual void unk_slot27();                       // slot 27 (pure in image)
  virtual void unk_slot28();                       // slot 28 (pure in image)
  // THE TAIL, corrected 2026-08-29 against the family tables: the image's
  // Sound table runs to slot 32 with get_time VIRTUAL at 29 (?get_time@@QAEHXZ
  // at 0x4C66A0 sits between the pure pair and unk_slot30 in 0x0066E3C0,
  // 0x0066E8C4 and every family table), attach at 31 and detach at 32. The
  // old tail declared get_time non-virtual after detach, which left Sound a
  // slot short and shifted every derived class's own slots one early.
  virtual int get_time();                          // slot 29 (0x4C66A0)
  virtual void unk_slot30();                       // slot 30 (0x448330, unrecovered)
  virtual int attach();                            // slot 31 (0x4C6370)
  virtual int detach();                            // slot 32 (0x4C63D0)

 protected:
  // PROTECTED, not private, because `Wave` is a real derived class - its
  // own 0x00..0x53 duplicated every one of these fields at the same
  // offsets until 2026-08-22. Access changes no layout and no codegen.
  // The fields below are the slice Wave inherits and mirrors at the same
  // offsets: the loop dword at 0x30, the wrapped device at 0x3C, the flag
  // dword at 0x40 (bit 0 the loaded bit, plus set_type's class bits), the
  // heap-owned filename at 0x4C, and the type at 0x50.
  uint32_t volume_;       // 0x04, low seven bits of set_volume's argument
  int32_t pan_8_;         // 0x08, set_pan's clamp to [-0x40, 0x3F]
  uint32_t field_C_;      // 0x0C, IDB field_C..field_2C for the nine dwords
  uint32_t field_10_;     // 0x10
  uint32_t field_14_;     // 0x14
  uint32_t field_18_;     // 0x18
  uint32_t field_1C_;     // 0x1C
  uint32_t field_20_;     // 0x20
  uint32_t field_24_;     // 0x24
  uint32_t field_28_;     // 0x28
  uint32_t field_2C_;     // 0x2C
  uint32_t loop_flag_30_;  // 0x30, IDB `loop_state`
  uint32_t delay_;        // 0x34, IDB `delay`; set_delay writes it
  uint32_t fade_38_;      // 0x38, the last nonzero fade/fade-in argument
  void *device_;
  uint32_t flags_40_;
  Sound *chain_prev_;     // 0x44, toward the head slot at 0x90DB20
  Sound *chain_next_;     // 0x48, toward the tail slot at 0x90DB1C
  void *fname_;
  uint32_t type_;
};

// Two independent sources agree on 0x54 and neither fails like the other:
// the IDB member table ends at the `type` dword, and the access lower bound
// is the mov [esi + 0x50] at 0x004C6107. The 0x4C tail this header used to
// declare contradicted both and covered nothing - no body reads it and
// nothing embeds a Sound by value.
//
// A THIRD witness landed later, from the other direction: 0x54 is exactly
// where Wave's own fields start (0x004C6774) and where Ambience's do
// (0x004C84xx), because both constructors inline this whole constructor on an
// unadjusted `this` before writing anything of their own. Neither is spelled
// with a Sound base - Sound::~Sound is out of line and a real base would emit
// a call the original inlines - but both begin with one.
static_assert(sizeof(Sound) == 0x54, "Sound layout must match the original executable");

void *__fastcall sound_scalar_dtor_redirect(Sound *self, void *,
                                            unsigned int mode);

// Sound subsystem bring-up, called once from WinMain with the main window and
// a bitmask of the requested backends. `void *` rather than `HWND` because the
// image's own name for it - ?init_sound@@YAHPAXK@Z - spells the first
// parameter `PAX`.
int __cdecl init_sound(void *window, unsigned long backends);

// Loads the sound driver DLL; returns zero on success. Called first thing by
// init_sound. Body not yet recovered.
int __cdecl load_sound_dll();

// The sound DLL's load state, at the addresses the original gave them.
// SoundDllProcs is the global instance at 0x0090DB24: eleven export slots
// load_sound_dll fills in ordinal order (GetProcAddress ordinal 1 lands at
// index 0) and zeroes as a block when the load fails. The tree reads the
// slots individually through typed accessors - wave.h's
// WaveDeviceCreateSlot and WaveDeviceReleaseSlot are indexes 0 and 1,
// wave_device.h's factory and destroy slots are 4 and 5, and
// SoundDllVersionSlot below is index 10, the version probe load_sound_dll
// checks the game's sound headers against. 0x0090DB78 holds the HMODULE
// LoadLibraryA produces - zero until it runs. Accessors rather than real
// storage, like those slot bindings: unrecovered image code reads these
// beside the tree, so they stay image-addressed. The table ends where the
// Wave_In_Device singleton begins (sounddevice.h, 0x0090DB50), and the
// module handle is a separate global above it - neither is a field of the
// other.
typedef void *SoundDllProcsTable[11];
// Returns the table as a plain pointer: VC6 cannot return an array by
// reference (C2101 '&' on constant on any use that decays or addresses it).
inline void **SoundDllProcs() {
  return reinterpret_cast<void **>(0x0090DB24);
}
typedef unsigned int (__cdecl *func_sound_dll_version)();
typedef void (__cdecl *func_sound_dll_init)(int, int);
// The module handle lives at 0x0090DB78, above the Wave_In_Device singleton,
// and STAYS image-addressed: the image reads it from unrecovered functions
// in the 0x4C5D8E and 0x4C5F68 bands (the only .text references, per a byte
// scan of the pinned image), so real storage here would leave those readers
// seeing zero while the tree sees the handle.
inline HMODULE &SoundDllModule() { return *reinterpret_cast<HMODULE *>(0x0090DB78); }
// Two slots of SoundDllProcs, named by role. Index 10 is the version probe
// load_sound_dll checks the game's sound headers against (the image's other
// reference, at 0x4C5FEE, reads the same slot through the table); index 2 is
// the hook init_sound calls once with two zero arguments as soon as the DLL
// is up, before any device is brought onto it.
//
// Both return the raw slot pointer, and the call sites cast it to the
// function-pointer type in ONE level (object pointer to function pointer).
// VC6 refuses the two-level form - reinterpret_cast from void** to F** is
// C2440 "types pointed to are unrelated" - and a call through a returned
// reference or through a materialized register double-indirects
// (`mov eax,[slot]; call [eax]`), where the image calls memory-indirect once
// (`call dword ptr [slot]`).
inline void **SoundDllVersionSlot() { return SoundDllProcs() + 10; }
inline void **SoundDllInitSlot() { return SoundDllProcs() + 2; }
