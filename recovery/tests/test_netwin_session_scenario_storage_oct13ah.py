"""Bounded controls for the named scenario-text view into session wire storage.

The host executable exercises literal current source with the real declared
session block. It is not native ABI, filename-capacity, or game-runtime proof.
"""
from dataclasses import fields
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
TASK = "netwin-session-scenario-storage-owner-oct13ah"
BASE = "75a020323f4ff637825807b3add2de540dd60a68"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DAEMON = "src/netdaemon.cpp"


def _tokens(source: str) -> list[str]:
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    result = []
    for match in token.finditer(source):
        value = match.group(0)
        if value.startswith("//") or value.startswith("/*") or value.isspace():
            continue
        result.append(value)
    return result


def _extract(source: str, signature: str) -> str:
    start = source.index(signature)
    sig_brace = signature.find("{")
    opening = (start + sig_brace if sig_brace >= 0
               else source.index("{", start + len(signature)))
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        ch = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if ch == "\\":
                index += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        else:
            if ch == "/" and nxt == "/":
                state = "line"
                index += 1
            elif ch == "/" and nxt == "*":
                state = "block"
                index += 1
            elif ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated C++ definition {signature}")


def _replace_once(source: str, before: str, after: str, label: str) -> str:
    assert source.count(before) == 1, (label, source.count(before), before)
    return source.replace(before, after, 1)


def _restore_cpp(current: str, baseline: str) -> str:
    # Reverse this task's owner-field migration before applying the historical
    # scenario-storage reversal below. This is test-only reconstruction; the
    # installed source retains direct typed member accesses.
    current = current.replace("NetDaemonNet.player_count_", "*NetWinPlayerCount")
    current = current.replace("NetWinSessionData.mode_", "*NetWinEighthSlotWord")
    current = current.replace("NetWinSessionData.scenario_text_wire_tail_[0]",
                              "*NetWinScenarioText")
    current = current.replace("NetWinSessionData.scenario_text_wire_tail_",
                              "NetWinScenarioText")
    current = current.replace("NetWinSessionOptions.time_control_", "NetWinNinthSlot[3]")
    for suffix, field in (("A", "rule_bits_a_"), ("B", "rule_bits_b_"),
                          ("C", "rule_bits_c_")):
        current = current.replace(f"NetWinSessionOptions.{field}",
                                  f"*NetWinRuleBits{suffix}")
    current = current.replace("NetDaemonNet.primary_state_.leading_words_[0]",
                              "*NetWinRulesGate")
    current = current.replace(
        "// Original 0x0090DB98 begins the eight NetWinLobbyRows (0x17C stride).",
        "static int *const NetWinPlayerCount = &NetDaemonNet.player_count_;\n\n"
        "// Original 0x0090DB98 begins the eight NetWinLobbyRows (0x17C stride).")
    current = current.replace(
        "/*\nPurpose: Seed the seven base faction records of the static faction table.",
        "static unsigned int *const NetWinEighthSlotWord = &NetWinSessionData.mode_;\n"
        "static unsigned char *const NetWinScenarioText = &NetWinSessionData.state_[0x20];\n"
        "static unsigned char *const NetWinNinthSlot =\n"
        "    reinterpret_cast<unsigned char *>(&NetWinSessionOptions);\n\n"
        "/*\nPurpose: Seed the seven base faction records of the static faction table.")
    current = current.replace(
        "// 0x0080A6F0 - the planet-size name count beside the fixed dialog",
        "static unsigned int *const NetWinRuleBitsA = &NetWinSessionOptions.rule_bits_a_;\n"
        "static unsigned int *const NetWinRuleBitsB = &NetWinSessionOptions.rule_bits_b_;\n"
        "static unsigned int *const NetWinRuleBitsC = &NetWinSessionOptions.rule_bits_c_;\n"
        "static unsigned int *const NetWinRulesGate =\n"
        "    &NetDaemonNet.primary_state_.leading_words_[0];\n\n"
        "// 0x0080A6F0 - the planet-size name count beside the fixed dialog")
    draw = _extract(current, "void NetWin::draw_session_info()")
    draw_new = "strcat(StringTemp,\n               NetWinScenarioText);"
    assert draw.count(draw_new) == 1
    current = current.replace(draw, draw.replace(
        draw_new,
        "strcat(StringTemp,\n               reinterpret_cast<const char *>(NetWinScenarioText));"), 1)

    pick = _extract(current, "void NetWin::pick_map()")
    clear = "*NetWinScenarioText = 0;"
    assert pick.count(clear) == 2
    assert pick.count("strcpy(NetWinScenarioText, scenario_name);") == 1
    pick = pick.replace(clear, "NetWinSessionData.state_[0x20] = 0;")
    pick = pick.replace("strcpy(NetWinScenarioText, scenario_name);",
                        "strcpy(reinterpret_cast<char *>(NetWinScenarioText),\n                       scenario_name);")
    current = current.replace(_extract(current, "void NetWin::pick_map()"), pick, 1)

    reset = _extract(current, 'extern "C" void __cdecl sub_4e2450()')
    reset_clear = "*NetWinScenarioText = 0;"
    assert reset.count(reset_clear) == 1
    current = current.replace(reset, reset.replace(
        reset_clear, "NetWinSessionData.state_[0x20] = 0;"), 1)
    assert _tokens(current) == _tokens(baseline), "netwin.cpp exact bounded reversal"
    return current


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def test_immutable_task_base_source_history_and_exact_storage_delta():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    for relative in ("src/netwin.h", "src/netwin.cpp", DAEMON):
        old = subprocess.check_output(["git", "show", f"{BASE}:{relative}"],
                                      cwd=ROOT, text=True, timeout=30)
        current = (ROOT / relative).read_text(encoding="utf-8")
        if relative == "src/netwin.cpp":
            _restore_cpp(current, old)
        elif relative == "src/netwin.h":
            block = _extract(current, "struct NetWinSessionBlock {")
            expected = """struct NetWinSessionBlock {
  uint32_t mode_;
  uint8_t reserved_session_prefix_[0x20];
  char scenario_text_wire_tail_[0x144];
  void receive_wire_block(const unsigned char *bytes);
  void write_row8_prefix(uint8_t value);
  char *row8_leader_name();
  char *row8_noun_faction();
}"""
            assert _tokens(block) == _tokens(expected), "bounded current session-block schema"
            block_old = _extract(old, "struct NetWinSessionBlock {")
            restored = current.replace(block, block_old, 1)
            offset_assert = (
                'static_assert(offsetof(NetWinSessionBlock, scenario_text_wire_tail_) == 0x24,\n'
                '              "scenario text begins at the observed session storage offset");\n')
            assert restored.count(offset_assert) == 1
            restored = restored.replace(offset_assert, "", 1)
            assert _tokens(restored) == _tokens(old), "netwin.h exact bounded reversal"
        else:
            assert current.encode() == old.encode(), "verification-only netdaemon.cpp changed"

        before = {row.address: row for row in read_text(old, ROOT / relative)}
        after = {row.address: row for row in read_text(current, ROOT / relative)}
        assert after.keys() == before.keys(), relative
        for address, old_row in before.items():
            row = after[address]
            for field in fields(old_row):
                if field.name not in {"line", "region", "origin"}:
                    assert getattr(row, field.name) == getattr(old_row, field.name), (
                        relative, hex(address), field.name)
            assert _history(row) == _history(old_row), (relative, hex(address), "history")


