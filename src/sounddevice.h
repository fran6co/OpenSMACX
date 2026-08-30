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
  * The vtable pointer at offset zero is held as opaque storage so no C++
  * vtable is generated that could disagree with the original's.
  */
class Midi_Device {
 public:
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E190).
  // The base-interface table (0x0066E098) is all-pure: the original compiled
  // Midi_Device's base as abstract.
  virtual int create_device(unsigned long a1);      // slot 0 (0x4C56D0)
  virtual int delete_device();                      // slot 1 (0x4C5710)
  virtual void unk_slot2();                         // slot 2 (0x404280 `ret` stub)
  virtual int init(void *window, unsigned long backends);  // slot 3
  virtual void release();                           // slot 4 (0x4C57F0)
  virtual int get_ndevices();                       // slot 5 (0x4C5830)
  virtual int select(unsigned int a1);              // slot 6 (0x4C5840)
  virtual int get_description(unsigned int a1, char *a2, unsigned int a3);  // slot 7
  // DEFINED IN sounddevice.cpp, not empty. Its marker claims 60 image bytes
  // and the body was `{ ; }` - the image writes nine fields and both vtable
  // pointers. Promoted from src/recovered/units/004c5740.cpp.
  Midi_Device();
  // DEFINED IN sounddevice.cpp, not empty: its marker claims 19 image bytes
  // and the body was `{ ; }` - the image re-installs the shared device vtable
  // and tears down the wrapped device through its own virtual slot 4. Promoted
  // from src/recovered/004c5780.cpp.
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
 private:
  uint32_t field_4_;  // 0x0004
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  void *device_;  // 0x14, the wrapped device this forwards to
  uint32_t field_18_;
  uint32_t field_1C_;
};  // vptr at 0x00 replaces the retired vtable_storage_

static_assert(sizeof(Midi_Device) == 0x20,
              "Midi_Device layout must match the original executable");

class Wave_In_Device {
 public:
  // THE VIRTUAL SET, in the image's own vftable slot order (0x0066E1F0).
  virtual int create_device(unsigned long a1);      // slot 0 (0x4C59A0)
  virtual int delete_device();                      // slot 1 (0x4C59E0)
  virtual void unk_slot2();                         // slot 2 (0x404280 `ret` stub)
  virtual int init(void *window, unsigned long backends);  // slot 3
  virtual void release();                           // slot 4 (0x4C5A50)
  virtual int get_ndevices();                       // slot 5 (0x4C5AC0)
  virtual int select(unsigned int a1);              // slot 6 (0x4C5AD0)
  virtual int get_description(unsigned int a1, char *a2, unsigned int a3);  // slot 7
  // DEFINED IN sounddevice.cpp, not empty - the same shape as
  // Midi_Device::Midi_Device() next door.
  Wave_In_Device();
  // DEFINED IN sounddevice.cpp, not empty - the same shape as
  // ~Midi_Device() next door. Promoted from src/recovered/004c5980.cpp.
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
 private:
  uint32_t field_4_;  // 0x0004
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  void *device_;  // 0x14, the wrapped device this forwards to
  uint32_t field_18_;
  uint32_t field_1C_;
};  // vptr at 0x00 replaces the retired vtable_storage_

static_assert(sizeof(Wave_In_Device) == 0x20,
              "Wave_In_Device layout must match the original executable");

// The wave-in device singleton, at the fixed address init_sound binds and
// brings up (sound.cpp) and VoiceTx's record path drives (net_class.cpp).
// Defined HERE rather than as a per-file address binding so uses fold to the
// immediate - the same convention as WaveDeviceGlobal in wave.h.
Wave_In_Device *const WaveInDeviceGlobal = (Wave_In_Device *)0x0090DB50;

