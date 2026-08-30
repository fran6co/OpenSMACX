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

#include "sound.h"          // VoiceTx's base
#include "stringstruct.h"      // StringAllocationBase - the virtual base of Net's subobjects
#include "hypothesis_layouts.h"  // NetFifo

 /*
  * VoiceTx class
  *
  * Net's first embedded subobject, and a real Sound: ??0VoiceTx (0x004C8CC0)
  * stages THREE vftables in construction order - 0x0066E444 (the family root,
  * which the IDB's member list mis-attributed to this class), 0x0066E3C0
  * (Sound's own, inlining ??0Sound store for store exactly as ??0Wave and
  * ??0Ambience do), then its own 0x0066E8C4 - and ??1VoiceTx (0x004C8DB0)
  * walks the same three back down. The voice dispatches in Net prove the
  * final table is what sits in the object: start_voice reads the subobject's
  * vptr and calls [vptr + 0x90] - slot 36 - and stop_voice calls
  * [vptr + 0x20] - slot 8 - of the SAME live pointer, which the old flat
  * model could not even name.
  *
  * The table at 0x0066E8C4 is 38 slots: Sound's 33 (0..32, with VoiceTx
  * overriding unload at 5, stop at 8 and release at 14) plus five of its own,
  * declared below in slot order. Two Sound-pure slots get real bodies here in
  * the image (15 and 28 answer through the 0x00406B30 `xor eax,eax; ret`
  * stub, 27 through a `mov eax,0xb; ret 4` getter); this tree leaves them
  * inherited from Sound's placeholders - their slot content is semantic debt
  * either way, because no claim measures the table's contents.
  *
  * The catalogue spells release `?release@VoiceTx@@QAEXXZ` (void), but both
  * of its epilogues are `xor eax,eax; ret` - the shape that CORRECTED
  * unload's catalogue name to int when unload's body was promoted out of
  * src/recovered/004c8f40.cpp, and the same test the int-returning redirect
  * that held these bytes passes. Measured int wins here too.
  *
  * THE OLD FLAT MODEL IS WITHDRAWN. It held 22 uint32_t members whose first
  * "field" was really the vptr, defended by two agreeing member sources -
  * but both sources catalogue offsets, and neither could see that ??0VoiceTx
  * stores 0x66E8C4, not the 0x66E444 its first stage writes. The size bound
  * survives unchanged: sizeof(Sound) is 0x54, one dword follows, and ??0Net
  * still constructs this at `lea ecx, [esi+0x58]` with Net's next subobject
  * at 0xB0 - exactly 0x58 of room.
  */
class VoiceTx : public Sound {
 public:
  // 0x004C8CC0, a pending_bodies forwarder - not this batch's address.
  VoiceTx();
  // NO DECLARED DESTRUCTOR. The old pending_bodies forwarder for 0x004C8DB0
  // is retired: with a Sound base the compiler-generated ??1VoiceTx calls the
  // REAL, claimed ~Sound (src/sound.cpp) once, which is both a smaller
  // compiled form and the first teardown path in this tree that runs instead
  // of faulting. The image's own ??1VoiceTx inlines the whole three-stage
  // descent and stays unrecovered.

  // THE VIRTUAL SET VoiceTx ADDS OR OVERRIDES, in 0x0066E8C4 slot order.
  // Sound's other slots (set_fade .. detach, 0..32) are inherited untouched.
  // init/get_next_buffer/return_buffer/get_nbuffers are unrecovered bodies -
  // placeholder definitions in net_class.cpp keep the slots true, semantic
  // debt until named, exactly like Sound's unk_slots in sound.cpp.
  // The overrides (unload/stop/release) would inherit virtualness from
  // Sound; the own slots BELOW THEM would not - a missing `virtual` on
  // start compiled start_voice's dispatch into a plain direct call and
  // regressed its claim - so every one spells the keyword.
  virtual int unload();                          // slot  5 (0x4C8F40)
  virtual int stop();                            // slot  8 (0x4C8F00), the
                                                 // voice stop Net dispatches
  virtual int release();                         // slot 14 (0x4C8EA0)
  virtual int init(unsigned long, void *, unsigned long);  // 33 (0x4C8F70)
  virtual int get_next_buffer();                 // slot 34 (0x4C9030)
  virtual int return_buffer(struct _MMIOINFO *);  // slot 35 (0x4C9050)
  virtual int start();                           // slot 36 (0x4C8EB0), the
                                                 // voice start Net dispatches
  virtual int get_nbuffers();                    // slot 37 (0x4C9010)

