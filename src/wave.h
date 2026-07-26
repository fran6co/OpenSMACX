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
  * Wave class
  *
  * Standalone. The extent is pinned at 0x6C by three independent witnesses:
  * the atexit array thunks walk Wave arrays with a 0x6C element stride, the
  * FX effect bank uses the same stride, and the destructor itself reads the
  * group slot at 0x68 as its last field.
  *
  * The vtable pointer at 0 is held as opaque storage so no C++ vtable is
  * generated; the original installs fixed vtable addresses itself (the
  * constructor writes one, the destructor republishes three as it descends
  * the inlined hierarchy). The wrapped device pointer sits at 0x3C, a flag
  * dword at 0x40 (bit 0 the loaded bit, bit 1 set while the wave is linked
  * into the global wave chain), the chain neighbours at 0x44/0x48, the
  * heap-owned sample buffer at 0x4C, and a flag byte region at 0x54. The
  * gaps between named fields stay unnamed padding.
  */
class DLLEXPORT Wave {
 public:
  Wave() { ; }
  ~Wave();
  int set_asdr();

  int set_bufflimit(unsigned int a1);
  int set_attack(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_sustain(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_decay(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_release(unsigned int a1, unsigned int a2, unsigned int a3);
  int unload();
  void set_pitch(int a1);
  int load(int a1, int a2);
  int get_ms_length();
  int is_playing();
  int play(int a1);
 private:
  uint32_t vtable_storage_;      // 0x00
  uint32_t field_4_;
  uint32_t field_8_;
  uint8_t memset_region_[0x24];  // 0x0C..0x2F
  uint32_t field_30_;
  uint32_t field_34_;
  uint32_t field_38_;
  void *device_;                 // 0x3C, the wrapped device or null
  uint32_t field_40_;            // 0x40, bit 0 cleared on unload, bit 1 chained
  Wave *chain_prev_;             // 0x44, chain neighbour; null at the head end
  Wave *chain_next_;             // 0x48, chain neighbour; null at the tail end
  void *buffer_;                 // 0x4C, heap block freed on destruction
  uint32_t field_50_;
  uint8_t flags_54_;             // 0x54, bit 1 suppresses the vtable callback
  uint8_t pad_55_[3];
  int32_t pitch_;                // 0x58, clamped semitone offset
  uint32_t field_5C_;
  int32_t ms_length_;            // 0x60, playing length in milliseconds
  uint32_t start_time_;          // 0x64, timeGetTime stamp when playing began
  uint32_t group_slot_;          // 0x68, pulled from its device group when < 0x10
};

static_assert(sizeof(Wave) == 0x6C, "Wave layout must match terranx.exe");

int __fastcall wave_set_asdr_redirect(Wave *self, void *);
int __fastcall wave_set_bufflimit_redirect(Wave *self, void *, unsigned int a1);
int __fastcall wave_set_attack_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_sustain_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_decay_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_set_release_redirect(Wave *self, void *, unsigned int a1, unsigned int a2, unsigned int a3);
int __fastcall wave_unload_redirect(Wave *self, void *);
void __fastcall wave_set_pitch_redirect(Wave *self, void *, int a1);
int __fastcall wave_load_redirect(Wave *self, void *, int a1, int a2);
int __fastcall wave_get_ms_length_redirect(Wave *self, void *);

// With no wrapped device the wave is timed against the clock, through the
// game's imported timeGetTime. The seam is the address of that import slot -
// the IAT entry the original calls indirectly - so it reads the live pointer
// at run time and stays rebindable for tests.
typedef DWORD(__stdcall func_time_get_time)(void);
extern func_time_get_time **WaveTimeGetTimeSlot;

int __fastcall wave_is_playing_redirect(Wave *self, void *);
int __fastcall wave_play_redirect(Wave *self, void *, int a1);

// The destructor's dependencies, each rebindable for the leaf tests. A wave
// still holding one of the 0x10 device group slots is pulled from its group
// by the device singleton at a fixed address; the sample buffer was allocated
// by the game CRT, so it must be returned to the game's own operator delete
// rather than ours; the release hook slot runs over the wrapped device when
// the guard dword says the hook is live; and a chained wave unlinks itself
// from the doubly-linked wave chain whose end slots the two chain seams name.
// Which end is "head" is inferred from the unlink shape (the slot written
// when the 0x44 neighbour is null); nothing else pins the labels.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef int(__thiscall func_wave_device_pull_from_group)(void *device,
                                                         Wave *wave);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
typedef void(__cdecl func_operator_delete)(void *block);
typedef void(__cdecl func_wave_device_release)(void *device);
extern func_wave_device_pull_from_group *WaveDevicePullFromGroup;
extern void *WaveDeviceGlobal;
extern func_operator_delete *WaveOperatorDelete;
extern func_wave_device_release **WaveDeviceReleaseSlot;
extern int *WaveDeviceReleaseGuard;
extern Wave **WaveChainHead;
extern Wave **WaveChainTail;

void __fastcall wave_dtor_redirect(Wave *self, void *);
