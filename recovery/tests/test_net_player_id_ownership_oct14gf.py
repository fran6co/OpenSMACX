"""Shared local/host IDs remain fields of NetDaemonNet, not NetWin copies."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start:offset + 1]
    raise AssertionError(f"unterminated definition {signature}")


def test_original_identities_bind_the_real_net_members():
    header = (ROOT / "src/net_class.h").read_text()
    win_header = (ROOT / "src/netwin.h").read_text()
    net = (ROOT / "src/net_class.cpp").read_text()
    daemon = (ROOT / "src/netdaemon.cpp").read_text()
    win = (ROOT / "src/netwin.cpp").read_text()

    assert "RECOVERY_MEMBER: v1 Net local_player_id_ 0x760" in header
    assert "RECOVERY_MEMBER: v1 Net host_player_id_ 0x764" in header
    assert "DPID local_player_id_;" in header
    assert "uint32_t host_player_id_;" in header
    assert "DPID local_player_id() const" in header
    assert "uint32_t host_player_id() const" in header
    assert "RECOVERY_OBJECT: v3 0x0093D4F0 ?NetWinPlayerId@@3HA member ?NetDaemonNet@@3VNetDaemon@@A local_player_id_" in daemon
    assert "RECOVERY_OBJECT: v3 0x0093D4F4 ?NetWinOtherPlayerId@@3HA member ?NetDaemonNet@@3VNetDaemon@@A host_player_id_" in daemon
    assert "NetWinPlayerId" not in win_header
    assert "NetWinOtherPlayerId" not in win_header
    assert "int NetWinPlayerId" not in win
    assert "int NetWinOtherPlayerId" not in win
    assert "local_player_id_ = 0;" in _definition(net, "Net::Net()")
    assert "host_player_id_ = 0;" in _definition(net, "void Net::close()")
    assert "NetDaemonNet.local_player_id()" in win
    assert "NetDaemonNet.host_player_id()" in win


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_live_netwin_sender_observes_host_id_updates_on_the_owner(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/netwin.cpp").read_text()
    body = _definition(source, "void __cdecl send_session_packet(int slot_id, int target)")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
using DPID = uint32_t;
struct NetWinSessionOptionsState { unsigned char bytes_[24]; };
struct NetWinSessionOptionsPacket {
  uint32_t opcode_;
  unsigned char header_[12];
  NetWinSessionOptionsState options_;
};
static_assert(sizeof(NetWinSessionOptionsPacket) == 40, "packet size");
NetWinSessionOptionsState NetWinSessionOptions{};
struct Net {
  uint32_t target_ = 0;
  int sends_ = 0;
  void send(void *, int, uint32_t target, int) { target_ = target; ++sends_; }
};
struct ProcessSlot { uint32_t pid_ = 0; };
struct NetDaemon : Net {
  DPID local_player_id_ = 0x87654321u;
  uint32_t host_player_id_ = 0xF1234567u;
  ProcessSlot process_slots_[2]{};
  DPID local_player_id() const { return local_player_id_; }
  uint32_t host_player_id() const { return host_player_id_; }
} NetDaemonNet;
''' + body + r'''
int main() {
  send_session_packet(0, 0);
  assert(NetDaemonNet.sends_ == 1);
  assert(NetDaemonNet.target_ == 0xF1234567u);
  NetDaemonNet.host_player_id_ = 0x80000009u;
  send_session_packet(0, 0);
  assert(NetDaemonNet.sends_ == 2);
  assert(NetDaemonNet.target_ == 0x80000009u);
}
'''
    path = tmp_path / "shared_net_id.cpp"
    path.write_text(harness)
    executable = tmp_path / f"shared-net-id-{compiler_name.replace('+', 'p')}"
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(path), "-o", str(executable)],
        capture_output=True, text=True, timeout=60, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True,
                         timeout=30, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr
