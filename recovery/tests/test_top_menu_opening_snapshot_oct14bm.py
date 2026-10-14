"""Focused public SetupWin snapshot and opening-presentation regressions.

These host controls compile the actual top_menu implementation with bounded
window doubles; they are not a full runtime or native ABI proof.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_newgame_menu import projected, run_control

ROOT = Path(__file__).resolve().parents[2]


FIRST_WRITE = (
    "    setup.opening_window_snapshot_ = opening_window();\n"
    "    TurnUpkeepPhase = 0;"
)
SECOND_WRITE = (
    "    setup.opening_window_snapshot_ = opening_window();\n"
    "    prefs_load(0);"
)


@pytest.mark.parametrize(
    "compiler,damage,diagnostic",
    [
        ("g++", (FIRST_WRITE, "    /* omitted first publication */\n    TurnUpkeepPhase = 0;"),
         "first opening snapshot publication missing"),
        ("g++", (SECOND_WRITE, "    /* omitted second publication */\n    prefs_load(0);"),
         "second opening snapshot publication missing"),
    ],
)
def test_each_snapshot_publication_has_a_compile_success_semantic_witness(
        tmp_path, compiler, damage, diagnostic):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    result = run_control(tmp_path, executable, projected("top_menu", damage))
    assert result.returncode == 97, result.stderr.decode(errors="replace")
    assert ("SEMANTIC97: " + diagnostic).encode() in result.stderr


def test_snapshot_uses_fresh_opening_window_not_cached_basepop(tmp_path):
    executable = shutil.which("g++")
    if executable is None:
        pytest.skip("g++ unavailable")
    source = projected("top_menu")
    helper_start = source.index("static void top_menu_present_opening(")
    helper_end = source.index("int __cdecl top_menu(int mode)", helper_start)
    helper = source[helper_start:helper_end]
    assert "BasePopDefaultField322C = opening_window();" in helper
    helper = helper.replace(
        "BasePopDefaultField322C = opening_window();",
        "BasePopDefaultField322C = check_first_publication_during_redraw "
        "? &stale_opening_window : opening_window();",
        1,
    )
    source = source[:helper_start] + helper + source[helper_end:]
    assert source.count("setup.opening_window_snapshot_ = opening_window();") == 2
    source = source.replace(
        "setup.opening_window_snapshot_ = opening_window();",
        "setup.opening_window_snapshot_ = static_cast<GraphicWin *>(BasePopDefaultField322C);",
    )
    result = run_control(tmp_path, executable, source)
    assert result.returncode == 97, result.stderr.decode(errors="replace")
    assert b"SEMANTIC97: first opening snapshot publication missing" in result.stderr


def test_opening_filename_family_width_and_retry_controls(tmp_path):
    executable = shutil.which("g++")
    if executable is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, executable, projected("top_menu"))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def extract_body(text, signature):
    start = text.index(signature + " {")
    opening = text.index("{", start)
    depth = 0
    for position in range(opening, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError("unterminated source body: " + signature)


def test_real_setupwin_public_member_and_constructor_snapshot(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    setup_source = (ROOT / "src/setupwin.cpp").read_text()
    header = (ROOT / "src/setupwin.h").read_text()
    declaration = "GraphicWin *opening_window_snapshot_;"
    assert declaration in header
    member_at = header.index(declaration)
    assert header.rfind("public:", 0, member_at) > header.rfind("private:", 0, member_at)
    constructor = extract_body(setup_source, "SetupWin::SetupWin()")
    fixture = r'''#include <stdlib.h>
struct GraphicWin {};
static GraphicWin *current_opening;
static GraphicWin *&opening_window() { return current_opening; }
static int WinScreenWidth = 800;
static int WinScreenHeight = 600;
struct SetupWin {
    int overlay_buffers_[3];
    int row_count_, selected_row_;
    GraphicWin *opening_window_snapshot_;
    void *menu_popup_;
    int field_A14_, field_A18_;
    SetupWin();
};
''' + constructor + r'''
int main() {
    GraphicWin first;
    current_opening = &first;
    SetupWin captured;
    if (captured.opening_window_snapshot_ != &first) return 1;
    current_opening = 0;
    SetupWin empty;
    return empty.opening_window_snapshot_ == 0 ? 0 : 2;
}
'''
    path = tmp_path / "setupwin_snapshot.cpp"
    path.write_text(fixture)
    binary = tmp_path / "setupwin_snapshot"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", str(path), "-o", str(binary)], check=True,
                   capture_output=True, timeout=45)
    result = subprocess.run([str(binary)], capture_output=True, timeout=15)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_real_setupwin_rightside_guards_missing_snapshot_before_dereference():
    setup_source = (ROOT / "src/setupwin.cpp").read_text()
    body = extract_body(setup_source, "int SetupWin::do_menu_rightside(char *filename, int flags)")
    guard = "if (opening_window_snapshot_ == nullptr) {"
    assert guard in body
    guard_at = body.index(guard)
    close_at = body.index("flic.close();", guard_at)
    return_at = body.index("return -1;", close_at)
    dereference_at = body.index("opening_window_snapshot_->")
    assert guard_at < close_at < return_at < dereference_at
