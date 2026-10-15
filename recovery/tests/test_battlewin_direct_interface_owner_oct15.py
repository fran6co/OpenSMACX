"""Actual-source controls for direct BattleWin interface release calls."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unbalanced body: {signature}")


PRELUDE = r"""
#include <stdio.h>
#ifndef __cdecl
#define __cdecl
#endif
struct StatusWin;
static StatusWin *watched_status;
struct SubInterface {
  int releases;
  int release_saw_cleared_flag;
  SubInterface() : releases(0), release_saw_cleared_flag(0) {}
  void release_iface_mode();
};
struct BattleWin : SubInterface {};
struct StatusWin {
  int field_15D4_;
  StatusWin() : field_15D4_(0) {}
  void reset();
};
void SubInterface::release_iface_mode() {
  ++releases;
  if (watched_status != 0 && watched_status->field_15D4_ == 0)
    release_saw_cleared_flag = 1;
}
extern BattleWin g_BattleWin;
extern BattleWin wrong_global;
"""


GLOBALS = r"""
BattleWin g_BattleWin;
BattleWin wrong_global;
"""


CONTROL = r"""
static int expect(int condition, const char *message) {
  if (!condition) fprintf(stderr, "FAIL: %s\n", message);
  return condition ? 0 : 1;
}

int main() {
  int failures = 0;
  StatusWin status;
  watched_status = &status;
  status.field_15D4_ = 0;
  status.reset();
  failures += expect(status.field_15D4_ == 0,
                     "zero flag remains zero");
  failures += expect(g_BattleWin.releases == 0,
                     "zero flag does not release interface mode");

  status.field_15D4_ = 7;
  status.reset();
  failures += expect(status.field_15D4_ == 0,
                     "nonzero reset flag is cleared");
  failures += expect(g_BattleWin.releases == 1,
                     "nonzero reset flag releases once on fixed object");
  failures += expect(g_BattleWin.release_saw_cleared_flag == 1,
                     "reset clears its flag before releasing");
  failures += expect(wrong_global.releases == 0,
                     "reset never releases the wrong object");

  teardown_00422eb0();
  teardown_00422ec0(12345);
  failures += expect(g_BattleWin.releases == 3,
                     "both callbacks release the fixed object exactly once");
  failures += expect(wrong_global.releases == 0,
                     "callbacks ignore ambient receiver and use fixed object");
  if (failures) return 1;
  puts("direct BattleWin interface-owner controls passed");
  return 0;
}
"""


def compile_and_run(compiler: str, tmp_path: Path, text: str, label: str) -> bool:
    source = tmp_path / f"{label}.cpp"
    binary = tmp_path / label
    source.write_text(text)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    return result.returncode == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["baseline", "wrong_receiver", "wrong_order"])
def test_direct_release_actual_bodies_and_mutants(compiler, mutation, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")

    status_source = (ROOT / "src/statuswin.cpp").read_text()
    battle_source = (ROOT / "src/battlewin.cpp").read_text()
    reset = extract_body(status_source, "void StatusWin::reset()")
    first_callback = extract_body(battle_source, "void __cdecl teardown_00422eb0()")
    second_callback = extract_body(battle_source, "void __cdecl teardown_00422ec0(int)")
    assert reset.count("g_BattleWin.release_iface_mode();") == 1
    assert first_callback.count("g_BattleWin.release_iface_mode();") == 1
    assert second_callback.count("g_BattleWin.release_iface_mode();") == 1
    assert "if (field_15D4_)" in reset
    assert reset.index("field_15D4_ = 0;") < reset.index("g_BattleWin.release_iface_mode();")

    if mutation == "wrong_receiver":
        reset = reset.replace("g_BattleWin.release_iface_mode();", "wrong_global.release_iface_mode();")
    elif mutation == "wrong_order":
        reset = reset.replace(
            "field_15D4_ = 0;\n        g_BattleWin.release_iface_mode();",
            "g_BattleWin.release_iface_mode();\n        field_15D4_ = 0;",
        )

    translation_unit = "\n".join((PRELUDE, GLOBALS, reset, first_callback,
                                   second_callback, CONTROL))
    passed = compile_and_run(executable, tmp_path, translation_unit, mutation)
    assert passed == (mutation == "baseline")


def test_on_left_click_keeps_fixed_receiver_and_side_effect_order():
    source = (ROOT / "src/statuswin.cpp").read_text()
    body = extract_body(source, "void StatusWin::on_left_click(int x, int y)")
    block = (
        "if (field_15D4_ != 0) {\n"
        "            field_15D4_ = 0;\n"
        "            g_BattleWin.release_iface_mode();\n"
        "        }"
    )
    assert block in body
    assert body.index(block) < body.index("TutWinBaseWindow.cancel_zoom_wait();")


def test_helper_has_no_remaining_production_binding_or_use():
    status_header = (ROOT / "src/statuswin.h").read_text()
    status_source = (ROOT / "src/statuswin.cpp").read_text()
    battle_source = (ROOT / "src/battlewin.cpp").read_text()
    assert "sub_interface_global" not in status_header
    assert "sub_interface_global" not in status_source
    assert "sub_interface_global" not in battle_source
    assert battle_source.count("g_BattleWin.release_iface_mode();") == 2
    assert status_source.count("g_BattleWin.release_iface_mode();") == 2
