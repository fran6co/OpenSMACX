"""Actual-source host controls for the six admitted leaf-owner migrations.

This file is a tracked-test candidate. It reads only the repository's active
src/ files and the pinned historical Git snapshot; no staged-source fallback,
packet manifest or emitter import is permitted. The host fixture is behavioral
coverage only, not evidence for the VC6 ABI or native storage layout.
"""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import SourceRole, read_text
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
BASELINE = "2c29aa969"
SEED = "src/leaf_recoveries.cpp"
OWNERS = {
    0x00559210: ("src/faction.cpp", r"int\s+communicate_round_down_multiple\s*\(\s*int\s+value\s*,\s*int\s+step\s*\)\s*\{"),
    0x005AD450: ("src/replaywin.cpp", r"void\s+ReplayWin::reset_base_sites\s*\(\s*\)\s*\{"),
    # The native lexical mask blanks the string literal "C" in linkage specs.
    0x004E2450: ("src/netwin.cpp", r"extern\s+void\s+__cdecl\s+sub_4e2450\s*\(\s*\)\s*\{"),
    0x004E7DB0: ("src/base.cpp", r"extern\s+int\s+__cdecl\s+sub_4e7db0\s*\(\s*\)\s*\{"),
    0x006051C0: ("src/scroll.cpp", r"extern\s+void\s+__cdecl\s+sub_6051c0\s*\(\s*\)\s*\{"),
    0x0059D230: ("src/basepop.cpp", r"int\s+BasePop::add_asterisk_item\s*\(\s*\)\s*\{"),
}
DURABLE = (
    "address", "name", "image_spans", "mode", "state", "body", "kind",
    "exclusion", "recipe", "byte_exact", "semantic", "levers", "ruled_out",
    "unrecoverable", "deferred",
)


def _masked(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), text)


def _definition(text: str, signature: str) -> tuple[str, int, int]:
    masked = _masked(text)
    matches = list(re.finditer(signature, masked, re.MULTILINE))
    assert len(matches) == 1, f"missing/ambiguous active definition: {signature}"
    start = matches[0].start()
    opening = masked.find("{", matches[0].start(), matches[0].end())
    assert opening >= 0
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1], start, pos + 1
    raise AssertionError(f"unterminated active definition: {signature}")


def _active() -> tuple[dict[int, str], dict[int, object], str]:
    methods, rows = {}, {}
    for address, (relative, signature) in OWNERS.items():
        path = ROOT / relative
        source = path.read_text(encoding="utf-8")
        mapped = [row for row in read_text(source, path.resolve())
                  if row.address == address]
        assert len(mapped) == 1, f"identity 0x{address:08X} missing/duplicate in {relative}"
        row = mapped[0]
        assert row.mode.value in ("body", "file") and row.region and not row.extract_error
        methods[address], _start, _end = _definition(source, signature)
        rows[address] = row
    basepop = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    # Find its active declaration prefix with the native lexer, then validate
    # the exact literal initializer in the corresponding original source slice.
    arrays = list(re.finditer(
        r"char\s+BasePopItemMarkerText\s*\[\s*2\s*\]\s*=",
        _masked(basepop), re.MULTILINE))
    assert len(arrays) == 1, "actual writable two-byte marker declaration missing/ambiguous"
    start = arrays[0].start()
    init = re.match(
        r"char\s+BasePopItemMarkerText\s*\[\s*2\s*\]\s*=\s*"
        r"\{\s*'\*'\s*,\s*'\\0'\s*\}\s*;",
        basepop[start:])
    assert init is not None, "source marker array initializer changed"
    declaration = basepop[start:start + init.end()]
    return methods, rows, declaration


def _lessons(row: object) -> tuple[tuple[object, ...], ...] | None:
    if row.origin is None:
        return None
    return tuple((lesson.kind, lesson.key, lesson.prose) for lesson in row.origin)


def _seed() -> dict[int, object]:
    source = subprocess.run(["git", "show", f"{BASELINE}:{SEED}"], cwd=ROOT,
                            check=True, capture_output=True, text=True, timeout=20).stdout
    rows = {row.address: row for row in read_text(source, ROOT / SEED)}
    assert set(OWNERS) <= rows.keys(), "pinned full-history snapshot lacks selected seed identities"
    return {address: rows[address] for address in OWNERS}


def test_current_owner_mappings_preserve_full_durable_history() -> None:
    _methods, current, _declaration = _active()
    historical = _seed()
    for address, row in current.items():
        old = historical[address]
        for field in DURABLE:
            assert getattr(row, field) == getattr(old, field), (
                f"0x{address:08X}: durable {field} changed")
        assert _lessons(row) == _lessons(old), (
            f"0x{address:08X}: full historical lesson prose changed")
        assert row.source_role is SourceRole.UNCLASSIFIED, (
            f"0x{address:08X}: no genuine-owner classification")
    assert_leaf_collection_retired(ROOT)
    for address, (relative, _signature) in OWNERS.items():
        assert_current_owner(ROOT, address, relative)
    assert not (ROOT / "src/leaf_recoveries.h").exists()


