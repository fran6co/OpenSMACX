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

#include "wave_device.h"
#include "sound.h"

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
  friend class FX;
  friend class Wave_Device;
 public:
  Wave();
  ~Wave();
  void set_volume(int volume);
  int set_fname(const char *fname);
  int dyna_load(char *fname);
  int unload();
  int is_playing();
  int get_game_hwnd();
  // Provisional behavioral name for original 0x004C7080: walk the wrapped
  // device chain and return 1. It is not Sound's unrelated slot-3 method;
  // the shared historical name UNK1 accidentally made this an override.
  int probe_device_chain(int unused);
  int set_attrib(unsigned long attrib);
  int get_attrib();

  // Slot 24 is inherited from Sound but Wave overrides it to call
  // the DLL-created SoundDeviceInterface at device_ (+0x3C).
  virtual int unk_slot24();  // slot 24, original 0x00448340
  // Original Wave (0x0066E44C) and Effect (0x0066B7A4) tables agree on
  // this derived virtual sequence, slots 33..56. In particular FX::init
  // calls set_pitch through slot 38, and Effect::init calls load at 35.
  // Wave's concrete implementations override Sound's pure slots.
  virtual int reload();                            // 33
  // `K`, not `I`. MSVC decorates `unsigned long` K and `unsigned int` I,
  // and this tree spells uint32_t as unsigned int - same width, different
  // decorated name, and the name is what the linker pairs on.
  virtual void init(char *group_id, unsigned long flags);  // 34
  // VC6 groups overloads at their first declaration and emits new slots in
  // reverse overload order. Keep the inherited overload in this group too.
  virtual int load(char *a1, unsigned long a2);      // 36
  virtual int load();                              // 35
  int load(const char *fname);                     // inherited 4
  virtual int play(int effect);                    // 37
  int play();                                     // inherited 7
  virtual int set_pitch(int pitch);                // 38
  virtual int set_bufflimit(unsigned int limit);    // 39
  virtual int set_asdr();                          // 40
  virtual int set_attack(unsigned int a1, unsigned int a2, unsigned int a3);   // 41
  virtual int set_sustain(unsigned int a1, unsigned int a2, unsigned int a3);  // 42
  virtual int set_decay(unsigned int a1, unsigned int a2, unsigned int a3);    // 43
  virtual int set_release(unsigned int a1, unsigned int a2, unsigned int a3);  // 44
  virtual int get_time(uint32_t a1);                // 45
  virtual int get_current_marker();                // 46
  virtual int get_ndevices();                      // 47
  virtual int get_device_description(char *a1, int a2, int a3);  // 48
  virtual int get_ms_length();                     // 49
  virtual int is_hwbuffer();                       // 50
  virtual int set_position3d(float x, float y, float z);  // 51
  virtual int set_xpos(float x);                    // 52
  virtual int set_ypos(float y);                    // 53
  virtual int set_zpos(float z);                    // 54
  virtual bool is_3d();                            // 55
  virtual int set_reverb_mix(float mix);            // 56
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

// Effect has the genuine Wave base and no additional storage. Its original
// destructor tail-calls Wave destruction; the FX bank owns 97 such objects
// at the same 0x6C stride. FX owns the initialization policy and the implicit
// lifetime mappings in fx.cpp; no user-defined destructor stage is needed.
class Effect : public Wave {
 public:
  void init(char *fname, long flags);  // original mangled parameter J
};

// Owned by the original process at 0x007FFF00, directly after MultiWindow.
extern Wave g_MULTIWIN_WAVE;

// The process-wide wave, at the fixed address wave.cpp pins it to. Declared
// here so control_game (game.cpp) reaches it by name.
extern Wave g_WAVE_GENERAL;  // 0x00945ED0

// The forty-five entry wave bank, at the fixed address wave.cpp pins it to
// (0x6c element stride, the image's own `VectorCtorIterator(g_CPU_WAVES,
// 0x6C, 45, ...)` array thunk). Declared here so popup_wave_callback
// (popup.cpp) reaches the bank by name instead of through a pointer
// binding into the image's data.
extern Wave g_CPU_WAVES[45];  // 0x0074C5F0

static_assert(sizeof(Wave) == 0x6C, "Wave layout must match terranx.exe");




// A wave holding one of the 0x10 device group slots is pulled from its
// group by WaveDeviceGlobal. Its filename allocation is returned through
// operator delete, and its wrapped device through the loader-owned release
// hook while the guard says it is live. The common Sound chain is declared
// in sound.h; Wave does not allocate separate ends or narrow its neighbors.
// The process Wave_Device and its release-hook guard are REAL OBJECTS - the
// device defined in wave_device.cpp (it was `Wave_Device *const
// WaveDeviceGlobal = (Wave_Device *)0x0090D978` there and a `g_WAVE_DEVICE`
// object beside it; the object now carries the public name), the guard an
// int defined in wave_device.cpp (it was `int *const WaveDeviceReleaseGuard
// = (int *)0x0090DB7C`). Both named terranx.exe data that is unmapped in a
// standalone build. Every `WaveDeviceGlobal->` receiver and every
// `*WaveDeviceReleaseGuard` read keeps its folded form - the immediates and
// disp32s become the same instructions against relocated symbols.
// SoundX ordinal 2's callable contract is centralized in sound.h.
extern int WaveDeviceReleaseGuard;  // 0x0090DB7C

// Filename allocations use the game CRT heap. Group volume and disabled
// state belong to WaveDeviceGlobal's real group records; play falls back
// to the implemented no-argument Wave::load().
// Filename and group allocations use the configured CRT operator new,
// shared with the existing MIDI allocation path.

// SoundX ordinal 1's callable contract is centralized in sound.h.

// Movement sound, owned and initialized with the other Wave objects.
extern Wave g_SCOOT_WAVE;  // 0x00749C18

// Effect::init installs three source-owned callbacks in Win's process-wide
// hook storage before resolving a filename. The Win global names are
// provisional descriptions of the observed consumers.
typedef void(__cdecl *func_effect_callback)();
void __cdecl input_callback();
void __cdecl ok_callback();
void __cdecl button_hover_callback();
