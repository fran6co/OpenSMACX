"""Executable host controls for the installed Win::display_action source."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file()
            and (parent / "src/win.h").is_file())
SOURCE = ROOT / "src/win.cpp"
HEADER = ROOT / "src/win.h"
FIXTURE = ROOT / "recovery/tests/fixtures/win_display_action_host.cpp"
INSERTION = "// ACTUAL_METHOD_INSERTION"
SIGNATURE = "int __cdecl Win::display_action(int action)"


def _method(source: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    matches = list(re.finditer(re.escape(SIGNATURE), masked))
    assert len(matches) == 1, f"expected one installed owner method, got {len(matches)}"
    start = matches[0].start()
    opening = masked.find("{", matches[0].end())
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated Win::display_action")


def _translation_unit(body: str) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    return fixture.replace(INSERTION, body, 1)


def _run(tu: str, compiler: str, tmp_path: Path, stem: str) -> subprocess.CompletedProcess:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(tu, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=30)


def test_installed_owner_returns_native_path_dependent_eax() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _method(source)
    header = HEADER.read_text(encoding="utf-8")
    assert "static int __cdecl display_action(int action);" in header
    assert "typedef int (__cdecl *WinDisplayActionHookFn)();" in header
    assert "return ShowWindow(HandleMain, SW_MAXIMIZE);" in body
    assert "return ShowWindow(HandleMain, SW_MINIMIZE);" in body
    assert "result = hook();" in body
    assert "return child->is_playing();" in body or "return playing;" in body
    assert "return child->play();" in body
    assert "static_cast<uint32_t>(action) - 2u" in body
    annotation = source[source.index("// ORIGINAL: 0x005F2830"):]
    assert "RECOVERY_ROLE: SCAFFOLD" not in annotation.split("*/", 1)[0]


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_method_executes_all_eax_routes_and_side_effects(compiler, tmp_path):
    body = _method(SOURCE.read_text(encoding="utf-8"))
    run = _run(_translation_unit(body), compiler, tmp_path, "win_display_action_actual")
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize(
    "old,new",
    [
        ("return ShowWindow(HandleMain, SW_MAXIMIZE);",
         "ShowWindow(HandleMain, SW_MAXIMIZE); return 0;"),
        ("result = hook();", "hook();"),
        ("return playing;", "return 1;"),
        ("static_cast<uint32_t>(action) - 2u", "0u"),
    ],
)
def test_actual_method_controls_reject_lost_result_paths(old, new, tmp_path):
    body = _method(SOURCE.read_text(encoding="utf-8"))
    mutant = body.replace(old, new, 1)
    assert mutant != body
    run = _run(_translation_unit(mutant), "g++", tmp_path,
               "win_display_action_mutant")
    assert run.returncode != 0
