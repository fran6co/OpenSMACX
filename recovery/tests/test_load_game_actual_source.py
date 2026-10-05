"""Compile the actual balanced product load_game body with C++98 controls."""
import pathlib
import shutil
import subprocess

import pytest


ROOT = next(p for p in pathlib.Path(__file__).resolve().parents if (p / "src/game.cpp").is_file())
PRODUCT = ROOT / "src" / "game.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/game_save_load" / "load_game_host_fixture.cpp"
INCLUDE = '#include "load_game_candidate.inc"'
SIGNATURE = "int __cdecl load_game(int"


def extract_body(text):
    start = text.find(SIGNATURE)
    assert start >= 0, "load_game product definition has not been integrated"
    brace = text.find("{", start)
    assert brace >= 0
    depth = 0
    for cursor in range(brace, len(text)):
        if text[cursor] == "{":
            depth += 1
        elif text[cursor] == "}":
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError("unbalanced load_game definition")


def compile_and_run(compiler, body, tmp_path, name, success, run_mode=None):
    fixture = FIXTURE.read_text()
    assert INCLUDE in fixture
    source = tmp_path / ("load-game-" + name + ".cpp")
    source.write_text(fixture.replace(INCLUDE, body))
    binary = tmp_path / ("load-game-" + name)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], check=True, timeout=30,
    )
    command = [str(binary)]
    if run_mode is not None:
        command.append(run_mode)
    else:
        command.append(name)
    result = subprocess.run(command, cwd=tmp_path, timeout=30)
    assert (result.returncode == 0) is success


def test_load_game_actual_body_branches_and_mutants(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [item for item in compilers if item]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    body = extract_body(PRODUCT.read_text())
    for compiler in compilers:
        for mode in ("cancel", "scenario", "dot", "retry", "daemon-error",
                     "password", "password-skip", "prompt-positive", "success"):
            compile_and_run(compiler, body, tmp_path, mode, True)

        wrong_dot = body.replace("strchr(save_stem, '.')", "strrchr(save_stem, '.')", 1)
        assert wrong_dot != body
        compile_and_run(compiler, wrong_dot, tmp_path, "mutant-dot", False, "dot")

        wrong_retry = body.replace("if (prompt_result < 0)",
                                   "if (prompt_result >= 0)", 1)
        assert wrong_retry != body
        compile_and_run(compiler, wrong_retry, tmp_path, "mutant-retry", False,
                        "retry")

        wrong_pbem = body.replace("NetWinDialog.field_7728_ = (mode != 0) + 2;",
                                  "NetWinDialog.field_7728_ = (mode != 0) + 1;", 1)
        assert wrong_pbem != body
        compile_and_run(compiler, wrong_pbem, tmp_path, "mutant-pbem", False,
                        "scenario")

        wrong_filter = body.replace("strcat(filter, pattern);",
                                    "strcat(filter, extension);", 1)
        assert wrong_filter != body
        compile_and_run(compiler, wrong_filter, tmp_path, "mutant-filter", False,
                        "success")
