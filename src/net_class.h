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

struct IDirectPlay2;

#include "sound.h"          // VoiceTx's base
#include "stringstruct.h"      // StringAllocationBase - the virtual base of Net's subobjects
#include "hypothesis_layouts.h"  // NetFifo
// DPNAME names the player-callback record enum_players reads; the typedef
// has no struct tag, so the DirectPlay header is what makes the parameter
// spellable in the class's own declaration.
#include <dplay.h>

#include "voice.h"

// Original 0x00633180 allocates 0x60 bytes, constructs VoiceRx at +0, then
// publishes primary table 0x670F48 and virtual-base table 0x670FE0. The latter
// gives StringAllocationBase at +0x58. This models the observed wrapper as a
// genuine derived object so construction and destruction own those stages.
class VoiceRxAllocation : public VoiceRx,
                          public virtual StringAllocationBase {
};
static_assert(sizeof(VoiceRxAllocation) == 0x60,
              "voice receiver allocation extent from 633180");

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
  * goes higher. A reach is a LOWER bound, so the object is at least 0x768.
  * AlphaNet's count at +0x768 and name-bearing records at +0x76C now replace
  * the former unsupported tail pad; see alphanet.h for the caller evidence.
  * Everything the members above do not name stays opaque storage.
  *
  * The source models the derived player count as AlphaNet-owned, rather than
  * inventing additional Net storage. Layout checks reprove the derived array
  * position and extent; the standalone Net allocation size is not known.
  */
// A circular-list entry, not its separately allocated voice payload.
// Insertion 0x00633180 requests 0x1C bytes and installs vbtable 0x006693C8:
// StringAllocationBase is at +0x14, with ID/payload/next/previous before it.
// VoiceRx construction 0x004C8960 stages the Sound base before its own table.
struct VoiceReceiverEntry : public virtual StringAllocationBase {
  int id_;                       // 0x04
  VoiceRxAllocation *payload_;   // 0x08
  VoiceReceiverEntry *next_;     // 0x0C
  VoiceReceiverEntry *previous_; // 0x10

  // Original node deletion 0x00402C20 republishes the saved allocation
  // owner, frees the complete node only without an owner, then clears the
  // handoff. The inherited destructor owns publication; this deallocator
  // owns the storage decision. Never free an individual Heap-owned block.
  static void operator delete(void *memory) {
    if (StringAllocationHeap == 0) {
      ::operator delete(memory);
    }
    StringAllocationHeap = 0;
  }
};

struct NetThreadInfo;

// Net's receiver list at +0xB0. Original constructor 0x0062D6CE..0x0062D733
// places the vbptr at +4, virtual StringAllocationBase at +0x1C, and clears
// all five list-state fields. Complete extent is 0x24, not the old 0x18 hint.
// The hooks at 0x0066EAFC dispatch to shared leaves 0x00404250 (zero, ret 4)
// and 0x00404270 (ret 4). Insertion 0x00633180 passes the newly constructed
// VoiceRxAllocation payload to the first hook; cleanup 0x00538EC0 passes it
// to the second. Pointer parameter types follow those callers, not the
// unrelated AlphaMovie catalogue labels sharing the same leaf bytes.
class JackalVoiceRx : public virtual StringAllocationBase {
  // Net::drop_player unlinkes one entry here directly - the image inlines the
  // walk inside the Net member - so the receiver's fields are Net-visible.
  friend class Net;
  // NetThreadInfo::internal_receive retains the original ring cursor updates
  // while dispatching voice packets; keep the ring fields private to owners.
  friend struct NetThreadInfo;
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
  // Original Net teardown drains the receiver after the queues, resets its
  // cursor index even when empty, then publishes the virtual base's owner.
  // Member destruction owns this sequence; containing classes must not repeat it.
  ~JackalVoiceRx() {
    clear();
    cursor_index_ = 0;
  }
  virtual int on_insert(void *payload);
  virtual void on_remove(void *payload);
  void clear();
  int add_player(int player_id); // original sub_633180

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
  // 0x00633180 prefers this explicit Heap for the first 0x1C entry and for
  // every 0x60 payload; append entries instead use the inherited saved owner.
  // RECOVERY_MEMBER: v3 JackalVoiceRx entries_heap_ Heap* 0x18 0x0062D6A0
  Heap *entries_heap_;          // 0x18
};

