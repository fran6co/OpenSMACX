#pragma once

#include "vc6_compat.h"

class Sound;

// Pinned SoundX SHA-256 ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5.
// SoundX mode-1 and mode-2 constructors transiently install root
// 0x10064D6C and Sound 0x10064CCC at offset zero before derived tables
// 0x10065738 and 0x10064794. The mode-5 factory's constructor chain reaches
// the mode-2/Midi constructor directly; its first 33 entries match mode 2
// except destructor slot 2 and load slot 4, while unload slot 5 is identical.
// Class names are reconstructed and less-inspected signatures remain
// provisional. Mode 5 calls 0x10001122 -> 0x100039C0, which calls
// 0x100022A7 -> 0x1001B5E0 before the final table 0x10064000 is installed.

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
