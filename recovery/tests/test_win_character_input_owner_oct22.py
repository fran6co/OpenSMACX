"""GCC/Clang actual-source controls for Win::on_char; no copied body."""

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source.reader import read_file
from test_application_link_recovery import ROOT


UNIT = "src/win_input.cpp"
ADDRESS = 0x5F5FB0
SYMBOL = "?on_char@Win@@UAEXDH@Z"
HARNESS = Path(__file__).with_name("win_keyboard_harness.cpp")
MUTANTS = [
    ("if (WinPointerOwner4 == &WinBubbleWindow) {\n                WinPointerOwner4 = 0;",
     "if (*(void **)WinPointerOwner4 == &WinBubbleWindow) {\n                *(int *)WinPointerOwner4 = 0;"),
    ("} while (i < debug_window.child_count_);", "} while (i < 2);"),
    ("Win **list = debug_window.children_;",
     "Win *snapshot[4]; for (int k = 0; k < 4; ++k) "
     "snapshot[k] = debug_window.children_[k]; Win **list = snapshot;"),
    ("Sound *w = debug_window.val_4_;", "Sound *w = val_4_;"),
]


def _actual_harness() -> str:
    records = [record for record in read_file(ROOT / UNIT)
               if record.address == ADDRESS]
    assert len(records) == 1
    record = records[0]
    assert record.name == "sub_5f5fb0"  # Historical identity is distinct from the emitted member symbol.
    assert (record.symbol or record.name) == SYMBOL
    assert record.region and not record.extract_error
    text = HARNESS.read_text()
    marker = "// LIVE_KEYBOARD"
    assert text.count(marker) == 1
    return text.replace(marker, record.region, 1)


def _compile_and_run(compiler: str, tmp_path: Path, source_text: str,
                     name: str) -> subprocess.CompletedProcess:
    source = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    source.write_text(source_text)
    built = subprocess.run(
        [compiler, "-std=c++11", "-O0", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


def _available_compilers() -> list[str]:
    return [path for name in ("g++", "clang++")
            if (path := shutil.which(name)) is not None]


@pytest.mark.parametrize("compiler", _available_compilers())
def test_on_char_actual_source_runtime(compiler, tmp_path):
    result = _compile_and_run(compiler, tmp_path, _actual_harness(), "on-char")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _available_compilers())
@pytest.mark.parametrize("before,after", MUTANTS)
def test_on_char_compilable_mutants_fail_actual_source_runtime(
        compiler, before, after, tmp_path):
    text = _actual_harness()
    assert text.count(before) == 1
    result = _compile_and_run(compiler, tmp_path, text.replace(before, after, 1),
                              "on-char-mutant")
    assert result.returncode != 0, "compiled mutant escaped runtime controls"
