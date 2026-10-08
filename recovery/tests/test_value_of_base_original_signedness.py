"""Complete actual-source host controls for value_of_base signed arithmetic.

The baseline compiles the current tracked helpers and complete current body
without candidate rewrites. The sole host adaptation is VC6's extended
for-init variable scope at the final vehicle loop.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file
from match_recovery.binding_source import parse as parse_bindings
from match_recovery.includes import COMMENTS_OR_STRINGS
import re


ROOT = next(p for p in Path(__file__).resolve().parents if (p / ".git").exists())
SOURCE = ROOT / "src/base.cpp"
HARNESS = Path(__file__).resolve().parent / "fixtures/value_of_base_host.cpp"
ADDRESS = 0x0054CB50
SIGNATURE = "int __cdecl value_of_base(int base_id, int faction_id_req, int faction_id_res,"
SIGNED_BITS = "int base_yield_signed_bits(uint32_t value)"


def _definition(text, signature):
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
        if depth == 0:
            return start, end + 1, text[start:end + 1]
    raise AssertionError("unterminated definition: " + signature)


def _actual_source_closure():
    rows = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert len(rows) == 1
    row = rows[0]
    assert row.mode == Mode.BODY and row.region and not row.extract_error
    source = SOURCE.read_text()
    assert row.line in parse_bindings(source, read_file(SOURCE))
    body_start, _, body = _definition(row.region, SIGNATURE)
    del body_start
    helper_rows = []
    helper_ends = []
    for signature in (SIGNED_BITS,):
        start, end, definition = _definition(source, signature)
        helper_rows.append(definition)
        helper_ends.append(end)

    source_function_start = source.index(SIGNATURE)
    metadata_start = source.rfind("/*\nPurpose: Determine the value", 0,
                                  source_function_start)
    assert metadata_start >= 0
    assert max(helper_ends) < metadata_start, (
        "helpers must remain before the Purpose/ORIGINAL/RECOVERY_BINDING block"
    )
    assert "base_yield_signed_bits(value) < 100" in body
    assert body.count("static_cast<uint32_t>(base_yield_signed_bits(value) / 2)") == 3
    assert "int base_pop_factor = Bases[base_id].population_size;" in body
    return "\n\n".join(helper_rows), body


def _compile(tmp_path, compiler, helpers, body):
    fixture = HARNESS.read_text()
    assert fixture.count("/* EXTRACTED_VALUE_OF_BASE_HELPERS */") == 1
    assert fixture.count("/* EXTRACTED_VALUE_OF_BASE_BODY */") == 1
    fixture = fixture.replace("/* EXTRACTED_VALUE_OF_BASE_HELPERS */", helpers)
    fixture = fixture.replace("/* EXTRACTED_VALUE_OF_BASE_BODY */", body)
    cpp = tmp_path / "value_of_base_host.cpp"
    exe = tmp_path / "value_of_base_host"
    cpp.write_text(fixture)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-sign-compare", "-Wno-unused-function",
         "-fsanitize=undefined,signed-integer-overflow",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stderr
    return exe


def _run(exe, scenario):
    return subprocess.run([str(exe), scenario], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_complete_body_signed_semantics_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    helpers, body = _actual_source_closure()

    # VC6 extends the first for-init `i` through the function. ISO C++98 does
    # not, so declare a new loop-local i; no game operation is changed.
    host_body = body.replace("for (i = veh_top(veh_id);",
                             "for (int i = veh_top(veh_id);", 1)
    assert host_body != body
    work = tmp_path / compiler_name
    work.mkdir()
    exe = _compile(work, compiler, helpers, host_body)
    for scenario in ("ordinary", "negative_population", "signed_clamp",
                     "vehicle_half", "wrapped_halves"):
        result = _run(exe, scenario)
        assert result.returncode == 0, scenario + ": " + result.stdout + result.stderr

    mutants = (
        ("population loses sign extension",
         "int base_pop_factor = Bases[base_id].population_size;",
         "uint32_t base_pop_factor = Bases[base_id].population_size;",
         "negative_population", 1),
        ("signed clamp becomes unsigned",
         "if (base_yield_signed_bits(value) < 100) {",
         "if (value < 100) {", "signed_clamp", 1),
        ("signed halvings become unsigned",
         "value = static_cast<uint32_t>(base_yield_signed_bits(value) / 2);", "value /= 2;",
         "wrapped_halves", 3),
    )
    for index, (label, old, new, scenario, count) in enumerate(mutants):
        assert old in host_body, label
        damaged = host_body.replace(old, new, count)
        mutant_dir = tmp_path / (compiler_name + "-mutant-" + str(index))
        mutant_dir.mkdir()
        mutant_exe = _compile(mutant_dir, compiler, helpers, damaged)
        result = _run(mutant_exe, scenario)
        assert result.returncode != 0, label + " escaped the complete-body control"