// The list subobject at Net+0x72C, not the receiver at Net+0xB0.
// Net construction (0x0062D788..0x0062D7FD) installs its vbptr at +4;
// vbtable 0x00669424 locates its allocation base at relative +0x24.
// Its list fields, virtual surface and two-stage teardown remain unfinished.
class NetTextPayload : public virtual StringAllocationBase {
 public:
  char *text;              // observed +04; removal hook owns string release
  uint32_t field_8_;       // observed +08; insertion hook clears this word
  // No field_C_, unseen tail or asserted complete extent is invented.
  // Virtual-base conversion/delete uses the object's actual vbtable.
};
struct NetTextListEntry : public virtual StringAllocationBase {
  int id;                       // observed link-prefix +04
  NetTextPayload *payload;       // +08, actual hook/deletion role
  NetTextListEntry *next;         // +0C
  NetTextListEntry *previous;     // +10
  // Complete allocation extent/nominal factory remain an explicit obligation.
};
class NetTextListBase : public virtual StringAllocationBase {
 public:
  typedef NetTextListEntry Entry;
  typedef NetTextPayload Payload;
  virtual int on_insert(Payload *payload);
  virtual void on_remove(Payload *payload);
  NetTextListBase();
  virtual ~NetTextListBase();
  void remove_all();
  int seek_pos(int position);
  int current_id();
  Payload *current_entry();
  Payload *next_entry();

  Entry *head_;                  // +08
  Entry *current_;               // +0C
  int entry_count_;              // +10
  int current_position_;         // +14
  Heap *allocator_;              // +18
};
class AlphaNetStringStruct;
class NetUnk72C : public NetTextListBase {
 public:
  NetUnk72C();
  virtual ~NetUnk72C();
  virtual int on_insert(Payload *payload);  // 5D4C50
  virtual void on_remove(Payload *payload); // 5D4CC0
  const char *source_text_;                 // +1C, borrowed hook input
  uint32_t field_20_;                       // +20, role not established
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
// vbptr is at +4 and its virtual base is at +0x1C. The current node has
// named session fields and a genuine nested-list destructor; unseen nested
// payload factory/nominal identity and complete lifetime proof remain open.
class NetAllocationNode : public virtual StringAllocationBase {
  friend class Net;
 public:
  NetAllocationNode();
  virtual ~NetAllocationNode();
  // RECOVERY_MEMBER: v1 NetAllocationNode session_name_ 0x4 0x005D4890
  char *session_name_;           // +04, caller hook owns string cleanup
  // RECOVERY_MEMBER: v1 NetAllocationNode password_ 0x8 0x005D4890
  char *password_;               // +08, caller hook owns string cleanup
  // RECOVERY_MEMBER: v1 NetAllocationNode instance_guid_ 0xC 0x005D4890
  GUID instance_guid_;           // +0C
  // RECOVERY_MEMBER: v1 NetAllocationNode session_flags_ 0x1C 0x005D4890
  uint32_t session_flags_;        // +1C
  // RECOVERY_MEMBER: v1 NetAllocationNode max_players_ 0x20 0x005D4890
  uint32_t max_players_;          // +20
  // RECOVERY_MEMBER: v1 NetAllocationNode current_players_ 0x24 0x005D4890
  uint32_t current_players_;      // +24
  // RECOVERY_MEMBER: v1 NetAllocationNode field_28_ 0x28 0x005D4890
  uint32_t field_28_;             // +28

