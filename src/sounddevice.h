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
  virtual int create_device(unsigned long) = 0;
  virtual int delete_device() = 0;
  virtual void unk_slot2() = 0;
  virtual int init(void *, unsigned long) = 0;
  virtual int release() = 0;
  virtual int get_ndevices() = 0;
  virtual int select(unsigned long) = 0;
  virtual int get_description(unsigned long, char *, unsigned long) = 0;
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
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E190).
  // The base-interface table (0x0066E098) is all-pure: the original compiled
  // Midi_Device's base as abstract.
  virtual int create_device(unsigned long a1);      // slot 0 (0x4C56D0)
  virtual int delete_device();                      // slot 1 (0x4C5710)
  virtual void unk_slot2();                         // slot 2 (0x404280 `ret` stub)
  virtual int init(void *window, unsigned long backends);  // slot 3
  virtual int release();                            // slot 4 (0x4C57F0)
  virtual int get_ndevices();                       // slot 5 (0x4C5830)
  virtual int select(unsigned long a1);              // slot 6 (0x4C5840)
  virtual int get_description(unsigned long a1, char *a2, unsigned long a3);  // slot 7
  // Base construction initializes the observed fields; compiler-generated
  // base/derived vptr transitions belong to the ordinary C++ lifetime.
  Midi_Device();
  // Base destruction releases the opaque provider through its observed ABI.
  ~Midi_Device();
  void update_sound();
  void suspend();
  void restart();

  void set_volume(unsigned int volume);
  void set_pan(int pan);
  int fade(unsigned int a1);
  void set_rate(unsigned int rate);
  int get_volume();
  int stop();
  int get_rate();
  int enable();
  int disable();
  int is_disabled();
};  // vptr at 0x00 replaces the retired vtable_storage_

static_assert(sizeof(Midi_Device) == 0x20,
              "observed Midi_Device prefix; original full extent unresolved");

class Wave_In_Device : public SoundDeviceBase {
 public:
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E1F0).
  virtual int create_device(unsigned long a1);      // slot 0 (0x4C59A0)
  virtual int delete_device();                      // slot 1 (0x4C59E0)
  virtual void unk_slot2();                         // slot 2 (0x404280 `ret` stub)
  virtual int init(void *window, unsigned long backends);  // slot 3
  virtual int release();                            // slot 4 (0x4C5A50)
  virtual int get_ndevices();                       // slot 5 (0x4C5AC0)
  virtual int select(unsigned long a1);              // slot 6 (0x4C5AD0)
  virtual int get_description(unsigned long a1, char *a2, unsigned long a3);  // slot 7
  // Ordinary base construction, as for Midi_Device.
  Wave_In_Device();
  // Ordinary base destruction, as for Midi_Device.
  ~Wave_In_Device();
  void update_sound();
  void suspend();
  void restart();

  int get_caps(unsigned int a1);
  void set_rate(unsigned int rate);
  int stop();
  int get_rate();
  int start_record();
  int end_record();

  // Observed external operations; no placeholder provider class or invented
  // earlier virtual signatures. Complete provider API remains unverified.
  int set_codec(unsigned long codec);
  int set_vxw_key(signed char *key);
};  // vptr at 0x00 replaces the retired vtable_storage_

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
