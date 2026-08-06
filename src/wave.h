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
#include "vector_teardown.h"

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
  * heap-owned filename copy at 0x4C, and a flag byte region at 0x54. The
  * gaps between named fields stay unnamed padding.
  */
class DLLEXPORT Wave {
 public:
  Wave();
  ~Wave();
  int set_asdr();
  void set_volume(int a1);
  int set_fname(const char *a1);
  int play();
  int load();
  int reload();
  void init(char *a1, uint32_t a2);
  int dyna_load(char *a1);
  int load(const char *a1);

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
  int is_hwbuffer();
  int get_time(uint32_t a1);
  int get_current_marker();
  int get_game_hwnd();
  int get_ndevices();
  int UNK1(int a1);
  int set_reverb_mix(float a1);
  int is_3d();
  int get_device_description(char *a1, int a2, int a3);
  int set_position3d(float a1, float a2, float a3);
  int set_xpos(float a1);
  int set_ypos(float a1);
  int set_zpos(float a1);
  void set_attrib(uint32_t a1);
  int get_attrib();
 private:
  uint32_t vtable_storage_;      // 0x00
  uint32_t volume_;              // 0x04, low 7 bits of set_volume's argument
  uint32_t field_8_;
  uint8_t memset_region_[0x24];  // 0x0C..0x2F
  uint32_t field_30_;
  uint32_t field_34_;
  uint32_t field_38_;
  void *device_;                 // 0x3C, the wrapped device or null
  uint32_t field_40_;            // 0x40, bit 0 cleared on unload, bit 1 chained
  Wave *chain_prev_;             // 0x44, chain neighbour; null at the head end
  Wave *chain_next_;             // 0x48, chain neighbour; null at the tail end
  void *fname_;                  // 0x4C, heap-owned filename copy
  uint32_t field_50_;
  uint8_t flags_54_;             // 0x54, bit 1 suppresses the vtable callback
  uint8_t pad_55_[3];
  int32_t pitch_;                // 0x58, clamped semitone offset
  float reverb_mix_;             // 0x5C, stored by set_reverb_mix
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
int __fastcall wave_is_hwbuffer_redirect(Wave *self, void *);
int __fastcall wave_get_time_redirect(Wave *self, void *, uint32_t a1);
int __fastcall wave_get_current_marker_redirect(Wave *self, void *);
int __fastcall wave_get_game_hwnd_redirect(Wave *self, void *);
int __fastcall wave_get_ndevices_redirect(Wave *self, void *);
int __fastcall wave_unk1_redirect(Wave *self, void *, int a1);
int __fastcall wave_set_reverb_mix_redirect(Wave *self, void *, float a1);
int __fastcall wave_is_3d_redirect(Wave *self, void *);
int __fastcall wave_get_device_description_redirect(Wave *self, void *, char *a1, int a2, int a3);
int __fastcall wave_set_position3d_redirect(Wave *self, void *, float a1, float a2, float a3);
int __fastcall wave_set_xpos_redirect(Wave *self, void *, float a1);
int __fastcall wave_set_ypos_redirect(Wave *self, void *, float a1);
int __fastcall wave_set_zpos_redirect(Wave *self, void *, float a1);
void __fastcall wave_set_attrib_redirect(Wave *self, void *, uint32_t a1);
int __fastcall wave_get_attrib_redirect(Wave *self, void *);
void __fastcall wave_set_volume_redirect(Wave *self, void *, int a1);
int __fastcall wave_set_fname_redirect(Wave *self, void *, const char *a1);
int __fastcall wave_play_empty_redirect(Wave *self, void *);
void *__fastcall wave_scalar_dtor_redirect(Wave *self, void *,
                                           unsigned int mode);
int __fastcall wave_load_empty_redirect(Wave *self, void *);
int __fastcall wave_reload_redirect(Wave *self, void *);
int __fastcall wave_dyna_load_redirect(Wave *self, void *, char *a1);
int __fastcall wave_load_fname_redirect(Wave *self, void *, const char *a1);
Wave *__fastcall wave_ctor_redirect(Wave *self, void *);
void __fastcall wave_init_redirect(Wave *self, void *, char *a1, uint32_t a2);

// The destructor's dependencies, each rebindable for the leaf tests. A wave
// still holding one of the 0x10 device group slots is pulled from its group
// by the device singleton at a fixed address; the filename copy was allocated
// by the game CRT, so it must be returned to the game's own operator delete
// rather than ours; the release hook slot runs over the wrapped device when
// the guard dword says the hook is live; and a chained wave unlinks itself
// from the doubly-linked wave chain whose end slots the two chain seams name.
// Which end is "head" is inferred from the unlink shape (the slot written
// when the 0x44 neighbour is null); nothing else pins the labels.
typedef int (OriginalObject::*func_wave_device_pull_from_group)(Wave *wave);
typedef void(__cdecl func_operator_delete)(void *block);
typedef void(__cdecl func_wave_device_release)(void *device);
extern func_wave_device_pull_from_group WaveDevicePullFromGroup;
extern void *WaveDeviceGlobal;
extern func_operator_delete *WaveOperatorDelete;
extern func_wave_device_release **WaveDeviceReleaseSlot;
extern int *WaveDeviceReleaseGuard;
extern Wave **WaveChainHead;
extern Wave **WaveChainTail;

// The rest of the Wave dependency surface. The filename copy is allocated on
// the game CRT heap (the destructor and set_fname free it there, so the new
// must match); the per-group volume table lives inside the device singleton,
// one dword every 24 bytes from 0x0090D9A0; is_group_disabled is the device
// singleton method play consults before starting; and the no-argument load
// is the not-yet-recovered Wave::load() that play falls back to.
// WaveOperatorNew and its typedef now live in "vector_teardown.h" above -
// the seam outgrew this header once GraphicWin::init needed it too.
extern uint32_t *WaveDeviceGroupVolumes;
typedef int (OriginalObject::*func_wave_device_is_group_disabled)(uint32_t slot);
typedef int (OriginalObject::*func_wave_original_load)();
extern func_wave_device_is_group_disabled WaveDeviceIsGroupDisabled;
extern func_wave_original_load WaveOriginalLoad;

// load()'s remaining surface: the device-creation hook lives in the slot
// right beside the release hook, behind the same guard dword, and builds the
// device straight into the wave's 0x3C field; Sound::load is the base-class
// loader - Wave derives from Sound, which is what the destructor's staged
// vtable descent walks - and it is not yet source-owned.
typedef int(__cdecl func_wave_device_create)(void **device_slot,
                                             const char *fname, int mode);
extern func_wave_device_create **WaveDeviceCreateSlot;
typedef int (OriginalObject::*func_sound_original_load)(const char *fname);
extern func_sound_original_load SoundOriginalLoad;
typedef void (OriginalObject::*func_sound_set_type)(uint32_t type);
extern func_sound_set_type SoundSetType;

void __fastcall wave_dtor_redirect(Wave *self, void *);
