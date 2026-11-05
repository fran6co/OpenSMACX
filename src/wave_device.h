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

#include "sounddevice.h"

class Wave;
struct EAX_REVERB_PROPERTIES;
struct IDirectSound;

// One node of a group's wave list, allocated on the game CRT heap by insert
// and freed by group removal operations.
struct WaveGroupNode {
  WaveGroupNode(Wave *value = 0) : prev(0), next(0), wave(value) {}
  WaveGroupNode *prev;
  WaveGroupNode *next;
  Wave *wave;
};

// The list subobject at WaveControlGroup+8, used by the group administrators
// and by its insertion/removal operations.
struct WaveGroupList {
  // RECOVERY_MEMBER: v3 WaveGroupList head WaveGroupNode* 0x0 0x004C5BF0
  WaveGroupNode *head;
  // RECOVERY_MEMBER: v3 WaveGroupList tail WaveGroupNode* 0x4 0x004C5BF0
  WaveGroupNode *tail;
  // RECOVERY_MEMBER: v3 WaveGroupList cursor WaveGroupNode* 0x8 0x004C5490
  WaveGroupNode *cursor;
  // RECOVERY_MEMBER: v1 WaveGroupList count 0xC 0x004C5B80
  uint32_t count;

  // 0x004C5BF0, the original typed list insertion operation.
  void insert(Wave *wave);

  // Recovered head removal used by group teardown. A null payload is still
  // removed and returned as the stopping value; cursor is not adjusted.
  Wave *remove_head() {
    WaveGroupNode *node = this->head;
    if (!node) {
      return 0;
    }
    WaveGroupNode *const next = node->next;
    if (next) {
      next->prev = 0;
      this->head = next;
    } else {
      this->head = (this->tail = 0);
    }
    Wave *const wave = node->wave;
    operator delete(node);
    this->count -= 1;
    return wave;
  }
};

// One 24-byte group record; the device holds sixteen of them from 0x24.
struct WaveControlGroup {
  // RECOVERY_MEMBER: v1 WaveControlGroup enabled 0x0 0x004C5490
  uint8_t enabled;        // +0x00, zero means the group is disabled
  uint8_t pad[3];
  // RECOVERY_MEMBER: v1 WaveControlGroup volume 0x4 0x004C5490
  uint32_t volume;        // +0x04, the scale Wave::set_volume folds in
  // RECOVERY_MEMBER: v2 WaveControlGroup list_ WaveGroupList 0x8 0x004C5240
  WaveGroupList list_;    // +0x08..+0x17

  // The actual element lifetime used by Wave_Device::groups_[16].
  WaveControlGroup();
  ~WaveControlGroup();
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
class Wave_Device : public SoundDeviceBase {
  friend class Wave;
 public:
  Wave_Device();
  ~Wave_Device();
  virtual int create_device(unsigned long a1);                         // 0
  virtual int delete_device();                                         // 1
  virtual void unk_slot2();                                            // 2
  virtual int init(void *group_id, unsigned long flags);               // 3
  virtual int release();                                               // 4
  virtual int get_ndevices();                                          // 5
  virtual int select(unsigned long a1);                                // 6
  virtual int get_description(unsigned long a1, char *a2, unsigned long a3); // 7
  virtual void set_volume(unsigned int volume);                        // 8; 32-bit ABI, nominal type varies in old identities
  virtual int get_volume();                                            // 9
  virtual void set_pan(int);                                           // 10
  virtual int fade(uint32_t);                                          // 11
  virtual int stop();                                                  // 12
  virtual void update_sound();                                         // 13
  virtual void set_rate(unsigned int rate);                            // 14
  virtual int get_rate();                                              // 15
  virtual void unknown_operation16(int);                                // 16: ABI-only provisional name
  virtual int unknown_query17();                                        // 17: ABI-only provisional name
  virtual void suspend();                                              // 18
  virtual void restart();                                               // 19
  virtual int start_raw_dump(char *path);                              // 20
  virtual int stop_raw_dump();                                          // 21
  virtual int get_hw_mem_size();                                        // 22
  // Pinned 0x404280 bare-ret entry; neutral provisional operation name.
  virtual void unknown_operation23();                                   // 23
  virtual int enable();                                                 // 24
  virtual int disable();                                                // 25
  virtual int is_disabled();                                            // 26
  virtual int set_hwnd(void *hwnd);                                     // 27
  virtual IDirectSound *get_ds();                                       // 28
  virtual bool is_eax();                                                // 29
  // VC6 emits this overload group in reverse declaration order: declaring
  // the pointer form first yields the pinned value form at slot 30 and
  // properties form at slot 31.
  virtual int set_eax(EAX_REVERB_PROPERTIES *properties);               // 31
  virtual int set_eax(unsigned long eax);                               // 30
  virtual int set_eax_mix(float mix);                                   // 32
  virtual bool is_3d();                                                 // 33
  virtual int set_listener_position(float x, float y, float z);         // 34
  virtual int get_listener_position(float *x, float *y, float *z);      // 35
  virtual int set_listener_xpos(float x);                               // 36
  virtual int get_listener_xpos(float *x);                              // 37
  virtual int set_listener_ypos(float y);                               // 38
  virtual int get_listener_ypos(float *y);                              // 39
  virtual int set_listener_zpos(float z);                               // 40
  virtual int get_listener_zpos(float *z);                              // 41

  int get_group_volume(unsigned int group);
  int add_to_group(unsigned int group, Wave *wave);
  int pull_from_group(Wave *wave);
  bool is_group_disabled(unsigned int group);
  int set_group_volume(unsigned int group, unsigned int volume);
  int enable_group(unsigned int group);
  int disable_group(unsigned int group);
 private:
  uint32_t field_20_;
  // Original4C4DD0 passes this+24, stride24 and count16 to the CRT
  // constructor iterator. Extent is independent of today's C++ sizeof.
  // RECOVERY_MEMBER: v4 Wave_Device groups_ WaveControlGroup[16] 0x24 0x180 0x004C4DD0
  WaveControlGroup groups_[16];
};

// THE OBJECT, at 0x0090D978 in the shipped image. Consumers (wave.cpp,
// net_class.cpp, sound.cpp) reach the device singleton through this name;
// see wave_device.cpp.
extern Wave_Device WaveDeviceGlobal;  // 0x0090D978

static_assert(sizeof(WaveControlGroup) == 0x18, "group records stride 24 bytes");


// SoundX ordinals 5/6 provide the opaque provider factory and destroy
// callbacks. Their callable contracts are centralized beside the shared
// table in sound.h.

// The 0x004C5490 / 0x004C5B80 lifecycle is owned directly by
// WaveControlGroup; Wave_Device's typed member array supplies compiler
// construction, reverse destruction, and constructed-prefix unwind.
// RECOVERY_GENERATED: v1 vtable 0x0066E0E8 0x0066E190 ??_7Wave_Device@@6B@ 0x004C4DD0
