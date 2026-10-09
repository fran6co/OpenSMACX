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
extern int MidiTracksetTransposition;

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
  virtual int slot044(int, int);
  virtual bool slot045(unsigned int);
  virtual int slot046(int, int);
  virtual int slot047(int, int);
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
  virtual int slot060(int);
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
  int set_mode(int mode);
  friend class Ambience;
 protected:
  uint8_t field_54_; // Original clears exactly one byte, leaving55..57 alone.
};

// Start the next available trackset through Midi's real trackset
// interface. The original identity's dynamic receiver type and lifecycle
// are not yet bound more narrowly than this Midi-compatible interface.
void __fastcall advance_midi_trackset_transposition(Midi *midi);