  // Constructor-throw cleanup must match native 402320. A global placement
  // new(void*) has a no-op matching delete and cannot supply this behavior.
  // The two contexts preserve primary receiver vs explicit Heap identities.
  static void *operator new(size_t size, Heap *owner) throw();
  static void *operator new(size_t size, AlphaNetStringStruct *list) throw();
  static void operator delete(void *memory, Heap *owner);
  static void operator delete(void *memory, AlphaNetStringStruct *list);
  static void operator delete(void *memory);

 private:
  // RECOVERY_MEMBER: v2 NetAllocationNode entries_ NetUnk72C 0x2C 0x0062D6A0
  NetUnk72C entries_;             // +2C, natural member destruction
};

static_assert(sizeof(NetAllocationNode) == 0x60,
              "allocation node extent from original allocation and virtual base");

/* One of the sixteen 0x58-byte player slots at Net+0x154. Two independent
   reads: init() clears the words at 0x0, 0x4, 0x8 and 0x10 and the byte at
   0x14 over sixteen slots of 0x58 bytes, 0x154..0x6D4; get_player_name
   (0x00631A20) reads the name text at slot + 0x15 - image 0x169 - stride
   0x58; and check_polling (0x006320E0) reads and writes the DWORD at slot +
   0x10 as a millisecond timestamp (`mov ebx,[edi+0x10]` /
   `mov dword ptr [edi+0x10],eax`). The separate byte at 0x14 is displayed
   as Flags by MultiDebug. */
class NetPlayer {
 public:
  uint32_t field_0_;  // 0x0
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint32_t stamp_10_;  // 0x10, timeGetTime() of the last packet from this
                       // player - the staleness clock check_polling reaps on
  // RECOVERY_MEMBER: v1 NetPlayer flags_14_ 0x14 0x005C9920
  uint8_t flags_14_;
  // Text starts at +0x15. This covers storage to the independently observed
  // +0x58 slot boundary, NOT a recovered maximum name length: the original
  // name capacity and any trailing padding are still unknown.
  // RECOVERY_MEMBER: v1 NetPlayer name_and_tail_ 0x15 0x00631A20
  char name_and_tail_[0x43];
};

class Net;
// Embedded at Net+4; offsets match the worker argument and accesses in
// 0x00632740/0x006327E0. The primary Net vfptr occupies Net+0.
struct NetThreadInfo {
  // RECOVERY_MEMBER: v1 NetThreadInfo thread_id_ 0x0 0x0062D8E0
  DWORD thread_id_;                    // +00 == Net+04
  // RECOVERY_MEMBER: v4 NetThreadInfo events_ HANDLE[5] 0x4 0x14 0x0062D8E0
  HANDLE events_[5];                   // +04..+14 == Net+08..+18
  // RECOVERY_MEMBER: v3 NetThreadInfo direct_play_ IDirectPlay2* 0x18 0x0062D8E0
  IDirectPlay2 *direct_play_;          // +18 == Net+1C
  // RECOVERY_MEMBER: v3 NetThreadInfo owner_ Net* 0x1C 0x0062D8E0
  Net *owner_;                         // +1C == Net+20
  // RECOVERY_MEMBER: v1 NetThreadInfo buffer_ 0x20 0x0062D8E0
  void *buffer_;                       // +20 == Net+24
  // RECOVERY_MEMBER: v1 NetThreadInfo lock_initialized_ 0x24 0x0062D8E0
  DWORD lock_initialized_;             // +24 == Net+28
  // RECOVERY_MEMBER: v2 NetThreadInfo receive_lock_ CRITICAL_SECTION 0x28 0x0062D8E0
  CRITICAL_SECTION receive_lock_;      // +28..+3F == Net+2C..+43
  int internal_receive();              // original 6327E0 receiver is Net+4
};
static_assert(sizeof(NetThreadInfo) == 0x40,
              "Net thread-info view extent from original accesses");

class ServiceProviderList;
struct ServiceStruct : public virtual StringAllocationBase {
    char *name_;                 // +4
    GUID provider_guid_;         // +8

