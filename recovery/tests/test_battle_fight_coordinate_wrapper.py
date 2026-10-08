"""Actual-source controls for the five-argument battle_fight wrapper.

The tests extract the five-argument definition and complete real table
definitions from their owning product TUs. They exercise wrapper selection,
single-wrap behavior, and argument forwarding; they are not native ABI or full
battle_fight runtime proof.
"""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
VEH = ROOT / "src" / "veh.cpp"
MAP = ROOT / "src" / "map.cpp"


def extract_function(source: str) -> str:
    candidates = list(re.finditer(
        r"\bint\s+__cdecl\s+battle_fight\s*\(([^)]*)\)\s*\{", source, re.S
    ))
    matches = [m for m in candidates if m.group(1).count(",") == 4]
    assert len(matches) == 1, "expected exactly one five-argument definition"
    match = matches[0]
    opening = source.find("{", match.start(), match.end())
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        ch = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if ch == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if ch == '"': state = "string"
            elif ch == "'": state = "char"
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():index + 1]
        elif state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state == "string":
            if ch == "\\":
                index += 2
                continue
            if ch == '"': state = "code"
        elif state == "char":
            if ch == "\\":
                index += 2
                continue
            if ch == "'": state = "code"
        index += 1
    raise AssertionError("unterminated five-argument definition")


def extract_array(source: str, name: str, extent: int) -> str:
    declaration = rf"(?m)^const int {name}\[{extent}\]\s*="
    match = re.search(declaration, source)
    if name.startswith("VehMoveDelta"):
        match = re.search(rf"(?m)^int {name}\[{extent}\]\s*=", source)
    assert match, f"missing actual owner definition {name}[{extent}]"
    opening = source.index("{", match.start())
    depth = 0
    closing = -1
    for index in range(opening, len(source)):
        if source[index] == "{": depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                closing = index
                break
    assert closing >= 0 and source[closing + 1] == ";"
    definition = source[match.start():closing + 2]
    values = [int(v) for v in re.findall(r"-?\d+", source[opening + 1:closing])]
    assert len(values) == extent, f"unexpected initializer length for {name}"
    return definition


def product_fixture(body: str | None = None) -> str:
    veh_source = VEH.read_text()
    map_source = MAP.read_text()
    if body is None:
        body = extract_function(veh_source)
    arrays = "\n".join((
        extract_array(map_source, "RadiusOffsetX", 289),
        extract_array(map_source, "RadiusOffsetY", 289),
        extract_array(veh_source, "VehMoveDeltaX", 9),
        extract_array(veh_source, "VehMoveDeltaY", 9),
    ))
    assert "VehMoveDeltaXAlt" not in veh_source
    assert "VehMoveDeltaYAlt" not in veh_source
    prefix = r'''
#include <cassert>
#include <cstdio>
#define __cdecl
struct Veh { int x; int y; };
Veh Vehs[64];
int MapIsFlat=1, MapLongitudeBounds=16;
extern const int RadiusOffsetX[289], RadiusOffsetY[289];
extern int VehMoveDeltaX[9], VehMoveDeltaY[9];
int seen_vehicle, seen_direction, seen_x, seen_y, seen_mode, seen_allow;
int *seen_odds;
int __cdecl battle_fight(int veh_id, int dir, int x, int y, int mode,
                         int allow_interceptor, int *odds_out) {
  seen_vehicle=veh_id; seen_direction=dir; seen_x=x; seen_y=y;
  seen_mode=mode; seen_allow=allow_interceptor; seen_odds=odds_out;
  return 0x5a17;
}
'''
    owner_anchor = "\nint fixture_owner_table_anchor() { return RadiusOffsetX[0] + RadiusOffsetY[0] + VehMoveDeltaX[8] + VehMoveDeltaY[8]; }\n"
    return prefix + arrays + owner_anchor + "\n" + body + "\n"


def compile_and_run(body: str | None, main: str, compiler: str) -> int:
    with tempfile.TemporaryDirectory(prefix="battle-fight-radius-owner-") as td:
        cpp = Path(td) / "actual_wrapper.cpp"
        exe = Path(td) / "actual_wrapper"
        main = main.replace("int main() {", "int main() {\n  assert(fixture_owner_table_anchor()==0);")
        cpp.write_text(product_fixture(body) + main)
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=undefined", "-fno-sanitize-recover=all",
             str(cpp), "-o", str(exe)], capture_output=True, text=True,
            timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], check=False, timeout=10).returncode


def compilers() -> list[str]:
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("neither GCC nor Clang is installed")
    return found