 private:
  // Bit 0 is the voice-started bit: set by start once the device chain has
  // answered, tested by stop (both `test byte ptr [this + 0x54], 1`), and
  // read by Net as the 0x20000000 flag word at +0xD8 alongside it.
  uint32_t flags_54_;  // 0x54
};

static_assert(sizeof(VoiceTx) == 0x58,
              "VoiceTx layout must match the original executable");

 /*
  * Net class
  *
  * Layout not established. "Reach as far as 0x770" stood here and is
  * withdrawn: the measurement is 0x768 - docs/recovery/access-lower-bounds.csv
  * records `Net,0x768` from `mov dword ptr [esi + 0x764], ebx` at 0x0062D850
  * in ??0Net, and no row of docs/recovery/member-accesses.csv attributed to
  * Net goes higher. A reach is a LOWER bound, so the object is at least
  * 0x768; tools/derive_class_layout.py finds no size evidence for Net at all,
  * and the 0x18 that carries the total to 0x780 below is pad, not a
  * measurement. The stubs recovered here are constant returns touching no
  * field, so the opaque storage below is only an object for the canary to
  * seed, not a modelled layout.
  *
  * That pad is now load-bearing: src/alphanet.h derives AlphaNet from Net, so
  * this size is where AlphaNet's own storage begins. Correcting it will fail
  * `static_assert(sizeof(AlphaNet) == 0x14A0)` there, which is the intended
  * alarm - re-measure AlphaNet's `data_` rather than silence it.
  */
// THE NET EMBEDDED RECEIVER at Net+0xB0: two virtual slots (the image's
// vftable 0x66EAFC, both entries leaves) over a virtual StringAllocationBase
// (vbtable 0x670DCC places the base at +0x18 - Net+0xCC). The construction
// staging the register transcription spelled by hand is the compiler's now.
class NetUnk72C : public virtual StringAllocationBase {
 public:
  virtual void unk_slot0();  // 0x005D4C50, unrecovered
  virtual void unk_slot1();  // 0x005D4CC0, unrecovered

 private:
  uint8_t field_8_[0x1C];  // 0x8..0x23, opaque
};

class Net {
 public:
  // ?start_voice@Net@@QAEHK@Z at 0x0062DF20. `K`, not `I` - the image spells
  // the argument `unsigned long`, and the wrong spelling stops the symbol
  // matching. PROMOTED out of src/recovered/0062df20.cpp; its
  // pending_bodies forwarder is gone.
  int start_voice(unsigned long key);
  // ?stop_voice@Net@@QAEXXZ at 0x0062DFC0. PROMOTED out of
  // src/recovered/units/0062dfc0.cpp; its pending_bodies forwarder is gone.
  void stop_voice();
  // 0x0062F8A0, still unrecovered - a 2015-byte reliable-UDP-style sender
  // with per-peer RTT tracking. `stop_voice` calls it; the declaration lets
  // its pending_bodies forwarder carry the receiver, so the call site keeps
  // the image's own `E8`.
  int send_packet_type(void *data, int mode, unsigned long who, int size,
                       int flags);
  int get(unsigned long *a, unsigned long *b);

 public:
  // 0x0062E010, a pending_bodies forwarder.
  void close();

 public:
  // 0x00631A60 and 0x006320E0, pending_bodies forwarders. `do_net` and
  // `check_net` TAIL JUMP to these - `jmp 0x631a60` and `jmp 0x6320e0`, read
  // off the image rather than guessed; my first pass wrote two addresses that
  // were nothing. Neither body is recovered yet, but naming them is what lets
  // those two emit the image's `E8`.
  void process_voice();
  void check_polling();

 public:
  // 0x0062D9E0, a pending_bodies forwarder. control_game (game.cpp) calls it
  // once at boot with the command line and the DirectPlay application GUID.
  int check_for_lobby(char *app_name, _GUID *guid, int option_a,
                      int option_b);

 public:
  Net();
  ~Net();
  int poll_players(int);
  // `K`, not `I`. MSVC decorates `unsigned long` K and `unsigned int` I,
  // and this tree spells uint32_t as unsigned int - same width, different
  // decorated name, and the name is what the linker pairs on.
  char *get_player_name(unsigned long key);

