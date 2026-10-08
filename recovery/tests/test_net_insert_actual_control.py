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
  uint32_t field_6D4_ = 0, field_6DC_ = 0, field_760_ = 1, field_764_ = 2;
  void *guid_6E8_ = nullptr;
  JackalVoiceRx jackal_voice_rx_{};
  int callbacks = 0, broadcasts = 0;
  uint32_t callback_mutation = 0;
  int mutate_slot = -1;
  uint32_t callback_id = 0;
  char callback_name[32]{};
  void broadcast_player_table() { ++broadcasts; }
  void on_player_added(char *name, unsigned long id) {
    ++callbacks;
    callback_id = static_cast<uint32_t>(id);
    std::strncpy(callback_name, name, sizeof(callback_name) - 1);
    if (mutate_slot >= 0) players_[mutate_slot].field_0_ = callback_mutation;
  }
  int sub_632cc0(char *, int, unsigned char);
};
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
  net.field_760_ = 44;
  const int second = net.sub_632cc0(const_cast<char *>("other"), 44, 1);
  assert(second == 71 && net.players_[1].field_0_ == 44);
  assert(net.players_[1].flags_14_ == 1 && net.field_764_ == 44);
  assert(net.field_6DC_ == 2 && net.broadcasts == 1 && net.callbacks == 2);
  assert(net.jackal_voice_rx_.last_player_id == 44);
  assert(std::strcmp(net.callback_name, "other") == 0 && net.callback_id == 44);
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