def test_expanded_game_collateral_journal_identity_history_and_bytes():
    store = TaskStore(ROOT)
    task = store.read(TASK)
    assert task.base_revision == BASE
    decisions = store.decisions(TASK)
    expanded = [subject for decision in decisions if decision["kind"] == "scope_expanded"
                for subject in decision["subjects"]]
    expected = {0x00455610, 0x00524870, 0x005A9EB0, 0x0051F2E0}
    assert {subject["address"] for subject in expanded} == expected
    assert all(subject["image_sha256"] == PIN and subject["address_space"] == "pe:va"
               for subject in expanded)

    relative = "src/game.cpp"
    baseline = subprocess.check_output(
        ["git", "show", f"{BASE}:{relative}"], cwd=ROOT, timeout=30)
    current = (ROOT / relative).read_bytes()
    assert current == baseline, "verification-only src/game.cpp must remain byte-exact"

    before = {row.address: row for row in read_text(baseline.decode("utf-8"), ROOT / relative)}
    after = {row.address: row for row in read_text(current.decode("utf-8"), ROOT / relative)}
    assert expected <= before.keys() and expected <= after.keys()
    for address in expected:
        assert after[address] == before[address], (hex(address), "expanded source mapping/history")
        assert _history(after[address]) == _history(before[address]), (
            hex(address), "expanded source history")


