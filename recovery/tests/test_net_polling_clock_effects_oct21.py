"""Pinned clock-read placement and DWORD wrap behavior in Net polling."""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from test_net_polling_global_owner_actual_source_oct10 import (
    _actual_sources,
    _compile_and_run,
    _harness,
)

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())


def _pinned_polling_instructions():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned image unavailable")
    image = PeImage(image_path.read_bytes(),
                    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    return {i.address: (i.mnemonic, i.op_str) for i in
            Cs(CS_ARCH_X86, CS_MODE_32).disasm(
                image.read(Span(0x6320E0, 0x632290)), 0x6320E0)}


def test_pinned_guard_unsigned_threshold_and_direct_clock_reads():
    instructions = _pinned_polling_instructions()
    for address, instruction in {
        0x6320E1: ("mov", "eax, dword ptr [0x9be600]"),
        0x6320EE: ("je", "0x63228a"),
        0x6320F4: ("mov", "ebp, dword ptr [0x669368]"),
        0x632102: ("sub", "eax, 0x7d0"),
        0x632107: ("cmp", "eax, ecx"),
        0x632109: ("jbe", "0x63228a"),
        0x632244: ("call", "dword ptr [0x669368]"),
        0x63226E: ("call", "dword ptr [0x669368]"),
        0x632274: ("mov", "edx, dword ptr [esp + 0x10]"),
        0x632278: ("mov", "dword ptr [esi + 0x1bc], eax"),
        0x63227E: ("mov", "dword ptr [edx], eax"),
    }.items():
        assert instructions[address] == instruction

    _, polling, _, _ = _actual_sources()
    guard = polling.index("if (NetDirectPlay == 0)")
    capture = polling.index("DWORD (WINAPI *const now)() = timeGetTime;")
    threshold_tick = polling.index("const DWORD initial_tick = now();")
    threshold = polling.index("const DWORD keepalive_threshold = initial_tick - 2000U;")
    assert guard < capture < threshold
    assert threshold_tick < threshold < polling.index(
        "if (keepalive_threshold <= poll_time_DC_)")
    for snapshot, clock, comparison in (
        ("const DWORD stamp = player.stamp_10_;", "const DWORD tick = now();",
         "if (tick - stamp > poll_timeout_E0_"),
        ("const DWORD stamp = players_->stamp_10_;", "const DWORD tick = now();",
         "if (tick - stamp <= poll_timeout_E0_"),
        ("const DWORD stamp = players_[checked].stamp_10_;",
         "const DWORD tick = timeGetTime();", "if (tick - stamp < poll_timeout_E0_"),
    ):
        snapshot_at = polling.index(snapshot)
        clock_at = polling.index(clock, snapshot_at)
        assert snapshot_at < clock_at < polling.index(comparison, clock_at)
    neither_host = polling.index("for (int checked = 0; checked < 2; ++checked)")
    assert polling.count("timeGetTime()") == 2
    assert polling.index("timeGetTime()", neither_host) > neither_host
    assert polling.rindex("timeGetTime()") > polling.index(
        "if (drop_player(0xFFFFFFFF, 0) == 0)")
    assert polling.index("players_[1].stamp_10_ = cut;") < polling.index(
        "players_[0].stamp_10_ = cut;")
    # The original neither-host early return intentionally bypasses the
    # shared busy clear; this recovery must retain that observed quirk.
    assert polling.index("if (stamp == 0)", neither_host) < polling.index(
        "NetReapBusy = 0;", neither_host)


CLOCK_CASES = r'''
  // The null DirectPlay return occurs before the import slot is read.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet no_direct_play;
  fake_now = 10000;
  NetDirectPlay = 0; clock_loads = 0;
  no_direct_play.check_polling();
  assert(clock_loads == 0 && no_direct_play.sends == 0);
  NetDirectPlay = 1;

  // The captured import remains selected in the host arm even when a callback
  // replaces the import slot after the keep-alive send.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet host;
  host.local_player_id_ = 88; host.host_player_id_ = 88;
  host.voice_flags_D8_ = 0x40000;
  host.poll_time_DC_ = 7000;
  host.players_[0].field_0_ = 123;
  host.players_[0].stamp_10_ = 7000;
  host.players_[1].field_0_ = 88;
  switch_count = 0; old_tick = 10000; replacement_tick = 20000;
  clock_loads = 0;
  clock_target = switching_clock;
  host.check_polling();
  assert(host.drops == 1 && host.players_[0].stamp_10_ == old_tick);
  assert(clock_target == replacement_clock && clock_loads == 1);

  // The neither-host arm reloads the import for each row test and after the
  // drop. The replacement clock therefore supplies both refreshed stamps.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet neither;
  neither.local_player_id_ = 77; neither.host_player_id_ = 88;
  neither.voice_flags_D8_ = 0x40000;
  neither.poll_time_DC_ = 7000;
  neither.poll_timeout_E0_ = 100;
  neither.players_[0].field_0_ = 123;
  neither.players_[1].field_0_ = 456;
  neither.players_[0].stamp_10_ = 9950;
  neither.players_[1].stamp_10_ = 9950;
  switch_count = 0; old_tick = 10000; replacement_tick = 20000;
  clock_loads = 0;
  clock_target = switching_clock;
  neither.check_polling();
  assert(neither.drops == 1);
  assert(neither.players_[0].stamp_10_ == replacement_tick);
  assert(neither.players_[1].stamp_10_ == replacement_tick);
  assert(clock_target == replacement_clock && clock_loads == 4);

  // Snapshot each stamp before calling the clock, then reload timeout and
  // player ID after that call. The callback changes all three live values.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet host_snapshot;
  host_snapshot.local_player_id_ = 88; host_snapshot.host_player_id_ = 88;
  host_snapshot.voice_flags_D8_ = 0x40000;
  host_snapshot.poll_time_DC_ = 7000;
  host_snapshot.poll_timeout_E0_ = 5000;
  host_snapshot.players_[0].field_0_ = 123;
  host_snapshot.players_[0].stamp_10_ = 7000;
  host_snapshot.players_[1].field_0_ = 88;
  clock_net = &host_snapshot; clock_mode = 1; clock_calls = 0;
  fake_now = 10000; clock_target = state_clock;
  host_snapshot.check_polling();
  assert(host_snapshot.drops == 1 && host_snapshot.last_drop_id == 456);
  assert(host_snapshot.players_[0].stamp_10_ == 10000);
  assert(host_snapshot.poll_timeout_E0_ == 100);

  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet one_snapshot;
  one_snapshot.local_player_id_ = 77; one_snapshot.host_player_id_ = 88;
  one_snapshot.voice_flags_D8_ = 0x40000;
  one_snapshot.poll_time_DC_ = 7000;
  one_snapshot.poll_timeout_E0_ = 5000;
  one_snapshot.players_[0].field_0_ = 123;
  one_snapshot.players_[0].stamp_10_ = 7000;
  one_snapshot.players_[1].field_0_ = 77;
  clock_net = &one_snapshot; clock_mode = 1; clock_calls = 0;
  fake_now = 10000; clock_target = state_clock;
  one_snapshot.check_polling();
  assert(one_snapshot.drops == 1 && one_snapshot.last_drop_id == 456);
  assert(one_snapshot.players_[0].stamp_10_ == 10000);

  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet neither_snapshot;
  neither_snapshot.local_player_id_ = 77; neither_snapshot.host_player_id_ = 88;
  neither_snapshot.voice_flags_D8_ = 0x40000;
  neither_snapshot.poll_time_DC_ = 7000;
  neither_snapshot.poll_timeout_E0_ = 5000;
  neither_snapshot.players_[0].field_0_ = 123;
  neither_snapshot.players_[1].field_0_ = 456;
  neither_snapshot.players_[0].stamp_10_ = 7000;
  neither_snapshot.players_[1].stamp_10_ = 7000;
  clock_net = &neither_snapshot; clock_mode = 1; clock_calls = 0;
  fake_now = 10000; clock_target = state_clock;
  neither_snapshot.check_polling();
  assert(neither_snapshot.drops == 1 &&
         neither_snapshot.last_drop_id == 0xffffffffU);
  assert(neither_snapshot.players_[0].stamp_10_ == 10000);
  assert(neither_snapshot.players_[1].stamp_10_ == 10000);

  // The clock callback changes poll_time_DC_. The original loads that member
  // after its clock call, so the new value takes the early return.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet poll_time_snapshot;
  poll_time_snapshot.poll_time_DC_ = 7000;
  poll_time_snapshot.voice_flags_D8_ = 0;
  clock_net = &poll_time_snapshot; clock_mode = 2; clock_calls = 0;
  clock_loads = 0;
  fake_now = 10000; clock_target = state_clock;
  poll_time_snapshot.check_polling();
  assert(poll_time_snapshot.poll_time_DC_ == 9000);
  assert(poll_time_snapshot.sends == 0 && clock_loads == 1);

  // The pinned unsigned absolute comparison suppresses this post-wrap poll:
  // now - 0x7d0 (0x830) <= 0xfffff000. A conventional elapsed comparison
  // sees 0x2000 elapsed and incorrectly sends the keep-alive.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet wrapped;
  wrapped.poll_time_DC_ = 0xfffff000U;
  wrapped.voice_flags_D8_ = 0;
  fake_now = 0x1000U; old_tick = 0x1000U; replacement_tick = 0x20000U;
  switch_count = 0; clock_target = fixed_clock;
  wrapped.check_polling();
  assert(wrapped.sends == 0 && wrapped.poll_time_DC_ == 0xfffff000U);
  assert(NetKeepAliveBusy == 0 && NetReapBusy == 0);
'''


def _clock_control_source(polling, *, damage=None):
    changes = {
        "elapsed_threshold": (
            "const DWORD keepalive_threshold = initial_tick - 2000U;\n    if (keepalive_threshold <= poll_time_DC_)",
            "const DWORD keepalive_threshold = initial_tick - poll_time_DC_;\n    if (keepalive_threshold <= 2000U)"),
        "poll_before_clock": (
            "const DWORD initial_tick = now();\n    const DWORD keepalive_threshold = initial_tick - 2000U;\n    if (keepalive_threshold <= poll_time_DC_)",
            "const DWORD previous_poll_time = poll_time_DC_;\n    const DWORD initial_tick = now();\n    const DWORD keepalive_threshold = initial_tick - 2000U;\n    if (keepalive_threshold <= previous_poll_time)"),
        "cache_neither_host": (
            "timeGetTime()", "now()"),
        "capture_before_guard": (
            "if (NetDirectPlay == 0) {\n        return;\n    }\n    DWORD (WINAPI *const now)() = timeGetTime;",
            "DWORD (WINAPI *const now)() = timeGetTime;\n    if (NetDirectPlay == 0) {\n        return;\n    }"),
        "clear_original_stuck_busy": (
            "const DWORD stamp = players_[checked].stamp_10_;\n            if (stamp == 0) {\n                return;",
            "const DWORD stamp = players_[checked].stamp_10_;\n            if (stamp == 0) {\n                NetReapBusy = 0;\n                return;"),
        "stamp_after_clock": (
            "const DWORD stamp = player.stamp_10_;\n                    if (stamp != 0) {\n                        const DWORD tick = now();",
            "if (player.stamp_10_ != 0) {\n                        const DWORD tick = now();\n                        const DWORD stamp = player.stamp_10_;"),
        "timeout_before_clock": (
            "const DWORD tick = now();\n                        if (tick - stamp > poll_timeout_E0_",
            "const DWORD timeout = poll_timeout_E0_;\n                        const DWORD tick = now();\n                        if (tick - stamp > timeout"),
        "id_before_clock": (
            "const DWORD stamp = player.stamp_10_;\n                    if (stamp != 0) {",
            "const DWORD stamp = player.stamp_10_;\n                    const DWORD id_before_clock = player.field_0_;\n                    if (stamp != 0) {")
    }
    if damage:
        before, after = changes[damage]
        assert before in polling
        polling = polling.replace(before, after, 1)

    player, _, abort, _ = _actual_sources()
    polling = polling.replace(
        "DWORD (WINAPI *const now)() = timeGetTime;",
        "DWORD (WINAPI *const now)() = load_clock_import();", 1)
    polling = polling.replace("timeGetTime()", "load_clock_import()()")
    source = _harness(player, polling, abort)
    source = source.replace("int sends, drops, reenter;",
                            "int sends, drops, reenter; uint32_t last_drop_id;")
    source = source.replace("drops(0), reenter(0)",
                            "drops(0), reenter(0), last_drop_id(0)")
    source = source.replace("int HarnessNet::drop_player(uint32_t, int)",
                            "int HarnessNet::drop_player(uint32_t player_id, int)")
    source = source.replace("  ++drops;", "  ++drops; last_drop_id = player_id;")
    clock_prefix = """uint32_t fake_now = 10000;
uint32_t old_tick = 10000, replacement_tick = 20000;
int switch_count = 0, clock_loads = 0, clock_mode = 0, clock_calls = 0;
DWORD fixed_clock();
DWORD (*clock_target)() = fixed_clock;
DWORD fixed_clock() { return fake_now; }
DWORD replacement_clock() { return replacement_tick; }
DWORD switching_clock() {
  if (++switch_count == 2) clock_target = replacement_clock;
  return fake_now;
}
DWORD timeGetTime() { return clock_target(); }
DWORD (*load_clock_import())() { ++clock_loads; return clock_target; }"""
    old_clock_prefix = "uint32_t fake_now = 10000;\nDWORD timeGetTime() { return fake_now; }"
    assert old_clock_prefix in source
    source = source.replace(old_clock_prefix, clock_prefix, 1)
    hook = "HarnessNet *nested;\n"
    assert source.count(hook) == 1
    source = source.replace(hook, hook + r'''HarnessNet *clock_net;
DWORD state_clock() {
  ++clock_calls;
  if (clock_net && clock_mode == 1 && clock_calls == 3) {
    clock_net->players_[0].stamp_10_ = 9999;
    clock_net->players_[0].field_0_ = 456;
    clock_net->poll_timeout_E0_ = 100;
  }
  if (clock_net && clock_mode == 2 && clock_calls == 1)
    clock_net->poll_time_DC_ = 9000;
  return fake_now;
}
''', 1)
    if damage == "id_before_clock":
        source = source.replace(
            "drop_player(players_[0x10 - count].field_0_, 0)",
            "drop_player(id_before_clock, 0)", 1)
    marker = "  ExitTurnLoop = 0;\n  assert(actual_abort_answer(0)"
    assert source.count(marker) == 1
    return source.replace(marker, CLOCK_CASES + marker, 1)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_polling_clock_effects_and_compiling_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    _, polling, _, _ = _actual_sources()
    source = _clock_control_source(polling)
    result = _compile_and_run(source, compiler)
    assert result.returncode == 0, result.stdout + result.stderr

    for damage in ("elapsed_threshold", "poll_before_clock", "cache_neither_host",
                   "capture_before_guard", "clear_original_stuck_busy",
                   "stamp_after_clock", "timeout_before_clock",
                   "id_before_clock"):
        mutant = _clock_control_source(polling, damage=damage)
        result = _compile_and_run(mutant, compiler)
        assert result.returncode != 0, damage + " mutant escaped host controls"
