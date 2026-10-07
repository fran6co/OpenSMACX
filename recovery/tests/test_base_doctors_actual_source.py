"""Host-only actual-source controls for the pinned base_doctors owner."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / ".git").exists())
SOURCE = ROOT / "src/base.cpp"
HARNESS = ROOT / "recovery/tests/fixtures/base_doctors_host_harness.cpp"
ADDRESS = 0x004F5F30


def _definition(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def _actual_owner_closure():
    row, = (item for item in read_file(SOURCE) if item.address == ADDRESS)
    assert row.mode == Mode.BODY and row.region and not row.extract_error
    source = SOURCE.read_text()
    signatures = (
        "int base_doctors_best_specialist()",
        "void base_doctors_add_specialist()",
        "void __cdecl base_doctors()",
    )
    helpers = "\n".join(_definition(source, signature) for signature in signatures[:-1])
    body = _definition(row.region, signatures[-1])
    assert "Rules.nutrient_req_citizen" in body
    assert "nutrient_surplus" in body
    assert "BaseCurrent" in body
    add = _definition(source, "void base_doctors_add_specialist()")
    assert add.index("base_doctors_best_specialist()") < add.index("specialist_total++")
    assert "if (slot < 16)" in add
    return helpers, body


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_owner_preserves_riot_matrix_and_warning_paths(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    fixture = HARNESS.read_text()
    helpers, body = _actual_owner_closure()
    assert fixture.count("/* EXTRACTED_BASE_DOCTORS_HELPERS */") == 1
    assert fixture.count("/* EXTRACTED_BASE_DOCTORS_BODY */") == 1
    cpp = tmp_path / "base_doctors.cpp"
    exe = tmp_path / "base_doctors"
    fixture = fixture.replace("/* EXTRACTED_BASE_DOCTORS_HELPERS */", helpers)
    fixture = fixture.replace("/* EXTRACTED_BASE_DOCTORS_BODY */", body)
    cpp.write_text(fixture)
    built = subprocess.run(
        (compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(cpp), "-o", str(exe)), capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    host_env = os.environ.copy()
    host_env["ASAN_OPTIONS"] = "detect_leaks=0"
    ran = subprocess.run((str(exe),), capture_output=True, text=True,
                         timeout=10, env=host_env)
    assert ran.returncode == 0, ran.stdout + ran.stderr
