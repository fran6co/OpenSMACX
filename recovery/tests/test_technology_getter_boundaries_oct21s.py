"""Source and host-runtime controls for the technology getter owner move.

These checks establish source ownership continuity and ordinary C++ behavior;
they do not establish VC6 ABI or instruction equivalence.
"""

from dataclasses import fields
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file, read_text, write_body


ROOT = Path(__file__).resolve().parents[2]
CPP_REL = Path("src/technology.cpp")
HEADER_REL = Path("src/technology.h")
GETTERS = {
    0x005BDD70: ("tech_mil", "power_value", 0),
    0x005BDD90: ("tech_tech", "tech_value", 1),
    0x005BDDC0: ("tech_infra", "wealth_value", 1),
    0x005BDDF0: ("tech_colonize", "growth_value", 1),
}
BASELINE = "33fa72a0ed456dd6946aeb9e90066b8379344acc"


def _masked(text):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )


def _extract_definition(text, name, *, inline=False):
    inline_part = r"\s+inline" if inline else r"(?:\s+inline)?"
    signature = re.compile(
        rf"\bMEASURED{inline_part}\s+int\s+__cdecl\s+{re.escape(name)}"
        r"\s*\(\s*int\s+tech_id\s*\)\s*\{"
    )
    masked = _masked(text)
    matches = list(signature.finditer(masked))
    if len(matches) != 1:
        raise AssertionError(f"expected one definition for {name}, got {len(matches)}")
    opening = matches[0].end() - 1
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[matches[0].start():end + 1]
    raise AssertionError(f"unterminated definition for {name}")


def _extract_picktech(text):
    masked = _masked(text)
    signature = re.compile(
        r"\bint\s+__cdecl\s+tech_pick\s*\(\s*int\s+faction\s*,\s*"
        r"int\s+mode\s*,\s*int\s+count\s*,\s*char\s*\*\s*section\s*\)\s*\{"
    )
    matches = list(signature.finditer(masked))
    assert len(matches) == 1, f"expected one tech_pick definition, got {len(matches)}"
    opening = matches[0].end() - 1
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[matches[0].start():end + 1]
    raise AssertionError("unterminated tech_pick definition")


def _git_show(revision, path):
    result = subprocess.run(
        ["git", "show", f"{revision}:{path.as_posix()}"],
        cwd=ROOT, capture_output=True, check=True,
    )
    return result.stdout


def _lesson_signature(record):
    return tuple((lesson.kind, lesson.key, lesson.prose) for lesson in record.origin or ())


def _record_signature(record):
    ignored = {"line", "region", "extract_error", "origin"}
    facts = tuple(
        (field.name, getattr(record, field.name))
        for field in fields(record)
        if field.name not in ignored
    )
    return facts, _lesson_signature(record)


def test_native_body_writer_preserves_four_mapping_records_and_moves_only_body_ownership():
    old_cpp = _git_show(BASELINE, CPP_REL)
    old_rows = read_text(old_cpp.decode("utf-8"), ROOT / CPP_REL)
    old_by_address = {row.address: row for row in old_rows}
    assert len(old_by_address) == len(old_rows)
    assert set(GETTERS).issubset(old_by_address)

    # Generate the expected mapping using the package writer, not a hand-built
    # marker.  The original four records were in technology.cpp but named the
    # header as their body location.
    expected_cpp = old_cpp
    for address in GETTERS:
        row = old_by_address[address]
        assert row.body == "src/technology.h"
        assert row.byte_exact is True
        expected_cpp = write_body(expected_cpp, row, "src/technology.cpp")

    expected_rows = read_text(expected_cpp.decode("utf-8"), ROOT / CPP_REL)
    expected_by_address = {row.address: row for row in expected_rows}
    current_rows = read_file(ROOT / CPP_REL)
    current_by_address = {row.address: row for row in current_rows}
    assert len(current_by_address) == len(current_rows)

    for address, (name, _field, _fallback) in GETTERS.items():
        assert address in current_by_address
        before = old_by_address[address]
        expected = expected_by_address[address]
        current = current_by_address[address]
        assert before.name == expected.name == current.name
        assert f"?{name}@@" in current.name
        assert before.body == "src/technology.h"
        assert expected.body == current.body == "src/technology.cpp"
        assert expected.byte_exact is current.byte_exact is True
        assert _record_signature(expected) == _record_signature(current)
        # The moved definitions now parse in this TU; the historical record's
        # old header location could not be extracted from technology.cpp.
        assert current.extract_error == ""

    old_header = _git_show(BASELINE, HEADER_REL).decode("utf-8")
    current_header = (ROOT / HEADER_REL).read_text()
    current_cpp = (ROOT / CPP_REL).read_text()
    for _address, (name, _field, _fallback) in GETTERS.items():
        old_definition = _extract_definition(old_header, name, inline=True)
        current_definition = _extract_definition(current_cpp, name)
        expected_definition = re.sub(r"\binline\s+", "", old_definition, count=1)
        assert current_definition == expected_definition
        declaration = re.compile(
            rf"\bMEASURED\s+int\s+__cdecl\s+{re.escape(name)}"
            r"\s*\(\s*int\s+tech_id\s*\)\s*;"
        )
        assert len(declaration.findall(current_header)) == 1
        with pytest.raises(AssertionError):
            _extract_definition(current_header, name)

    old_picktech = _git_show(BASELINE, Path("src/picktech.cpp")).decode("utf-8")
    current_picktech = (ROOT / "src/picktech.cpp").read_text()
    assert _extract_picktech(old_picktech) == _extract_picktech(current_picktech)

    # The five observed call sites are owned by PickTech::pick, not by the
    # free tech_pick wrapper above. Preserve the complete source record/body
    # for that actual caller across this owner move.
    pick_rel = Path("src/picktech.cpp")
    old_pick_records = read_text(
        _git_show(BASELINE, pick_rel).decode("utf-8"), ROOT / pick_rel
    )
    current_pick_records = read_file(ROOT / pick_rel)
    old_pick = [row for row in old_pick_records if row.address == 0x00486510]
    current_pick = [row for row in current_pick_records if row.address == 0x00486510]
    assert len(old_pick) == len(current_pick) == 1
    old_pick, current_pick = old_pick[0], current_pick[0]
    assert old_pick.extract_error == current_pick.extract_error == ""
    assert old_pick.region and current_pick.region
    assert old_pick.region == current_pick.region
    for attribute in (
        "address", "mode", "state", "name", "symbol", "image_spans",
        "byte_exact", "semantic", "kind", "exclusion", "context", "body",
    ):
        assert getattr(old_pick, attribute) == getattr(current_pick, attribute), attribute


