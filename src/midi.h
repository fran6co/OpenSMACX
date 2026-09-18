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

/* Midi and its delegate, recovered 2026-08-23 (see 2bbecdbd): the
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
 * The wrapped sequencer is MidiDelegate below: a Sound-family table of
 * 75 slots extended with the sequencer interface at 0x84..0x128, the
 * same family shape ambience.h gives its own. Two of those slots (0x94,
 * 0xdc) carry neutral slotNNN names: VC6 does not place same-name
 * virtual overload groups in declaration order - measured on this class
 * 2026-09-18, both pairs emit with the higher-arity member first, and
 * the image's own Sound pairs (play, fade, fade_in) agree, so plain
 * declaration reversal cannot be the rule - and giving both members
 * Midi's forwarder name landed every dispatch of each pair on the wrong
 * slot (all four bodies measured exactly one constant apart: 0x90<->0x94
 * and 0xd8<->0xdc). Distinct names pin each slot by position under any
 * group ordering, and claim nothing: the wrapped class's own member
 * names are unevidenced, which is what this header's slotNNN spelling
 * has meant all along. Midi's own storage begins
 * at 0x54 - one flag byte, the byte init clears; nothing else recovered
 * reads past the Sound sub-object yet, so no extent is pinned. */

struct TrackSet;
struct ChannelCtrl;
struct ProgramCtrl;
struct TrackCtrl;

class MidiDelegate {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual int slot005();  // the family's unload (Sound slot 5); unload wants its answer
  virtual void slot006();
  virtual int play();                                        // slot 0x1c
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void set_volume(int volume);                      // slot 0x040
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual int set_track(uint8_t *, unsigned long);           // slot 0x84
  virtual int set_tempo(unsigned long);                      // slot 0x88
  virtual int mute_track(unsigned long);                     // slot 0x8c
  virtual int set_active_tracks(unsigned int);               // slot 0x90
  virtual int slot037(unsigned long, unsigned long);         // slot 0x94
  virtual int add_active_trackset(TrackSet *);               // slot 0x98
  virtual int get_trackset(unsigned int);                    // slot 0x9c
  virtual int set_active_range_lo(unsigned int, unsigned int);  // slot 0xa0
  virtual int set_active_range_hi(unsigned int, unsigned int);  // slot 0xa4
  virtual int remove_active_trackset(unsigned int);          // slot 0xa8
  virtual int unmute_track(unsigned long);                   // slot 0xac
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual int set_track_ctrl(TrackCtrl *);                   // slot 0xc0
  virtual int get_ntracks();                                 // slot 0xc4
  virtual int reset();                                       // slot 0xc8
  virtual void slot051();
  virtual void slot052();
  virtual int get_time(unsigned long);                       // slot 0xd4
  virtual int map_patch(char *, unsigned long);              // slot 0xd8
  virtual int slot055(char *, unsigned long, unsigned long);  // slot 0xdc
  virtual int clear_patch(unsigned long);                    // slot 0xe0
  virtual int set_channel_ctrl(ChannelCtrl *);               // slot 0xe4
  virtual int set_program_ctrl(ProgramCtrl *);               // slot 0xe8
  virtual int set_base_path(const char *path);               // slot 0x0ec
  virtual void slot060();
  virtual int get_control_track();                           // slot 0xf4
  virtual int get_total_track_ticks(unsigned long);          // slot 0xf8
  virtual int switch_state(bool on);                        // slot 0x0fc
  virtual int set_switch_type(unsigned long);                // slot 0x100
  virtual int set_nswitch_threads(unsigned long);            // slot 0x104
  virtual int add_switch_range(unsigned long, unsigned long);   // slot 0x108
  virtual int play_trackset(unsigned int);                   // slot 0x10c
  virtual int stop_trackset(unsigned int);                   // slot 0x110
  virtual int xpose_trackset(unsigned int, int);             // slot 0x114
  virtual bool is_trackset_playing(unsigned int);            // slot 0x118
  virtual int set_play_type(unsigned int type);             // slot 0x11c
  virtual int load_patch(unsigned int);                      // slot 0x120
  virtual int unload_patch(unsigned int);                    // slot 0x124
  virtual int set_patch(unsigned int, unsigned int);         // slot 0x128
};

class Midi : public Sound {
 public:
  int play();
  int set_switch_type(unsigned long);
  int add_switch_range(unsigned long, unsigned long);
  int set_nswitch_threads(unsigned long);
  int map_patch(char *, unsigned long, unsigned long);
  int map_patch(char *, unsigned long);
  int clear_patch(unsigned long);
  int reset();
  int set_track(uint8_t *, unsigned long);
  int mute_track(unsigned long);
  int unmute_track(unsigned long);
  int set_active_tracks(unsigned long, unsigned long);
  int set_active_tracks(unsigned int);
  int play_trackset(unsigned int);
  int xpose_trackset(unsigned int, int);
  int stop_trackset(unsigned int);
  int add_active_trackset(TrackSet *);
  int set_active_range_lo(unsigned int, unsigned int);
  int set_active_range_hi(unsigned int, unsigned int);
  int remove_active_trackset(unsigned int);
  int get_trackset(unsigned int);
  int get_ntracks();
  int load_patch(unsigned int);
  int unload_patch(unsigned int);
  int set_patch(unsigned int, unsigned int);
  int set_tempo(unsigned long);
  int get_time(unsigned long);
  int get_control_track();
  int get_total_track_ticks(unsigned long);
  int set_channel_ctrl(ChannelCtrl *);
  int set_program_ctrl(ProgramCtrl *);
  int set_track_ctrl(TrackCtrl *);

  // THE NINE remaining Midi methods, homed from src/recovered and
  // src/unrecovered 2026-09-03: five one-guarded-call forwarders plus
  // the four lifecycle bodies and init recovered 2026-09-18 on the
  // Sound base the class now declares. Each reaches 0x4C (fname_) or
  // the sequencer at 0x3C (Sound::device_), both inherited members now.
  void set_volume(int volume);
  int switch_state(bool on);
  int set_base_path(const char *path);
  bool is_trackset_playing(unsigned int trackset);
  int set_play_type(unsigned int type);

  // The five lifecycle bodies, midi.cpp. set_fname and init own the
  // filename copy at 0x4C; the two loads and unload wrap the sequencer
  // at 0x3C and were unreachable until the Sound base existed. play(),
  // set_volume(int), load(const char*) and unload() override their
  // Sound slots and mangle UAE - each body's marker carries both facts.
  int set_fname(const char *fname);
  int init(char *fname, unsigned long a2);
  int load();
  int load(const char *fname);
  int unload();

 private:
  // 0x54, the first Midi-own byte past the 0x54 Sound sub-object: the
  // one byte init clears with memset. No recovered body reads it yet,
  // so the storage model stops here and no extent is asserted.
  uint8_t field_54_;
};

