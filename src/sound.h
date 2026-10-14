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

#include "vc6_compat.h"
#include <cstring>
#include <new>

// Provisional behavioral name for the original base published at 0x0066E444.
// Sound construction initializes this prefix before installing its own table;
// destruction and the shared unwind cleanup restore the same two-slot table.
// Both original slots select 0x004482F0. That method reaches fade_38_, while
// Sound's first additional field is device_ at 0x3C.
class SoundFadeState {
 public:
  SoundFadeState();
  virtual int set_fade(unsigned long interval);           // root slot 0
  virtual int set_fade_interval(unsigned long interval);  // root slot 1

 protected:
  // Nonvirtual: the original root table has no deleting-destructor slot.
  // This real destructor lets the compiler own the final base-table store
  // and the derived constructor's exception cleanup.
  ~SoundFadeState() {}

  uint32_t volume_;       // 0x04
  int32_t pan_8_;         // 0x08
  uint32_t state_C_[9];   // 0x0C..0x2F, zeroed block; element roles unknown
  uint32_t loop_flag_30_; // 0x30
  uint32_t delay_;        // 0x34, left untouched by construction
  uint32_t fade_38_;      // 0x38, remembered nonzero fade interval
};

// RECOVERY_MEMBER: v1 SoundFadeState volume_ 0x04 0x004C6080
// RECOVERY_MEMBER: v1 SoundFadeState pan_8_ 0x08 0x004C6080
// RECOVERY_MEMBER: v1 SoundFadeState state_C_ 0x0C 0x004C6080
// RECOVERY_MEMBER: v1 SoundFadeState loop_flag_30_ 0x30 0x004C6080
// RECOVERY_MEMBER: v1 SoundFadeState delay_ 0x34 0x004C6560
// RECOVERY_MEMBER: v1 SoundFadeState fade_38_ 0x38 0x004482F0
static_assert(sizeof(SoundFadeState) == 0x3C, "Sound fade prefix extent");

// Sound owns the common 0x54-byte state inherited by the audio classes.
// The compiler owns its primary virtual table; incomplete method contracts
// below remain recovery obligations, not an opaque substitute for that table.
// RECOVERY_BASE: v1 Sound SoundFadeState 0x00 0x004C6080
class Sound : public SoundFadeState {
  friend int __cdecl release_sound();
  // Both consumers rethread these common links in the original: Wave's
  // destruction and Wave_Device::select (0x004C5030). Friendship is this
  // reconstruction's access policy, not an inferred original access label.
  friend class Wave;
  friend class Wave_Device;
 public:
  Sound();
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E3C0).
  // The compiler now owns the vftable these slots name; Sound's storage no
  // longer carries a hand-rolled table. Slots 15, 27 and 28 remain pure
  // here and are supplied by real derived owners.
  virtual int set_fade(unsigned long fade);        // slot 0 (0x4C6580)
  // Slot 1 is inherited from SoundFadeState (0x4482F0).
  virtual ~Sound();                                // slot 2 (0x4C92D0 ??_GSound)
  virtual int UNK1(int a1);                        // slot 3 (0x4C6430)
  virtual int load(const char *fname);             // slot 4 (0x4C6280)
  virtual int unload();                            // slot 5 (0x4C6440)
  // VC6 emits an overload group in reverse declaration order. These two
  // declarations must still produce original slots 6 (argument) and 7 (none).
  virtual int play();                              // slot 7 (0x4C6480)
  virtual int play(unsigned int effect);           // slot 6 (0x4C64A0)
  virtual int stop();                              // slot 8 (0x4C64D0)
  virtual int fade();                              // slot 10 (0x4C65E0)
  virtual void fade(unsigned long a1);             // slot 9 (0x4C6600)
  virtual int fade_in();                           // slot 12 (0x4C6620)
  virtual void fade_in(unsigned int a1);           // slot 11 (0x4C6660)
  virtual void ramp(int a1, int a2, unsigned int a3);  // slot 13 (0x4C6640)
  virtual int release();                           // slot 14 (0x4C64F0)
  virtual int get_game_hwnd() = 0;                 // slot 15 is pure in the pinned Sound table
  virtual void set_volume(int volume);             // slot 16 (0x4C6510)
  virtual void set_pan(int pan);                   // slot 17 (0x4C66B0)
  virtual void set_loop_state(long loop_state);    // slot 18 (0x4C6540)
  virtual void set_delay(unsigned int delay);      // slot 19 (0x4C6560)
  virtual void set_type(unsigned int type);        // slot 20 (0x4C61E0)
  virtual int set_fade_in(unsigned int fade_in);   // slot 21 (0x4C65B0)
  virtual int is_looping();                        // slot 22 (0x4C6690)
  virtual int is_playing();                        // slot 23 (0x4C64C0)
  virtual int unk_slot24();                        // slot 24 (0x448380)
  virtual Sound *get_next();                       // slot 25 (0x448310)
  virtual Sound *get_prev();                       // slot 26 (0x448320)
  // Pure in the original base; Wave/Effect select 0x004C6F20, whose
  // stack argument is the mask and whose common return clears eax.
  virtual int set_attrib(unsigned long attrib) = 0; // slot 27 is pure in the pinned Sound table
  // Pure in the original base table. Wave's table selects get_attrib at
  // 0x004C6F80; Wave_Device::select consumes the integer result twice.
  virtual int get_attrib() = 0;                   // slot 28 is pure in the pinned Sound table
  // THE TAIL, corrected 2026-08-29 against the family tables: the image's
  // Sound table runs to slot 32 with get_time VIRTUAL at 29 (?get_time@@QAEHXZ
  // at 0x4C66A0 sits between the pure pair and get_type in 0x0066E3C0,
  // 0x0066E8C4 and every family table), attach at 31 and detach at 32. The
  // old tail declared get_time non-virtual after detach, which left Sound a
  // slot short and shifted every derived class's own slots one early.
  virtual int get_time();                          // slot 29 (0x4C66A0)
  virtual uint32_t get_type();                     // slot 30 (0x448330)
  virtual int attach();                            // slot 31 (0x4C6370)
  virtual int detach();                            // slot 32 (0x4C63D0)

 protected:
  // The fade prefix is inherited. Sound owns the wrapped device, loaded/
  // chain flags, linked-list neighbors, filename allocation and sound type.
  void *device_;         // 0x3C
  uint32_t flags_40_;
  Sound *chain_prev_;     // 0x44, toward the head slot at 0x90DB20
  Sound *chain_next_;     // 0x48, toward the tail slot at 0x90DB1C
  char *fname_;           // 0x4C: owned filename or Voice display-label copy
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
// unadjusted `this` before writing anything of their own. Both now declare
// that Sound base; original inlining is not a reason to flatten the hierarchy.
static_assert(sizeof(Sound) == 0x54, "Sound layout must match the original executable");

