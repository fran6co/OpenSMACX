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

class Wave;
struct EAX_REVERB_PROPERTIES;

// One node of a group's wave list, allocated on the game CRT heap by the
// insert helper and freed by pull_from_group.
struct WaveGroupNode {
  WaveGroupNode *prev;
  WaveGroupNode *next;
  Wave *wave;
};

// The view the list-insert helper takes: its receiver is the address of a
// group's head field, so it sees the trailing four fields of WaveControlGroup.
struct WaveGroupList {
  WaveGroupNode *head;
  WaveGroupNode *tail;
  WaveGroupNode *cursor;
  uint32_t count;

  // 0x004C5BF0, a pending_bodies forwarder: not yet source-owned.
  void insert(Wave *wave);
};

// One 24-byte group record; the device holds sixteen of them from 0x24.
struct WaveControlGroup {
  uint8_t enabled;        // +0x00, zero means the group is disabled
  uint8_t pad[3];
  uint32_t volume;        // +0x04, the scale Wave::set_volume folds in
  WaveGroupNode *head;    // +0x08
  WaveGroupNode *tail;    // +0x0C, updated only when the last node leaves
  WaveGroupNode *cursor;  // +0x10, the node after the last removal, or null
  uint32_t count;         // +0x14
};

 /*
  * Wave_Device class
  *
  * Layout partially established. The one plain field these methods name is
  * the pointer to the wrapped device at 0x14. The group methods pin a
  * sixteen-entry table of 24-byte group records from 0x24 (whose volume
  * dwords at +4 are the 0x28-based table get_group_volume reads), taking the
  * extent to at least 0x1A4. That is still a floor, not a size.
  */
class Wave_Device {
 public:
  Wave_Device();
  ~Wave_Device();
  void set_pan(int);
  int fade(uint32_t);
  int enable();
  int disable();
  int get_volume();
  int stop();
  void suspend();
  void restart();
  void update_sound();
  int get_ndevices();
  int get_hw_mem_size();
  int get_rate();
  int get_ds();
  int is_eax();
  int is_disabled();
  int stop_raw_dump();
  int is_3d();
  void set_rate(unsigned long rate);
  void set_volume(unsigned long volume);
  int set_hwnd(void *hwnd);
  int get_group_volume(unsigned int group);
  int add_to_group(unsigned int group, Wave *wave);
  int pull_from_group(Wave *wave);
  int is_group_disabled(unsigned int group);
  int select(unsigned long a1);
  int create_device(unsigned long a1);
  int delete_device();
  int init(void *group_id, unsigned long flags);
  int release();
  int set_group_volume(unsigned int group, unsigned int volume);
  int enable_group(unsigned int group);
  int disable_group(unsigned int group);
  int get_description(unsigned long a1, char *a2, unsigned long a3);
  int start_raw_dump(char *path);
  int set_eax(EAX_REVERB_PROPERTIES *properties);
  int set_eax(unsigned long eax);
  int set_eax_mix(float mix);
  int set_listener_position(float x, float y, float z);
  int get_listener_position(float *x, float *y, float *z);
  int set_listener_xpos(float x);
  int get_listener_xpos(float *x);
  int set_listener_ypos(float y);
  int get_listener_ypos(float *y);
  int set_listener_zpos(float z);
  int get_listener_zpos(float *z);

 private:
  uint32_t vtable_storage_;  // 0x00, opaque so no C++ vtable is generated
  uint32_t field_4_;         // 0x04, zeroed at construction
  uint32_t volume_8_;        // 0x08, 0x7F at construction
  uint32_t field_C_;
  uint32_t field_10_;
  void *device_14_;       // 0x14, the wrapped device the forwarders consult
  uint32_t field_18_;
  uint32_t field_1C_;
  uint32_t field_20_;        // 0x20, zeroed at construction
  WaveControlGroup groups_[16];  // 0x24..0x1A3
};

static_assert(sizeof(WaveControlGroup) == 0x18, "group records stride 24 bytes");


// The device factory/destroy hooks: two more function-pointer slots beside
// the creation hook the waves use, consulted by the device lifecycle. The
// factory builds a device of the requested kind straight into the 0x14
// field; the destroy hook takes no arguments at all.
typedef int(__cdecl func_wave_device_factory)(void **device_slot,
                                              unsigned long kind);
typedef void(__cdecl func_wave_device_destroy)(void);
inline func_wave_device_factory *&WaveDeviceFactorySlot() { return *reinterpret_cast<func_wave_device_factory **>(0x0090DB34); }
inline func_wave_device_destroy *&WaveDeviceDestroySlot() { return *reinterpret_cast<func_wave_device_destroy **>(0x0090DB38); }

// The per-group construct/teardown pair the device's own lifetime hands to
// the CRT vector iterators; both stay rebindable while they double as the
// iterator arguments.
// The group element lifecycle, recovered from 0x004C5490 / 0x004C5B80.
void __fastcall WaveControlGroupOriginalCtor(void *group);
void __fastcall WaveControlGroupOriginalDtor(void *group);

void __fastcall wave_control_group_ctor_redirect(WaveControlGroup *self,
                                                 void *);
void __fastcall wave_control_group_dtor_redirect(WaveControlGroup *self,
                                                 void *);

void __fastcall wave_group_insert_redirect(WaveGroupList *self, void *,
                                           Wave *a1);

