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
#include "wave_device.h"
#include "sound.h"
#include "vector_teardown.h"

 /*
  * Wave class
  *
  * OFFSETS 0x00..0x53 ARE A SOUND SUB-OBJECT, inlined rather than declared.
  * ??0Wave@@QAE@XZ (0x004C66E0) replays ??0Sound@@QAE@XZ (0x004C6080) store
  * for store on an unadjusted `this` - the same two vftable writes, the same
  * `memset(this + 0xC, 0, 0x24)`, the same 0x3E8 into 0x38, the same guarded
  * call through [this + 0x3C] - and only then writes its own vftable at
  * 0x004C676E and its own first field at 0x004C6774, which is 0x54. That is
  * exactly where src/sound.h now pins sizeof(Sound).
  *
  * WITHDRAWN 2026-08-22. This said Wave "stays spelled FLAT, not
  * `class Wave : Sound`", to protect a `~Wave` that was "already recovered".
  * `~Wave` was NOT recovered - it measured 2 of 101 - so the reason had
  * outlived itself, and the flat spelling was costing both bodies almost
  * everything. Wave IS a Sound: its 0x00..0x53 duplicated every one of
  * Sound's fields at the same offsets, names and all, and sizeof(Sound)
  * 0x54 + Wave's own 0x18 is exactly the pinned 0x6C.
  *
  * Declaring the real base restores the SEH UNWIND FRAME the image has and
  * this tree could not produce: constructing a base with a non-trivial
  * destructor, then calling something VC6 cannot prove nothrow, is what
  * emits it. All twelve prologue instructions now agree.
  * Wave::Wave 3/73 -> 23/73, ~Wave 2/101 -> 44/101.
  *
  * The withdrawn note, for the record: Sound::~Sound is
  * out-of-line, so a real base would make ~Wave emit a `call` where the
  * original inlines the teardown, breaking a body that is already recovered.
  * base-edges.csv records this edge as `unchecked` for the same reason its
  * evidence line gives - the constructor "builds nothing this catalogue
  * names" because the base constructor was inlined away.
  *
  * The extent is pinned at 0x6C by three independent witnesses:
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
  * heap-owned filename copy at 0x4C, and a SECOND FLAG DWORD at 0x54. The
  * gaps between named fields stay unnamed padding.
  *
  * That last one used to be spelled `uint8_t flags_54_` plus three pad bytes
  * and described here as "a flag byte region". WITHDRAWN - the image reads
  * and writes 0x54 four bytes at a time, in four independent places:
  *   - both initialisers clear it with a FOUR-byte memset, not a one-byte
  *     store: `push 4 / lea eax,[esi+0x54] / push 0 / push eax / call memset`
  *     in ??0Wave@@QAE@XZ at 0x004C6764 and the same shape in
  *     ?init@Wave@@QAEXPADK@Z at 0x004C6A1A;
  *   - init sets bit 1 with a dword read-modify-write - `mov ecx,[esi]` /
  *     `or ecx,2` / `mov [esi],ecx` at 0x004C6A2E - and folds four more mode
  *     bits on with `or dword ptr [esi], imm` (0x004C6A95, 0x004C6AA0,
  *     0x004C6AB7, 0x004C6ABE) and a fifth at 0x004C6AC6, where
  *     `mov eax,[esi]` / `or al,0x20` / `mov [esi],eax` modifies one byte but
  *     STORES four - a byte field could not be written that way;
  *   - ?set_attrib@Wave@@QAEXK@Z is six `or dword ptr [ecx+0x54], imm` in a
  *     row, 0x004C6F36 through 0x004C6F69;
  *   - ?is_playing@Wave@@QAEHXZ (0x004C6B1A), both loads (0x004C6C76,
  *     0x004C6D43) and ?get_attrib@Wave@@QAEHXZ (0x004C6F9A) read it with
  *     `mov reg, dword ptr [esi+0x54]`.
  * The byte declaration made the four-byte memset unspellable: the recovered
  * ?init body had to write it as a loop over `&flags_54_ .. &flags_54_ + 4`,
  * which is the tell. Only ?unload@Wave@@QAEXXZ and ?play@Wave@@QAEHXZ narrow
  * the field - `mov al, byte ptr [esi+0x54]` at 0x004C6EB4 and
  * `test byte ptr [esi+0x54], 0x10` at 0x004C6945 - and a byte access OF a
  * dword is what a uint8_t local, or a mask that fits in the low byte,
  * compiles to. tools/verify_member_offsets.py --class Wave reported the
  * straddle that withdrew the old declaration.
  */
