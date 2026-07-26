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
  * Standalone. The constructor writes fields at 0, 4, and 8 then memsets
  * 0x24 bytes from 0xC, which places the extent at 0x30 or beyond;
  * g_ALPHAMENU_WAVE's global slot bounds it above at 0x70. A bound is not a
  * size, so nothing pins this and the storage below is the evidenced
  * minimum rather than the object.
  *
  * Most methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they could be replaced ahead of that mapping.
  * unload reaches further, and the fields it uses are named below on the
  * constructor's evidence: it writes a vtable at 0 (held as opaque storage so
  * no C++ vtable is generated), the wrapped device pointer at 0x3C, a flag
  * dword at 0x40 whose bit 0 it clears, and a flag byte region at 0x54. The
  * gaps between named fields stay unnamed padding, and the object's end is
  * still not established.
  */
class DLLEXPORT Wave {
 public:
  Wave() { ; }
  ~Wave() { ; }
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
 private:
  uint32_t vtable_storage_;      // 0x00
  uint32_t field_4_;
  uint32_t field_8_;
  uint8_t memset_region_[0x24];  // 0x0C..0x2F
  uint32_t field_30_;
  uint32_t field_34_;
  uint32_t field_38_;
  void *device_;                 // 0x3C, the wrapped device or null
  uint32_t field_40_;            // 0x40, bit 0 cleared on unload
  uint32_t field_44_;
  uint32_t field_48_;
  uint32_t field_4C_;
  uint32_t field_50_;
  uint8_t flags_54_;             // 0x54, bit 1 suppresses the vtable callback
  uint8_t pad_55_[3];
  int32_t pitch_;                // 0x58, clamped semitone offset
  uint32_t field_5C_;
  int32_t ms_length_;            // 0x60, playing length in milliseconds
  uint32_t start_time_;          // 0x64, timeGetTime stamp when playing began
};

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
