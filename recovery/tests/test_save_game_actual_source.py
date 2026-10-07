"""Actual-source C++98 behavior controls for the saved-game owner.

The test extracts the balanced product definition, substitutes it into the
existing mock fixture, and requires controls for cancellation, daemon polarity,
the equal/different basename branches, and the ironman sentinel. It never
compiles the retained candidate as if that were the product body.
"""
import pathlib
import shutil
import subprocess

import pytest


ROOT = next(p for p in pathlib.Path(__file__).resolve().parents if (p / "src/game.cpp").is_file())
PRODUCT = ROOT / "src" / "game.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/game_save_load" / "save_game_host_fixture.cpp"
INCLUDE = '#include "save_game_candidate.inc"'
SIGNATURE = "int __cdecl save_game(int mode)"


def extract_body(text):
    start = text.find(SIGNATURE)
    assert start >= 0, "save_game product definition has not been integrated"
    brace = text.find("{", start)
    assert brace >= 0, "save_game product definition has no body"
    depth = 0
    for cursor in range(brace, len(text)):
        if text[cursor] == "{":
            depth += 1
        elif text[cursor] == "}":
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError("save_game product body is unbalanced")


def compile_and_run(compiler, fixture, body, tmp_path, name, expect_success):
    assert INCLUDE in fixture
    source = tmp_path / (name + ".cpp")
    source.write_text(fixture.replace(INCLUDE, body))
    binary = tmp_path / name
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        check=True,
        timeout=30,
    )
    result = subprocess.run([str(binary)], timeout=30)
    assert (result.returncode == 0) is expect_success


def test_save_game_actual_product_body_and_mutant_controls(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")

    body = extract_body(PRODUCT.read_text())
    fixture = FIXTURE.read_text()
    for compiler in compilers:
        compile_and_run(compiler, fixture, body, tmp_path,
                        "save-game-" + pathlib.Path(compiler).name, True)

        wrong_result = body.replace(
            "if (daemon_result != 0)", "if (daemon_result == 0)", 1)
        assert wrong_result != body
        compile_and_run(compiler, fixture, wrong_result, tmp_path,
                        "save-game-result-mutant-" + pathlib.Path(compiler).name,
                        False)

        missing_sentinel = body.replace(
            "state[2].trailing_words_[6] = 0xFF;",
            "state[2].trailing_words_[6] = 0;", 1)
        assert missing_sentinel != body
        compile_and_run(
            compiler, fixture, missing_sentinel, tmp_path,
            "save-game-sentinel-mutant-" + pathlib.Path(compiler).name,
            False,
        )

        wrong_name_equality = body.replace(
            "_stricmp(selected_basename, default_basename) != 0",
            "_stricmp(selected_basename, default_basename) == 0", 1,
        )
        assert wrong_name_equality != body
        compile_and_run(
            compiler, fixture, wrong_name_equality, tmp_path,
            "save-game-name-mutant-" + pathlib.Path(compiler).name, False,
        )

        wrong_auto_path = body.replace(
            'if (strcmp(filename, "auto\\\\") == 0)',
            'if (strcmp(filename, "auto\\\\") != 0)', 1,
        )
        assert wrong_auto_path != body
        compile_and_run(
            compiler, fixture, wrong_auto_path, tmp_path,
            "save-game-auto-path-mutant-" + pathlib.Path(compiler).name,
            False,
        )


def test_save_game_fixture_tracks_typed_game_data_owner():
    owner = (ROOT / "src/game_data_state_window.h").read_text()
    fixture = FIXTURE.read_text()
    for declaration in (
        "uint32_t game_state_;", "uint32_t game_rules_;",
        "char legacy_game_save_name_[256];",
        "struct GameDataStateWindow {", "GameDataStateWindowFields fields_;",
    ):
        assert declaration in owner
    assert "GameDataStateWindowFields" in fixture
    assert "GameDataStateStorage.fields_.game_state_" in fixture
    assert "GameDataStateStorage.fields_.game_rules_" in fixture
    assert "GameDataStateStorage.fields_.legacy_game_save_name_" in fixture
    assert "LegacyGameSaveName" not in fixture
    assert "static unsigned GameState" not in fixture
