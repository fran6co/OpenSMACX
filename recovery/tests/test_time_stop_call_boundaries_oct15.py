"""The public stop entry is out of line; its shared cancellation core stays inline."""

from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start:offset + 1]
    raise AssertionError(f"unterminated definition {signature!r}")


def actual_parts():
    header = (ROOT / "src/time.h").read_text()
    source = (ROOT / "src/time.cpp").read_text()
    lifecycle = (ROOT / "src/time_lifecycle.cpp").read_text()
    helper = definition(header, "void stop_pending_events()")
    wrapper = definition(source, "void Time::stop()")
    return header, source, lifecycle, helper, wrapper


def test_mapped_stop_is_a_wrapper_and_internal_paths_keep_the_inline_core():
    header, source, lifecycle, _, wrapper = actual_parts()
    assert "MEASURED void stop(); // 00616730" in header
    assert "MEASURED void stop() {" not in header
    assert wrapper == "void Time::stop() {\n    stop_pending_events();\n}"
    assert source.count("stop_pending_events();") == 7  # six setup paths and public wrapper
    close_impl = definition(lifecycle, "inline void Time::close_impl()")
    assert close_impl.count("stop_pending_events();") == 1
    assert close_impl.count("stop();") == 0
    private = header.index(" private:")
    helper_start = header.index("void stop_pending_events()", private)
    assert helper_start < header.index("// ONE-SHOT EMULATION", helper_start)
    row, = (item for item in read_file(ROOT / "src/time.cpp") if item.address == 0x616730)
    assert row.body == "src/time.cpp" and row.byte_exact
    assert row.name == "?stop@Time@@QAEXXZ"


HARNESS = r'''#include <cassert>
using UINT_PTR = unsigned int;
using HWND = void*;
static const int TimeOneShot = 1;
static HWND HandleMain = reinterpret_cast<HWND>(0x1234);
struct Time;
static Time* active;
static bool expected_flush;
static int events[4];
static int event_count;
int timeKillEvent(UINT_PTR);
int KillTimer(HWND, UINT_PTR);
void flush_timer();

struct Time {
  int oneshot_state_;
  UINT_PTR id_event_;
  unsigned int count_;
  int resolution_;
  int tick_posted_;
  int cb_param1_;
  void* cb_param2_;
  void (*callback1_)(int);
  void (*callback2_)(int, void*);
  void stop();
  HELPER
};
WRAPPER

int timeKillEvent(UINT_PTR id) {
  assert(active->id_event_ == 17 && id == 17);
  events[event_count++] = 1;
  active->oneshot_state_ ^= TimeOneShot;
  return 0;
}
int KillTimer(HWND window, UINT_PTR id) {
  assert(window == HandleMain && active->id_event_ == 17 && id == 17);
  events[event_count++] = 2;
  active->oneshot_state_ ^= TimeOneShot;
  return 1;
}
void flush_timer() {
  assert(expected_flush);
  assert(active->id_event_ == 0);
  events[event_count++] = 3;
}
int main() {
  Time timer = {};
  active = &timer;
  timer.oneshot_state_ = 0;
  timer.id_event_ = 17;
  timer.count_ = 50;
  timer.resolution_ = 71;
  timer.tick_posted_ = 19;
  timer.cb_param1_ = 42;
  timer.cb_param2_ = &timer;
  expected_flush = true;
  timer.stop();
  assert(event_count == 2 && events[0] == 2 && events[1] == 3);
  assert(timer.id_event_ == 0);
  assert(timer.resolution_ == 71 && timer.tick_posted_ == 19);
  return 0;
}
'''


def compile_and_run(tmp_path: Path, compiler: str, helper: str, wrapper: str):
    source = tmp_path / "time_stop.cpp"
    executable = tmp_path / "time_stop"
    rendered = HARNESS.replace("HELPER", helper).replace("WRAPPER", wrapper)
    source.write_text(rendered)
    compile_result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Werror", str(source), "-o", str(executable)],
        capture_output=True, text=True, check=False)
    assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, check=False)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["late flush snapshot", "wrong 50ms boundary", "uncleared timer id"])
def test_actual_source_core_rejects_compiling_behavior_mutants(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    _, _, _, actual_helper, wrapper = actual_parts()
    baseline = compile_and_run(tmp_path, compiler, actual_helper, wrapper)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    if damage == "late flush snapshot":
        needle = "const int had_flush = ~oneshot_state_ & TimeOneShot;"
        assert actual_helper.count(needle) == 1
        mutant = actual_helper.replace(needle, "int had_flush = 0;", 1)
        needle = "    if (had_flush) {"
        assert mutant.count(needle) == 1
        mutant = mutant.replace(
            needle,
            "    had_flush = ~oneshot_state_ & TimeOneShot;\n" + needle,
            1)
    elif damage == "wrong 50ms boundary":
        needle = "count_ < 50"
        assert actual_helper.count(needle) == 1
        mutant = actual_helper.replace(needle, "count_ <= 50", 1)
    else:
        needle = "id_event_ = 0;"
        assert actual_helper.count(needle) == 1
        mutant = actual_helper.replace(needle, "id_event_ = 23;", 1)

    assert mutant != actual_helper
    damaged = compile_and_run(tmp_path, compiler, mutant, wrapper)
    assert damaged.returncode != 0, f"actual-source controls accepted {damage} mutant"
