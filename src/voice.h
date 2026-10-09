#pragma once

// Sound-family voice owners and the DLL-provided device contracts.
#include "sound.h"
struct _MMIOINFO;

// DLL-provided Sound-family interface. Complete inherited contracts, not
// positional filler declarations. The DLL creates/releases these objects;
// the application never constructs one or supplies a fabricated vtable.
// The external contract follows the independently observed VC6 Sound table.
class SoundDeviceInterface {
 public:
  virtual int set_fade(unsigned long) = 0;            // 0
  virtual int set_fade_interval(unsigned long) = 0;   // 1
  virtual ~SoundDeviceInterface();                   // 2, deleting entry
  virtual int UNK1(int) = 0;                         // 3
  virtual int load(const char *) = 0;                // 4
  virtual int unload() = 0;                          // 5
  virtual int play() = 0;                            // 7, VC6 overload group
  virtual int play(unsigned int) = 0;                // 6
  virtual int stop() = 0;                            // 8
  virtual int fade() = 0;                            // 10
  virtual void fade(unsigned long) = 0;              // 9
  virtual int fade_in() = 0;                         // 12
  virtual void fade_in(unsigned int) = 0;             // 11
  virtual void ramp(int, int, unsigned int) = 0;      // 13
  virtual int release() = 0;                         // 14
  virtual int get_game_hwnd() = 0;                   // 15
  virtual void set_volume(int) = 0;                  // 16
  virtual void set_pan(int) = 0;                     // 17
  virtual void set_loop_state(long) = 0;             // 18
  virtual void set_delay(unsigned int) = 0;          // 19
  virtual void set_type(unsigned int) = 0;           // 20
  virtual int set_fade_in(unsigned int) = 0;          // 21
  virtual int is_looping() = 0;                      // 22
  virtual int is_playing() = 0;                      // 23
  virtual int unk_slot24() = 0;                      // 24, provider return forwarded by Wave
  virtual Sound *get_next() = 0;                    // 25
  virtual Sound *get_prev() = 0;                    // 26
  virtual int set_attrib(unsigned long) = 0;         // 27
  virtual int get_attrib() = 0;                      // 28
  virtual int get_time() = 0;                        // 29
  virtual uint32_t get_type() = 0;                   // 30
  virtual int attach() = 0;                          // 31
  virtual int detach() = 0;                          // 32
};

// The create_sound mode-1 wave provider exposes the same 33-slot Sound
// prefix followed by Wave's observed extension. This is an external call
// view only: application Wave objects are not instances of this interface.
class WavePlaybackDeviceInterface : public SoundDeviceInterface {
 public:
  virtual int reload() = 0;                                      // 33
  virtual void init(char *, unsigned long) = 0;                  // 34
  // Expose the inherited overload beside its Wave overload group. A
  // using-declaration adds no virtual entry.
  using SoundDeviceInterface::load;
  virtual int load(char *, unsigned long) = 0;                   // 36
  virtual int load() = 0;                                        // 35, overload group
  using SoundDeviceInterface::play;
  virtual int play(int) = 0;                                     // 37
  virtual int set_pitch(int) = 0;                                // 38
  virtual int set_bufflimit(unsigned int) = 0;                   // 39
  virtual int set_asdr() = 0;                                    // 40
  virtual int set_attack(unsigned int, unsigned int, unsigned int) = 0; // 41
  virtual int set_sustain(unsigned int, unsigned int, unsigned int) = 0;// 42
  virtual int set_decay(unsigned int, unsigned int, unsigned int) = 0;  // 43
  virtual int set_release(unsigned int, unsigned int, unsigned int) = 0;// 44
  using SoundDeviceInterface::get_time;
  virtual int get_time(uint32_t) = 0;                            // 45
  virtual int get_current_marker() = 0;                           // 46
  virtual int get_ndevices() = 0;                                 // 47
  virtual int get_device_description(char *, int, int) = 0;       // 48
  virtual int get_ms_length() = 0;                                // 49
  virtual int is_hwbuffer() = 0;                                  // 50
  virtual int set_position3d(float, float, float) = 0;            // 51
  virtual int set_xpos(float) = 0;                                // 52
  virtual int set_ypos(float) = 0;                                // 53
  virtual int set_zpos(float) = 0;                                // 54
  virtual int is_3d() = 0;                                        // 55
  virtual int set_reverb_mix(float) = 0;                          // 56
};