    static void *operator new(size_t size, Heap *owner) throw();
    static void *operator new(size_t size, ServiceProviderList *list) throw();
    static void operator delete(void *memory, Heap *owner);
    static void operator delete(void *memory, ServiceProviderList *list);
    static void operator delete(void *memory);
};
struct ServiceListEntry : public virtual StringAllocationBase {
    int id_;                     // +4
    ServiceStruct *payload_;     // +8
    ServiceListEntry *next_;      // +C
    ServiceListEntry *previous_;  // +10; vbase14, extent1C

    static void *operator new(size_t size, Heap *owner) throw();
    static void *operator new(size_t size, ServiceProviderList *list) throw();
    static void operator delete(void *memory, Heap *owner);
    static void operator delete(void *memory, ServiceProviderList *list);
    static void operator delete(void *memory);
};
class ServiceProviderList : public virtual StringAllocationBase {
 public:
    typedef ServiceListEntry Entry;
    typedef ServiceStruct Payload;
    virtual int on_insert(ServiceStruct *);
    virtual void on_remove(ServiceStruct *);
    ServiceProviderList();
    virtual ~ServiceProviderList();
    int add(int id);
    void remove_all();
    int seek_pos(int position);
    ServiceStruct *next_entry();
    ServiceListEntry *head_;      // +8
    ServiceListEntry *current_;   // +C
    int entry_count_;            // +10
    int current_position_;       // +14
    Heap *allocator_;            // +18; vbase1C, extent24
};
struct ServiceList : public ServiceProviderList {
    virtual int on_insert(ServiceStruct *);
    virtual void on_remove(ServiceStruct *);
    virtual ~ServiceList();
    const char *name_;           // +1C, borrowed enumeration input
    const GUID *provider_guid_;  // +20, borrowed enumeration input
};

// The former unresolved list-cleanup bridge is retired: genuine provider,
// ledger and nested-list destructors own their natural cleanup stages.
// Historical bridge evidence remains in retained source history.


class AlphaNetLedger;
class Net {
  friend class AlphaNet;
  friend struct NetThreadInfo;
  // Original MultiDebug paint 0x005C9920 displays these session/player fields.
  friend class MultiDebug;
 public:
  // Original construction publishes the primary table at 0x0066EB04.
  // Six slots: original drop_player calls primary slot 5 at 0x00632612.
  // NetDaemon's same primary chain ends at 0x0066EF24 (a nonrelocated,
  // non-code word after six relocated code pointers). Its zero-offset base
  // construction and AlphaNet's table preserve all six inherited slots.
  // Slot 0's two stack words are known; their meaning/types remain provisional.
  // Shared no-op code addresses do not make these AlphaMovie methods.
  // RECOVERY_GENERATED: v1 vtable 0x0066EB04 0x0066EB1C ??_7Net@@6B@ 0x0062D6A0
  virtual void on_net_event_0(int, int);
  virtual void on_player_added(char *name, unsigned long id);
  // 0x0062E420 is the non-virtual session enumeration operation. The former
  // on_player_added(char *, unsigned int) interpretation is superseded:
  // its actual arguments are the session ledger and optional password.
  // The distinct inherited virtual slot above and six-slot table remain.
  int enumerate_sessions(AlphaNetLedger *ledger, const char *password);
  virtual void on_player_deleted(char *name, unsigned long id);
  virtual void on_player_name_changed(char *name, unsigned long id);
  virtual void on_session_lost();
  virtual void on_host_changed();

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
  // 0x00632350, homed into net_class.cpp. check_polling calls it to cut a
  // stale player - by key, or 0xFFFFFFFF for everyone - and reads a zero
  // answer as "cut", which is when it refreshes the player's stamp.
  int drop_player(unsigned long key, int reason);
  // 0x00632CC0. Adds the player record, announces it, then creates and
  // initializes its VoiceRx receiver. Full-table recovery deliberately
  // rejects the extra player instead of reproducing the native Net overrun.
  int sub_632cc0(char *text, int id, unsigned char flag);
  // 0x0062E2A0. Joins one enumerated service provider: copies its GUID into
  // the 0x6EC session slot, launches the Zone's "mplaynow.exe" for the
  // internet service, and otherwise creates and publishes the DirectPlay
  // object and seeds the sixteen player slots with the provider's latency.
  int join_service(ServiceStruct *service);
  // 0x0062EAA0. Hosts a named session: opens the migrated-host session
  // description (two players on the modem/serial services) and files the
  // live session's name, password, GUID, flags and ceiling in the
  // allocation node, answering its address.
  NetAllocationNode *create_session(char *name, int max_players, char *password);
  // 0x00631B30. Closes the joined lobby session to further joins by the
  // GetSessionDesc/SetSessionDesc round trip; enable_join is its mirror.
  void close_game();
  // 0x00632070. Reopens the joined session to joins - close_game's mirror.
  void enable_join();
  // 0x00632DB0. Drains the system-message queue: the DPSYS_* envelopes and
  // peer packets the session thread receives, dispatching lost sessions,
  // name changes, player cuts and the join-list refresh.
  void handle_sys_msg();
  int get(unsigned long *a, unsigned long *b);
  // 0x0062EDB0 and 0x0062EDD0, homed into net_class.cpp: the DirectPlay
  // group calls. add_to_group's two ids travel in this order; the catalogue
  // does not say which is the group and which the member.
  int create_group(char *group_name);
  int send_group(void *data, int size, long group_id, int flags);
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
  // 0x00630C80. The DirectPlay player-callback body: appends one NetPlayer
  // record through the write seam below NetGetScratch and answers 1.
  int enum_players(unsigned long player_id, unsigned long, const DPNAME *name,
                   unsigned long, void *);

