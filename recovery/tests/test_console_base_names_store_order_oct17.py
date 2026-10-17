"""Exercise the actual Console::base_names body and its original store order."""

from pathlib import Path
import resource
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, State, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"
MARKER = "void Console::base_names() {"


def _function_body(source: str) -> str:
    start = source.index(MARKER)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and following == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 2
                continue
        elif char == "\\":
            index += 2
            continue
        elif (state == "string" and char == '"') or (
                state == "character" and char == "'"):
            state = "code"
        index += 1
    raise AssertionError("unterminated Console::base_names body")


HARNESS = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Event {
    int kind;
    int argument;
    uint32_t game_value;
    uint32_t alpha_value;
};

static Event events[8];
static int event_count;

struct TrackedInt {
    uint32_t value;
    int kind;
    TrackedInt(int initial, int event_kind) : value((uint32_t)initial), kind(event_kind) {}
    operator int32_t() const { return (int32_t)value; }
    TrackedInt &operator=(int32_t next);
    TrackedInt &operator=(const TrackedInt &next);
};

struct GameFields { TrackedInt game_more_preferences_; GameFields() : game_more_preferences_(0, 1) {} };
struct GameState { GameFields fields_; };
struct IniPrefs { TrackedInt more_preferences; IniPrefs() : more_preferences(0, 2) {} };
struct Console { void base_names(); };

static GameState GameDataStateStorage;
static IniPrefs AlphaIniPrefs;

static void record_event(int kind, int argument) {
    if (event_count >= 8) {
        fprintf(stderr, "console-base-names:too-many-effects\n");
        exit(98);
    }
    events[event_count].kind = kind;
    events[event_count].argument = argument;
    events[event_count].game_value = GameDataStateStorage.fields_.game_more_preferences_.value;
    events[event_count].alpha_value = AlphaIniPrefs.more_preferences.value;
    ++event_count;
}

TrackedInt &TrackedInt::operator=(int32_t next) {
    value = (uint32_t)next;
    record_event(kind, (int)value);
    return *this;
}

TrackedInt &TrackedInt::operator=(const TrackedInt &next) {
    value = next.value;
    record_event(kind, (int)value);
    return *this;
}

static void require(bool condition, const char *label) {
    if (!condition) {
        fprintf(stderr, "console-base-names:%s\n", label);
        exit(98);
    }
}

static void prefs_save(int value) { record_event(3, value); }
static void draw_map(int value) { record_event(4, value); }

ACTUAL_BODY

static uint32_t expected_flags(uint32_t flags) {
    if (flags & 0x1000u) return flags & 0xFFFFCFFFu;
    if (flags & 0x2000u) return flags | 0x1000u;
    return flags | 0x2000u;
}

static void run_case(uint32_t input) {
    const uint32_t old_alpha = 0x13579BDFu;
    const uint32_t expected = expected_flags(input);
    event_count = 0;
    GameDataStateStorage.fields_.game_more_preferences_.value = input;
    AlphaIniPrefs.more_preferences.value = old_alpha;

    Console console;
    console.base_names();

    require(event_count == 4, "effect-count");
    require(events[0].kind == 1, "game-preference-store-first");
    require(events[0].argument == (int)expected, "game-preference-store-value");
    require(events[0].game_value == expected && events[0].alpha_value == old_alpha,
            "game-store-snapshot");
    require(events[1].kind == 2, "alpha-preference-store-second");
    require(events[1].argument == (int)expected, "alpha-preference-store-value");
    require(events[1].game_value == expected && events[1].alpha_value == expected,
            "alpha-store-snapshot");
    require(events[2].kind == 3 && events[2].argument == 0, "prefs-save-argument-order");
    require(events[2].game_value == expected && events[2].alpha_value == expected,
            "prefs-save-after-stores");
    require(events[3].kind == 4 && events[3].argument == 1, "draw-map-argument-order");
    require(events[3].game_value == expected && events[3].alpha_value == expected,
            "draw-map-after-save");
    require(GameDataStateStorage.fields_.game_more_preferences_.value == expected &&
            AlphaIniPrefs.more_preferences.value == expected, "final-values");
}

int main() {
    run_case(0x00000000u);
    run_case(0x00001000u);
    run_case(0x00002000u);
    run_case(0x00003000u);
    run_case(0xFFFFFFFFu);
    run_case(0x80002005u);
    run_case(0x80000005u);
    return 0;
}
'''


def _core_disabled():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def _compile_run(tmp_path: Path, body: str, compiler: str, expected_failure=None):
    source = tmp_path / (Path(compiler).name.replace("+", "p") + ".cpp")
    executable = tmp_path / (source.stem + ".exe")
    source.write_text(HARNESS.replace("ACTUAL_BODY", body), encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30, preexec_fn=_core_disabled,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=10, preexec_fn=_core_disabled,
    )
    if expected_failure is None:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode == 98, result.stdout + result.stderr
        assert "console-base-names:" + expected_failure in result.stderr


def _compilers():
    return [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


def test_actual_console_base_names_preserves_store_and_call_order(tmp_path):
    rows = [row for row in read_file(SOURCE) if row.address == 0x515600]
    row, = rows
    assert row.name == "?base_names@Console@@QAEXXZ"
    assert row.mode is Mode.FILE and row.state is State.IMPLEMENTED
    assert row.image_spans == ((0x515600, 0x515638),)

    body = _function_body(SOURCE.read_text(encoding="utf-8"))
    compilers = _compilers()
    if not compilers:
        pytest.skip("host g++ or clang++ required")
    for compiler in compilers:
        _compile_run(tmp_path, body, compiler)


@pytest.mark.parametrize(("damage", "diagnostic"), [
    ("swap-stores", "game-preference-store-first"),
    ("chain-stores", "game-preference-store-first"),
])
def test_actual_source_controls_reject_store_order_regressions(tmp_path, damage, diagnostic):
    source = SOURCE.read_text(encoding="utf-8")
    body = _function_body(source)
    ordered = (
        "    GameDataStateStorage.fields_.game_more_preferences_ = new_flags;\n"
        "    AlphaIniPrefs.more_preferences = new_flags;"
    )
    assert body.count(ordered) == 1
    if damage == "swap-stores":
        replacement = (
            "    AlphaIniPrefs.more_preferences = new_flags;\n"
            "    GameDataStateStorage.fields_.game_more_preferences_ = new_flags;"
        )
    else:
        replacement = (
            "    GameDataStateStorage.fields_.game_more_preferences_ = "
            "AlphaIniPrefs.more_preferences = new_flags;"
        )
    mutated = body.replace(ordered, replacement, 1)
    compilers = _compilers()
    if not compilers:
        pytest.skip("host g++ or clang++ required")
    for compiler in compilers:
        _compile_run(tmp_path, mutated, compiler, expected_failure=diagnostic)
