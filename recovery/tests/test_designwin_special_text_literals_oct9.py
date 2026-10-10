"""Scoped source/host controls for DesignWin's recovered special text."""
from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/designwin.cpp").read_text(encoding="utf-8")


def body(signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), SOURCE
    )
    start = masked.find(signature)
    assert start >= 0 and masked.find(signature, start + len(signature)) < 0
    opening = masked.find("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return SOURCE[start:index + 1]
    raise AssertionError(f"unterminated {signature}")


def test_special_text_prefix_suffix_and_labels_are_source_owned():
    special = body("void DesignWin::select_special(int which)")
    assert special.index("abil_cond(") < special.index("StringTemp[0] = 0;", special.index("abil_cond("))
    prefix = [
        "Ability[ability].name", "strcat(StringTemp, \": \")",
        "Ability[ability].description", "strcat(StringTemp, \"|\")",
        "Labels.strings_ptr)[0x58]", "strcat(StringTemp, \": \")",
    ]
    offsets = []
    cursor = special.index("StringTemp[0] = 0")
    for token in prefix:
        cursor = special.index(token, cursor)
        offsets.append(cursor)
        cursor += len(token)
    assert offsets == sorted(offsets)
    assert "const int kind = static_cast<int>(Ability[ability].cost_factor)" in special
    assert special.count("special_cost && Chassis[protoChassisType_].triad == 0") == 1
    assert "if (special_cost) {" in special
    assert "_itoa(cost, digits, 10)" in special
    assert "Triad[triad]" in special
    for old in ("0x1F0", "0x154", "0xB14"):
        assert old not in special
    for slot in ("0x7C", "0x55", "0x2C5"):
        assert f"labels[{slot}]" in special
    assert "StringTemp[0] = 0;" in special


def test_fixed_address_popup_and_drawing_strings_are_inline_literals():
    design = body("int DesignWin::design_it()")
    stack = body("void DesignWin::draw_stack()")
    flash = body("void DesignWin::draw_flash()")
    special = body("void DesignWin::select_special(int which)")
    all_bodies = design + stack + flash + special
    for obsolete in (
        "EditorNamePopupLabel", "EditorNamePopupAlienLabel",
        "EditorNamePopupSprite", "EditorNamePopupAlienSprite",
        "EditorNameXPopCaption", "EditorNameXPopAlienCaption",
        "EditorNameXPopSprite", "EditorNameXPopAlienSprite",
        "EditorTypePopupLabel", "EditorTypePopupAlienLabel",
        "EditorTypePopupSprite", "EditorTypePopupAlienSprite",
        "EditorOverwritePopupLabel", "EditorOverwritePopupAlienLabel",
        "EditorOverwritePopupSprite", "EditorOverwritePopupAlienSprite",
        "EditorOpenParen", "EditorCloseParen", "EditorDotSpace",
        "EditorColonSpace", "EditorHashSign", "SpecialTwoPopupLabel",
        "SpecialTwoPopupAlienLabel", "SpecialMissilePopupLabel",
        "SpecialMissilePopupAlienLabel", "SpecialUnitsPopupLabel",
        "SpecialTwoPopupSprite", "SpecialTwoPopupAlienSprite",
        "SpecialMissilePopupSprite", "SpecialMissilePopupAlienSprite",
        "SpecialSlashSign", "SpecialMinusOneText", "SpecialPlusSignText",
        "SpecialMinusTwoText",
    ):
        assert obsolete not in all_bodies
    for literal in ('"INTRINSICNAME"', '"INTRINSICTYPE"',
                    '"OVERWRITEPROTO"', '"TWOSPECIAL"',
                    '"MISSILESPECIAL"', '"UNITSPECIAL"',
                    '"alunit_sm.pcx"', '"unit_sm.pcx"', '". "',
                    '"#"', '": "', '", "', '"("', '")"', '"/"',
                    '"-1"', '"+"', '"-2"'):
        assert literal in all_bodies
    assert 'strcat(StringTemp, ", ")' in flash
    assert 'strcat(StringTemp, ": ")' not in flash
    assert 'strcat(StringTemp, ": ")' in special


def test_positive_cost_control_runs_the_actual_source_branch():
    special = body("void DesignWin::select_special(int which)")
    match = re.search(r"if \(kind >= 0\) \{", special)
    assert match
    start = match.end()
    depth = 1
    end = start
    while depth:
        if special[end] == "{":
            depth += 1
        elif special[end] == "}":
            depth -= 1
        end += 1
    branch = special[start:end - 1]
    # Host pointers are represented by integer string handles; this is the only
    # ABI adaptation needed for the source's 32-bit pointer-to-int lookup.
    branch = re.sub(
        r"StringTable\.get\(\s*reinterpret_cast<int>\(Triad\[triad\]\)\s*\)",
        "StringTable.get(Triad[triad])", branch,
    )
    compilers = [shutil.which(name) for name in ("clang++", "g++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "host C++ compiler required for source branch control"
    program = f'''#include <cassert>
#include <cstdio>
#include <cstring>
struct A {{ int cost_factor; unsigned flags; }} Ability[1];
static int chassis_reads;
struct C {{ unsigned char triad; }};
struct ChassisArray {{ C value; C &operator[](int) {{ ++chassis_reads; return value; }} }} Chassis;
int Triad[3] = {{10, 11, 12}};
char StringTemp[128];
const char *BufferJoinSpace = " ";
struct Table {{ const char *get(int id) {{ return id == 10 ? "land" : id == 11 ? "sea" : "air"; }} }} StringTable;
static char *number(int n, char *out, int) {{ std::sprintf(out, "%d", n); return out; }}
#define _itoa number
static void render(int kind, unsigned flags, unsigned char triad_input) {{
  const int ability = 0, protoChassisType_ = 0;
  Ability[0] = {{kind, flags}}; Chassis.value.triad = triad_input;
  chassis_reads = 0;
  StringTemp[0] = 0;
{branch}
}}
int main() {{
  render(3, 0, 0); assert(std::strcmp(StringTemp, "3") == 0);
  assert(chassis_reads == 0);
  render(3, 0x400, 0); assert(std::strcmp(StringTemp, "4 (land)") == 0);
  assert(chassis_reads == 2);
  render(3, 0x400, 1); assert(std::strcmp(StringTemp, "3 (sea)") == 0);
  assert(chassis_reads == 2);
  render(3, 0x400, 2); assert(std::strcmp(StringTemp, "3 (air)") == 0);
  assert(chassis_reads == 2);
}}
'''
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="designwin-special-text-") as temp:
            cpp, exe = Path(temp) / "control.cpp", Path(temp) / "control"
            cpp.write_text(program, encoding="utf-8")
            subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], check=True, timeout=60)
            subprocess.run([str(exe)], check=True, timeout=10)


