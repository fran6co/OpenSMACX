"""Host control that compiles the actual Net player-insertion body."""

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def find_root(start: Path) -> Path:
    for parent in start.resolve().parents:
        if (parent / "src/net_class.cpp").is_file() and (parent / "src/netwin.cpp").is_file():
            return parent
    raise AssertionError("cannot find OpenSMACX source root")


ROOT = find_root(Path(__file__))


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start : i + 1]
    raise AssertionError("unterminated function")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_player_insert_body_normal_callback_mutation(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("g++ is unavailable")
    source = (ROOT / "src/net_class.cpp").read_text()
    actual = body(source, "int Net::sub_632cc0(char *text, int id, unsigned char flag)")
    fixture = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
using uint32_t = std::uint32_t;
struct VoiceRxAllocation {
  int init_calls = 0;
  int init(unsigned long, void *, unsigned long) { ++init_calls; return 71; }
};
struct VoiceEntry { VoiceRxAllocation *payload_ = nullptr; };
struct JackalVoiceRx {
  VoiceEntry *head_ = nullptr, *cursor_ = nullptr;
  int last_player_id = -1, add_calls = 0;
  VoiceEntry entry_{}; VoiceRxAllocation voice_{};
  int add_player(int id) {
    ++add_calls; last_player_id = id;
    entry_.payload_ = &voice_; head_ = cursor_ = &entry_;
    return 0;
  }
};
struct NetPlayer {
  uint32_t field_0_ = 0, field_4_ = 0, field_8_ = 0, field_C_ = 0, stamp_10_ = 0;
  unsigned char flags_14_ = 0;
  char name_and_tail_[0x43]{};
};
struct Net {
  NetPlayer players_[16]{};
  // Keep the neighboring words needed to witness the pinned index-16 aliases:
  // players_[16].field_0_ is field_6D4_, and its name pointer is guid_6E8_+1.
  uint32_t field_6D4_ = 0, field_6D8_ = 0, field_6DC_ = 0;
  uint32_t buffer_6E0_ = 0;
  uint32_t field_6E4_ = 0;
  uint32_t guid_6E8_ = 0;
  uint32_t local_player_id_ = 1, host_player_id_ = 2;
  JackalVoiceRx jackal_voice_rx_{};
  int callbacks = 0, broadcasts = 0;
  uint32_t callback_mutation = 0;
  int mutate_slot = -1;
  bool mutate_full_table_id = false;
  bool capture_callback_name_bytes = true;
  uint32_t callback_id = 0;
  char *callback_name_source = nullptr;
  char callback_name[32]{};
  void broadcast_player_table() { ++broadcasts; }
  void on_player_added(char *name, unsigned long id) {
    ++callbacks;
    callback_name_source = name;
    callback_id = static_cast<uint32_t>(id);
    if (capture_callback_name_bytes)
      std::strncpy(callback_name, name, sizeof(callback_name) - 1);
    if (mutate_slot >= 0) players_[mutate_slot].field_0_ = callback_mutation;
    if (mutate_full_table_id) field_6D4_ = callback_mutation;
  }
  int sub_632cc0(char *, int, unsigned char);
};
static_assert(sizeof(NetPlayer) == 0x58, "original player row stride");
static_assert(offsetof(Net, guid_6E8_) == 16 * 0x58 + 0x14,
              "the full-table name alias is one byte into the GUID pointer word");
'''
    tail = r'''
int main() {
  Net net;
  net.callback_mutation = 77;
  net.mutate_slot = 0;
  const int result = net.sub_632cc0(const_cast<char *>("fresh"), 33, 0);
  assert(result == 71);
  assert(net.players_[0].field_0_ == 77);
  assert(net.players_[0].flags_14_ == 0);
  assert(std::strcmp(net.players_[0].name_and_tail_, "fresh") == 0);
  assert(net.field_6DC_ == 1 && net.callbacks == 1);
  assert(net.jackal_voice_rx_.add_calls == 1);
  assert(net.jackal_voice_rx_.last_player_id == 77);
  assert(net.jackal_voice_rx_.voice_.init_calls == 1);
  assert(net.callbacks == 1 && net.callback_id == 33);
  assert(std::strcmp(net.callback_name, "fresh") == 0);

  // Duplicate lookup returns immediately, before any shared side effect.
  net.callback_mutation = 0;
  net.mutate_slot = -1;
  const int duplicate = net.sub_632cc0(const_cast<char *>("ignored"), 77, 1);
  assert(duplicate == 77 && net.callbacks == 1 && net.jackal_voice_rx_.add_calls == 1);

  // A second defined slot exercises the flag-controlled local-ID write and
  // conditional broadcast while continuing through callback and receiver.
  net.local_player_id_ = 44;
  const int second = net.sub_632cc0(const_cast<char *>("other"), 44, 1);
  assert(second == 71 && net.players_[1].field_0_ == 44);
  assert(net.players_[1].flags_14_ == 1 && net.host_player_id_ == 44);
  assert(net.field_6DC_ == 2 && net.broadcasts == 1 && net.callbacks == 2);
  assert(net.jackal_voice_rx_.last_player_id == 44);
  assert(std::strcmp(net.callback_name, "other") == 0 && net.callback_id == 44);

  // Full-table fallthrough must pass the computed index-16 aliases, then
  // reload the aliased ID after the callback changes that adjacent word.
  Net full;
  full.local_player_id_ = full.host_player_id_ = 44;
  for (int i = 0; i < 16; ++i) full.players_[i].field_0_ = 100 + i;
  full.field_6D4_ = 0x12345678;
  full.guid_6E8_ = 0x12345678;
  full.callback_mutation = 77;
  full.mutate_full_table_id = true;
  full.capture_callback_name_bytes = false;
  const int full_result = full.sub_632cc0(const_cast<char *>("ignored"), 33, 0);
  assert(full_result == 71);
  assert(full.field_6DC_ == 1 && full.broadcasts == 1 && full.callbacks == 1);
  assert(full.callback_id == 0x12345678);
  assert(full.callback_name_source == reinterpret_cast<char *>(&full.guid_6E8_) + 1);
  assert(full.field_6D4_ == 77 && full.jackal_voice_rx_.last_player_id == 77);
  assert(full.jackal_voice_rx_.add_calls == 1 && full.jackal_voice_rx_.voice_.init_calls == 1);
  for (int i = 0; i < 16; ++i) assert(full.players_[i].field_0_ == static_cast<uint32_t>(100 + i));
}
'''
    unit = fixture + "\n" + actual + "\n" + tail
    with tempfile.TemporaryDirectory(prefix="net-player-source-host-") as directory:
        cpp = Path(directory) / "control.cpp"
        binary = Path(directory) / "control"
        cpp.write_text(unit)
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert ran.returncode == 0, ran.stdout + ran.stderr
