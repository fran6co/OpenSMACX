"""Shared compiler failure controls and project-owned VC6 option semantics."""

import json
import subprocess
import sys

import pytest

from match_recovery.compilation_database import entries
from match_recovery.source import read_file
from match_recovery.source.asm import compiled_asm, CompileFailed
from recovery.environment import compile as environment
from recovery.environment.recipe import invocation, build_command, MissingRecipe, flag_dependent


def recipe(tmp_path, args):
    entry, = entries(json.dumps([{"file": "unit.cpp", "directory": str(tmp_path),
                                  "arguments": args}]).encode(), tmp_path)
    return invocation(entry)


def test_recipe_preserves_argument_boundaries_and_resolves_both_include_forms(tmp_path):
    args = ["./vc6-cl", "/I", "headers with spaces", "-I../shared", "/FI", "config.h",
            '-DMESSAGE="hello world"', "/c", "/O2", "/Oy-", "/Z7", "/Foout.obj", "unit.cpp"]
    assert recipe(tmp_path, args) == [str(tmp_path / "vc6-cl"), "/nologo",
        "/I" + str(tmp_path / "headers with spaces"), "/I" + str(tmp_path.parent / "shared"),
        "/FI" + str(tmp_path / "config.h"), '/DMESSAGE="hello world"']


def test_pch_filter_does_not_delete_an_unrelated_define(tmp_path):
    assert recipe(tmp_path, ["cl", "/FI", "cmake_pch.hxx", "/Yucmake_pch.hxx", "/Fp", "pch.bin",
                             "/DRESOURCE=cmake_pch", "unit.cpp"]) == ["cl", "/nologo", "/DRESOURCE=cmake_pch"]


@pytest.mark.parametrize("args", [["compiler", "@options"], ["compiler", "/I"],
    ["compiler", "/FI"], ["compiler", "/Fo"], ["python", "wrapper.py", "unit.cpp"]])
def test_unsupported_recipe_is_not_silently_changed(tmp_path, args):
    with pytest.raises(ValueError):
        recipe(tmp_path, args)


def test_missing_recipe_is_distinct_from_conflicting_or_malformed_recipe(tmp_path):
    database = tmp_path / "compilation.json"
    rows = [{"file": "unit.cpp", "directory": str(tmp_path), "arguments": ["cl", flag, "unit.cpp"]}
            for flag in ("/DVALUE=1", "/DVALUE=2")]
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="conflicting") as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)
    with pytest.raises(MissingRecipe):
        build_command(database, tmp_path / "seed.cpp")
    database.write_text("invalid")
    with pytest.raises(ValueError) as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)


@pytest.mark.parametrize("code,body,accepted", [(0, b"object", True), (1, b"object", False),
                                               (0, b"", False), (0, None, False), (7, None, False)])
def test_compiler_exit_and_object_are_both_required(tmp_path, monkeypatch, code, body, accepted):
    seen = []
    def run(argv, *, cwd, timeout):
        seen.append(cwd)
        assert timeout == 300 and argv[-2] == "/Founit.obj"
        assert '/DMESSAGE=hello world' in argv
        if body is not None:
            (cwd / "unit.obj").write_bytes(body)
        return subprocess.CompletedProcess(argv, code, b"compiler diagnostic", b"")
    monkeypatch.setattr(environment, "run_bounded", run)
    source = tmp_path / "unit.cpp"
    if accepted:
        assert environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"') == body
    else:
        with pytest.raises(ValueError):
            environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"')
    assert seen and not seen[0].exists()


def test_timeout_is_a_comparison_failure_and_temporary_directory_is_removed(tmp_path, monkeypatch):
    seen = []
    def stall(argv, *, cwd, timeout):
        seen.append(cwd)
        raise subprocess.TimeoutExpired(argv, timeout)
    monkeypatch.setattr(environment, "run_bounded", stall)
    source = tmp_path / "unit.cpp"
    source.write_text("// ORIGINAL: 0x00401000 f 0x00401000-0x00401008\nvoid f() {}\n")
    with pytest.raises(CompileFailed, match="exceeded 300 seconds"):
        compiled_asm(read_file(source)[0], ["compiler"], "/c", compiler=environment.compile_object)
    assert seen and not seen[0].exists()


def test_real_failed_process_cannot_supply_an_object_to_the_comparer(tmp_path):
    # A synthetic failing tool actually creates an object before exiting. This
    # reproduced the old subprocess path's mistake without proprietary fixtures.
    script = "from pathlib import Path; Path('unit.obj').write_bytes(b'object'); raise SystemExit(7)"
    with pytest.raises(ValueError, match="compilation failed"):
        environment.compile_object(tmp_path / "unit.cpp", (sys.executable, "-c", script), ())


def test_vc6_retry_policy_remains_in_the_environment():
    assert not flag_dependent("error C2065: identifier not declared")
    assert flag_dependent("fatal error C1001: internal compiler error")
    assert flag_dependent("fatal error C1060: out of heap space")
