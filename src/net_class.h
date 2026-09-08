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
  * Layout partly established, and now partly MODELLED: init (0x0062D8E0) gave
  * the 0x0..0x50 words their roles - thread id, five events, the DirectPlay
  * pointer, the self registration at 0x20, the buffer pointer pair and the
  * queue lock at 0x2C - and the sixteen 0x58-byte player slots at 0x154 are
  * NetPlayer below. "Reach as far as 0x770" stood here and is withdrawn: the
  * measurement is 0x768 - docs/recovery/access-lower-bounds.csv records
  * `Net,0x768` from `mov dword ptr [esi + 0x764], ebx` at 0x0062D850 in
  * ??0Net, and no row of docs/recovery/member-accesses.csv attributed to Net
  * goes higher. A reach is a LOWER bound, so the object is at least 0x768;
  * tools/derive_class_layout.py finds no size evidence for Net at all, and
  * the 0x18 that carries the total to 0x780 below is pad, not a measurement.
  * Everything the members above do not name stays opaque storage.
  *
  * That pad is now load-bearing: src/alphanet.h derives AlphaNet from Net, so
  * this size is where AlphaNet's own storage begins. Correcting it will fail
  * `static_assert(sizeof(AlphaNet) == 0x14A0)` there, which is the intended
  * alarm - re-measure AlphaNet's `data_` rather than silence it.
  */
// Provisional node identity: the receiver's circular list has separate
// allocation-owning entries and payload objects. Their complete declarations
// and destruction are still recovery work; do not reuse StringStruct's
// string-payload type merely because its entry has the same physical layout.
struct VoiceReceiverEntry;

// Net's receiver list at +0xB0. Original constructor 0x0062D6CE..0x0062D733
// places the vbptr at +4, virtual StringAllocationBase at +0x1C, and clears
// all five list-state fields. Complete extent is 0x24, not the old 0x18 hint.
// The hooks at 0x0066EAFC dispatch to shared leaves 0x00404250 (zero, ret 4)
// and 0x00404270 (ret 4). Insertion 0x00633180 passes the newly constructed
// payload to the first hook; cleanup 0x00538EC0 passes it to the second.
// Pointer parameter types follow those callers, not the unrelated AlphaMovie
// catalogue labels sharing the same leaf bytes. Payload's precise static type
// remains unresolved here.
class JackalVoiceRx : public virtual StringAllocationBase {
 public:
  // Body assignments preserve the original table-then-state initialization
  // order under VC6; member initializers put these stores before its vfptr.
  JackalVoiceRx() {
    head_ = 0;
    cursor_ = 0;
    count_ = 0;
    cursor_index_ = 0;
    entries_heap_ = 0;
  }
  virtual int on_insert(void *payload);
  virtual void on_remove(void *payload);

 private:
  // 0x00538EC0 advances through entry+0xC, stores the cursor, and clears
  // head/count/index after deleting the payload and allocation-owning node.
  // RECOVERY_MEMBER: v1 JackalVoiceRx head_ 0x08 0x0062D6A0
  VoiceReceiverEntry *head_;    // 0x08
  // RECOVERY_MEMBER: v1 JackalVoiceRx cursor_ 0x0C 0x0062D6A0
  VoiceReceiverEntry *cursor_;  // 0x0C
  // RECOVERY_MEMBER: v1 JackalVoiceRx count_ 0x10 0x0062D6A0
  int count_;                   // 0x10
  // RECOVERY_MEMBER: v1 JackalVoiceRx cursor_index_ 0x14 0x0062D6A0
  int cursor_index_;            // 0x14, insertion stores the previous count
  // 0x00633180 tries this explicit Heap before the virtual base's saved
  // allocation owner when allocating the 0x1C entry and 0x60 payload.
  // RECOVERY_MEMBER: v3 JackalVoiceRx entries_heap_ Heap* 0x18 0x0062D6A0
  Heap *entries_heap_;          // 0x18
};

// The list subobject at Net+0x72C, not the receiver at Net+0xB0.
// Net construction (0x0062D788..0x0062D7FD) installs its vbptr at +4;
// vbtable 0x00669424 locates its allocation base at relative +0x24.
// Its list fields, virtual surface and two-stage teardown remain unfinished.
class NetUnk72C : public virtual StringAllocationBase {
 public:
  virtual void unk_slot0();  // 0x005D4C50, unrecovered
  virtual void unk_slot1();  // 0x005D4CC0, unrecovered

 private:
  uint8_t field_8_[0x1C];  // 0x8..0x23, opaque
};

