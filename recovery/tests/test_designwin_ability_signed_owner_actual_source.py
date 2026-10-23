"""Actual-source controls for DesignWin's signed ability-slot sentinel."""

from __future__ import annotations

import re
import resource
import shutil
import subprocess
from pathlib import Path

import pytest


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/designwin.cpp").is_file():
            return parent
    raise AssertionError("cannot locate isolated worktree")


def _definition(source: str, signature: str) -> str:
    matches = list(re.finditer(rf"(?m)^{re.escape(signature)}\s*\{{", source))
    assert len(matches) == 1, f"expected one actual definition: {signature}"
    start = matches[0].start()
    opening = source.index("{", start)
    depth, state, index = 0, "code", opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
        elif char == "/" and following == "/":
            state = "line"
            index += 1
        elif char == "/" and following == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
        index += 1
    raise AssertionError(f"unclosed definition: {signature}")


def _if_else(source: str, condition: str) -> str:
    start = source.find(condition)
    assert start >= 0 and source.find(condition, start + 1) < 0
    opening = source.index("{", start)

    def close_brace(opening: int) -> int:
        depth, state, index = 0, "code", opening
        while index < len(source):
            char = source[index]
            following = source[index + 1] if index + 1 < len(source) else ""
            if state == "line":
                if char == "\n": state = "code"
            elif state == "block":
                if char == "*" and following == "/": state = "code"; index += 1
            elif state in ("string", "char"):
                if char == "\\": index += 1
                elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                    state = "code"
            elif char == "/" and following == "/": state = "line"; index += 1
            elif char == "/" and following == "*": state = "block"; index += 1
            elif char == '"': state = "string"
            elif char == "'": state = "char"
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0: return index
            index += 1
        raise AssertionError("unclosed source branch")

    then_end = close_brace(opening)
    after_then = then_end + 1
    whitespace = re.match(r"\s*else\s*", source[after_then:])
    assert whitespace, f"expected else after {condition}"
    else_start = after_then + whitespace.end()
    assert source[else_start] == "{"
    else_end = close_brace(else_start)
    return source[start : else_end + 1]


def _actual_parts():
    root = _root()
    cpp = (root / "src/designwin.cpp").read_text(encoding="utf-8")
    header = (root / "src/designwin.h").read_text(encoding="utf-8")
    fields = {}
    for name in ("protoAbilityFlags_", "protoAbilityID1_", "protoAbilityID2_"):
        found = re.findall(rf"(?m)^\s*(?:uint32_t|int)\s+{name};\s*//\s*(0x[0-9A-Fa-f]+)\s*$", header)
        assert len(found) == 1, f"missing/duplicate actual field declaration: {name}"
        line = re.search(rf"(?m)^\s*(?:uint32_t|int)\s+{name};\s*//\s*0x[0-9A-Fa-f]+\s*$", header)
        assert line
        fields[name] = (line.group(0).strip(), found[0])

    vehicle = _definition(cpp, "void DesignWin::draw_vehicle(int redraw)")
    branches = {
        name: _if_else(vehicle, f"if ({name} < 0)")
        for name in ("protoAbilityID1_", "protoAbilityID2_")
    }
    delete = _definition(cpp, "void DesignWin::delete_abil(int a1)")
    assert "BODY GOES HERE." not in delete
    return fields, branches, delete