def test_actual_session_storage_shape_and_only_first_tail_byte_is_cleared():
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    source = (ROOT / "src/netwin.cpp").read_text(encoding="utf-8")
    assert "uint8_t reserved_session_prefix_[0x20];" in header
    assert "char scenario_text_wire_tail_[0x144];" in header
    assert 'static_assert(sizeof(NetWinSessionBlock) == 0x168, "session block extent");' in header
    assert 'static_assert(offsetof(NetWinSessionBlock, scenario_text_wire_tail_) == 0x24,' in header
    assert "NetWinScenarioText" not in source
    clear = "NetWinSessionData.scenario_text_wire_tail_[0] = 0;"
    assert source.count(clear) == 4
    assert "state_[0x20]" not in source
    assert "strcpy(NetWinSessionData.scenario_text_wire_tail_, scenario_name);" in source
    # The argument pointer and condition are reevaluated at their original sites.
    draw = _extract(source, "void NetWin::draw_session_info()")
    assert draw.count("NetWinSessionData.scenario_text_wire_tail_[0]") == 5
    assert draw.count("strcat(StringTemp,\n               NetWinSessionData.scenario_text_wire_tail_);") == 1
    assert "char *scenario_text" not in draw
    pick = _extract(source, "void NetWin::pick_map()")
    assert pick.count(clear) == 2
    assert pick.index(clear) < pick.index("strcpy(NetWinSessionData.scenario_text_wire_tail_, scenario_name);")
    config = _extract(source, "void NetWin::config_players()")
    assert config.count(clear) == 1


RUNTIME = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#define __cdecl
/* ACTUAL_ROW_AND_SESSION_BLOCK */
static_assert(sizeof(NetWinSessionBlock) == 0x168, "full wire block");
static_assert(offsetof(NetWinSessionBlock, mode_) == 0, "mode prefix");
static_assert(offsetof(NetWinSessionBlock, reserved_session_prefix_) == 4, "opaque prefix");
static_assert(offsetof(NetWinSessionBlock, scenario_text_wire_tail_) == 0x24, "tail offset");
static NetWinSessionBlock NetWinSessionData;
/* ACTUAL_RESET */
/* ACTUAL_RECEIVER */
int main() {
  unsigned char *bytes = reinterpret_cast<unsigned char *>(&NetWinSessionData);
  for (unsigned i = 0; i < sizeof(NetWinSessionData); ++i) bytes[i] = (i * 37u + 0xA5u) & 0xFFu;
  NetWinSessionData.mode_ = 0x78563412u;
  unsigned char expected[0x168];
  std::memcpy(expected, bytes, sizeof(expected));
  expected[0] = expected[1] = expected[2] = expected[3] = 0;
  expected[0x24] = 0;
  sub_4e2450();
  if (std::memcmp(bytes, expected, sizeof(expected)) != 0) {
    std::fprintf(stderr, "CONTROL FAILURE: reset clears mode and only scenario byte zero\n");
    return 98;
  }
  unsigned char wire[0x168];
  for (unsigned i = 0; i < sizeof(wire); ++i) wire[i] = (i * 19u + 0x63u) & 0xFFu;
  std::memset(bytes, 0xC7, sizeof(NetWinSessionData));
  NetWinSessionData.receive_wire_block(wire);
  if (std::memcmp(bytes, wire, sizeof(wire)) != 0) {
    std::fprintf(stderr, "CONTROL FAILURE: receiver copies all 360 session bytes\n");
    return 98;
  }
  std::puts("scenario storage first-byte control passed");
  return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_literal_reset_body_preserves_all_adjacent_wire_bytes_and_mutant_is_caught(
    compiler_name, tmp_path
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    source = (ROOT / "src/netwin.cpp").read_text(encoding="utf-8")
    row = _extract(header, "struct NetWinLobbyRow {") + ";"
    block = _extract(header, "struct NetWinSessionBlock {") + ";"
    reset = _extract(source, 'extern "C" void __cdecl sub_4e2450()')
    receiver = _extract(source, "void NetWinSessionBlock::receive_wire_block(")
    harness = (RUNTIME.replace("/* ACTUAL_ROW_AND_SESSION_BLOCK */", row + "\n" + block)
               .replace("/* ACTUAL_RESET */", reset)
               .replace("/* ACTUAL_RECEIVER */", receiver))
    clear = "NetWinSessionData.scenario_text_wire_tail_[0] = 0;"
    changed = reset.replace(clear, "NetWinSessionData.scenario_text_wire_tail_[1] = 0;")
    assert changed != reset
    damaged = harness.replace(reset, changed, 1)
    short_receiver = receiver.replace("sizeof(*this)", "sizeof(*this) - 1")
    assert short_receiver != receiver
    short_copy = harness.replace(receiver, short_receiver, 1)
    for name, program, expected_code, expected_text in (
        ("actual", harness, 0, "scenario storage first-byte control passed"),
        ("wrong-tail-byte", damaged, 98, "CONTROL FAILURE: reset clears mode and only scenario byte zero"),
        ("short-receiver-copy", short_copy, 98, "CONTROL FAILURE: receiver copies all 360 session bytes"),
    ):
        cpp = tmp_path / f"scenario-{compiler_name}-{name}.cpp"
        binary = tmp_path / f"scenario-{compiler_name}-{name}"
        cpp.write_text(program, encoding="utf-8")
        build = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=40)
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert run.returncode == expected_code, run.stdout + run.stderr
        assert expected_text in (run.stdout + run.stderr)
