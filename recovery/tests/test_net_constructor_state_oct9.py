"""Execute the actual recovered Net constructor body with untouched-byte controls."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _constructor_body() -> str:
    source = (ROOT / "src/net_class.cpp").read_text()
    start = source.index("Net::Net() {")
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:offset]
    raise AssertionError("Net constructor body is unterminated")


HARNESS = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <new>
#include <string.h>

typedef uint32_t DWORD;
struct GUID { uint32_t a, b, c, d; };
struct Net;
static Net *ActiveNet;
static void *NetDirectPlay;
static int voice_ctor_count, receiver_ctor_count, fifo_ctor_count, node_ctor_count;
struct VoiceTx { VoiceTx() { ++voice_ctor_count; } };
struct JackalVoiceRx { JackalVoiceRx() { ++receiver_ctor_count; } };
struct NetFifo { NetFifo() { ++fifo_ctor_count; } };
struct NetAllocationNode { NetAllocationNode() { ++node_ctor_count; } };
struct NetPlayer {
  uint32_t field_0_, field_4_, field_8_, field_C_, stamp_10_;
  uint8_t flags_14_;
  char name_and_tail_[0x43];
};
struct NetThreadInfo {
  DWORD thread_id_;
  void *events_[5];
  void *direct_play_;
  Net *owner_;
  void *buffer_;
  DWORD lock_initialized_;
};
struct Net {
  NetThreadInfo thread_info_;
  uint32_t field_44_, voice_hotkey_48_, voice_hotkey_4c_;
  unsigned long voice_target_50_;
  uint32_t field_54_;
  VoiceTx voice_tx_;
  JackalVoiceRx jackal_voice_rx_;
  uint32_t voice_key_D4_, voice_flags_D8_, poll_time_DC_, poll_timeout_E0_, field_E4_;
  NetFifo queue_e8_, queue_10c_, queue_130_;
  NetPlayer players_[16];
  uint32_t field_6D4_, field_6D8_, field_6DC_;
  void *buffer_6E0_;
  uint32_t field_6E4_;
  GUID service_provider_guid_;
  uint32_t field_6FC_;
  NetAllocationNode allocation_node_;
  DWORD local_player_id_;
  DWORD host_player_id_;
  Net();
};
static bool bytes_are(const void *object, size_t length, unsigned char value) {
  const unsigned char *bytes = static_cast<const unsigned char *>(object);
  for (size_t i = 0; i != length; ++i) {
    if (bytes[i] != value) return false;
  }
  return true;
}
'''


MAIN = r'''int main() {
  static_assert(sizeof(NetPlayer) == 0x58, "player slot stride");
  static_assert(offsetof(NetPlayer, field_C_) == 0x0C, "untouched player word");
  static_assert(offsetof(NetPlayer, stamp_10_) == 0x10, "cleared player timestamp");
  alignas(Net) unsigned char storage[sizeof(Net)];
  memset(storage, 0xA5, sizeof(storage));
  Net *net = new (storage) Net;
  assert(ActiveNet == net && NetDirectPlay == 0);
  assert(net->poll_time_DC_ == 0 && net->poll_timeout_E0_ == 0x4E20);
  assert(net->field_6DC_ == 0 && net->voice_flags_D8_ == 0 && net->field_E4_ == 1);
  assert(net->local_player_id_ == 0 && net->host_player_id_ == 0);
  assert(net->buffer_6E0_ == 0 && net->field_6E4_ == 0);
  assert(bytes_are(&net->service_provider_guid_, sizeof(net->service_provider_guid_), 0xA5));
  assert(net->field_6FC_ == 0xC8 && net->field_6D4_ == 0x64);
  assert(bytes_are(&net->field_6D8_, sizeof(net->field_6D8_), 0xA5)); // no +0x6D8 store
  assert(bytes_are(&net->voice_key_D4_, sizeof(net->voice_key_D4_), 0xA5));
  assert(bytes_are(&net->field_44_, sizeof(net->field_44_), 0xA5));
  assert(voice_ctor_count == 1 && receiver_ctor_count == 1);
  assert(fifo_ctor_count == 3 && node_ctor_count == 1);
  for (int i = 0; i != 16; ++i) {
    assert(net->players_[i].field_0_ == 0);
    assert(net->players_[i].field_4_ == 0);
    assert(net->players_[i].field_8_ == 0);
    assert(bytes_are(&net->players_[i].field_C_, sizeof(net->players_[i].field_C_), 0xA5));
    assert(net->players_[i].stamp_10_ == 0);         // [slot+0x10]
    assert(net->players_[i].flags_14_ == 0);
    assert(bytes_are(net->players_[i].name_and_tail_,
                     sizeof(net->players_[i].name_and_tail_), 0xA5));
  }
  net->~Net();
  return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_net_constructor_stores_and_untouched_words(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = HARNESS + "\nNet::Net() {" + _constructor_body() + "}\n" + MAIN
    path = tmp_path / "net_constructor.cpp"
    binary = tmp_path / "net_constructor"
    path.write_text(source)
    subprocess.run([compiler, "-std=c++11", str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
