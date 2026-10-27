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
#include "soundx_device_abi.h"

 /*
  * Sound device classes
  *
  * Two audio devices the game drives through a common interface. Their
  * constructors are byte-for-byte the same shape - the same shared vtable
  * installed at offset zero, the same four-byte memset at 0x4, 0x7F stored at
  * 0x8, zeros through 0x1C, then each swapping in its own vtable - which is
  * what places their common fields where they are here.
  *
  * The suspend, restart, and update_sound hooks recovered here do nothing in
  * the original: these devices decline to be suspended, restarted, or polled.
  *
  * Neither layout is established. The fields below are those the constructors
  * write, so the object extends at least to 0x20; g_MIDI_DEVICE's global slot
  * bounds Midi_Device above at 0x28, which is a bound and not a size. Nothing
  * pins their sizeof and no field may be appended without deriving it first.
  * SoundDeviceBase now owns the original pure interface table and its
  * shared fields, so each concrete device has one real C++ base subobject.
  */
class SoundDeviceBase {
 public:
  virtual int create_device(unsigned long) = 0;                         // 0
  virtual int delete_device() = 0;                                      // 1
  virtual void unk_slot2() = 0;                                        // 2
  virtual int init(void *, unsigned long) = 0;                          // 3
  virtual int release() = 0;                                            // 4
  virtual int get_ndevices() = 0;                                       // 5
  virtual int select(unsigned long) = 0;                                // 6
  virtual int get_description(unsigned long, char *, unsigned long) = 0;// 7
  virtual void set_volume(unsigned int) = 0;                            // 8
  virtual int get_volume() = 0;                                         // 9
  virtual void set_pan(int) = 0;                                        // 10
  virtual int fade(unsigned int) = 0;                                    // 11
  virtual int stop() = 0;                                               // 12
  virtual void update_sound() = 0;                                      // 13
  virtual void set_rate(unsigned int) = 0;                              // 14
  virtual int get_rate() = 0;                                           // 15
  // ABI-only provisional names. Pinned shared targets establish thiscall
  // cleanup/return shapes, not nominal source names or signedness.
  virtual void unknown_operation16(int) = 0;                             // 16: ret 4
  virtual int unknown_query17() = 0;                                     // 17: zero, no stack args
  virtual void suspend() = 0;                                           // 18
  virtual void restart() = 0;                                           // 19
  SoundDeviceBase();
 protected:
  ~SoundDeviceBase();
  uint32_t field_4_;
  uint32_t volume_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  // Opaque DLL-owned provider. Its observed call ABI does not establish that
  // it inherits the game's SoundDeviceBase wrapper or shares its field layout.
  void *device_14_;
  uint32_t field_18_;
  uint32_t field_1C_;
};
static_assert(sizeof(SoundDeviceBase) == 0x20,
              "original common sound-device prefix");

class Midi_Device : public SoundDeviceBase {
 public:
  virtual int create_device(unsigned long a1);
  virtual int delete_device();
  virtual void unk_slot2();
  virtual int init(void *window, unsigned long backends);
  virtual int release();
  virtual int get_ndevices();
  virtual int select(unsigned long a1);
  virtual int get_description(unsigned long a1, char *a2, unsigned long a3);
  virtual void set_volume(unsigned int volume);
  virtual int get_volume();
  virtual void set_pan(int pan);
  virtual int fade(unsigned int a1);
  virtual int stop();
  virtual void update_sound();
  virtual void set_rate(unsigned int rate);
  virtual int get_rate();
  virtual void unknown_operation16(int);
  virtual int unknown_query17();
  virtual void suspend();
  virtual void restart();
  Midi_Device();
  ~Midi_Device();
  virtual void unknown_operation20();
  virtual int enable();
  virtual int disable();
  virtual int is_disabled();
};
static_assert(sizeof(Midi_Device) == 0x20,
              "observed Midi_Device prefix; original full extent unresolved");

class Wave_In_Device : public SoundDeviceBase {
 public:
  virtual int create_device(unsigned long a1);
  virtual int delete_device();
  virtual void unk_slot2();
  virtual int init(void *window, unsigned long backends);
  virtual int release();
  virtual int get_ndevices();
  virtual int select(unsigned long a1);
  virtual int get_description(unsigned long a1, char *a2, unsigned long a3);
  virtual void set_volume(unsigned int);
  virtual int get_volume();
  virtual void set_pan(int);
  virtual int fade(unsigned int);
  virtual int stop();
  virtual void update_sound();
  virtual void set_rate(unsigned int);
  virtual int get_rate();
  virtual void unknown_operation16(int);
  virtual int unknown_query17();
  virtual void suspend();
  virtual void restart();
  Wave_In_Device();
  ~Wave_In_Device();
  virtual int get_caps(unsigned int a1);
  virtual void unknown_operation21();
  virtual int start_record();
  virtual int end_record();
  virtual int set_codec(unsigned long codec);
  virtual int set_vxw_key(signed char *key);
};
static_assert(sizeof(Wave_In_Device) == 0x20,
              "observed Wave_In_Device prefix; original full extent unresolved");

// Genuine process-global objects, constructed before init_sound and destroyed
// through compiler-generated atexit callbacks. Original starts and lifecycle
// evidence live beside their definitions; complete original extents are unknown.
extern Midi_Device g_MIDI_DEVICE;
extern Wave_In_Device g_WAVE_IN_DEVICE;

inline SoundDeviceBase::~SoundDeviceBase() {
    void *const device = device_14_;
    if (device) {
        SoundXDeviceAbi::release_device(device);
    }
}

// SoundX ordinals 7..10 provide opaque MIDI and Wave-In provider callbacks;
// their callable contracts are centralized beside the shared table in sound.h.
// RECOVERY_GENERATED: v1 vtable 0x0066E098 0x0066E0E8 ??_7SoundDeviceBase@@6B@ 0x004C4D70
// RECOVERY_GENERATED: v1 vtable 0x0066E190 0x0066E1F0 ??_7Midi_Device@@6B@ 0x004C5740
// RECOVERY_GENERATED: v1 vtable 0x0066E1F0 0x0066E258 ??_7Wave_In_Device@@6B@ 0x004C5940