 private:
  uint32_t field_0_;  // 0x0
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint32_t field_10_;  // 0x10
  uint32_t field_14_;  // 0x14
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  uint8_t field_2C_[0x18];  // 0x2C
  uint32_t field_44_;  // 0x44
  uint32_t field_48_;  // 0x48
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  // 0x58 - THE SUBOBJECT ??0Net BUILDS, not opaque storage that happens to
  // be 0x58 wide. `lea ecx, [esi+0x58]` / `call ??0VoiceTx@@QAE@XZ` at
  // 0x0062D6C9, and ??1Net tears it down at the same offset. The two members
  // this replaces covered exactly 0x58 bytes, so nothing moves.
  VoiceTx voice_tx_;  // 0x58
  JackalVoiceRx jackal_voice_rx_;  // 0xB0 (vbase StringAllocationBase at 0xCC)
  // The voice-transmitter state between the receiver and the fifos.
  uint32_t voice_key_D4_;      // 0xD4, start_voice's key
  uint32_t voice_flags_D8_;    // 0xD8, 0x20000000 started / 0x60000000 in-flight
  uint8_t field_DC_[0xC];      // 0xDC..0xE7, opaque
  NetFifo netfifo_[3];  // 0xE8, 0x10C, 0x130 - the three message queues
  uint32_t field_154_;  // 0x154
  uint32_t field_158_;  // 0x158
  uint32_t field_15C_;  // 0x15C
  uint8_t field_160_[0x4];  // 0x160
  uint32_t field_164_;  // 0x164
  uint8_t field_168_;  // 0x168
  uint8_t field_169_[0x56B];  // 0x169
  uint32_t field_6D4_;  // 0x6D4
  uint8_t field_6D8_[0x4];  // 0x6D8
  uint32_t field_6DC_;  // 0x6DC
  uint32_t field_6E0_;  // 0x6E0
  uint32_t field_6E4_;  // 0x6E4
  uint32_t field_6E8_;  // 0x6E8
  uint8_t field_6EC_[0x10];  // 0x6EC
  uint32_t field_6FC_;  // 0x6FC
  // THE 0x700 FAMILY - a second JackalVoiceRx-shaped receiver at 0x700
  // (its final table is the same two leaf slots), then an unknown
  // two-virtual class at 0x72C whose virtual base is a StringAllocationBase
  // at 0x750, then one more bare StringAllocationBase at 0x758. The image
  // stages all of these by hand inside ??0Net; here the classes construct
  // and the owner captures run implicitly.
  JackalVoiceRx jackal_voice_rx_700_;  // 0x700
  uint8_t field_724_[0x8];  // 0x724..0x72B, opaque
  NetUnk72C unk_72c_;       // 0x72C (vbase StringAllocationBase at 0x750)
  StringAllocationBase unk_base_758_;  // 0x758
  // 0x760/0x764 are close()'s timer words and 0x768 the slot count
  // AlphaNet::close zeroes; the rest is opaque to 0x780.
  uint8_t field_760_[0x20];  // 0x760..0x77F
};

// Pinned by AlphaNet's own assert: AlphaNet = Net + data_[0xD20] = 0x14A0.
static_assert(sizeof(Net) == 0x780, "Net is the 0x780-byte base AlphaNet pads from");



// THE LIVE NET AND THE FLAG THAT GATES IT. `do_net` and `check_net` both read
// 0x009BE608 for the object and 0x009BE600 for the enable flag, and do nothing
// unless both are set. Defined here rather than declared `extern`, so each
// unit folds them to the image's absolute operand - see the note in
// `init_thunks.h`.
// AN `inline T *&` ACCESSOR, not a `Net **const`. The latter is a variable
// holding the address, so `*NetCurrent` compiles TWO loads - `mov eax, [addr];
// mov ecx, [eax]` - where the image has one, `mov ecx, [0x9be608]`. This is
// the same form `BaseCurrent()` and `console_map_win()` already use.
inline Net *&NetCurrent() { return *reinterpret_cast<Net **>(0x009BE608); }
uint32_t *const NetEnabled = (uint32_t *)0x009BE600;

// Net::get's scratch destination: passed BY ADDRESS to NetFifo::get as the
// buffer to fill, and that same address is the "got a message" return value,
// not the message's own bytes - Net::get never reads through it. An
// `extern` symbol, not a plain literal, keeps the relocation the image emits
// at each of the two uses.
extern uint32_t NetGetScratch;  // 0x009BC4BC

// 0x009BE608, the live Net the class registers itself in. Declared here
// rather than in net_class.cpp because win.cpp reads it too - it used to
// carry its own `int *const` binding to the same address, which is one
// address under two names.
static Net **const WinNetBuffer = reinterpret_cast<Net **>(0x009BE608);
