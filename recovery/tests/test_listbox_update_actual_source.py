"""Bounded host controls extracting the installed ListBox::update_changes."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/listbox.cpp").is_file())
SOURCE = ROOT / "src/listbox.cpp"
FIXTURE = Path(__file__).with_name("listbox_update_host_control.cpp")
SIGNATURE = "int ListBox::update_changes()"
INSERTION = "// UPDATE_METHOD_INSERTION"


def extract_method(source: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    assert masked.count(SIGNATURE) == 1, "expected one installed update method"
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start + len(SIGNATURE))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated ListBox::update_changes body")


def actual_fixture(mutant: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    method = extract_method(source)
    if mutant == "virtual-attach":
        old = "this->ListBox::attach("
        assert method.count(old) == 1
        method = method.replace(old, "attach(", 1)
    elif mutant == "minus-one":
        old = "set_selected_pos((old_selected == -1) ? -1 : 0, 1);"
        assert method.count(old) == 1
        method = method.replace(old, "set_selected_pos(0, 1);", 1)
    elif mutant == "vertical-precedence":
        old = "if (win_flags & 8)"
        assert method.count(old) == 1
        method = method.replace(
            old, "if ((win_flags & 8) && !(win_flags & 4))", 1
        )
    elif mutant == "skip-parent-reload":
        old = "Win *target = window->parent_window();"
        assert method.count(old) == 2
        prefix, suffix = method.rsplit(old, 1)
        method = prefix + "Win *target = 0;" + suffix
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    # The host model predates the recovered Sound view. Keep its existing
    # fake receiver interface while the separate source assertion above
    # verifies that production uses legacy_sound_36().
    method = method.replace(
        "Sound *child = target->legacy_sound_36();",
        "Win *child = target->legacy_window_36();",
    )
    method = method.replace(
        "if (child != 0) {\n"
        "                if (child->is_playing() == 0) {\n"
        "                    child->play();\n"
        "                }\n"
        "            }",
        "if (child != 0) { /* host fixture has no Sound interface */ }",
    )
    return fixture.replace(INSERTION, method, 1)


def compile_run(source: str, compiler: str, tmp: Path) -> subprocess.CompletedProcess:
    cpp = tmp / "update.cpp"
    exe = tmp / "update"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-Wno-unused-function", str(cpp), "-o", str(exe)],
        cwd=tmp, capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], cwd=tmp, capture_output=True,
                          text=True, timeout=30)


def available_compilers() -> list[str]:
    return [path for name in ("g++", "clang++")
            if (path := shutil.which(name)) is not None]


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_update_guard_counts_callbacks_ranges_and_redraw(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    method = extract_method(SOURCE.read_text(encoding="utf-8"))
    # The owner walk is typed through the pinned Sound receiver view.
    assert "static_cast<GraphicWin *>(this)" in method
    assert "static_cast<Dialog *>(this)" in method
    assert "this->ListBox::attach(" in method
    assert "legacy_sound_36()" in method
    assert "reinterpret_cast<Sound" not in method
    assert "vbt[" not in method and "dlg_base" not in method
    with tempfile.TemporaryDirectory(prefix="listbox-update-host-") as name:
        result = compile_run(actual_fixture(), compiler, Path(name))
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", (
    "virtual-attach", "minus-one", "vertical-precedence", "skip-parent-reload",
))
def test_damage_controls_reject_lost_dispatch_or_branch_contract(
        compiler_name: str, mutant: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    with tempfile.TemporaryDirectory(prefix="listbox-update-mutant-") as name:
        result = compile_run(actual_fixture(mutant), compiler, Path(name))
    assert result.returncode != 0, f"{mutant} damage control unexpectedly passed"