MAIN = r'''
static void check(int dir, int mode, int base_x, int base_y, int flat,
                  int expected_x, int expected_y, int allow, int *odds) {
  Vehs[0x13].x=base_x; Vehs[0x13].y=base_y;
  MapIsFlat=flat; MapLongitudeBounds=16;
  const int result=battle_fight(0x13,dir,mode,allow,odds);
  assert(result==0x5a17);
  assert(seen_vehicle==0x13 && seen_direction==dir);
  if (seen_x!=expected_x || seen_y!=expected_y)
    std::fprintf(stderr,"dir=%d mode=%d flat=%d got=(%d,%d) expected=(%d,%d)\\n",
                 dir,mode,flat,seen_x,seen_y,expected_x,expected_y);
  assert(seen_x==expected_x && seen_y==expected_y);
  assert(seen_mode==mode && seen_allow==allow && seen_odds==odds);
}
int main() {
  int odds=77;
  // Ordinary nine-entry movement table, including the stationary center.
  const int dx[9]={1,2,1,0,-1,-2,-1,0,0};
  const int dy[9]={-1,0,1,2,1,0,-1,-2,0};
  for (int i=0;i<9;++i) check(i,0,10,20,1,10+dx[i],20+dy[i],1,&odds);
  check(1,1,10,20,1,11,19,0,&odds); // pinned action_destroy mode-one domain
  check(9,1,10,20,1,12,18,0,&odds);
  check(24,2,10,20,1,10,16,1,0);
  check(80,7,10,20,1,9,13,1,&odds);
  check(288,1,10,20,1,9,5,1,&odds);
  // One wrap only, gated by MapIsFlat bit zero (not integer truthiness).
  check(9,1,15,1,0,1,-1,1,&odds); // x wraps once; y is not wrapped
  check(9,1,15,1,2,1,-1,1,&odds);
  check(9,1,15,1,3,17,-1,1,&odds); // bit zero set: no wrap
  check(9,1,40,1,0,26,-1,1,&odds); // x remains out of range after one adjustment
  check(0,0,-40,20,0,-23,19,1,&odds); // still negative after one adjustment
  return 0;
}
'''


@pytest.mark.parametrize("compiler", compilers())
def test_actual_five_argument_wrapper_uses_full_owner_tables_and_forwards(compiler):
    assert compile_and_run(None, MAIN, compiler) == 0


def mutants(body: str):
    changes = (
        ("invert mode selection", "if (mode != 0)", "if (mode == 0)"),
        ("clamp to eight-entry range", "RadiusOffsetX[dir]",
         "RadiusOffsetX[(dir < 8 ? dir : 8)]"),
        ("read Y offsets for X", "RadiusOffsetX[dir]", "RadiusOffsetY[dir]"),
        ("use step table in nonzero mode", "RadiusOffsetX[dir]", "VehMoveDeltaX[dir]"),
    )
    for label, old, new in changes:
        assert old in body, f"mutation anchor missing: {label}"
        yield label, body.replace(old, new, 1)


MUTANT_CASES = {
    "invert mode selection": r'''
int main() {
  Vehs[0].x=10; Vehs[0].y=20; MapIsFlat=1;
  int odds=9;
  (void)battle_fight(0,1,0,1,&odds);
  assert(seen_x==12 && seen_y==20);
}
''',
    "clamp to eight-entry range": r'''
int main() {
  Vehs[0].x=10; Vehs[0].y=20; MapIsFlat=1;
  int odds=9;
  (void)battle_fight(0,9,1,1,&odds);
  assert(seen_x==12 && seen_y==18);
}
''',
    "read Y offsets for X": r'''
int main() {
  Vehs[0].x=10; Vehs[0].y=20; MapIsFlat=1;
  int odds=9;
  (void)battle_fight(0,24,1,1,&odds);
  assert(seen_x==10 && seen_y==16);
}
''',
    "use step table in nonzero mode": r'''
int main() {
  Vehs[0].x=10; Vehs[0].y=20; MapIsFlat=1;
  int odds=9;
  (void)battle_fight(0,0,1,1,&odds);
  assert(seen_x==10 && seen_y==20);
}
''',
}


@pytest.mark.parametrize("compiler", compilers())
@pytest.mark.parametrize("mutation", tuple(MUTANT_CASES))
def test_safe_domain_mutants_are_rejected(mutation, compiler):
    actual = extract_function(VEH.read_text())
    label, damaged = next(row for row in mutants(actual) if row[0] == mutation)
    assert label == mutation
    assert compile_and_run(damaged, MUTANT_CASES[mutation], compiler) != 0


def test_extraction_is_five_argument_overload_and_owner_storage_is_unique():
    source = VEH.read_text()
    body = extract_function(source)
    signature = re.search(r"battle_fight\s*\(([^)]*)\)", body)
    assert signature and signature.group(1).count(",") == 4
    assert "RadiusOffsetX[dir]" in body and "RadiusOffsetY[dir]" in body
    assert "VehMoveDeltaXAlt" not in body and "VehMoveDeltaYAlt" not in body
    assert len(re.findall(r"(?m)^const int RadiusOffsetX\[289\]\s*=", MAP.read_text())) == 1
    assert len(re.findall(r"(?m)^const int RadiusOffsetY\[289\]\s*=", MAP.read_text())) == 1
    assert len(re.findall(r"(?m)^int VehMoveDeltaXAlt\[", source)) == 0
    assert len(re.findall(r"(?m)^int VehMoveDeltaYAlt\[", source)) == 0
