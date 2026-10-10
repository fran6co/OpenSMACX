"""Actual-source controls for make_proto's basic/faction row boundary."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
SOURCE = ROOT / "src/veh.cpp"
FIXTURE = (ROOT / "recovery/tests/fixtures/"
           "prototype_row_boundary_owner_oct10.cpp")


def actual_function(source: str, name: str) -> str:
    # Preserve character positions while removing comments so braces in
    # explanatory text cannot affect extraction of the complete live body.
    code = re.sub(
        r"/\*.*?\*/|//[^\n]*",
        lambda match: "".join("\n" if char == "\n" else " "
                               for char in match[0]),
        source, flags=re.S)
    match = re.search(
        r"(?:void|int)\s+__cdecl\s+" + re.escape(name)
        + r"\s*\([^;{]*\)\s*\{",
        code)
    assert match, f"missing live {name} definition"
    brace = code.index("{", match.start())
    depth = 0
    for end in range(brace, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            return source[match.start():end + 1]
    raise AssertionError(f"unterminated live {name} definition")


def compile_and_run(tmp_path: Path, compiler: str, body: str, name: str):
    fixture = FIXTURE.read_text()
    assert fixture.count("@@MAKE_PROTO@@") == 1
    cpp = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    cpp.write_text(fixture.replace("@@MAKE_PROTO@@", body))
    built = subprocess.run(
        [compiler, "-std=c++11", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=15)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_make_proto_row_63_and_64_against_actual_body(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = SOURCE.read_text()
    body = actual_function(source, "make_proto")

    # Adjacent get_plan is an unchanged control: its faction-specific test
    # intentionally starts at >=64 and uses the typed has_tech_call path.
    plan = actual_function(source, "get_plan")
    assert "proto_id >= MaxVehProtoFactionNum\n                || has_tech_call" in plan
    assert "proto_id_loop < MaxVehProtoFactionNum" in body
    assert "proto_id_loop >= MaxVehProtoFactionNum" in body

    actual = compile_and_run(tmp_path, compiler, body, "actual")
    assert actual.returncode == 0, actual.stderr

    # Each one-sided reversion compiles, then fails the corresponding row-64
    # output/call observation. This prevents a syntax failure from passing as
    # a successful mutation control.
    mutants = {
        "old_basic_inclusive": (
            "proto_id_loop < MaxVehProtoFactionNum &&",
            "proto_id_loop <= MaxVehProtoFactionNum &&"),
        "old_typed_exclusive": (
            "proto_id_loop >= MaxVehProtoFactionNum\n                        &&",
            "proto_id_loop > MaxVehProtoFactionNum\n                        &&"),
    }
    for name, (old, new) in mutants.items():
        assert body.count(old) == 1
        result = compile_and_run(tmp_path, compiler, body.replace(old, new), name)
        assert result.returncode != 0, name + " unexpectedly passed actual-source controls"
