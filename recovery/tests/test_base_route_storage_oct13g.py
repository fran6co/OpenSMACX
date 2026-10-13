"""Source-grounded Base route storage and reserved-word controls."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated source function")


def source_fragments():
    header = (ROOT / "src/base.h").read_text()
    base_cpp = (ROOT / "src/base.cpp").read_text()
    assert "RECOVERY_MEMBER: v1 Base auto_forward_slots 0x110 0x0046D5D0" in header
    assert "RECOVERY_MEMBER: v1 Base talent_total 0x118" in header
    assert "sizeof(Base) == 0x134" in header
    assert "not evidence\n    // for a declared four-element route array" in header
    base_decl = braced(header, "struct Base {") + ";"

    init, = re.findall(
        r"base\.auto_forward_slots\[Base::AutoForwardLand\]\s*=\s*"
        r"base\.auto_forward_slots\[Base::AutoForwardSea\]\s*=\s*"
        r"base\.auto_forward_slots\[Base::AutoForwardAir\]\s*=\s*-1;",
        braced(base_cpp, "int __cdecl base_init(int faction_id, int x, int y)"),
    )
    kill = braced(base_cpp, "void __cdecl base_kill(int base_id)")
    remaps = re.findall(
        r"base_remap_reference\(Bases\[remaining\]\.auto_forward_slots\["
        r"Base::(AutoForward(?:Land|Sea|Air))\], base_id\);",
        kill,
    )
    assert remaps == ["AutoForwardLand", "AutoForwardSea", "AutoForwardAir"]
    helper = braced(base_cpp, "static void base_remap_reference(")
    assert "if (reference == removed) reference = -1;" in helper
    assert "else if (reference > removed) --reference;" in helper
    return base_decl, init, helper, kill


HARNESS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
ACTUAL_BASE
static_assert(sizeof(Base) == 0x134, "complete Base object extent");
static_assert(offsetof(Base, auto_forward_slots) == 0x110, "route storage offset");
static_assert(offsetof(Base, talent_total) == 0x118, "following member offset");
static_assert(sizeof(((Base *)0)->auto_forward_slots) == 8, "four observed words");
static_assert(Base::AutoForwardLand == 0 && Base::AutoForwardSea == 1 &&
              Base::AutoForwardAir == 2 && Base::AutoForwardReserved == 3 &&
              Base::AutoForwardSlotCount == 4, "typed slot indices");
HELPER

int main() {
    Base base = {};
    base.auto_forward_slots[0] = 20;
    base.auto_forward_slots[1] = 21;
    base.auto_forward_slots[2] = 22;
    base.auto_forward_slots[3] = 0x1234;
    INIT
    assert(base.auto_forward_slots[0] == -1);
    assert(base.auto_forward_slots[1] == -1);
    assert(base.auto_forward_slots[2] == -1);
    assert(base.auto_forward_slots[3] == 0x1234);

    Base Bases[1] = {};
    Bases[0].auto_forward_slots[0] = 2;
    Bases[0].auto_forward_slots[1] = 3;
    Bases[0].auto_forward_slots[2] = -1;
    Bases[0].auto_forward_slots[3] = 0x4567;
    const int16_t before_reserved = Bases[0].auto_forward_slots[Base::AutoForwardReserved];
    const int remaining = 0;
    const int base_id = 2;
    REMAP_CASE
    assert(Bases[0].auto_forward_slots[Base::AutoForwardLand] == -1);
    assert(Bases[0].auto_forward_slots[Base::AutoForwardSea] == 2);
    assert(Bases[0].auto_forward_slots[Base::AutoForwardAir] == -1);
    assert(Bases[0].auto_forward_slots[Base::AutoForwardReserved] == before_reserved);
}
'''


def render_source(base_decl, init, helper, remaps, *, mutation=None):
    if mutation == "init-reserved":
        init = init.replace("Base::AutoForwardAir] = -1",
                            "Base::AutoForwardReserved] = -1")
    elif mutation == "remap-reserved":
        remaps = remaps.replace("Base::AutoForwardAir]", "Base::AutoForwardReserved]")
    source = HARNESS.replace("ACTUAL_BASE", base_decl)
    source = source.replace("HELPER", helper)
    source = source.replace("REMAP_CASE", remaps)
    source = source.replace("INIT", init)
    return source


def run(compiler, tmp_path, base_decl, init, helper, remaps, *, mutation=None):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is not installed")
    source = render_source(base_decl, init, helper, remaps, mutation=mutation)
    cpp = tmp_path / f"route-storage-{mutation or 'control'}.cpp"
    binary = cpp.with_suffix("")
    cpp.write_text(source)
    subprocess.run([compiler_path, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=30)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_base_init_and_kill_route_statements_preserve_reserved_word(
        compiler, tmp_path):
    base_decl, init, helper, kill = source_fragments()
    remaps = "\n".join(re.findall(
        r"^\s*base_remap_reference\(Bases\[remaining\]\.auto_forward_slots\["
        r"Base::AutoForward(?:Land|Sea|Air)\], base_id\);",
        kill, re.M))
    result = run(compiler, tmp_path, base_decl, init, helper, remaps)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_route_fallback_branches_remain_explicit_and_default_to_air():
    console = (ROOT / "src/console.cpp").read_text()
    mapwin = (ROOT / "src/mapwin.cpp").read_text()
    assert re.search(
        r"triad == 0\s*\?\s*Bases\[base\]\.auto_forward_slots\[Base::AutoForwardLand\]"
        r"\s*:\s*triad == 1\s*\?\s*Bases\[base\]\.auto_forward_slots\[Base::AutoForwardSea\]"
        r"\s*:\s*Bases\[base\]\.auto_forward_slots\[Base::AutoForwardAir\]",
        console,
    )
    assert re.search(
        r"if \(!forward_triad\).*?Base::AutoForwardLand.*?"
        r"else if \(forward_triad == 1\).*?Base::AutoForwardSea.*?"
        r"else Bases\[subject\]\.auto_forward_slots\[Base::AutoForwardAir\]",
        console, re.S,
    )
    assert re.search(
        r"triad == 0\s*\?\s*source\.auto_forward_slots\[Base::AutoForwardLand\]"
        r"\s*:\s*triad == 1\s*\?\s*source\.auto_forward_slots\[Base::AutoForwardSea\]"
        r"\s*:\s*source\.auto_forward_slots\[Base::AutoForwardAir\]",
        mapwin,
    )


@pytest.mark.parametrize("mutation", ["init-reserved", "remap-reserved"])
def test_reserved_word_controls_reject_mutations(tmp_path, mutation):
    base_decl, init, helper, kill = source_fragments()
    remaps = "\n".join(re.findall(
        r"^\s*base_remap_reference\(Bases\[remaining\]\.auto_forward_slots\["
        r"Base::AutoForward(?:Land|Sea|Air)\], base_id\);",
        kill, re.M))
    result = run("g++", tmp_path, base_decl, init, helper, remaps,
                 mutation=mutation)
    assert result.returncode != 0, (mutation, "mutant unexpectedly passed")


@pytest.mark.parametrize("mutation,before,after", [
    ("reserved-enum", "AutoForwardReserved = 3", "AutoForwardReserved = 2"),
    ("slot-count", "AutoForwardSlotCount = 4", "AutoForwardSlotCount = 3"),
])
def test_actual_base_layout_controls_reject_header_mutations(
        tmp_path, mutation, before, after):
    base_decl, init, helper, kill = source_fragments()
    assert base_decl.count(before) == 1
    changed = base_decl.replace(before, after)
    remaps = "\n".join(re.findall(
        r"^\s*base_remap_reference\(Bases\[remaining\]\.auto_forward_slots\["
        r"Base::AutoForward(?:Land|Sea|Air)\], base_id\);",
        kill, re.M))
    source = render_source(changed, init, helper, remaps)
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is not installed")
    cpp = tmp_path / f"route-storage-{mutation}.cpp"
    binary = cpp.with_suffix("")
    cpp.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                             str(cpp), "-o", str(binary)],
                            capture_output=True, timeout=30)
    assert result.returncode != 0, (mutation, "header mutant unexpectedly compiled")
