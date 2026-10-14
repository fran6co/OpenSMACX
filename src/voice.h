#pragma once

// Sound-family voice owners and the DLL-provided device contracts.
#include "sound.h"
#include "soundx_sound_abi.h"
struct _MMIOINFO;

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
  virtual bool is_3d() = 0;                                       // 55
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
