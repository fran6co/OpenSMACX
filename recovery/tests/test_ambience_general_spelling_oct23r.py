"""Compile the literal MA/SA ambience general definitions with narrow doubles.

These bounded host checks exercise state transitions and callback timing. They
do not claim the game's ABI or reproduce the full application runtime.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


HERE = Path(__file__).parent
ROOT = next(parent for parent in HERE.parents if (parent / "src").is_dir())


def extract(source, signature):
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
    raise AssertionError("unterminated function: " + signature)


HARNESS = r"""
#include <cassert>
#include <vector>

typedef unsigned int DWORD;
#define WINAPI

struct Call { int kind, a, b; };
static std::vector<Call> calls;
static void record(int kind, int a, int b) {
    Call call = {kind, a, b};
    calls.push_back(call);
}

static DWORD mock_now = 0;
static int clock_calls = 0;
DWORD WINAPI timeGetTime() { ++clock_calls; return mock_now; }

static int playing = 0;
int is_trackset_playing(int) { return playing; }

static int AmbienceClockArmed = 1;
static int AmbienceClock = 0;
static int MAmbiencePass = 0;
static int MAmbienceStep = 0;
static int MAmbienceCents = 0;
static int MAmbienceStepBase = 0;
static int SAmbiencePass0 = 0;
static int SAmbiencePass1 = 0;
static int SAmbienceCents = 0;
static int AmbienceGeneralTick = 0;

class MAmbience {
public:
    int field_70_;
    void xpose_trackset(int, int);
    void play_trackset(int);
    void general();
};

class SAmbience {
public:
    void xpose_trackset(int, int);
    void play_trackset(int);
    void general();
};

static int mutate_tick_on_first_xpose = 0;
static int sa_xpose_calls = 0;
void MAmbience::xpose_trackset(int track, int cents) {
    record(1, track, cents);
}
void MAmbience::play_trackset(int track) {
    record(2, track, 0);
}
void SAmbience::xpose_trackset(int track, int cents) {
    record(3, track, cents);
    ++sa_xpose_calls;
    if (mutate_tick_on_first_xpose && sa_xpose_calls == 1)
        AmbienceGeneralTick = 8;
}
void SAmbience::play_trackset(int track) {
    record(4, track, 0);
}

@@MA_GENERAL@@
@@SA_GENERAL@@

static void clear_calls() { calls.clear(); }

static void test_ma_clock_track_and_postincrement_threshold() {
    MAmbience music;
    music.field_70_ = 450;
    AmbienceClockArmed = 0;
    AmbienceClock = 0;
    MAmbiencePass = 3;
    MAmbienceStep = 7;
    MAmbienceCents = 500;
    MAmbienceStepBase = 6;
    playing = 1;
    mock_now = 100;
    clock_calls = 0;
    clear_calls();
    music.general();
    assert((AmbienceClockArmed & 1) != 0 && AmbienceClock == 100);
    assert(clock_calls == 1 && MAmbiencePass == 3 && calls.empty());

    playing = 0;
    mock_now = 5099;
    music.general();
    assert(clock_calls == 2 && MAmbiencePass == 4 && calls.empty());

    // Starting at pass 4, the increment to 5 happens before the threshold
    // check. At exactly 5000 elapsed ticks, the body runs and resets the pass.
    mock_now = 5100;
    music.general();
    assert(clock_calls == 3 && MAmbiencePass == 0);
    assert(calls.size() == 3);
    assert(calls[0].kind == 1 && calls[0].a == 7 && calls[0].b == 500);
    assert(calls[1].kind == 2 && calls[1].a == 7);
    assert(calls[2].kind == 1 && calls[2].a == 6 && calls[2].b == -700);
    assert(MAmbienceStep == 5 && MAmbienceCents == -600);
    assert(music.field_70_ == -700 && AmbienceClock == 5100);
}

static void test_sa_clamp_boundary_and_strict_pass_counters() {
    SAmbience music;
    AmbienceGeneralTick = 18;
    SAmbiencePass0 = 1;
    SAmbiencePass1 = 6;
    SAmbienceCents = 300;
    mutate_tick_on_first_xpose = 0;
    sa_xpose_calls = 0;
    clear_calls();
    music.general();
    assert(AmbienceGeneralTick == 19);
    assert(SAmbiencePass0 == 2 && SAmbiencePass1 == 7);
    assert(SAmbienceCents == 400 && calls.empty());
}

static void test_sa_clamp_callback_reload_wrap_and_tick_increment() {
    SAmbience music;
    AmbienceGeneralTick = 19;
    SAmbiencePass0 = 2;
    SAmbiencePass1 = 8;
    SAmbienceCents = 350;
    mutate_tick_on_first_xpose = 1;
    sa_xpose_calls = 0;
    clear_calls();
    music.general();
    assert(calls.size() == 4);
    assert(calls[0].kind == 3 && calls[0].a == 17 && calls[0].b == 350);
    // xpose changes the shared tick to 8; play must reload the member after it.
    assert(calls[1].kind == 4 && calls[1].a == 8);
    assert(calls[2].kind == 3 && calls[2].a == 16 && calls[2].b == 350);
    assert(calls[3].kind == 4 && calls[3].a == 16);
    assert(SAmbiencePass0 == 0 && SAmbiencePass1 == 0);
    assert(SAmbienceCents == -300 && AmbienceGeneralTick == 9);
}

int main() {
    test_ma_clock_track_and_postincrement_threshold();
    test_sa_clamp_boundary_and_strict_pass_counters();
    test_sa_clamp_callback_reload_wrap_and_tick_increment();
    return 0;
}
"""


def fixture(source):
    ma = extract(source, "void MAmbience::general()")
    sa = extract(source, "void SAmbience::general()")
    assert "int pass = ++MAmbiencePass;" in ma
    return HARNESS.replace("@@MA_GENERAL@@", ma).replace("@@SA_GENERAL@@", sa)


def compile_and_run(tmp_path, compiler, source):
    cpp = tmp_path / "ambience_general.cpp"
    executable = tmp_path / "ambience_general"
    cpp.write_text(fixture(source))
    built = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_literal_ambience_general_state_transitions_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    source = (ROOT / "src/ambience.cpp").read_text()
    run = compile_and_run(tmp_path, compiler, source)
    assert run.returncode == 0, run.stdout + run.stderr

    old_pass_threshold = source.replace(
        "elapsed >= 0x1388 && pass >= 5",
        "elapsed >= 0x1388 && pass > 5",
        1,
    )
    assert old_pass_threshold != source
    run = compile_and_run(tmp_path, compiler, old_pass_threshold)
    assert run.returncode != 0, "off-by-one MA pass threshold survived"

    cached_play_tick = source.replace(
        "xpose_trackset(AmbienceGeneralTick, SAmbienceCents);\n"
        "        play_trackset(AmbienceGeneralTick);",
        "int tick = AmbienceGeneralTick;\n"
        "        xpose_trackset(tick, SAmbienceCents);\n"
        "        play_trackset(tick);",
        1,
    )
    assert cached_play_tick != source
    run = compile_and_run(tmp_path, compiler, cached_play_tick)
    assert run.returncode != 0, "cached pre-callback SA tick survived"
