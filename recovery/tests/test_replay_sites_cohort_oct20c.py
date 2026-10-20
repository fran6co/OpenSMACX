"""Actual-source host controls for ReplayWin's fixed base-site table."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src" / "replaywin.cpp"
METHODS = {
    0x005AD470: r"void\s+ReplayWin::add_base_site\s*\(\s*int\s+x\s*,\s*int\s+y\s*,\s*int\s+faction_id\s*\)\s*\{",
    0x005AD4C0: r"int\s+ReplayWin::update_base_site\s*\(\s*int\s+x\s*,\s*int\s+y\s*,\s*int\s+faction_id\s*\)\s*\{",
    0x005AD500: r"void\s+ReplayWin::remove_base_site\s*\(\s*int\s+x\s*,\s*int\s+y\s*,\s*int\s*\)\s*\{",
}

HOST = r"""
#include <cassert>
struct ReplayBaseSite { int x, y, faction_id; };
struct ReplayWin {
    ReplayBaseSite base_sites_[512];
    void add_base_site(int x, int y, int faction_id);
    int update_base_site(int x, int y, int faction_id);
    void remove_base_site(int x, int y, int unused);
};
METHODS
int main() {
    ReplayWin win;
    for (int i = 0; i < 512; ++i) {
        win.base_sites_[i].x = i;
        win.base_sites_[i].y = 1000 + i;
        win.base_sites_[i].faction_id = 2000 + i;
    }

    // A hole only at the final record verifies the inclusive 512th slot.
    win.base_sites_[511].x = -1;
    win.add_base_site(71, 72, 73);
    assert(win.base_sites_[511].x == 71 && win.base_sites_[511].y == 72 &&
           win.base_sites_[511].faction_id == 73);

    // A full table has no insertion slot and must remain unchanged.
    win.add_base_site(81, 82, 83);
    assert(win.base_sites_[510].x == 510 && win.base_sites_[511].x == 71);

    // Duplicate keys update only the first match and preserve the legacy 3x
    // index return; a miss retains the sentinel 512.
    win.base_sites_[7].x = 42; win.base_sites_[7].y = 43;
    win.base_sites_[19].x = 42; win.base_sites_[19].y = 43;
    assert(win.update_base_site(42, 43, 99) == 21);
    assert(win.base_sites_[7].faction_id == 99);
    assert(win.base_sites_[19].faction_id == 2019);
    assert(win.update_base_site(-8, -9, 1) == 512);

    // Removal also stops at the first duplicate and clears all three fields
    // in the original order's final state, leaving the later record intact.
    win.remove_base_site(42, 43, 0);
    assert(win.base_sites_[7].x == -1 && win.base_sites_[7].y == -1 &&
           win.base_sites_[7].faction_id == -1);
    assert(win.base_sites_[19].x == 42 && win.base_sites_[19].y == 43 &&
           win.base_sites_[19].faction_id == 2019);
    return 0;
}
"""


def _definition(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    matches = list(re.finditer(signature, masked, re.MULTILINE))
    assert len(matches) == 1, f"missing/ambiguous source method: {signature}"
    start = matches[0].start()
    opening = masked.find("{", start, matches[0].end())
    depth = 0
    for position in range(opening, len(masked)):
        if masked[position] == "{":
            depth += 1
        elif masked[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated active source method")


def _methods() -> dict[int, str]:
    source = SOURCE.read_text(encoding="utf-8")
    rows = {row.address: row for row in read_text(source, SOURCE.resolve())}
    assert set(METHODS) <= rows.keys()
    result = {}
    for address, signature in METHODS.items():
        row = rows[address]
        assert row.path.resolve() == SOURCE.resolve()
        assert row.region and not row.extract_error
        result[address] = _definition(source, signature)
    return result


def _program(methods: dict[int, str], damage: str | None = None) -> str:
    rendered = dict(methods)
    if damage == "skip_last_site":
        address = 0x005AD470
        before, after = "index < 512", "index < 511"
        assert rendered[address].count(before) == 1
        rendered[address] = rendered[address].replace(before, after, 1)
    elif damage == "skip_first_duplicate":
        address = 0x005AD4C0
        before, after = "return index * 3;", "index += 1;"
        assert rendered[address].count(before) == 1
        rendered[address] = rendered[address].replace(before, after, 1)
    elif damage is not None:
        raise AssertionError(f"unknown damage control: {damage}")
    return HOST.replace("METHODS", "\n".join(rendered[address] for address in METHODS))


def _compile_run(program: str, compiler: str, tmp_path: Path) -> int:
    unit = tmp_path / "replay-sites.cpp"
    executable = tmp_path / "replay-sites"
    unit.write_text(program, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         str(unit), "-o", str(executable)],
        check=True, cwd=ROOT, capture_output=True, timeout=30,
    )
    return subprocess.run([str(executable)], cwd=ROOT, timeout=10).returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_replay_sites_actual_source_and_damage_controls(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    methods = _methods()
    assert _compile_run(_program(methods), compiler, tmp_path) == 0
    assert _compile_run(_program(methods, "skip_last_site"), compiler, tmp_path) != 0
    assert _compile_run(_program(methods, "skip_first_duplicate"), compiler, tmp_path) != 0