def test_special_prefix_executes_source_lookup_order_and_detects_damage(tmp_path):
    special = body("void DesignWin::select_special(int which)")
    name_at = special.index("Ability[ability].name")
    start = special.rfind("strcat(StringTemp, StringTable.get(", 0, name_at)
    label_at = special.index("Labels.strings_ptr)[0x58]", name_at)
    end = special.index('strcat(StringTemp, ": ");', label_at) + len(
        'strcat(StringTemp, ": ");'
    )
    prefix = special[start:end]
    prefix = prefix.replace(
        "reinterpret_cast<int>(Ability[ability].name)", "Ability[ability].name"
    ).replace(
        "reinterpret_cast<int>(Ability[ability].description)",
        "Ability[ability].description",
    ).replace(
        "reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x58]",
        "Labels[0x58]",
    )
    assert "reinterpret_cast" not in prefix
    compilers = [shutil.which(name) for name in ("clang++", "g++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "host C++ compiler required for prefix control"

    def run(compiler: str, path: Path, statements: str) -> subprocess.CompletedProcess:
        program = f'''#include <cassert>
#include <cstring>
#include <vector>
struct AbilityRow {{ int name, description; }} Ability[1] = {{{{11, 22}}}};
int Labels[0x5A] = {{}};
char StringTemp[128];
struct Table {{ std::vector<int> ids;
  const char *get(int id) {{ ids.push_back(id); return id == 11 ? "ABILITY" : id == 22 ? "DESCRIPTION" : id == 58 ? "SPECIAL" : "WRONG"; }}
}} StringTable;
static void build() {{ const int ability = 0; (void)ability; StringTemp[0] = 0; {statements} }}
int main() {{ Labels[0x58] = 58; Labels[0x59] = 59; build();
  assert(std::strcmp(StringTemp, "ABILITY: DESCRIPTION|SPECIAL: ") == 0);
  assert((StringTable.ids == std::vector<int>{{11, 22, 58}}));
}}
'''
        cpp, exe = path / "prefix.cpp", path / "prefix"
        cpp.write_text(program, encoding="utf-8")
        subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)], check=True, timeout=60)
        return subprocess.run([str(exe)], timeout=10)

    for compiler in compilers:
        assert run(compiler, tmp_path, prefix).returncode == 0
        assert run(compiler, tmp_path, "").returncode != 0
        misindexed = prefix.replace("Labels[0x58]", "Labels[0x59]")
        assert misindexed != prefix
        assert run(compiler, tmp_path, misindexed).returncode != 0