def _fixture(getter_bodies):
    return r"""
#include <climits>
#define MEASURED
#define __cdecl
const int MaxTechnologyNum = 89;
struct RulesTechnology {
    int power_value;
    int tech_value;
    int wealth_value;
    int growth_value;
};
RulesTechnology Technology[MaxTechnologyNum + 1];
BODY
int main() {
    Technology[0] = {10, 20, 30, 40};
    Technology[88] = {108, 208, 308, 408};
    // Sentinel makes a mistakenly inclusive upper-bound mutant observable
    // without reading outside this host fixture.
    Technology[89] = {109, 209, 309, 409};
    if (tech_mil(0) != 10 || tech_tech(0) != 20 ||
        tech_infra(0) != 30 || tech_colonize(0) != 40) return 1;
    if (tech_mil(88) != 108 || tech_tech(88) != 208 ||
        tech_infra(88) != 308 || tech_colonize(88) != 408) return 2;
    if (tech_mil(89) != 0 || tech_tech(89) != 1 ||
        tech_infra(89) != 1 || tech_colonize(89) != 1) return 3;
    if (tech_mil(90) != 0 || tech_tech(90) != 1 ||
        tech_infra(90) != 1 || tech_colonize(90) != 1) return 4;
    if (tech_mil(INT_MAX) != 0 || tech_tech(INT_MAX) != 1 ||
        tech_infra(INT_MAX) != 1 || tech_colonize(INT_MAX) != 1) return 5;
    // Negative inputs are intentionally not executed: the recovered original
    // has no lower-bound guard, so such an access is undefined behavior.
    return 0;
}
""".replace("BODY", "\n".join(getter_bodies))


def _actual_bodies():
    source = (ROOT / CPP_REL).read_text()
    return {name: _extract_definition(source, name) for name, _, _ in GETTERS.values()}


def _compile_and_run(compiler, tmp_path, source, label):
    cpp = tmp_path / f"{label}.cpp"
    exe = tmp_path / label
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_getter_bodies_cover_valid_upper_boundary_and_fallbacks(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _actual_bodies()
    source = _fixture(list(bodies.values()))
    ran = _compile_and_run(compiler, tmp_path, source, "actual_getters")
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["wrong_field", "inclusive_guard", "wrong_fallback"])
def test_compile_success_mutants_are_detected(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = _actual_bodies()

    if damage == "wrong_field":
        original = bodies["tech_infra"]
        mutated = original.replace(".wealth_value", ".growth_value", 1)
        assert mutated != original and original.count(".wealth_value") == 1
        bodies["tech_infra"] = mutated
    elif damage == "inclusive_guard":
        original = bodies["tech_colonize"]
        mutated = original.replace("tech_id < MaxTechnologyNum", "tech_id <= MaxTechnologyNum", 1)
        assert mutated != original and original.count("tech_id < MaxTechnologyNum") == 1
        bodies["tech_colonize"] = mutated
    else:
        original = bodies["tech_mil"]
        mutated = original.replace("return 0;", "return 1;", 1)
        assert mutated != original and original.count("return 0;") == 1
        bodies["tech_mil"] = mutated

    ran = _compile_and_run(compiler, tmp_path, _fixture(list(bodies.values())), f"mutant_{damage}")
    assert ran.returncode != 0, f"{damage} mutant survived"
