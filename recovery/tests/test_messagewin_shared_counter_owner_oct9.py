"""Actual-source controls for MessageWin's NetDaemon-owned network counter."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def _braced(source: str, opening: int) -> tuple[str, int]:
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index], index + 1
    raise AssertionError("unterminated source block")


def test_shared_counter_signed_order_increment_and_identity(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    net_source = (ROOT / "src/netdaemon.cpp").read_text()
    case_start = net_source.index("case 0x4305: {")
    case_open = net_source.index("{", case_start)
    case_body, _ = _braced(net_source, case_open)
    message_source = (ROOT / "src/messagewin.cpp").read_text()
    increment = "++NetDaemonNet.network_message_count();"
    assert increment in message_source
    assert "int MessageWinNetworkMessageCount;" not in message_source
    assert "extern int MessageWinNetworkMessageCount" not in net_source
    game_source = (ROOT / "src/game.cpp").read_text()
    assert "MessageWinNetworkMessageCount" not in game_source
    assert "RECOVERY_OBJECT: v3 0x0093E964 0x0093E968 ?MessageWinNetworkMessageCount@@3HA 0x004713B0 member ?NetDaemonNet@@3VNetDaemon@@A lock_state_[2].trailing_words_[7]" in net_source

    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
struct LockState { uint32_t lead_[4]; uint32_t payload_; uint32_t trailing_words_[10]; };
struct NetDaemon { LockState lock_state_[3];
  uint32_t &network_message_count() { return lock_state_[2].trailing_words_[7]; }
} NetDaemonNet;
struct OtherDaemon { LockState lock_state_[3];
  uint32_t &network_message_count() { return lock_state_[2].trailing_words_[7]; }
} other;
int logs=0;
void log_say(const char *,int,int,int) { ++logs; }
static bool table_f_case(NetDaemon *daemon, unsigned short type, unsigned char *message) {
  switch (type) {
  case 0x4305: {
'''+case_body+r'''
  }
  default: return false;
  }
}
static void increment_actual_source_statement() {
  ++NetDaemonNet.network_message_count();
}
int main() {
  unsigned char message[0x18] = {};
  int incoming = 0;
  std::memcpy(message+0x10, &incoming, sizeof(incoming));
  NetDaemon passed;
  passed.network_message_count()=77;

  NetDaemonNet.network_message_count()=0x80000000u; // signed INT_MIN
  incoming=0; std::memcpy(message+0x10,&incoming,sizeof(incoming));
  assert(table_f_case(&passed,0x4305,message));
  assert(NetDaemonNet.network_message_count()==0 && passed.network_message_count()==77);

  NetDaemonNet.network_message_count()=0x7fffffffu;
  incoming=-1; std::memcpy(message+0x10,&incoming,sizeof(incoming));
  table_f_case(&passed,0x4305,message);
  assert(NetDaemonNet.network_message_count()==0x7fffffffu);

  NetDaemonNet.network_message_count()=0xffffffffu;
  incoming=-1; std::memcpy(message+0x10,&incoming,sizeof(incoming));
  table_f_case(&passed,0x4305,message);
  assert(NetDaemonNet.network_message_count()==0xffffffffu);

  NetDaemonNet.network_message_count()=0xffffffffu;
  increment_actual_source_statement();
  assert(NetDaemonNet.network_message_count()==0);
  assert(other.network_message_count()==0 && logs==3);
}
'''
    source = tmp_path / "shared_counter.cpp"
    executable = tmp_path / "shared_counter"
    source.write_text(harness)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                            "-Wno-unused-parameter", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