 public:
  // 0x0062E010. Releases the DirectPlay/thread state and drains the three
  // embedded queues after resetting session/player state.
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
  // 0x0062D8E0. check_for_lobby opens with it; it refreshes the thread-info
  // view, DirectPlay buffer and events after closing any prior session.
  int init(_GUID *guid, int option_a, int option_b);

 public:
  // 0x0062E540. Joins the supplied session (or the already-open lobby
  // session), creates the local DirectPlay player, records the live session,
  // and waits for its first player event. check_for_lobby passes null.
  int join_session(NetAllocationNode *session, char *player_name,
                   char *password);

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
  // init() fills the next six: the thread that owns the session, five events
  // it creates, and the DirectPlay interface copied out of 0x009BE600.
  // RECOVERY_MEMBER: v2 Net thread_info_ NetThreadInfo 0x4 0x0062D8E0
  NetThreadInfo thread_info_;  // 0x4..0x43
  uint32_t field_44_;  // 0x44
 public:
  // Win::on_key compares these modifier-encoded hotkeys, then starts the
  // corresponding voice overload. Construction clears all four words.
  // RECOVERY_MEMBER: v1 Net voice_hotkey_48_ 0x48 0x005F5D10
  uint32_t voice_hotkey_48_;  // unsigned-key voice shortcut
  // RECOVERY_MEMBER: v1 Net voice_hotkey_4c_ 0x4C 0x005F5D10
  uint32_t voice_hotkey_4c_;  // signed-key voice shortcut
  // RECOVERY_MEMBER: v1 Net voice_target_50_ 0x50 0x005F5D10
  unsigned long voice_target_50_;
  // Original on_key passes +0x54 to start_voice(long), but the two start
  // overloads also overwrite it with 0/1 for process_voice's routing choice.
  // Keep this dual-use field provisional rather than invent a second target.
  // RECOVERY_MEMBER: v1 Net field_54_ 0x54 0x005F5D10
  uint32_t field_54_;  // 0x54
 private:
  // 0x58 - THE SUBOBJECT ??0Net BUILDS, not opaque storage that happens to
  // be 0x58 wide. `lea ecx, [esi+0x58]` / `call ??0VoiceTx@@QAE@XZ` at
  // 0x0062D6C9, and ??1Net tears it down at the same offset. The two members
  // this replaces covered exactly 0x58 bytes, so nothing moves.
  VoiceTx voice_tx_;  // 0x58
 protected:
  JackalVoiceRx jackal_voice_rx_;  // 0xB0 (vbase StringAllocationBase at 0xCC)
 private:
  // The voice-transmitter state between the receiver and the fifos.
  uint32_t voice_key_D4_;      // 0xD4, start_voice's key
  uint32_t voice_flags_D8_;    // 0xD8, 0x20000000 started / 0x60000000 in-flight
  // The keep-alive pair check_polling drives: the clock reading it last sent
  // a 0x20 packet at, and the staleness threshold a player stamp is measured
  // against (close() writes 0x4E20 - twenty seconds - into the latter).
  uint32_t poll_time_DC_;      // 0xDC, timeGetTime() of the last keep-alive
  uint32_t poll_timeout_E0_;   // 0xE0, the staleness threshold in ms
  uint32_t field_E4_;          // 0xE4, opaque - close() writes 1 here
  // Distinct members: original construction 0x0062D733..0x0062D75E and
  // destruction 0x004E3563..0x004E358B protect each queue with its own EH
  // state. An array instead emits VC6's vector lifetime helpers. Queue roles
  // remain provisional; offsets disambiguate the source names, not storage.
  // RECOVERY_MEMBER: v2 Net queue_e8_ NetFifo 0xE8 0x0062D6A0
  NetFifo queue_e8_;   // 0xE8
  // RECOVERY_MEMBER: v2 Net queue_10c_ NetFifo 0x10C 0x0062D6A0
  NetFifo queue_10c_;  // 0x10C
  // RECOVERY_MEMBER: v2 Net queue_130_ NetFifo 0x130 0x0062D6A0
  NetFifo queue_130_;  // 0x130
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
  // zeroes both. AlphaNet owns the following player count and record array.
 protected:
  // NetDaemon's player callbacks compare this inherited host ID directly.
  uint32_t field_760_;       // 0x760
  // Read by NetDaemon::send_message (netdaemon.cpp) as the substitution for
  // an out-of-range target, so it is reachable from the derived class rather
  // than a this-pun over the offset.
 public:
  uint32_t field_764_;       // 0x764
};

static_assert(sizeof(Net) == 0x768, "Net ends before AlphaNet's player count");



// Published by Net construction; the window and network pumps read the
// same source-owned pointer. Its original binding is beside the definition.
extern Net *ActiveNet;
// The old host ID is published before drop_player installs its successor.
extern uint32_t NetHostChangeSlot;
// QueryInterface requests IID_IDirectPlay2A; the SDK aliases that interface
// to IDirectPlay2. All source reads and writes share this one pointer object.
extern IDirectPlay2 *NetDirectPlay;
// Shared callback selection storage at original 9BC4B8 and 9BE4BC.
extern unsigned int NetPlayerWriteIndex;
extern NetPlayer *NetPlayerTable;

// THE TWO RE-ENTRANCY GUARDS beside them, each set to 1 while its own pass is
// inside the code that must not be re-entered and cleared on the way out.
// check_polling sends its 0x20 keep-alive under the 0x9BE610 flag and reaps
// stale players under the 0x9BE60C one. These remaining fixed-address guards
// still need source-owned storage; they are not aliases of NetDirectPlay.
uint32_t *const NetReapBusy = (uint32_t *)0x009BE60C;
uint32_t *const NetKeepAliveBusy = (uint32_t *)0x009BE610;

// Shared receive/queue-copy storage. Original Receive advertises the 0x2000
// capacity initialized by Net::init, not the old four-byte scalar declaration.
extern unsigned char NetGetScratch[0x2000];  // 0x009BC4BC

// Shared node allocation contracts; original handoff semantics are retained.
void *allocate_net_node(size_t size, Heap *owner);
void cleanup_failed_net_construction(void *memory);
