"""Actual-source controls for the NetDaemon-owned faction wait mask."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from test_caviar_draw_callbacks_actual_source import extract

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_wait_mask_owner_and_callback_sensitive_server_paths(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    cpp = (ROOT / "src/netdaemon.cpp").read_text()
    wire = (extract(cpp, "struct MessageDataRecord {") + ";\n" +
            extract(cpp, "class MessageCommandView {") + ";\n")
    withdraw = _body(cpp, "static void netdaemon_handle_withdraw_end_turn")
    approval = _body(cpp, "static void netdaemon_complete_local_turn_wait")
    reset = _body(cpp, "void NetDaemon::reset()")
    console = (ROOT / "src/console.cpp").read_text()
    header = (ROOT / "src/console.h").read_text()
    assert "int ConsoleFactionsWaitingMask;" not in console
    assert "extern int ConsoleFactionsWaitingMask;" not in header
    netdaemon_header = (ROOT / "src/netdaemon.h").read_text()
    assert "return primary_state_.trailing_words_[7];" in netdaemon_header
    assert sum(line.count("ConsoleFactionsWaitingMask") for line in cpp.splitlines()
               if "RECOVERY_OBJECT:" not in line) == 0
    assert "RECOVERY_OBJECT: v3 0x0093E8EC ?ConsoleFactionsWaitingMask@@3HA member ?NetDaemonNet@@3VNetDaemon@@A primary_state_.trailing_words_[7]" in cpp

    harness = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
struct Entry {
  uint32_t leading_words_[4];
  union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_;
  uint32_t trailing_words_[10];
};
struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; };
struct DeletionRecord { uint8_t ack_; uint8_t reserved_[3]; int owner_; int16_t count_; int16_t vehicles_[25]; };
struct DeletionList { DeletionRecord records_[24]; int current_owner_; uint32_t sequence_bits() const { uint32_t x; std::memcpy(&x,&current_owner_,4); return x; } void set_sequence_bits(uint32_t x) { std::memcpy(&current_owner_,&x,4); } uint32_t advance_sequence() { uint32_t x=sequence_bits()+1; set_sequence_bits(x); return x; } };
struct ProcessSlot { int identity_; };
struct AlphaNet { int pid_2_who(uint32_t id) { assert(id == 77); return 2; }
  void close() {} };
struct LockOwner { void clear() {} };
struct NetDaemon : AlphaNet {
  DeletionList deletion_list_; PrimaryState primary_state_; Entry channels_[2];
  void *file_index_records_; uint32_t field_1BE4_;
  ProcessSlot process_slots_[4]; int player_count_; LockOwner locks_;
  NetDaemon() : player_count_(1) { std::memset(&deletion_list_,0,sizeof(deletion_list_)); std::memset(&primary_state_,0,sizeof(primary_state_)); std::memset(channels_,0,sizeof(channels_)); }
  uint32_t &factions_waiting_mask() { return primary_state_.trailing_words_[7]; }
      uint32_t &network_message_count() { return channels_[1].trailing_words_[7]; }
  void reset();
};
NetDaemon NetDaemonNet;
int turn_number=9, game_state=2, local_faction=2, draws=0, messages=0;
int log_mutation=0;
struct Fields { int turn_current_num_; uint32_t game_state_; };
struct { Fields fields_; } GameDataStateStorage = {{9,2}};
struct { int faction_id_; } ConsoleGlobal = {2};
struct { void draw(int n) { assert(n==0); ++draws; } } MultiWindow;
int message_data(int opcode,int,int,int,int,int) {
  ++messages; assert(opcode==0x1301 || opcode==0x1302); return 0;
}
void log_say(const char *,int,int,int) {
  if (log_mutation) NetDaemonNet.factions_waiting_mask() = (1u<<2) | (1u<<4);
}
'''+wire+"\n"+withdraw+"\n"+approval+"\n"+reset+r'''
int main() {
  unsigned char message[0x20] = {};
  *reinterpret_cast<int *>(message+0x10)=9;
  NetDaemon passed;
  passed.process_slots_[1].identity_=4;
  passed.player_count_=1;
  NetDaemonNet.process_slots_[1].identity_=4;
  NetDaemonNet.player_count_=1;

  NetDaemonNet.factions_waiting_mask()=0xA5;
  NetDaemonNet.network_message_count()=0x1234;
  NetDaemonNet.primary_state_.trailing_words_[4]=0x55;
  NetDaemonNet.reset();
  assert(NetDaemonNet.factions_waiting_mask()==0);
  assert(NetDaemonNet.network_message_count()==0);
  assert(NetDaemonNet.primary_state_.trailing_words_[4]==0);
  NetDaemonNet.process_slots_[1].identity_=4;
  NetDaemonNet.player_count_=1;

  // The callback changes the singleton mask: native code reloads after log_say.
  NetDaemonNet.factions_waiting_mask()=1u<<2;
  log_mutation=1;
  netdaemon_handle_withdraw_end_turn(&passed,message,2);
  assert(NetDaemonNet.factions_waiting_mask()==(1u<<4));
  assert(passed.factions_waiting_mask()==0);
  assert(messages==2 && draws==1);

  // Approval reads and clears singleton lock-state even with another receiver.
  GameDataStateStorage.fields_.game_state_=2;
  GameDataStateStorage.fields_.turn_current_num_=9;
  NetDaemonNet.factions_waiting_mask()=(1u<<5);
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  NetDaemonNet.channels_[0].leading_words_[0]=0x31;
  passed.primary_state_.leading_words_[0]=0;
  passed.channels_[0].leading_words_[0]=0x777;
  netdaemon_complete_local_turn_wait(&passed,message);
  assert(NetDaemonNet.factions_waiting_mask()==(1u<<5));
  assert(NetDaemonNet.channels_[0].leading_words_[0]==(0x31u & ~(1u<<2)));
  assert(passed.channels_[0].leading_words_[0]==0x777);
  assert(GameDataStateStorage.fields_.game_state_==0 && draws==2);

  // 0xFF sentinel remains untouched and still reaches the ordinary draw tail.
  GameDataStateStorage.fields_.game_state_=2;
  NetDaemonNet.factions_waiting_mask()=0xFF;
  netdaemon_complete_local_turn_wait(&passed,message);
  assert(NetDaemonNet.factions_waiting_mask()==0xFF && draws==3);
}
'''
    source = tmp_path / "wait_mask.cpp"
    executable = tmp_path / "wait_mask"
    source.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-parameter",
                            str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr

    # These executed mutants demonstrate that the controls detect the three
    # ownership/reload failures this closure is intended to prevent.
    mutants = {
        "reset-omits-wait-mask": harness.replace(
            "primary_state_.trailing_words_[7] = 0;", "/* mutant omitted reset */", 1),
        "withdraw-snapshots-before-log": harness.replace(
            '    log_say("From Client: WITHDRAW END-OF-TURN", sender, 0, 0);\n\n'
            '    // The native handler reloads the shared mask after log_say; the callback\n'
            '    // may change it, so do not carry a pre-callback snapshot into the test.\n'
            '    const uint32_t waiting = NetDaemonNet.factions_waiting_mask();',
            '    const uint32_t waiting = NetDaemonNet.factions_waiting_mask();\n'
            '    log_say("From Client: WITHDRAW END-OF-TURN", sender, 0, 0);', 1),
        "approval-uses-passed-daemon": harness.replace(
            "NetDaemonNet.primary_state_.leading_words_[0];",
            "daemon->primary_state_.leading_words_[0];", 1),
        "approval-clears-passed-daemon": harness.replace(
            "NetDaemonNet.channels_[0].leading_words_[0] &= ~faction_bit;",
            "daemon->channels_[0].leading_words_[0] &= ~faction_bit;", 1),
    }
    for name, mutant_source in mutants.items():
        assert mutant_source != harness, name
        mutant_cpp = tmp_path / (name + ".cpp")
        mutant_exe = tmp_path / name
        mutant_cpp.write_text(mutant_source)
        mutant_build = subprocess.run(
            [compiler, "-std=c++11", "-Wno-unused-parameter", str(mutant_cpp), "-o", str(mutant_exe)],
            capture_output=True, text=True)
        assert mutant_build.returncode == 0, mutant_build.stdout + mutant_build.stderr
        mutant_run = subprocess.run([str(mutant_exe)], capture_output=True, text=True)
        assert mutant_run.returncode != 0, name
