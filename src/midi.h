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

/* Midi and its delegate, recovered 2026-08-23 (see 2bbecdbd): the
 * declarations live here now, not in hypothesis_layouts.h - correct
 * work does not feed the scaffold it is meant to retire. */

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
  virtual void slot005();
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
  virtual void slot016();
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
  virtual int set_active_tracks(unsigned long, unsigned long);  // slot 0x94
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
  virtual int map_patch(char *, unsigned long, unsigned long);  // slot 0xdc
  virtual int clear_patch(unsigned long);                    // slot 0xe0
  virtual int set_channel_ctrl(ChannelCtrl *);               // slot 0xe4
  virtual int set_program_ctrl(ProgramCtrl *);               // slot 0xe8
  virtual void slot059();
  virtual void slot060();
  virtual int get_control_track();                           // slot 0xf4
  virtual int get_total_track_ticks(unsigned long);          // slot 0xf8
  virtual void slot063();
  virtual int set_switch_type(unsigned long);                // slot 0x100
  virtual int set_nswitch_threads(unsigned long);            // slot 0x104
  virtual int add_switch_range(unsigned long, unsigned long);   // slot 0x108
  virtual int play_trackset(unsigned int);                   // slot 0x10c
  virtual int stop_trackset(unsigned int);                   // slot 0x110
  virtual int xpose_trackset(unsigned int, int);             // slot 0x114
  virtual bool is_trackset_playing(unsigned int);            // slot 0x118
  virtual void slot071();
  virtual int load_patch(unsigned int);                      // slot 0x120
  virtual int unload_patch(unsigned int);                    // slot 0x124
  virtual int set_patch(unsigned int, unsigned int);         // slot 0x128
};

class Midi {
 public:
  uint8_t field_0_[0x3C];  // 0x0
  MidiDelegate *delegate_;  // 0x3C - the sequencer this forwards to
  uint8_t field_40_[0x10];  // 0x40

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
};

