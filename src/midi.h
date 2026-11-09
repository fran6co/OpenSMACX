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
 * You should have received a copy of the GNU General Public License along
 * with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "vc6_compat.h"
#include "sound.h"

/* Midi and its provider relationship, recovered 2026-08-23 (see 1b8ba49f): the
 * declarations live here now, not in hypothesis_layouts.h - correct
 * work does not feed the scaffold it is meant to retire.
 *
 * MIDI IS A SOUND, the same relationship wave.h recovered for Wave on
 * 2026-08-22. The first 0x54 bytes are the Sound sub-object - the
 * wrapped sequencer at 0x3C (Sound::device_), the flag dword at 0x40,
 * the heap-owned filename copy at 0x4C - and Midi::unload's own image
 * dispatches prove the base: it loads the vtable at [this+0] and calls
 * slots 0x5C and 0x80, Sound slots 23 (is_playing) and 32 (detach) in
 * sound.h's own numbering, then clears bit 0 of [this+0x40] exactly as
 * Sound::unload's `flags_40_ &= ~1u` does. The flat opaque prefix this
 * header carried could spell neither dispatch; the compiler now owns
 * the primary vtable, and four same-signature members (play,
 * set_volume, load(const char*), unload) implicitly override their
 * Sound slots - the image's QAE spelling is catalogued on each body's
 * marker, the build's UAE on its `// symbol` fact.
 *
 * The wrapped sequencer is another reconstructed Midi object: its common
 * Sound base and 42 added virtual slots match the EXE Midi table and pinned
 * DLL mode-2/mode-5 tables. The SDK constructor bodies at EXE 0x004C75B0
 * and DLL 0x10029520 are 189 bytes / 68 instructions with equal instruction
 * encodings, registers, constants and field offsets, differing only in seven
 * reference immediates (handler, tables and memset targets). Original debug
 * names and full provider identity remain inferred. The old separate
 * MidiDelegate declaration duplicated these same slots and hid the proven
 * common object relationship. Midi's own storage begins
 * at 0x54 - one flag byte, the byte init clears; nothing else recovered
 * reads past the Sound sub-object yet, so no extent is pinned. */

struct TrackSet;
struct ChannelCtrl;
struct ProgramCtrl;
struct TrackCtrl;
extern int MidiTracksetTransposition;

class Midi : public Sound {
 public:
  Midi();
  ~Midi();
  // Concrete implementation of the three Sound slots selected by the
  // pinned Midi and Ambience tables.
  virtual int get_game_hwnd();
  virtual int set_attrib(unsigned long);
  virtual int get_attrib();
  int play();
  void set_volume(int volume);
  int unload();
  // Genuine added primary slots33..74, verified against66E538.
  virtual int set_track(uint8_t *, unsigned long);             // 33
  virtual int set_tempo(unsigned long);                       // 34
  virtual int mute_track(unsigned long);                      // 35
  // VC6 emits each new overload group in reverse declaration order.
  virtual int set_active_tracks(unsigned long, unsigned long);// 37
  virtual int set_active_tracks(unsigned int);                // 36
  virtual int add_active_trackset(TrackSet *);                 // 38
  virtual int get_trackset(unsigned int);                     // 39
  virtual int set_active_range_lo(unsigned int, unsigned int);// 40
  virtual int set_active_range_hi(unsigned int, unsigned int);// 41
  virtual int remove_active_trackset(unsigned int);           // 42
  virtual int unmute_track(unsigned long);                    // 43
  virtual int sub_4c7f70(int, int);                            // 44
  virtual bool sub_4c7fa0(unsigned int);                       // 45
  virtual int sub_4c7ff0(int, int);                            // 46
  virtual int sub_4c7fc0(int, int);                            // 47
  virtual int set_track_ctrl(TrackCtrl *);                    // 48
  virtual int get_ntracks();                                  // 49
  virtual int reset();                                        // 50
  virtual int init(char *fname, unsigned long flags);          // 51
  virtual int load();                                         // 52
  // Keep the overload group here: VC6 would otherwise append load() at
  // slot33 when the inherited load(filename) override is declared first.
  int load(const char *fname);                                // inherited4
  virtual int get_time(unsigned long);                        // 53
  virtual int map_patch(char *, unsigned long, unsigned long);// 55
  virtual int map_patch(char *, unsigned long);               // 54
  virtual int clear_patch(unsigned long);                     // 56
  virtual int set_channel_ctrl(ChannelCtrl *);                // 57
  virtual int set_program_ctrl(ProgramCtrl *);                // 58
  virtual int set_base_path(const char *);                    // 59
  virtual int sub_4c8020(int);                                // 60
  virtual int get_control_track();                           // 61
  virtual int get_total_track_ticks(unsigned long);           // 62
  virtual int switch_state(bool);                            // 63
  virtual int set_switch_type(unsigned long);                 // 64
  virtual int set_nswitch_threads(unsigned long);             // 65
  virtual int add_switch_range(unsigned long, unsigned long);// 66
  virtual int play_trackset(unsigned int);                    // 67
  virtual int stop_trackset(unsigned int);                    // 68
  virtual int xpose_trackset(unsigned int, int);               // 69
  virtual bool is_trackset_playing(unsigned int);             // 70
  virtual int set_play_type(unsigned int);                    // 71
  virtual int load_patch(unsigned int);                       // 72
  virtual int unload_patch(unsigned int);                     // 73
  virtual int set_patch(unsigned int, unsigned int);          // 74
  int set_fname(const char *fname);
  int set_mode(uint8_t mode);
  // 0x00447D20, defined in ambience.cpp (it sits in the ambience block).
  // A thiscall member: the free __fastcall spelling scheduled `mov ecx`
  // before the pushes. The exact receiver class may be narrower.
  void advance_trackset_transposition();
 protected:
  uint8_t field_54_; // Original clears exactly one byte, leaving55..57 alone.
};