class VoiceRxDeviceInterface : public SoundDeviceInterface {
 public:
  virtual int init(unsigned long, void *, unsigned long) = 0; // 33
  virtual int start() = 0;                                  // 34
  virtual int add_buffer(_MMIOINFO *) = 0;                   // 35
  virtual int get_buffer_size() = 0;                        // 36
  virtual int get_nbuffers() = 0;                           // 37
};

class VoiceTxDeviceInterface : public SoundDeviceInterface {
 public:
  virtual int init(unsigned long, void *, unsigned long) = 0; // 33
  virtual _MMIOINFO *get_next_buffer() = 0;                        // 34, borrowed MMIOINFO pointer
  virtual int return_buffer(_MMIOINFO *) = 0;                // 35
  virtual int start() = 0;                                  // 36
  virtual int get_nbuffers() = 0;                           // 37
};

// RECOVERY_BASE: v1 VoiceRx Sound 0x0 0x4c8960
class VoiceRx : public Sound {
 public:
  VoiceRx();
  virtual ~VoiceRx();
  virtual int unload();
  virtual int stop();
  virtual int release();
  // Shared dependency contracts; root owns the original shared identities.
  // RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_game_hwnd@VoiceRx@@UAEHXZ 0x4c8960
  virtual int get_game_hwnd() { return 0; }
  // RECOVERY_SHARED: v1 0x4c93e0 0x4c93e8 ?set_attrib@VoiceRx@@UAEHK@Z 0x4c8960
  virtual int set_attrib(unsigned long) { return 11; }
  // RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_attrib@VoiceRx@@UAEHXZ 0x4c8960
  virtual int get_attrib() { return 0; }
  virtual int init(unsigned long, void *, unsigned long); // 33
  virtual int start();                                  // 34
  virtual int add_buffer(_MMIOINFO *);                   // 35
  virtual int get_buffer_size();                        // 36
  virtual int get_nbuffers();                           // 37
};

// RECOVERY_BASE: v1 VoiceTx Sound 0x0 0x4c8cc0
class VoiceTx : public Sound {
 public:
  VoiceTx();
  virtual ~VoiceTx();
  virtual int unload();
  virtual int stop();
  virtual int release();
  // RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_game_hwnd@VoiceTx@@UAEHXZ 0x4c8cc0
  virtual int get_game_hwnd() { return 0; }
  // RECOVERY_SHARED: v1 0x4c93e0 0x4c93e8 ?set_attrib@VoiceTx@@UAEHK@Z 0x4c8cc0
  virtual int set_attrib(unsigned long) { return 11; }
  // RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_attrib@VoiceTx@@UAEHXZ 0x4c8cc0
  virtual int get_attrib() { return 0; }
  virtual int init(unsigned long, void *, unsigned long); // 33
  virtual _MMIOINFO *get_next_buffer();                        // 34, borrowed MMIOINFO pointer
  virtual int return_buffer(_MMIOINFO *);                // 35
  virtual int start();                                  // 36
  virtual int get_nbuffers();                           // 37

 private:
  // RECOVERY_MEMBER: v1 VoiceTx flags_54_ 0x54 0x4c8cc0
  uint32_t flags_54_;
};

// The actual 32-bit product must retain these assertions. Host syntax probes
// define VOICE_OWNER_HOST_PROBE because host pointer/vtable ABI is different.
#ifndef VOICE_OWNER_HOST_PROBE
static_assert(sizeof(VoiceRx) == 0x54, "VoiceRx Sound-only extent");
static_assert(sizeof(VoiceTx) == 0x58, "VoiceTx started flag extent");
#endif
