"""Execute actual Net player-member methods in small host controls.

These controls validate source behavior only; they are not VC6 ABI, native
matching, or whole-application acceptance evidence.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
ADDRESSES = {
    0x631A20: "char *Net::get_player_name(unsigned long key)",
    0x62D8E0: "int Net::init(_GUID *guid, int option_a, int option_b)",
    0x62E2A0: "int Net::join_service(ServiceStruct *service)",
}

HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

typedef int HRESULT;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef void *HANDLE;
static const unsigned int SW_SHOW = 5;
struct _GUID { uint32_t a, b, c, d; };
typedef _GUID GUID;
typedef GUID *LPGUID;
typedef void *LPDIRECTPLAY;
struct DPCAPS { DWORD dwSize, dwLatency; unsigned char rest[0x20]; };
static const GUID IID_IDirectPlay2A = {1, 2, 3, 4};
static const GUID KeptServiceProviderGuid = {9, 8, 7, 6};
static char NetZoneLauncher[] = "launcher";

struct IDirectPlay2 {
  virtual HRESULT QueryInterface(const GUID &, void **) = 0;
  virtual ULONG Release() = 0;
  virtual HRESULT GetCaps(DPCAPS *, DWORD) = 0;
};
static IDirectPlay2 *NetDirectPlay;
static DPCAPS NetServiceCaps;
static unsigned char allocation[0x2000];
static int create_event_count;
static void *mem_get(int size) {
  assert(size == 0x2000);
  return allocation;
}
static DWORD GetCurrentThreadId() { return 123; }
static HANDLE CreateEventA(void *, int, int, const char *) {
  return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(++create_event_count));
}
static void InitializeCriticalSection(void *) {}
static unsigned int WinExec(const char *, unsigned int) { return 33; }
static void sub_631ba0(HRESULT) {}

struct NetPlayer {
  uint32_t field_0_, field_4_, field_8_, field_C_, stamp_10_;
  uint8_t flags_14_;
  char name_and_tail_[0x43];
};
struct ServiceStruct { GUID provider_guid_; };
struct VoiceTx { int init(int, int, int) { return 0; } };
struct ThreadInfo {
  void *owner_, *buffer_, *direct_play_;
  DWORD thread_id_;
  HANDLE events_[5];
  unsigned char receive_lock_[32];
  int lock_initialized_;
};
class Net {
 public:
  uint32_t field_44_, field_6E4_, field_6D8_, field_6D4_;
  void *buffer_6E0_;
  _GUID *guid_6E8_;
  uint32_t voice_flags_D8_;
  GUID service_provider_guid_;
  NetPlayer players_[16];
  VoiceTx voice_tx_;
  ThreadInfo thread_info_;
  void close() {}
  char *get_player_name(unsigned long);
  int init(_GUID *, int, int);
  int join_service(ServiceStruct *);
};

static IDirectPlay2 *created_provider;
static HRESULT create_result;
static DWORD provider_latency;
static HRESULT caps_result;
static int provider_releases;
static LPGUID directplay_guid;
struct FakeDirectPlay : IDirectPlay2 {
  HRESULT QueryInterface(const GUID &, void **out) {
    *out = created_provider;
    return 0;
  }
  ULONG Release() { ++provider_releases; return 1; }
  HRESULT GetCaps(DPCAPS *caps, DWORD flags) {
    assert(flags == 1 && caps->dwSize == 0x28);
    caps->dwLatency = provider_latency;
    return caps_result;
  }
};
static FakeDirectPlay provider;
static HRESULT DirectPlayCreate(LPGUID guid, LPDIRECTPLAY *out, void *) {
  directplay_guid = guid;
  if (create_result != 0) return create_result;
  created_provider = &provider;
  *out = &provider;
  return 0;
}

'''

MAIN = r'''int main() {
  Net net = {};
  memset(net.players_, 0xA5, sizeof(net.players_));
  _GUID session = {5, 6, 7, 8};
  NetDirectPlay = &provider;
  assert(net.init(&session, 0x1234, 0x5678) == 0);
  assert(net.voice_flags_D8_ == 0x1234 && net.field_44_ == 0x5678);
  for (int i = 0; i < 16; ++i) {
    const NetPlayer &p = net.players_[i];
    assert(p.field_0_ == 0 && p.field_4_ == 0 && p.field_8_ == 0);
    assert(p.flags_14_ == 0 && p.stamp_10_ == 0);
    assert(p.field_C_ == 0xA5A5A5A5u);
    for (unsigned char byte : p.name_and_tail_) assert(byte == 0xA5);
  }

  for (int i = 0; i < 16; ++i) {
    net.players_[i].field_0_ = static_cast<uint32_t>(1000 + i);
    memset(net.players_[i].name_and_tail_, 'a' + i,
           sizeof(net.players_[i].name_and_tail_));
    net.players_[i].name_and_tail_[sizeof(net.players_[i].name_and_tail_) - 1] = 0;
  }
  net.players_[4].field_0_ = 777;
  net.players_[10].field_0_ = 777;  // Original returns the first matching row.
  assert(net.get_player_name(777) == net.players_[4].name_and_tail_);
  assert(net.get_player_name(1015) == net.players_[15].name_and_tail_);
  assert(net.get_player_name(0xDEADBEEF) == 0);

  ServiceStruct service = {{0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00}};
  provider_latency = 37;
  caps_result = 0;
  assert(net.join_service(&service) == 0);
  assert(directplay_guid == &service.provider_guid_);
  assert(std::memcmp(&net.service_provider_guid_, &service.provider_guid_, sizeof(GUID)) == 0);
  for (int i = 0; i < 16; ++i) {
    assert(net.players_[i].field_C_ == 37);
    assert(net.players_[i].field_0_ == static_cast<uint32_t>(i == 4 || i == 10 ? 777 : 1000 + i));
  }
  assert(net.field_6D4_ == 37);

  provider_latency = 0;
  assert(net.join_service(&service) == 0);
  for (int i = 0; i < 16; ++i) assert(net.players_[i].field_C_ == 100);
  assert(net.field_6D4_ == 100);
  return 0;
}
'''


def _actual_definitions() -> str:
    rows = {row.address: row for row in read_file(ROOT / "src/net_class.cpp")}
    fragments = []
    for address, signature in ADDRESSES.items():
        row = rows[address]
        assert row.region and signature in row.region
        assert "BYTE_EXACT" in row.region if address != 0x62E2A0 else True
        fragments.append(_definition(row.region, signature))
    return "\n\n".join(fragments)


def _run(source: str, compiler: str, directory: Path, label: str) -> int:
    cpp = directory / f"{label}.cpp"
    exe = directory / label
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(cpp), "-o", str(exe)],
        check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], check=False, capture_output=True,
                         text=True, timeout=20)
    return ran.returncode


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_netplayer_members_preserve_init_lookup_and_provider_latency(
    compiler: str, tmp_path: Path,
) -> None:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    actual = HARNESS + _actual_definitions() + MAIN
    assert _run(actual, compiler_path, tmp_path, "net_player_owner") == 0


def test_actual_source_controls_reject_player_member_mutants(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    actual = HARNESS + _actual_definitions() + MAIN
    mutations = (
        ("player->stamp_10_ = 0;", "player->stamp_10_ &= 0xFF000000u;"),
        ("return players_[index].name_and_tail_;", "return players_[0].name_and_tail_;"),
        ("player->field_C_ = 100;", "player->field_C_ = latency;"),
    )
    for index, (old, new) in enumerate(mutations):
        mutant = actual.replace(old, new, 1)
        assert mutant != actual
        assert _run(mutant, compiler, tmp_path, f"net_player_mutant_{index}") != 0


def test_raw_player_strides_and_stale_stamp_comment_are_gone() -> None:
    source = (ROOT / "src/net_class.cpp").read_text(encoding="utf-8")
    init = _actual_definitions().split("int Net::init(", 1)[1].split("\n}", 1)[0]
    join = _actual_definitions().split("int Net::join_service(", 1)[1].split("\n}", 1)[0]
    lookup = _actual_definitions().split("char *Net::get_player_name(", 1)[1].split("\n}", 1)[0]
    assert "BUG IN THE ORIGINAL: only the FIRST byte of stamp_10_" not in source
    assert "player->stamp_10_ = 0;" in init
    assert "player->flags_14_ = 0;" in init
    assert "player->field_C_ = latency;" in join
    assert "player->field_C_ = 100;" in join
    assert "players_[index].field_0_ == key" in lookup
    assert "players_[index].name_and_tail_" in lookup
    assert "reinterpret_cast<uint8_t *>(this)" not in lookup