HOST = r'''#include <cassert>
#include <cstdint>
#define __cdecl
#define __fastcall
struct ReplayBaseSite { int x, y, faction_id; };
struct ReplayWin { ReplayBaseSite base_sites_[512]; void reset_base_sites(); };
/* NETWIN_ROW_AND_SESSION_BLOCK_INSERTION */
NetWinSessionBlock NetWinSessionData;
int TileYieldRestricted;
int ScrollDefaultThickness;
int metric_seen = -1;
int GetSystemMetrics(int metric) { metric_seen = metric; return 13; }
struct Dialogs { char *text = 0; int index = 0; int item(char *s, int i) { text=s; index=i; return 37; } };
struct BasePop { Dialogs dialogs_; int add_asterisk_item(); };
'''


def _program(methods: dict[int, str], declaration: str,
             damage: int | None = None) -> str:
    parts = dict(methods)
    if damage == 0x005AD450:
        old = "index < 512"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(old, "index < 511", 1)
    elif damage == 0x00559210:
        old = "return value / step * step;"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(old, "return value / step;", 1)
    elif damage == 0x004E2450:
        old = "NetWinSessionData.scenario_text_wire_tail_[0] = 0;"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(
            old, "NetWinSessionData.scenario_text_wire_tail_[1] = 0;", 1)
    elif damage == 0x004E7DB0:
        old = "TileYieldRestricted = 0;"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(old, "TileYieldRestricted = 1;", 1)
    elif damage == 0x006051C0:
        old = "GetSystemMetrics(3)"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(old, "GetSystemMetrics(2)", 1)
    elif damage == 0x0059D230:
        old = "dialogs_.Dialogs::item(BasePopItemMarkerText, -2)"
        assert parts[damage].count(old) == 1
        parts[damage] = parts[damage].replace(
            old, "dialogs_.Dialogs::item(BasePopItemMarkerText, 2)", 1)
    elif damage is not None:
        raise AssertionError(f"unknown behavior control {damage:#x}")
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    row_decl = re.search(r"struct NetWinLobbyRow \{.*?\n\};", header, re.S)
    session_decl = re.search(r"struct NetWinSessionBlock \{.*?\n\};", header, re.S)
    assert row_decl is not None and session_decl is not None
    actual_declarations = row_decl.group() + "\n" + session_decl.group()
    host = HOST.replace("/* NETWIN_ROW_AND_SESSION_BLOCK_INSERTION */", actual_declarations)
    return host + declaration + "\n" + "\n".join(parts[a] for a in OWNERS) + r'''
int main() {
  ReplayWin replay;
  for (int i=0;i<512;++i) replay.base_sites_[i]={i,i+1000,i+2000};
  replay.reset_base_sites();
  for (int i=0;i<512;++i) assert(replay.base_sites_[i].x==-1 && replay.base_sites_[i].y==-1 && replay.base_sites_[i].faction_id==-1);
  assert(communicate_round_down_multiple(127,25)==125);
  assert(communicate_round_down_multiple(-7,3)==-6);
  assert(communicate_round_down_multiple(-9,3)==-9);
  NetWinSessionData.mode_=9; NetWinSessionData.scenario_text_wire_tail_[0]=8; NetWinSessionData.scenario_text_wire_tail_[1]=7;
  sub_4e2450(); assert(NetWinSessionData.mode_==0 && NetWinSessionData.scenario_text_wire_tail_[0]==0 && NetWinSessionData.scenario_text_wire_tail_[1]==7);
  TileYieldRestricted=4; assert(sub_4e7db0()==0 && TileYieldRestricted==0);
  sub_6051c0(); assert(metric_seen==3 && ScrollDefaultThickness==13);
  BasePop popup; assert(popup.add_asterisk_item()==37 && popup.dialogs_.index==-2 && popup.dialogs_.text[0]=='*' && popup.dialogs_.text[1]=='\0');
  BasePopItemMarkerText[0]='?'; assert(popup.add_asterisk_item()==37 && popup.dialogs_.text[0]=='?');
  return 0;
}
'''


def _compile_run(program: str, tmp_path: Path) -> subprocess.CompletedProcess:
    tmp_path.mkdir(parents=True, exist_ok=True)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required for actual-source controls")
    source, binary = tmp_path / "actual_owner_controls.cpp", tmp_path / "actual_owner_controls"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_actual_source_behaviors_and_six_mutation_controls(tmp_path: Path) -> None:
    methods, _rows, declaration = _active()
    program = _program(methods, declaration)
    actual = _compile_run(program, tmp_path / "actual")
    assert actual.returncode == 0, actual.stdout + actual.stderr
    for index, address in enumerate(OWNERS):
        program = _program(methods, declaration, address)
        changed = _compile_run(program, tmp_path / f"mutant-{index}")
        assert changed.returncode != 0, f"mutation for 0x{address:08X} escaped its behavioral control"