// Provisional name for the distinct allocation-owning node at Net+0x700.
// Original construction at 0x0062D763 installs a vbptr at relative zero:
// vbtable 0x00669430 locates StringAllocationBase at +0x58. Construction
// captures that base's owner before constructing the list at +0x2C;
// destruction through 0x00402CC0 adjusts the base receiver back by 0x58
// and delegates the list teardown to 0x00402DD0. The separate allocator
// path at 0x00401ED3 requests 0x60 bytes and installs the same vbtable.
// Thus the base at Net+0x758 belongs to this object, not to Net as another
// sequential member. This is not a second JackalVoiceRx: that class's
// vbptr is at +4 and its virtual base is at +0x1C. Remaining node fields
// and the list's complete cleanup still need recovery.
class NetAllocationNode : public virtual StringAllocationBase {
 private:
  uint8_t field_4_[0x28];  // 0x4, roles not yet recovered
  // RECOVERY_MEMBER: v2 NetAllocationNode entries_ NetUnk72C 0x2C 0x0062D6A0
  NetUnk72C entries_;  // 0x2C
};

static_assert(sizeof(NetAllocationNode) == 0x60,
              "allocation node extent from original allocation and virtual base");

/* One of the sixteen 0x58-byte player slots at Net+0x154. Two independent
   reads: init() clears the words at 0x0, 0x4, 0x8 and 0xC and ONE BYTE at
   0x10 over sixteen slots of 0x58 bytes, 0x154..0x6D4; get_player_name
   (0x00631A20) reads the name text at slot + 0x15 - image 0x169 - stride
   0x58; and check_polling (0x006320E0) reads and writes the DWORD at slot +
   0x10 as a millisecond timestamp (`mov ebx,[edi+0x10]` /
   `mov dword ptr [edi+0x10],eax`), which is what makes it a dword rather
   than the byte flag the init store alone would suggest. */
class NetPlayer {
 public:
  uint32_t field_0_;  // 0x0
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint32_t stamp_10_;  // 0x10, timeGetTime() of the last packet from this
                       // player - the staleness clock check_polling reaps on
  uint8_t field_14_[0x44];  // 0x14..0x57, the name text starts at 0x15
};

struct ServiceList;

class Net {
 public:
  // ?start_voice@Net@@QAEHK@Z at 0x0062DF20. `K`, not `I` - the image spells
  // the argument `unsigned long`, and the wrong spelling stops the symbol
  // matching. PROMOTED out of src/recovered/0062df20.cpp; its
  // pending_bodies forwarder is gone.
  int start_voice(unsigned long key);
  // 0x0062DF70, homed into net_class.cpp. `J`, not `K` - the second
  // start_voice spells its key signed, and the wrong spelling moves the
  // symbol. It guards on the 0xD8 flag word, remembers the key at 0xD4 and
  // starts the VoiceTx subobject.
  int start_voice(long key);
  // 0x00632300, homed into net_class.cpp: clears the sixteen polling stamps
  // at 0x164 and sets the 0xD8 polling bit to match.
  void set_polling(int flag);
  // ?stop_voice@Net@@QAEXXZ at 0x0062DFC0. PROMOTED out of
  // src/recovered/units/0062dfc0.cpp; its pending_bodies forwarder is gone.
  void stop_voice();
  // 0x0062F8A0, still unrecovered - a 2015-byte reliable-UDP-style sender
  // with per-peer RTT tracking. `stop_voice` calls it; the declaration lets
  // its pending_bodies forwarder carry the receiver, so the call site keeps
  // the image's own `E8`.
  int send_packet_type(void *data, int mode, unsigned long who, int size,
                       int flags);
  // 0x0062F390, still unrecovered (1293 bytes). process_voice routes a voice
  // packet here instead of send_packet_type whenever field_54_ is set, and it
  // calls close/check_polling like the rest of the session paths, so it is a
  // member with the same five arguments. `sub_` is the tree's name for a body
  // nothing has identified yet; the forwarder that gives its callers the
  // image's `E8` lives in net_class.cpp beside process_voice.
  int sub_62f390(void *data, int mode, unsigned long who, int size, int flags);
  // 0x00632350, still unrecovered (966 bytes, archived). check_polling calls
  // it to cut a stale player - by key, or 0xFFFFFFFF for everyone - and reads
  // a zero answer as "cut", which is when it refreshes the player's stamp.
  int drop_player(unsigned long key, int reason);
  int get(unsigned long *a, unsigned long *b);
  // 0x0062EDB0 and 0x0062EDD0, homed into net_class.cpp: the DirectPlay
  // group calls. add_to_group's two ids travel in this order; the catalogue
  // does not say which is the group and which the member.
  void delete_group(int group);
  int add_to_group(long group, unsigned long member);
  // 0x0062E250, homed into net_class.cpp. ServiceList is opaque here - only
  // the pointer travels - so a forward declaration keeps the catalogue's
  // `PAUServiceList@@` spelling.
  int poll_services(ServiceList *list);
  // 0x00630A40, homed into net_class.cpp: reads one message off the middle
  // queue. No catalogue name, so the fact records what this tree emits.
  int sub_630a40(int first, int second);
  // 0x00630980, homed into net_class.cpp: the sender's twin, answering the
  // buffer's address through the queue's own reader.
  int sub_630980(int first, int second);
  // 0x006309C0, homed into net_class.cpp: the sibling sender on the second
  // reader of the same queue.
  int sub_6309c0(int first, int second);
  // 0x00630080, archived in 00630080.cpp (2232 bytes). send_message hands it
  // every packet it builds, so the receiver has to be real for that call to
  // keep the image's `E8`.
  int send(void *data, int mode, unsigned long who, int size);

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
  // 0x0062D8E0, a pending_bodies forwarder. check_for_lobby opens with it.
  int init(_GUID *guid, int option_a, int option_b);

