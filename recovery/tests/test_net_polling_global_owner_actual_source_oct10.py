"""Host controls for actual Net polling source and its row/global owners."""
from pathlib import Path
import shutil
import subprocess
import tempfile

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
HEADER = ROOT / "src/net_class.h"
SOURCE = ROOT / "src/net_class.cpp"


def _mask(text):
    return COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), text)


def _definition(text, marker):
    masked = _mask(text)
    start = masked.index(marker)
    opening = masked.index("{", start)
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{":
            depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unbalanced source definition: " + marker)


def _actual_sources():
    header = HEADER.read_text()
    source = SOURCE.read_text()
    player = _definition(header, "class NetPlayer")
    polling = _definition(source, "void Net::check_polling()")
    polling = polling.replace("void Net::check_polling()",
                              "void HarnessNet::check_polling()", 1)
    abort = _definition(source, "if (answer == 0)")
    return player, polling, abort, source


HARNESS_PREFIX = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define WINAPI
typedef uint32_t DWORD;
int ExitTurnLoop = 0;
int NetDirectPlay = 1;
uint32_t NetReapBusy = 0;
uint32_t NetKeepAliveBusy = 0;
uint32_t fake_now = 10000;
DWORD timeGetTime() { return fake_now; }
struct NetPlayer {
  uint32_t field_0_, field_4_, field_8_, field_C_, stamp_10_;
  uint8_t flags_14_;
  char name_and_tail_[0x43];
};
'''


def _harness(player, polling, abort):
    # Compile the exact declared row (not a pointer-shaped stand-in); the
    # separately repeated declaration in the small host prefix is removed.
    prefix = HARNESS_PREFIX.replace(
        "struct NetPlayer {\n  uint32_t field_0_, field_4_, field_8_, field_C_, stamp_10_;\n  uint8_t flags_14_;\n  char name_and_tail_[0x43];\n};\n", "")
    return prefix + player + ";\n" + r'''
typedef char actual_row_extent[(sizeof(NetPlayer) == 0x58) ? 1 : -1];
typedef char actual_stamp_offset[(offsetof(NetPlayer, stamp_10_) == 0x10) ? 1 : -1];
struct HarnessNet {
  NetPlayer players_[16];
  uint32_t poll_time_DC_;
  uint32_t voice_flags_D8_;
  uint32_t field_760_, field_764_, poll_timeout_E0_;
  int sends, drops, reenter;
  HarnessNet() : poll_time_DC_(0), voice_flags_D8_(0), field_760_(0),
      field_764_(0), poll_timeout_E0_(100), sends(0), drops(0), reenter(0) {
    memset(players_, 0, sizeof(players_));
  }
  void send_packet_type(int, int, int, int, int);
  int drop_player(uint32_t, int);
  void check_polling();
};
HarnessNet *nested;
void HarnessNet::send_packet_type(int, int, int, int, int) {
  ++sends;
  if (reenter == 1 && nested) nested->check_polling();
}
int HarnessNet::drop_player(uint32_t, int) {
  ++drops;
  if (reenter == 2 && nested) nested->check_polling();
  return 0;
}
''' + polling + r'''
int actual_abort_answer(int answer) {
''' + abort + r'''
  return 0;
}
int main() {
  assert(NetReapBusy == 0 && NetKeepAliveBusy == 0);
  HarnessNet outer, inner;
  nested = &inner;
  outer.reenter = 1;
  outer.poll_time_DC_ = 1;
  inner.poll_time_DC_ = 1;
  outer.check_polling();
  assert(outer.sends == 1 && inner.sends == 0);
  assert(NetKeepAliveBusy == 0 && NetReapBusy == 0);
  assert(outer.poll_time_DC_ == fake_now);

  // Branch 3: both leading rows stale; preserve the all-player drop and
  // timestamp both real row members with one post-drop clock read.
  HarnessNet both;
  both.field_760_ = 77;
  both.field_764_ = 88;
  both.voice_flags_D8_ = 0x40000;
  both.players_[0].stamp_10_ = 100;
  both.players_[1].stamp_10_ = 100;
  both.reenter = 2;
  both.poll_time_DC_ = fake_now;
  inner.poll_time_DC_ = 1;
  fake_now += 3000;
  both.check_polling();
  assert(both.drops == 1);
  assert(inner.sends == 1 && inner.drops == 0);
  assert(both.players_[0].stamp_10_ == fake_now);
  assert(both.players_[1].stamp_10_ == fake_now);
  assert(NetReapBusy == 0 && NetKeepAliveBusy == 0);

  // Equality with the unsigned timeout is not the early-return case.
  HarnessNet equal;
  equal.field_760_ = 77;
  equal.field_764_ = 88;
  equal.voice_flags_D8_ = 0x40000;
  equal.poll_timeout_E0_ = 100;
  equal.players_[0].stamp_10_ = fake_now - 100;
  equal.players_[1].stamp_10_ = fake_now - 100;
  equal.poll_time_DC_ = fake_now - 3000;
  equal.check_polling();
  assert(equal.drops == 1 && NetReapBusy == 0);

  // The original branch-3 early return intentionally leaves ReapBusy set.
  HarnessNet stuck;
  stuck.field_760_ = 77;
  stuck.field_764_ = 88;
  stuck.voice_flags_D8_ = 0x40000;
  stuck.players_[0].stamp_10_ = 0;
  stuck.players_[1].stamp_10_ = 100;
  stuck.poll_time_DC_ = fake_now;
  fake_now += 3000;
  stuck.check_polling();
  assert(stuck.drops == 0 && NetReapBusy == 1);
  NetReapBusy = 0; // isolate the next independent original-path control

  ExitTurnLoop = 0;
  assert(actual_abort_answer(0) == 1 && ExitTurnLoop == 1);
  ExitTurnLoop = 0;
  assert(actual_abort_answer(1) == 0 && ExitTurnLoop == 0);
  return 0;
}
'''


def _compile_and_run(source, compiler):
    with tempfile.TemporaryDirectory(prefix="net-polling-actual-") as temp:
        cpp = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_polling_ownership_and_abort_branch():
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        import pytest
        pytest.skip("host C++ compiler unavailable")
    player, polling, abort, source = _actual_sources()
    assert "uint32_t NetReapBusy;" in source
    assert "uint32_t NetKeepAliveBusy;" in source
    assert "NetReapBusy = 1;" in polling and "NetKeepAliveBusy = 1;" in polling
    assert "uint32_t *stamps" not in polling
    assert "players_[checked].stamp_10_" in polling
    assert "players_[1].stamp_10_ = cut;" in polling
    assert "players_[0].stamp_10_ = cut;" in polling
    assert polling.index("players_[1].stamp_10_ = cut;") < polling.index(
        "players_[0].stamp_10_ = cut;")
    assert "ExitTurnLoop = 1;" in abort and "return 1;" in abort
    assert "NetDropAbortFlag" not in source
    result = _compile_and_run(_harness(player, polling, abort), compiler)
    assert result.returncode == 0, result.stdout + result.stderr

    # This compiles, but redirects the second checked row to row 2. The actual
    # branch witness must fail instead of accepting a cosmetic source match.
    damaged = polling.replace(
        "players_[checked].stamp_10_",
        "players_[checked == 0 ? 0 : 2].stamp_10_")
    assert damaged != polling
    mutant = _compile_and_run(_harness(player, damaged, abort), compiler)
    assert mutant.returncode != 0, "row-selection mutant escaped host controls"