def _fixture(fields, branches, delete, *, unsigned_mutant=False) -> str:
    field_lines = [fields[name][0] for name in (
        "protoAbilityFlags_", "protoAbilityID1_", "protoAbilityID2_")]
    assert fields["protoAbilityFlags_"][0].startswith("uint32_t ")
    assert fields["protoAbilityID1_"][0].startswith("int ")
    assert fields["protoAbilityID2_"][0].startswith("int ")
    if unsigned_mutant:
        field_lines = [
            line.replace("int protoAbilityID1_", "uint32_t protoAbilityID1_")
                 .replace("int protoAbilityID2_", "uint32_t protoAbilityID2_")
            for line in field_lines
        ]

    return r'''#include <cassert>
#include <cstdint>
#include <cstring>

struct AbilityRecord { int name; };
struct AbilityTable {
  uint32_t reads[8]; int count;
  AbilityRecord record;
  AbilityTable() : count(0), record{7} { memset(reads, 0, sizeof(reads)); }
  AbilityRecord &operator[](uint32_t index) {
    if (count < 8) reads[count] = index;
    ++count;
    return record;
  }
} Ability;
struct TableStrings {
  char *get(int key) { assert(key == 7); return const_cast<char *>("ABILITY"); }
} StringTable;
char StringTemp[128];

struct DesignWin {
''' + "\n".join("  " + line for line in field_lines) + r'''
  void delete_abil(int a1);
  void render_slot_one();
  void render_slot_two();
};

''' + delete + "\nvoid DesignWin::render_slot_one() {\n  char ability_names[128] = {0};\n  StringTemp[0] = 0;\n  strcat(StringTemp, \"{ONE} \");\n" + branches["protoAbilityID1_"] + "\n}\n" + "void DesignWin::render_slot_two() {\n  char ability_names[128] = {0};\n  StringTemp[0] = 0;\n  strcat(StringTemp, \"{TWO} \");\n" + branches["protoAbilityID2_"] + r'''
}

int main(int argc, char **argv) {
  DesignWin win;
  assert(argc == 2);
  if (strcmp(argv[1], "delete") == 0) {
  win.protoAbilityFlags_ = 0x5;
  win.protoAbilityID1_ = -1;
  win.protoAbilityID2_ = -1;
  win.delete_abil(0x2);
  assert(win.protoAbilityFlags_ == 0x5);  // disjoint input is a no-op
  assert(win.protoAbilityID1_ == -1 && win.protoAbilityID2_ == -1);

  win.protoAbilityFlags_ = 0x25;  // bits 0, 2, and 5 survive
  win.protoAbilityID1_ = 8;
  win.protoAbilityID2_ = 9;
  win.delete_abil(0x4);            // clear bit 2, then pick lowest two
  assert(win.protoAbilityFlags_ == 0x21);
  assert(win.protoAbilityID1_ == 0 && win.protoAbilityID2_ == 5);

  win.protoAbilityFlags_ = 0x80000001u; // bit 31 remains, but is not a slot
  win.delete_abil(0x1);
  assert(win.protoAbilityFlags_ == 0x80000000u);
  assert(win.protoAbilityID1_ == -1 && win.protoAbilityID2_ == -1);
    return 0;
  }

  Ability.count = 0;
  if (strcmp(argv[1], "render-one") == 0) {
    win.protoAbilityID1_ = -1;
    win.protoAbilityID2_ = 4;
    win.render_slot_one();
    assert(strcmp(StringTemp, "{ONE} -") == 0);
    assert(Ability.count == 0); // this empty slot must not index Ability
    win.protoAbilityID1_ = 3;
    win.render_slot_one();
    assert(Ability.count == 1 && Ability.reads[0] == 3);
    return 0;
  }
  if (strcmp(argv[1], "render-two") == 0) {
    win.protoAbilityID1_ = 3;
    win.protoAbilityID2_ = -1;
    win.render_slot_two();
    assert(strcmp(StringTemp, "{TWO} -") == 0);
    assert(Ability.count == 0); // this empty slot must not index Ability
    win.protoAbilityID2_ = 4;
    win.render_slot_two();
    assert(Ability.count == 1 && Ability.reads[0] == 4);
    return 0;
  }
  return 2;
}
'''


def _disable_core_dumps():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def _compile(tmp_path: Path, compiler: str, fixture: str, name: str) -> Path:
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(fixture, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-O2", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=60, cwd=tmp_path,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return binary


def _run(binary: Path, tmp_path: Path, scenario: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(binary), scenario], text=True, capture_output=True, timeout=10,
        cwd=tmp_path, preexec_fn=_disable_core_dumps,
    )


def _compilers() -> list[str]:
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "both GCC and Clang are required for actual-source controls"
    return [compiler for compiler in compilers if compiler]


def test_actual_signed_fields_preserve_delete_behavior(tmp_path):
    fields, branches, delete = _actual_parts()
    assert fields["protoAbilityFlags_"][1] == "0x14208"
    assert fields["protoAbilityID1_"][1] == "0x14218"
    assert fields["protoAbilityID2_"][1] == "0x1421C"
    assert "protoAbilityID1_ = -1" in delete or "protoAbilityID1_ = static_cast<uint32_t>(-1)" in delete
    assert "i < 0x1d" in delete

    for compiler in _compilers():
        run_dir = tmp_path / Path(compiler).name
        baseline = _compile(run_dir, compiler, _fixture(fields, branches, delete), "signed-delete")
        run = _run(baseline, run_dir, "delete")
        assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("slot", ["one", "two"])
def test_empty_render_slot_rejects_unsigned_member_mutant(tmp_path, slot):
    fields, branches, delete = _actual_parts()
    scenario = f"render-{slot}"
    for compiler in _compilers():
        run_dir = tmp_path / Path(compiler).name
        baseline = _compile(run_dir, compiler, _fixture(fields, branches, delete), "signed-render")
        baseline_run = _run(baseline, run_dir, scenario)
        assert baseline_run.returncode == 0, baseline_run.stdout + baseline_run.stderr

        mutant = _compile(
            run_dir, compiler,
            _fixture(fields, branches, delete, unsigned_mutant=True), "unsigned-render-mutant",
        )
        mutant_run = _run(mutant, run_dir, scenario)
        assert mutant_run.returncode != 0, "unsigned member mutant retained empty-slot behavior"
