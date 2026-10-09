"""Actual-source controls for selected-chassis ability cost suffixes."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def positive_branch() -> str:
    source = (ROOT / "src/designwin.cpp").read_text(encoding="utf-8")
    masked = re.sub(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'",
                    lambda m: "\n" * m[0].count("\n") + " " * (len(m[0]) - m[0].count("\n")),
                    source, flags=re.S)
    start = masked.index("void DesignWin::select_special(int which)")
    end = masked.index("\n}", start)
    body = source[start:end]
    match = re.search(r"if \(kind >= 0\) \{", body)
    assert match
    cursor, depth = match.end(), 1
    while depth:
        if body[cursor] == "{":
            depth += 1
        elif body[cursor] == "}":
            depth -= 1
        cursor += 1
    branch = body[match.end():cursor - 1]
    return re.sub(
        r"StringTable\.get\(\s*reinterpret_cast<int>\(Triad\[triad\]\)\s*\)",
        "StringTable.get(Triad[triad])", branch,
    )


def run_control(compiler: str, tmp_path: Path, branch: str) -> subprocess.CompletedProcess:
    program = f'''#include <cassert>
#include <cstdio>
#include <cstring>
struct AbilityEntry {{ int cost_factor; unsigned flags; }} Ability[1];
struct ChassisEntry {{ unsigned char triad; }};
static int chassis_reads, chassis_read_indices[4];
struct ChassisArray {{ ChassisEntry entries[8];
  ChassisEntry &operator[](int index) {{ chassis_read_indices[chassis_reads++] = index; return entries[index]; }}
}} Chassis;
int Triad[3] = {{10, 11, 12}};
char StringTemp[128];
const char *BufferJoinSpace = " ";
struct Table {{ const char *get(int id) {{ return id == 10 ? "land" : id == 11 ? "sea" : "air"; }} }} StringTable;
static char *number(int n, char *out, int) {{ std::sprintf(out, "%d", n); return out; }}
#define _itoa number
static void render(int kind, unsigned flags, int selected, int faction,
                   unsigned char selected_triad, unsigned char faction_triad) {{
  const int ability = 0, protoChassisType_ = selected, factionID_ = faction;
  (void)protoChassisType_;
  (void)factionID_;
  Ability[0] = {{kind, flags}};
  Chassis.entries[selected].triad = selected_triad;
  Chassis.entries[faction].triad = faction_triad;
  chassis_reads = 0;
  StringTemp[0] = 0;
{branch}
}}
int main() {{
  render(3, 0, 3, 1, 0, 2);
  assert(std::strcmp(StringTemp, "3") == 0 && chassis_reads == 0);
  render(3, 0x400, 3, 1, 0, 2);
  assert(std::strcmp(StringTemp, "4 (land)") == 0 && chassis_reads == 2);
  assert(chassis_read_indices[0] == 3 && chassis_read_indices[1] == 3);
  render(3, 0x400, 3, 1, 1, 0);
  assert(std::strcmp(StringTemp, "3 (sea)") == 0 && chassis_reads == 2);
  assert(chassis_read_indices[0] == 3 && chassis_read_indices[1] == 3);
  render(3, 0x400, 3, 1, 2, 1);
  assert(std::strcmp(StringTemp, "3 (air)") == 0 && chassis_reads == 2);
  assert(chassis_read_indices[0] == 3 && chassis_read_indices[1] == 3);
}}
'''
    cpp = tmp_path / "designwin_special_chassis.cpp"
    exe = tmp_path / "designwin_special_chassis"
    cpp.write_text(program, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, timeout=60)
    return subprocess.run([str(exe)], timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_selected_chassis_controls_cost_and_localized_suffix(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    result = run_control(compiler, tmp_path, positive_branch())
    assert result.returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_damage_mutants_are_caught_for_faction_index_and_or_predicate(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = positive_branch()
    faction_mutant = source.replace(
        "Chassis[protoChassisType_]", "Chassis[factionID_]"
    )
    assert faction_mutant != source
    assert run_control(compiler, tmp_path, faction_mutant).returncode != 0

    predicate = "special_cost && Chassis[protoChassisType_].triad == 0"
    or_mutant = source.replace(
        predicate,
        "special_cost || Chassis[protoChassisType_].triad != 0",
    )
    assert or_mutant != source
    assert run_control(compiler, tmp_path, or_mutant).returncode != 0
