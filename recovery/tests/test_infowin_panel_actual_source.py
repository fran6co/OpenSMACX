"""Host controls for the current InfoWin panel-bound method body."""

from pathlib import Path
import resource
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "infowin.cpp"
SIGNATURE = "void InfoWin::configure_panel_bounds() {"


def actual_method() -> str:
    source = SOURCE.read_text()
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    assert masked.count(SIGNATURE) == 1, "expected one actual panel definition"
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated InfoWin::configure_panel_bounds")


HARNESS = r"""
#include <cassert>

struct RECT { int left, top, right, bottom; };
int WinScreenWidth = 0;
unsigned int StatusFlags = 0;
RECT InfoPanelBounds = { -7, -8, -9, -10 };

struct ConsoleProbe {
  int height;
  int calls;
  int left_seen;
  int right_seen;
  int mutate_width_after_read;
  int width_after_read;

  int pixel_height() {
    ++calls;
    left_seen = InfoPanelBounds.left;
    right_seen = InfoPanelBounds.right;
    if (mutate_width_after_read)
      WinScreenWidth = width_after_read;
    return height;
  }
} ConsoleGlobal = { 0, 0, 0, 0, 0, 0 };

struct InfoWin { void configure_panel_bounds(); };

METHOD_BODY

static void run_case(int width, int height, unsigned int initial_flags,
                     int mutate_width, int width_after_read) {
  const int expected_width = width < 1000 ? 200 : 250;
  const int expected_height = width < 1000 ? 180 : 212;
  WinScreenWidth = width;
  StatusFlags = initial_flags;
  InfoPanelBounds.left = -7;
  InfoPanelBounds.top = -8;
  InfoPanelBounds.right = -9;
  InfoPanelBounds.bottom = -10;
  ConsoleGlobal.height = height;
  ConsoleGlobal.calls = 0;
  ConsoleGlobal.left_seen = -99;
  ConsoleGlobal.right_seen = -99;
  ConsoleGlobal.mutate_width_after_read = mutate_width;
  ConsoleGlobal.width_after_read = width_after_read;

  InfoWin owner;
  owner.configure_panel_bounds();

  assert(ConsoleGlobal.calls == 1);
  assert(ConsoleGlobal.left_seen == 0);
  assert(ConsoleGlobal.right_seen == expected_width);
  assert(InfoPanelBounds.left == 0);
  assert(InfoPanelBounds.right == expected_width);
  assert(InfoPanelBounds.top == height);
  assert(InfoPanelBounds.bottom == height + expected_height);
  assert(StatusFlags == (initial_flags | 4u));
}

int main() {
  const int widths[] = { -1, 0, 800, 999, 1000, 1920, 100000 };
  const int heights[] = { 0, 480, 900 };
  const unsigned int flags[] = { 0u, 1u, 0xA5u };
  for (unsigned int f = 0; f != sizeof(flags) / sizeof(flags[0]); ++f)
    for (unsigned int h = 0; h != sizeof(heights) / sizeof(heights[0]); ++h)
      for (unsigned int w = 0; w != sizeof(widths) / sizeof(widths[0]); ++w)
        run_case(widths[w], heights[h], flags[f], 0, 0);

  // Instrumentation only: this mock mutation checks that the chosen paired
  // dimensions are staged before the getter and aren't selected by a reread.
  run_case(1000, 777, 0x20u, 1, 0);
}
"""


def _compiler(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} is unavailable")
    return path


def _compile_and_run(tmp_path: Path, compiler: str, method: str,
                     should_pass: bool) -> None:
    program = HARNESS.replace("METHOD_BODY", method)
    source_path = tmp_path / "infowin_panel.cpp"
    executable = tmp_path / "infowin_panel"
    source_path.write_text(program)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(executable)],
        capture_output=True,
        text=True,
        cwd=tmp_path,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run(
        [str(executable)], capture_output=True, cwd=tmp_path, timeout=10,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )
    assert (result.returncode == 0) is should_pass, result.stderr.decode()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_panel_method_stages_dimensions_and_preserves_state(
    tmp_path: Path, compiler_name: str
) -> None:
    method = actual_method()
    assert "if (WinScreenWidth < 1000)" in method
    assert "InfoPanelBounds.right = panel_width;\n    InfoPanelBounds.top = ConsoleGlobal.pixel_height();" in method
    _compile_and_run(tmp_path, _compiler(compiler_name), method, True)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compile_success_mutants_change_observed_panel_behavior(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = _compiler(compiler_name)
    method = actual_method()

    wrong_threshold = method.replace(
        "if (WinScreenWidth < 1000)", "if (WinScreenWidth < 800)", 1
    )
    assert wrong_threshold != method
    _compile_and_run(tmp_path, compiler, wrong_threshold, False)

    wrong_height = method.replace("panel_height = 212;", "panel_height = 213;", 1)
    assert wrong_height != method
    _compile_and_run(tmp_path, compiler, wrong_height, False)

    overwritten_flags = method.replace("StatusFlags |= 4;", "StatusFlags = 4;", 1)
    assert overwritten_flags != method
    _compile_and_run(tmp_path, compiler, overwritten_flags, False)

    early_height = method.replace(
        "    InfoPanelBounds.left = 0;\n"
        "    InfoPanelBounds.right = panel_width;\n"
        "    InfoPanelBounds.top = ConsoleGlobal.pixel_height();",
        "    const int panel_top = ConsoleGlobal.pixel_height();\n"
        "    InfoPanelBounds.left = 0;\n"
        "    InfoPanelBounds.right = panel_width;\n"
        "    InfoPanelBounds.top = panel_top;",
        1,
    )
    assert early_height != method
    _compile_and_run(tmp_path, compiler, early_height, False)