 public:
  // 0x0062E540, a pending_bodies forwarder. SessionStruct is opaque to this
  // tree - only the pointer travels through here, and check_for_lobby passes
  // zero for it.
  int join_session(struct SessionStruct *session, char *player_name,
                   char *address);

 public:
  // 0x0062D9E0, recovered into net_class.cpp. control_game (game.cpp) calls
  // it once at boot with the command line and the DirectPlay application GUID.
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
  // init() fills the next six: the thread that owns the session, five events
  // it creates, and the DirectPlay interface copied out of 0x009BE600.
  uint32_t thread_id_4_;  // 0x4, GetCurrentThreadId
  HANDLE events_8_[5];  // 0x8, 0xC, 0x10, 0x14, 0x18 - CreateEventA(NULL...)
  uint32_t field_1C_;  // 0x1C, the DirectPlay interface pointer
  Net *self_20_;  // 0x20, this object's own address: a session is open when
                  // it is non-null, which is what close() tests
  void *buffer_24_;  // 0x24, the 0x2000-byte DirectPlay buffer
  uint32_t field_28_;  // 0x28, set to 1 once the lock below exists
  CRITICAL_SECTION lock_2C_;  // 0x2C, guards the fifo queue heads
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
  // The keep-alive pair check_polling drives: the clock reading it last sent
  // a 0x20 packet at, and the staleness threshold a player stamp is measured
  // against (close() writes 0x4E20 - twenty seconds - into the latter).
  uint32_t poll_time_DC_;      // 0xDC, timeGetTime() of the last keep-alive
  uint32_t poll_timeout_E0_;   // 0xE0, the staleness threshold in ms
  uint32_t field_E4_;          // 0xE4, opaque - close() writes 1 here
  NetFifo netfifo_[3];  // 0xE8, 0x10C, 0x130 - the three message queues
  NetPlayer players_[16];  // 0x154, sixteen 0x58-byte slots to 0x6D4
  uint32_t field_6D4_;  // 0x6D4
  uint32_t field_6D8_;  // 0x6D8, init() sets 0xC8 here
  uint32_t field_6DC_;  // 0x6DC
  void *buffer_6E0_;  // 0x6E0, the same 0x2000-byte buffer as buffer_24_
  uint32_t field_6E4_;  // 0x6E4, its size
  _GUID *guid_6E8_;  // 0x6E8, the session GUID init() was handed
  uint8_t field_6EC_[0x10];  // 0x6EC
  uint32_t field_6FC_;  // 0x6FC
  // The node owns the list at +0x72C and virtual allocation base at +0x758.
  // Its list has a distinct allocation base at +0x750.
  // RECOVERY_MEMBER: v2 Net allocation_node_ NetAllocationNode 0x700 0x0062D6A0
  NetAllocationNode allocation_node_;  // 0x700
  // 0x760 and 0x764 are the two words check_polling compares to decide
  // whether the whole player table is stale or only a slice of it - close()
  // zeroes both - and 0x768 the slot count AlphaNet::close zeroes; the rest
  // is opaque to 0x780.
  uint32_t field_760_;       // 0x760
  // Read by NetDaemon::send_message (netdaemon.cpp) as the substitution for
  // an out-of-range target, so it is reachable from the derived class rather
  // than a this-pun over the offset.
 public:
  uint32_t field_764_;       // 0x764
 private:
  uint8_t field_768_[0x18];  // 0x768..0x77F, opaque
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

// THE TWO RE-ENTRANCY GUARDS beside them, each set to 1 while its own pass is
// inside the code that must not be re-entered and cleared on the way out.
// check_polling sends its 0x20 keep-alive under the 0x9BE610 flag and reaps
// stale players under the 0x9BE60C one; the same `uint32_t *const` binding as
// NetEnabled, so the reads and the stores fold to the image's absolute
// operand.
uint32_t *const NetReapBusy = (uint32_t *)0x009BE60C;
uint32_t *const NetKeepAliveBusy = (uint32_t *)0x009BE610;

// The game's imported timeGetTime, as check_polling reads it: the slot the
// import table hands over, not the function - `NetTimeGetTimeSlot()` returns
// the live pointer, so the call goes through it the way the image's
// `call dword ptr [0x669368]` does. wave.h binds the same slot for its own
// timing; this file keeps its own typedef so the link closure stays
// self-contained.
typedef unsigned long(__stdcall *func_net_time)();
inline func_net_time &NetTimeGetTimeSlot() {
    return *reinterpret_cast<func_net_time *>(0x00669368);
}

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