class Wave : public Sound {
 public:
  Wave();
  ~Wave();
  int set_asdr();
  void set_volume(int volume);
  int set_fname(const char *fname);
  int play();
  int load();
  int reload();
  // `K`, not `I`. MSVC decorates `unsigned long` K and `unsigned int` I,
  // and this tree spells uint32_t as unsigned int - same width, different
  // decorated name, and the name is what the linker pairs on.
  void init(char *a1, unsigned long a2);
  int dyna_load(char *fname);
  int load(const char *fname);

  int set_bufflimit(unsigned int limit);
  int set_attack(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_sustain(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_decay(unsigned int a1, unsigned int a2, unsigned int a3);
  int set_release(unsigned int a1, unsigned int a2, unsigned int a3);
  int unload();
  void set_pitch(int pitch);
  // PADK per ?load@Wave@@QAEHPADK@Z: a string and an unsigned long, not two
  // integers.
  int load(char *a1, unsigned long a2);
  int get_ms_length();
  int is_playing();
  int play(int a1);
  int is_hwbuffer();
  int get_time(uint32_t a1);
  int get_current_marker();
  int get_game_hwnd();
  int get_ndevices();
  int UNK1(int a1);
  int set_reverb_mix(float mix);
  int is_3d();
  int get_device_description(char *a1, int a2, int a3);
  int set_position3d(float x, float y, float z);
  int set_xpos(float x);
  int set_ypos(float y);
  int set_zpos(float z);
  void set_attrib(unsigned long attrib);
  int get_attrib();
 private:
  // 0x54, a whole 32-bit field - see the withdrawal above. Bit 1 suppresses
  // the vtable callback in unload, and set_attrib/init/get_attrib map the
  // mode mask onto bits 0..5.
  uint32_t flags_54_;
  int32_t pitch_;                // 0x58, clamped semitone offset
  float reverb_mix_;             // 0x5C, stored by set_reverb_mix
  int32_t ms_length_;            // 0x60, playing length in milliseconds
  uint32_t start_time_;          // 0x64, timeGetTime stamp when playing began
  uint32_t group_slot_;          // 0x68, pulled from its device group when < 0x10
};

static_assert(sizeof(Wave) == 0x6C, "Wave layout must match terranx.exe");


// With no wrapped device the wave is timed against the clock, through the
// game's imported timeGetTime. The seam is the address of that import slot -
// the IAT entry the original calls indirectly - so it reads the live pointer
// at run time and stays rebindable for tests.
typedef DWORD(__stdcall func_time_get_time)(void);
inline func_time_get_time *&WaveTimeGetTimeSlot() { return *reinterpret_cast<func_time_get_time **>(0x00669368); }

void *__fastcall wave_scalar_dtor_redirect(Wave *self, void *,
                                           unsigned int mode);

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
typedef void(__cdecl func_wave_device_release)(void *device);
extern func_wave_device_pull_from_group WaveDevicePullFromGroup;
// TYPED, not `void *`: it is the process Wave_Device at 0x0090D978 -
// the same object init_thunks.cpp calls `g_WAVE_DEVICE` - and the call
// sites in wave.cpp reach its methods by name.
Wave_Device *const WaveDeviceGlobal = (Wave_Device *)0x0090D978;
inline func_wave_device_release *&WaveDeviceReleaseSlot() { return *reinterpret_cast<func_wave_device_release **>(0x0090DB28); }
int *const WaveDeviceReleaseGuard = (int *)0x0090DB7C;
inline Wave *&WaveChainHead() { return *reinterpret_cast<Wave **>(0x0090DB20); }
inline Wave *&WaveChainTail() { return *reinterpret_cast<Wave **>(0x0090DB1C); }

// The rest of the Wave dependency surface. The filename copy is allocated on
// the game CRT heap (the destructor and set_fname free it there, so the new
// must match); the per-group volume table lives inside the device singleton,
// one dword every 24 bytes from 0x0090D9A0; is_group_disabled is the device
// singleton method play consults before starting; and the no-argument load
// is the not-yet-recovered Wave::load() that play falls back to.
// WaveOperatorNew and its typedef now live in "vector_teardown.h" above -
// the seam outgrew this header once GraphicWin::init needed it too.
uint32_t *const WaveDeviceGroupVolumes = (uint32_t *)0x0090D9A0;
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
inline func_wave_device_create *&WaveDeviceCreateSlot() { return *reinterpret_cast<func_wave_device_create **>(0x0090DB24); }
typedef int (OriginalObject::*func_sound_original_load)(const char *fname);
extern func_sound_original_load SoundOriginalLoad;
typedef void (OriginalObject::*func_sound_set_type)(uint32_t type);