// Common active-sound chain, not a Wave-only collection. Sound::attach
// (0x004C6370) establishes the ends; detach/destruction maintain them.
// Definitions and bounded original storage bindings live in sound.cpp.
extern Sound *SoundChainHead;
extern Sound *SoundChainTail;



// Sound subsystem bring-up, called once from WinMain with the main window and
// a bitmask of the requested backends. `void *` rather than `HWND` because the
// image's own name for it - ?init_sound@@YAHPAXK@Z - spells the first
// parameter `PAX`.
int __cdecl init_sound(void *window, unsigned long backends);

// Loads the sound driver DLL; returns zero on success. Called first thing by
// init_sound. Body not yet recovered.
int __cdecl load_sound_dll();

// The sound DLL's load state, at the addresses the original gave them.
// THE SOUND DLL'S EXPORT TABLE AND MODULE HANDLE - plain global
// instances, the original's own spelling (per direction 2026-09-03, after
// accessor-function and name-macro interims both measured wrong-shaped
// semantically however they measured bytewise). load_sound_dll fills
// SoundDllModule once (null means the DLL is absent or was unloaded) and
// keeps the LoadLibraryA reference there; SoundDllProcs is the eleven
// slots it fills in ordinal order (GetProcAddress ordinal 1 lands at
// index 0) and zeroes as a block on failure. The tree reads slots
// individually by role through the typed value-loading accessors below.
// The original provider objects behind these exports are DLL-private opaque
// interfaces, not the game's Wave_Device, Midi_Device, or Wave_In_Device
// facades. Definitions in sound_loader.cpp.
//
// BYTES: a use is one deref with the address a relocation to our storage -
// the image's own shape with its own relocation - which is why the natural
// spelling is also the faithful one (init_sound BYTE_EXACT through it).
// What measurably does NOT work is any BINDING whose value is the image's
// address: a const-pointer or reference global double-derefs because VC6
// never folds it (mov eax,[binding]; cmp [eax],0 against the image's one
// load), and the array reference is C2101. The image's own readers of the
// 0x0090DB24/0x0090DB78 words (the unrecovered 0x4C5D8E/0x4C5F68 bands)
// read the shipped binary's storage until they are recovered.
typedef void *SoundDllProcsTable[11];
extern SoundDllProcsTable SoundDllProcs;
extern HMODULE SoundDllModule;

