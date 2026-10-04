// Host-only typed projection. The test replaces the marker with the exact
// definition extracted from src/net_class.cpp; this is not target ABI proof.
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint32_t DPID;
typedef unsigned long HANDLE;
typedef int HRESULT;
#define __cdecl
static const HRESULT DP_OK = 0;
static const HRESULT DPERR_ALREADYINITIALIZED = 0x88770005;
static const DWORD DPOPEN_JOIN = 1;
static const DWORD DPSESSION_JOINDISABLED = 0x20;

struct GUID { DWORD Data1; WORD Data2, Data3; unsigned char Data4[8]; };
struct DPCAPS { DWORD dwSize; unsigned char rest[0x24]; };
struct DPSESSIONDESC2 {
  DWORD dwSize, dwFlags;
  GUID guidInstance, guidApplication;
  DWORD dwMaxPlayers, dwCurrentPlayers;
  char *lpszSessionNameA, *lpszPasswordA;
  DWORD dwReserved1, dwReserved2, dwUser1, dwUser2, dwUser3, dwUser4;
};
struct DPNAME { DWORD dwSize, dwFlags; char *lpszShortNameA, *lpszLongNameA; };

struct Net;
struct Font {
  static int live;
  static Net *mutate_target;
  Font() { ++live; }
  ~Font();
  int init(const char *, int, int) { return 0; }
};
int Font::live = 0;

int parse_calls = 0;
int caption_calls = 0;
void parse_num(int, int) { ++parse_calls; }
void pop_caption(const char *, const char *, int) { ++caption_calls; }
void *mem_get(int amount) { return malloc(static_cast<size_t>(amount)); }
DWORD timeGetTime() { return 10; }
int MessageBoxA(void *, const char *, const char *, int) { return 0; }

struct NetAllocationNode {
  char *session_name_;
  char *password_;
  GUID instance_guid_;
  DWORD session_flags_;
  DWORD max_players_;
};
struct NetThreadInfo {
  HANDLE events_[5];
  NetThreadInfo() : events_() {}
};
struct Net;
struct IDirectPlay2 {
  DPSESSIONDESC2 live;
  DPSESSIONDESC2 opened;
  DPID next_id;
  DPID *created_id_address;
  int open_calls, cap_calls, desc_queries, desc_reads, create_calls, set_calls;
  HRESULT create_result;
  IDirectPlay2()
      : live(), opened(), next_id(88), created_id_address(0), open_calls(0),
        cap_calls(0), desc_queries(0), desc_reads(0), create_calls(0),
        set_calls(0), create_result(DP_OK) {}
  HRESULT Open(DPSESSIONDESC2 *desc, DWORD flags) {
    ++open_calls; opened = *desc; assert(flags == DPOPEN_JOIN); return DP_OK;
  }
  HRESULT Close() { return DP_OK; }
  HRESULT GetCaps(DPCAPS *caps, DWORD) {
    ++cap_calls; assert(caps->dwSize == sizeof(DPCAPS)); return DP_OK;
  }
  HRESULT GetSessionDesc(void *desc, DWORD *size) {
    if (desc == 0) { ++desc_queries; *size = sizeof(live); return DP_OK; }
    ++desc_reads; *static_cast<DPSESSIONDESC2 *>(desc) = live;
    *size = sizeof(live); return DP_OK;
  }
  HRESULT CreatePlayer(DPID *id, DPNAME *, HANDLE, void *, DWORD, DWORD) {
    ++create_calls; created_id_address = id; *id = next_id; return create_result;
  }
  HRESULT SetSessionDesc(DPSESSIONDESC2 *, DWORD) { ++set_calls; return DP_OK; }
};
IDirectPlay2 *NetDirectPlay = 0;
DPCAPS NetServiceCaps;
struct Net {
  DWORD field_44_;
  DWORD voice_flags_D8_;
  DWORD field_6DC_;
  DPID field_760_;
  DWORD field_764_;
  GUID *guid_6E8_;
  NetAllocationNode allocation_node_;
  NetThreadInfo thread_info_;
  DPID id_seen_by_add_player;
  int add_calls, close_calls, task_calls;
  Net()
      : field_44_(5), voice_flags_D8_(0x10000), field_6DC_(1), field_760_(7),
        field_764_(12), guid_6E8_(0), allocation_node_(), thread_info_(),
        id_seen_by_add_player(0), add_calls(0), close_calls(0), task_calls(0) {}
  int join_session(NetAllocationNode *, char *, char *);
  int sub_632cc0(char *, int id, unsigned char) {
    ++add_calls;
    id_seen_by_add_player = static_cast<DPID>(id);
    field_760_ = 99;
    return 0;
  }
  void close() { ++close_calls; }
  void do_task() { ++task_calls; }
};
Net *Font::mutate_target = 0;
Font::~Font() {
  --live;
  if (mutate_target != 0) mutate_target->field_760_ = 123;
}
void __cdecl net_receive_thread(void *) {}
unsigned long _beginthread(void (__cdecl *)(void *), unsigned, void *) { return 1; }

// @@JOIN_SESSION_DEFINITION@@

int main() {
  Net net;
  IDirectPlay2 direct_play;
  GUID app = {1, 2, 3, {4,5,6,7,8,9,10,11}};
  NetAllocationNode session;
  memset(&session, 0, sizeof(session));
  session.instance_guid_.Data1 = 0x12345678;
  net.guid_6E8_ = &app;
  direct_play.live.dwUser1 = net.field_44_;
  direct_play.live.guidInstance.Data1 = 0x87654321;
  direct_play.live.lpszSessionNameA = const_cast<char *>("room");
  direct_play.live.dwMaxPlayers = 8;
  NetDirectPlay = &direct_play;
  Font::mutate_target = &net;

  const int result = net.join_session(&session, const_cast<char *>("Ari"),
                                      const_cast<char *>("pw"));
  if (direct_play.created_id_address != &net.field_760_) return 31;
  if (net.field_764_ != 88) return 32;
  if (net.add_calls != 1 || net.id_seen_by_add_player != 88) return 33;
  if (net.field_760_ != 123 || result != 99) return 34;
  if (Font::live != 0 || direct_play.set_calls != 1) return 35;
  free(net.allocation_node_.session_name_);
  free(net.allocation_node_.password_);
  Font::mutate_target = 0;
  return 0;
}
