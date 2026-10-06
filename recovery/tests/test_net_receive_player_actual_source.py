"""Exercise actual Net receive/player bodies; host controls are not target ABI proof."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile


from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
METHODS = {
    0x630A80: "void *Net::read_queue_head_until_timeout(unsigned int *key_out, unsigned int *length_out)",
    0x630B00: "void *Net::read_queue_key_until_timeout(unsigned int *key, int *length_out)",
    0x630B80: "void Net::receive_directplay_messages(int player_id)",
    0x62D5F0: "void Net::broadcast_player_table()",
    0x632290: "int Net::set_player_host_flag(int is_host)",
    0x632FA0: "int Net::check_message(unsigned long player, unsigned long stamp)",
}

HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

typedef uint32_t DWORD;
typedef uint32_t DPID;
typedef int HRESULT;
typedef DPID *LPDPID;
static unsigned char NetGetScratch[0x2000];
class Net;
class IDirectPlay2 {
 public:
  virtual HRESULT Receive(LPDPID from, LPDPID to, DWORD flags, void *data, DWORD *size) = 0;
  virtual HRESULT GetPlayerData(DPID player, void *data, DWORD *size, DWORD flags) = 0;
  virtual HRESULT SetPlayerData(DPID player, void *data, DWORD size, DWORD flags) = 0;
};
static IDirectPlay2 *NetDirectPlay;
static Net *active_net;
static DWORD time_values[16];
static int time_count, time_index;
DWORD timeGetTime() { assert(time_index < time_count); return time_values[time_index++]; }
void *mem_get(int size) { return malloc(static_cast<size_t>(size)); }
static unsigned char sent_packet[4096];
static int sent_size, sent_count;
int MessageBoxA(void *, const char *, const char *, unsigned int) { return 0; }

class NetFifo {
 public:
  int head_result, keyed_result, head_calls, keyed_calls;
  unsigned int returned_key, returned_size;
  int returned_signed_size;
  NetFifo() : head_result(0), keyed_result(0), head_calls(0), keyed_calls(0),
              returned_key(0), returned_size(0), returned_signed_size(0) {}
  int get(void *, unsigned int *key, int *, unsigned int *size) {
    ++head_calls;
    if (key) *key = returned_key;
    if (size) *size = returned_size;
    return head_result;
  }
  int get(void *, unsigned int *key, int *, int *size) {
    ++keyed_calls;
    if (keyed_result != 0 && key && *key != returned_key) return 0;
    if (size) *size = returned_signed_size;
    return keyed_result;
  }
};
struct NetPlayer {
  uint32_t field_0_, field_4_, field_8_, field_C_, stamp_10_;
  uint8_t flags_14_;
  char name_and_tail_[0x43];
};
class Net {
 public:
  NetFifo queue_10c_;
  DWORD field_6FC_, field_6E4_, field_6DC_;
  DPID field_760_;
  DWORD field_764_;
  NetPlayer players_[16];
  int send_packet_type(void *packet, int size, DWORD target, int type, int flags) {
    assert(target == 0 && type == 0x10 && flags == 1);
    assert(size >= 0 && size <= static_cast<int>(sizeof(sent_packet)));
    memcpy(sent_packet, packet, static_cast<size_t>(size));
    sent_size = size;
    ++sent_count;
    return 0;
  }
  void *read_queue_head_until_timeout(unsigned int *, unsigned int *);
  void *read_queue_key_until_timeout(unsigned int *, int *);
  void receive_directplay_messages(int);
  void broadcast_player_table();
  int set_player_host_flag(int);
  int check_message(unsigned long, unsigned long);
};

class FakeDirectPlay : public IDirectPlay2 {
 public:
  int receive_calls;
  DPID from_values[4];
  DWORD flags_values[4];
  int set_calls;
  DPID get_player, set_player;
  unsigned char set_data, input_data;
  DWORD set_size, set_flags;
  HRESULT get_result, set_result;
  FakeDirectPlay() : receive_calls(0), set_calls(0), get_player(0), set_player(0),
                     set_data(0), input_data(1), set_size(0), set_flags(0),
                     get_result(0), set_result(-1) {}
  HRESULT Receive(LPDPID from, LPDPID to, DWORD flags, void *, DWORD *size) {
    assert(receive_calls < 4);
    from_values[receive_calls] = *from;
    flags_values[receive_calls] = flags;
    assert(*to == active_net->field_760_ && *size == active_net->field_6E4_);
    *from = 0xFEED;
    ++receive_calls;
    return receive_calls == 1 ? 0 : 7;
  }
  HRESULT GetPlayerData(DPID player, void *data, DWORD *size, DWORD flags) {
    get_player = player;
    assert(*size == 1 && flags == 0);
    if (get_result != 0) return get_result;
    *static_cast<unsigned char *>(data) = input_data;
    // Model the original second read of Net+0x760 after this call.
    active_net->field_760_ = 88;
    return 0;
  }
  HRESULT SetPlayerData(DPID player, void *data, DWORD size, DWORD flags) {
    ++set_calls;
    set_player = player;
    set_data = *static_cast<unsigned char *>(data);
    set_size = size;
    set_flags = flags;
    return set_result;
  }
};

'''


MAIN = r'''int main() {
  Net net;
  active_net = &net;
  FakeDirectPlay direct;
  NetDirectPlay = &direct;

  net.field_6FC_ = 50;
  net.queue_10c_.head_result = 19;
  net.queue_10c_.returned_key = 0x1234;
  net.queue_10c_.returned_size = 27;
  time_values[0] = 100; time_values[1] = 101; time_values[2] = 102;
  time_count = 3; time_index = 0;
  unsigned int key_out = 0, size_out = 0;
  assert(net.read_queue_head_until_timeout(&key_out, &size_out) == NetGetScratch);
  assert(key_out == 0x1234 && size_out == 27 && net.queue_10c_.head_calls == 1);

  time_values[0] = 200; time_values[1] = 251;
  time_count = 2; time_index = 0;
  assert(net.read_queue_head_until_timeout(&key_out, &size_out) == 0);
  assert(net.queue_10c_.head_calls == 1);  // Expired before first read.

  net.queue_10c_.keyed_result = 23;
  net.queue_10c_.returned_key = 0x5678;
  net.queue_10c_.returned_signed_size = -3;
  key_out = 0x5678;
  time_values[0] = 300; time_values[1] = 300;
  time_count = 2; time_index = 0;
  assert(net.read_queue_key_until_timeout(&key_out, reinterpret_cast<int *>(&size_out)) == NetGetScratch);
  assert(net.queue_10c_.keyed_calls == 1 && static_cast<int>(size_out) == -3);

  net.field_760_ = 77;
  net.field_6E4_ = 0x2000;
  net.receive_directplay_messages(7);
  assert(direct.receive_calls == 2);
  assert(direct.from_values[0] == 7 && direct.from_values[1] == 7);
  assert(direct.flags_values[0] == 6 && direct.flags_values[1] == 6);

  net.field_760_ = 1; net.field_764_ = 1; net.field_6DC_ = 1;
  net.players_[0].field_0_ = 0x10203040;
  net.players_[0].flags_14_ = 0xA5;
  strcpy(net.players_[0].name_and_tail_, "Player");
  net.players_[0].name_and_tail_[0x1F] = 'X';
  net.broadcast_player_table();
  assert(sent_count == 1 && sent_size == 0x30);
  assert(sent_packet[0] == 1);
  assert(*reinterpret_cast<uint32_t *>(sent_packet + 4) == 0x10203040);
  assert(strcmp(reinterpret_cast<char *>(sent_packet + 8), "Player") == 0);
  assert(net.players_[0].name_and_tail_[0x1F] == 0);
  assert(*reinterpret_cast<uint32_t *>(sent_packet + 0x28) == 0xA5);

  net.field_760_ = 77;
  direct.set_result = 0;
  assert(net.set_player_host_flag(1) == 0);
  assert(direct.get_player == 77 && direct.set_player == 88);
  assert(direct.set_data == 3 && direct.set_size == 1 && direct.set_flags == 2);
  net.field_760_ = 77;
  direct.input_data = 3;
  direct.set_result = -1;
  assert(net.set_player_host_flag(0) == -1);
  assert(direct.set_data == 1);

  net.field_760_ = 77;
  direct.get_result = -7;
  const int before_failed_get_set_calls = direct.set_calls;
  assert(net.set_player_host_flag(1) == -7);
  assert(direct.set_calls == before_failed_get_set_calls);
  direct.get_result = 0;
  NetDirectPlay = 0;
  assert(net.set_player_host_flag(1) == 0);

  net.players_[2].field_0_ = 99;
  net.players_[2].stamp_10_ = 700;
  assert(net.check_message(99, 700) == 1);
  assert(net.check_message(99, 699) == 1);
  assert(net.check_message(99, 701) == 0 && net.players_[2].stamp_10_ == 701);
  assert(net.check_message(100, 800) == 0);
  return 0;
}
'''


def _definitions() -> str:
    records = {row.address: row for row in read_file(ROOT / "src/net_class.cpp")}
    return "\n\n".join(_definition(records[address].region, signature)
                         for address, signature in METHODS.items())


def test_actual_net_owner_bodies_and_damage_control() -> None:
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    source = HARNESS + _definitions() + "\n" + MAIN
    mutant = source.replace("working_id = player_id;", "working_id = 0;", 1)
    hresult_mutant = source.replace("return result;", "return 0;", 1)
    assert mutant != source and hresult_mutant != source
    with tempfile.TemporaryDirectory(prefix="osmx-net-owner-host-") as temp:
        directory = Path(temp)
        for compiler in compilers:
            for label, contents, expect_success in (
                ("owner", source, True), ("mutant", mutant, False),
                ("hresult-mutant", hresult_mutant, False),
            ):
                cpp = directory / f"{compiler.replace('+', 'p')}-{label}.cpp"
                exe = directory / f"{compiler.replace('+', 'p')}-{label}"
                cpp.write_text(contents)
                subprocess.run(
                    [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                     str(cpp), "-o", str(exe)],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                )
                result = subprocess.run([str(exe)], check=False, text=True,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT)
                if expect_success:
                    assert result.returncode == 0, f"{compiler} owner control failed:\n{result.stdout}"
                else:
                    assert result.returncode != 0, f"{compiler} failed to detect the receiver-state mutant"