// Pinned soundx.dll exports ordinals 1..11 form one fixed callable contract.
// Keep their physically observed storage as eleven opaque pointer words, but
// centralize each ABI conversion here. These accessors return the current
// slot value on every call; they do not cache DLL-private provider objects or
// turn them into game-side SoundDeviceBase subclasses.
typedef int (__cdecl *SoundXCreateSoundProc)(void **, const char *, int);
typedef int (__cdecl *SoundXDeleteSoundProc)(void *);
typedef int (__cdecl *SoundXInitTimerProc)(unsigned long, unsigned long);
typedef int (__cdecl *SoundXReleaseSoundProc)();
typedef int (__cdecl *SoundXWaveDeviceCreateProc)(void **, unsigned long);
typedef int (__cdecl *SoundXWaveDeviceDeleteProc)();
typedef int (__cdecl *SoundXMidiDeviceCreateProc)(void **, unsigned long);
typedef int (__cdecl *SoundXMidiDeviceDeleteProc)();
typedef int (__cdecl *SoundXWaveInDeviceCreateProc)(void **, unsigned long);
typedef int (__cdecl *SoundXWaveInDeviceDeleteProc)();
typedef unsigned int (__cdecl *SoundXVersionProc)();

inline SoundXCreateSoundProc SoundXOrdinal1CreateSound() {
  return reinterpret_cast<SoundXCreateSoundProc>(SoundDllProcs[0]);
}
inline SoundXDeleteSoundProc SoundXOrdinal2DeleteSound() {
  return reinterpret_cast<SoundXDeleteSoundProc>(SoundDllProcs[1]);
}
inline SoundXInitTimerProc SoundXOrdinal3InitTimer() {
  return reinterpret_cast<SoundXInitTimerProc>(SoundDllProcs[2]);
}
inline SoundXReleaseSoundProc SoundXOrdinal4ReleaseSound() {
  return reinterpret_cast<SoundXReleaseSoundProc>(SoundDllProcs[3]);
}
inline SoundXWaveDeviceCreateProc SoundXOrdinal5WaveDeviceCreate() {
  return reinterpret_cast<SoundXWaveDeviceCreateProc>(SoundDllProcs[4]);
}
inline SoundXWaveDeviceDeleteProc SoundXOrdinal6WaveDeviceDelete() {
  return reinterpret_cast<SoundXWaveDeviceDeleteProc>(SoundDllProcs[5]);
}
inline SoundXMidiDeviceCreateProc SoundXOrdinal7MidiDeviceCreate() {
  return reinterpret_cast<SoundXMidiDeviceCreateProc>(SoundDllProcs[6]);
}
inline SoundXMidiDeviceDeleteProc SoundXOrdinal8MidiDeviceDelete() {
  return reinterpret_cast<SoundXMidiDeviceDeleteProc>(SoundDllProcs[7]);
}
inline SoundXWaveInDeviceCreateProc SoundXOrdinal9WaveInDeviceCreate() {
  return reinterpret_cast<SoundXWaveInDeviceCreateProc>(SoundDllProcs[8]);
}
inline SoundXWaveInDeviceDeleteProc SoundXOrdinal10WaveInDeviceDelete() {
  return reinterpret_cast<SoundXWaveInDeviceDeleteProc>(SoundDllProcs[9]);
}
inline SoundXVersionProc SoundXOrdinal11Version() {
  return reinterpret_cast<SoundXVersionProc>(SoundDllProcs[10]);
}

// Product wrappers around the loader-owned module/export table.
void __cdecl unload_sound_dll();
int __cdecl init_sound_timer(unsigned long timer, unsigned long resolution);
int __cdecl get_sound_version();
// The accessors' value conversions are the unavoidable DLL boundary. Every
// invocation reloads the current opaque table cell; the loader's ordinal
// storage and ordering remain unchanged.

int __cdecl release_sound();

int __cdecl unload_active_sound_chain();

// All derived translation units must see the same inline base teardown.
// The canonical original identity and shared cleanup spans remain in sound.cpp.
extern int WaveDeviceReleaseGuard;
inline Sound::~Sound() {
    // The original closes with the root table at 0x004C61C1 and has an
    // exception cleanup at 0x004C8450. SoundFadeState owns both lifetime
    // effects; do not install a table by hand or flatten this base to alter
    // the destructor's code shape. Remaining hooks/chain globals still need
    // independent recovery before the whole sound lifetime is accepted.
    if (fname_) {
        operator delete(fname_);
        fname_ = nullptr;
    }
    if (device_) {
        if (WaveDeviceReleaseGuard) {
            (SoundXOrdinal2DeleteSound())(device_);
        }
        device_ = nullptr;
    }
    if (flags_40_ & 2) {
        Sound *const prev = chain_prev_;
        if (prev) {
            prev->chain_next_ = chain_next_;
        } else {
            SoundChainHead = chain_next_;
        }
        Sound *const next = chain_next_;
        if (next) {
            next->chain_prev_ = chain_prev_;
        } else {
            SoundChainTail = chain_prev_;
        }
        chain_next_ = nullptr;
        chain_prev_ = nullptr;
        flags_40_ &= ~2u;
    }
}
