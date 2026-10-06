"""Bounded host controls that compile the actual installed vehdraw_unk1 body."""

import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).parent / "fixtures" / "vehicle_renderer_host.cpp.in"


def extract_actual_function(text):
    signature = "void __cdecl vehdraw_unk1("
    assert text.count(signature) == 1, "actual src/veh.cpp must contain one vehdraw_unk1"
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    state = "code"
    i = brace
    while i < len(text):
        c = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line_comment":
            if c == "\n": state = "code"
        elif state == "block_comment":
            if c == "*" and nxt == "/": state = "code"; i += 1
        elif state == "string":
            if c == "\\": i += 1
            elif c == '"': state = "code"
        elif state == "char":
            if c == "\\": i += 1
            elif c == "'": state = "code"
        else:
            if c == "/" and nxt == "/": state = "line_comment"; i += 1
            elif c == "/" and nxt == "*": state = "block_comment"; i += 1
            elif c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        i += 1
    raise AssertionError("actual vehdraw_unk1 body has unbalanced braces")


def compile_run(compiler, body, tmp_path, name):
    template = FIXTURE.read_text()
    marker = "// ACTUAL_EMITTED_FUNCTION"
    assert template.count(marker) == 1
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(template.replace(marker, body))
    build = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def test_vehicle_terrain_renderer_actual_source(tmp_path):
    source = (ROOT / "src/veh.cpp").read_text()
    body = extract_actual_function(source)
    if os.environ.get("OSMX_HOST_CXX"):
        compilers = [shutil.which(os.environ["OSMX_HOST_CXX"])]
    else:
        compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("host GCC/Clang required for actual-body controls")

    mutants = {
        "terrain-zero-map-read": ("if (terrain == 0)", "if (terrain == 0 && terrain != 0)"),
        "table-zero-bias": ("VehDrawFacetNormals0[index - 400]", "VehDrawFacetNormals0[index]"),
        "render-flags": ("((original_flags & 0x2000) | 0x1000) >> 12", "((original_flags & 0x2000) | 0x1000) >> 13"),
        "camera-ratio": ("* 0.07f", "* 0.7f"),
        "premature-integer-float-rounding": (
            "static_cast<double>(scale_numerator)",
            "static_cast<float>(scale_numerator)",
        ),
    }
    for compiler_index, compiler in enumerate(compilers):
        baseline = compile_run(compiler, body, tmp_path, f"renderer-{compiler_index}")
        assert baseline.returncode == 0, baseline.stdout + baseline.stderr
        assert "vehdraw actual-body host controls PASS" in baseline.stdout
        for name, (old, new) in mutants.items():
            assert body.count(old) == 1, f"behavior mutant anchor must be unique: {name}"
            mutant = compile_run(compiler, body.replace(old, new, 1), tmp_path,
                                 f"{name}-{compiler_index}")
            assert mutant.returncode == 42 and "FAIL:" in mutant.stderr, (
                f"{compiler} control failed to catch {name}: {mutant.stdout}{mutant.stderr}"
            )
