"""Actual-source controls for BattleWin's timer callback and button base call."""
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
                return source[start:index + 1]
    raise AssertionError(f"unbalanced body: {signature}")


PRELUDE = r"""
#include <stdio.h>
#ifndef __cdecl
#define __cdecl
#endif
struct Calls {
  const void *released;
  const void *button_receiver;
  int button_id;
  int events[8];
  int count;
} calls;
static void event(int value) { calls.events[calls.count++] = value; }
struct SubInterface {
  void release_iface_mode() { calls.released = this; event(1); }
};
struct Time {
  void (__cdecl *callback)(int);
  int param;
  unsigned count;
  unsigned resolution;
  int calls;
  Time() : callback(0), param(-1), count(0), resolution(0), calls(0) {}
  void pulse(void (__cdecl *fn)(int), int p, unsigned c, unsigned r) {
    callback = fn; param = p; count = c; resolution = r; ++calls;
  }
};
struct MainInterface : SubInterface {
  void on_iface_button_clicked(int button) {
    calls.button_receiver = this;
    calls.button_id = button;
    event(2);
  }
};
struct BattleWin : SubInterface {
  Time time_;
  void on_iface_button_clicked(int);
  void pulse_timer();
};
extern BattleWin g_BattleWin;
extern MainInterface MainInterfaceGlobal;
"""


GLOBALS = r"""
BattleWin g_BattleWin;
BattleWin wrong_global;
MainInterface MainInterfaceGlobal;
"""

CONTROL = r"""
static int expect(int condition, const char *message) {
  if (!condition) fprintf(stderr, "FAIL: %s\n", message);
  return condition ? 0 : 1;
}

int main() {
  int failures = 0;
  BattleWin other_receiver;
  other_receiver.pulse_timer();
  failures += expect(other_receiver.time_.calls == 1,
                     "pulse_timer makes one pulse call");
  failures += expect(other_receiver.time_.callback == &teardown_00422ec0,
                     "pulse_timer passes the directly typed callback");
  failures += expect(other_receiver.time_.param == 0
                     && other_receiver.time_.count == 0x1388
                     && other_receiver.time_.resolution == 5,
                     "pulse_timer preserves parameter, period, resolution");
  other_receiver.time_.callback(0);
  failures += expect(calls.released == static_cast<SubInterface *>(&g_BattleWin),
                     "callback releases the fixed singleton, not pulse receiver");
  calls.released = 0;
  other_receiver.time_.callback(12345);
  failures += expect(calls.released == static_cast<SubInterface *>(&g_BattleWin),
                     "callback ignores its integer parameter");

  BattleWin button_receiver;
  calls.count = 0;
  button_receiver.on_iface_button_clicked(37);
  failures += expect(calls.released == static_cast<SubInterface *>(&button_receiver),
                     "button handler releases its SubInterface base");
  failures += expect(calls.button_receiver == &MainInterfaceGlobal
                     && calls.button_id == 37,
                     "button handler forwards to MainInterface with original ID");
  failures += expect(calls.count == 2 && calls.events[0] == 1
                     && calls.events[1] == 2,
                     "release precedes the direct MainInterface callback");
  if (failures) return 1;
  puts("BattleWin typed timer actual-source controls passed");
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
@pytest.mark.parametrize(
    "mutation",
    ["baseline", "wrong_callback", "wrong_param", "wrong_period", "wrong_resolution",
     "wrong_global", "wrong_receiver", "reversed_calls"],
)
def test_typed_battlewin_callback_and_button_actual_bodies(compiler, mutation, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    cpp = (ROOT / "src/battlewin.cpp").read_text()
    callback = extract_body(cpp, "void __cdecl teardown_00422ec0(")
    button = extract_body(cpp, "void BattleWin::on_iface_button_clicked(")
    pulse = extract_body(cpp, "void BattleWin::pulse_timer(")
    assert "reinterpret_cast<SubInterface*>(this)" not in button
    assert "reinterpret_cast<void (__cdecl *)(int)>" not in pulse
    assert "time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);" in pulse

    if mutation == "wrong_callback":
        callback += "\nvoid __cdecl wrong_callback(int) {}\n"
        pulse = pulse.replace("&teardown_00422ec0", "&wrong_callback")
    elif mutation == "wrong_param":
        pulse = pulse.replace(", 0, 0x1388, 5)", ", 1, 0x1388, 5)")
    elif mutation == "wrong_period":
        pulse = pulse.replace("0x1388", "0x1389")
    elif mutation == "wrong_resolution":
        pulse = pulse.replace(", 0x1388, 5)", ", 0x1388, 6)")
    elif mutation == "wrong_global":
        callback = callback.replace("g_BattleWin", "wrong_global")
    elif mutation == "wrong_receiver":
        button = button.replace(
            "SubInterface::release_iface_mode()",
            "static_cast<SubInterface *>(&MainInterfaceGlobal)->release_iface_mode()",
        )
    elif mutation == "reversed_calls":
        first = "SubInterface::release_iface_mode();"
        second = "MainInterfaceGlobal.MainInterface::on_iface_button_clicked(button_id);"
        button = button.replace(first + "\n    " + second, second + "\n    " + first)

    prelude = PRELUDE
    translation_unit = "\n".join((prelude, GLOBALS, callback, button, pulse, CONTROL))
    passed = compile_and_run(executable, tmp_path, translation_unit, mutation)
    assert passed == (mutation == "baseline")


def test_battle_report_uses_same_direct_timer_callback_and_arguments():
    cpp = (ROOT / "src/battlewin.cpp").read_text()
    report = extract_body(cpp, "void BattleWin::battle_report(")
    assert report.count("time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);") == 1
    assert "reinterpret_cast<void (__cdecl *)(int)>" not in report


def test_old_no_argument_callback_cannot_satisfy_time_pulse_type(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is not installed")
    source = tmp_path / "wrong_callback_signature.cpp"
    source.write_text(
        "#define __cdecl\n"
        "void __cdecl callback() {}\n"
        "void pulse(void (__cdecl *)(int)) {}\n"
        "int main() { pulse(&callback); }\n"
    )
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(tmp_path / "bad")],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode != 0
