"""Bounded actual-source controls for BaseWin's previous/next traversal.

The fixture supplies a small, controlled Base array and collaborator spies. It
does not claim live-game reachability or prove full executable acceptance.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

import pytest
from match_recovery.source import read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basewin.cpp"
BASE = "f279d1cb887106b7fb43363089e3185b051a2f8a"
ORIGINALS = (
    "// ORIGINAL: 0x0041A010 ?prev@BaseWin@@QAEXXZ 0x0041A010-0x0041A15D",
    "// ORIGINAL: 0x0041A160 ?next@BaseWin@@QAEXXZ 0x0041A160-0x0041A2AD",
)


def method_body(source: str, method: str) -> str:
    start = source.index("void BaseWin::" + method + "()")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("BaseWin::" + method + " body is unterminated")


def current_source() -> str:
    return SOURCE.read_text(encoding="utf-8")


def omniscient_state_value() -> str:
    header = (ROOT / "src/game.h").read_text(encoding="utf-8")
    match = re.search(r"\bSTATE_OMNISCIENT_VIEW\s*=\s*(0x[0-9A-Fa-f]+)", header)
    assert match, "game-state enum value is missing from src/game.h"
    assert int(match.group(1), 16) == 0x80, "pinned omniscient-view bit is 0x80"
    return match.group(1)


def test_source_preserves_both_original_rows_and_history():
    baseline = subprocess.run(
        ["git", "show", BASE + ":src/basewin.cpp"], cwd=ROOT,
        capture_output=True, text=True, check=True,
    ).stdout
    current = current_source()
    for marker in ORIGINALS:
        assert marker in baseline and marker in current
        old = {row.address: row for row in read_text(baseline, SOURCE)}
        new = {row.address: row for row in read_file(SOURCE)}
        address = int(marker.split("0x")[1][:8], 16)
        assert address in old and address in new
        for field in ("address", "name", "symbol", "image_spans", "byte_exact",
                      "levers", "ruled_out", "deferred", "unrecoverable"):
            assert getattr(new[address], field) == getattr(old[address], field), field


@pytest.mark.parametrize("method", ["prev", "next"])
def test_method_retains_typed_array_access_and_per_iteration_focus_reads(method):
    body = method_body(current_source(), method)
    loop_at = body.index("for (")
    loop = body[loop_at:]
    assert "candidate < GameDataStateStorage.fields_.base_current_count_" in loop
    assert "candidate == static_cast<int>(focusedBaseID_)" in loop
    assert "Bases[candidate].faction_id_current" in loop
    assert "Bases[focusedBaseID_].faction_id_current" in loop
    assert "Bases[candidate].name_string" in loop
    assert "Bases[nearest].name_string" in loop
    assert "Bases[selected].name_string" in loop


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef int BOOL;
#define nullptr 0
#define STATE_OMNISCIENT_VIEW @STATE_OMNISCIENT_VIEW@

struct Base {
    char prefix[4];
    uint8_t faction_id_current;
    char before_name[14];
    char name_string[25];
    char tail[0x134 - 0x2c];
};
typedef char base_size_must_match[(sizeof(Base) == 0x134) ? 1 : -1];
static Base Bases[16];
static Base *BaseCurrent;
struct ConsoleState { int faction_id_; };
struct GameFields { int base_current_count_; uint32_t game_state_; };
struct GameState { GameFields fields_; };
static ConsoleState ConsoleGlobal;
static GameState GameDataStateStorage;
static int ConsoleControlTurnActive;

struct Event { char kind; int a; int b; int observed_focus; };
static Event events[128];
static int event_count;
static void record(char kind, int a, int b, int focus) {
    assert(event_count < 128);
    events[event_count].kind = kind;
    events[event_count].a = a;
    events[event_count].b = b;
    events[event_count].observed_focus = focus;
    ++event_count;
}

static int _strcmpi(const char *left, const char *right) {
    while (*left && *right) {
        int a = tolower((unsigned char)*left++);
        int b = tolower((unsigned char)*right++);
        if (a != b) return a - b;
    }
    return tolower((unsigned char)*left) - tolower((unsigned char)*right);
}

class BaseWin {
public:
    uint32_t focusedBaseID_;
    void prev();
    void next();
    void set_base(int id) {
        record('S', id, 0, (int)focusedBaseID_);
        BaseCurrent = &Bases[id];
    }
    void X_pop(const char *label, void *) {
        assert(strcmp(label, "ARROWUPKEEP") == 0);
        record('P', 0, 0, (int)focusedBaseID_);
    }
    void zoom(int id, int wait) {
        record('Z', id, wait, (int)focusedBaseID_);
    }
};

static void reset_case(BaseWin &window, int focus, int count,
                       int console_faction, int upkeep, uint32_t game_state) {
    event_count = 0;
    window.focusedBaseID_ = (uint32_t)focus;
    GameDataStateStorage.fields_.base_current_count_ = count;
    GameDataStateStorage.fields_.game_state_ = game_state;
    ConsoleGlobal.faction_id_ = console_faction;
    ConsoleControlTurnActive = upkeep;
    BaseCurrent = &Bases[focus];
}
static void set_base_row(int index, const char *name, int faction) {
    strncpy(Bases[index].name_string, name, sizeof(Bases[index].name_string) - 1);
    Bases[index].name_string[sizeof(Bases[index].name_string) - 1] = '\0';
    Bases[index].faction_id_current = (uint8_t)faction;
}
static void expect_event(int at, char kind, int a, int b, int focus) {
    assert(at < event_count);
    if (events[at].kind != kind || events[at].a != a || events[at].b != b ||
        events[at].observed_focus != focus) {
        fprintf(stderr, "event %d got %c(%d,%d) focus=%d wanted %c(%d,%d) focus=%d\n",
                at, events[at].kind, events[at].a, events[at].b,
                events[at].observed_focus, kind, a, b, focus);
        assert(0);
    }
}
static void expect_normal(int selected, int old_focus) {
    assert(event_count == 2);
    expect_event(0, 'S', old_focus, 0, old_focus);
    expect_event(1, 'Z', selected, 0, selected);
}
'''


SUFFIX = r'''
int main() {
    BaseWin window;
    int i;
    set_base_row(0, "Alpha", 1);
    set_base_row(1, "bravo", 1);
    set_base_row(2, "Current", 1);
    set_base_row(3, "Delta", 1);
    set_base_row(4, "echo", 1);
    set_base_row(5, "ECHO", 1);
    set_base_row(6, "charlie", 2);
    set_base_row(7, "Zulu", 1);

    // Previous chooses the closest lower same-faction name, ignoring the
    // foreign-faction "charlie" row and the case-insensitive ECHO tie.
    reset_case(window, 2, 8, 1, 0, 0);
    window.prev();
    assert((int)window.focusedBaseID_ == 1);
    expect_normal(1, 2);

    // Next chooses the closest greater name, preserving the first duplicate
    // on case-insensitive equality.
    reset_case(window, 2, 8, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 3);
    expect_normal(3, 2);

    // With Delta moved past both ECHO spellings, equal nearest names retain
    // the first matching Base row encountered by the original strict compare.
    strcpy(Bases[3].name_string, "Foxtrot");
    reset_case(window, 2, 8, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 4);
    expect_normal(4, 2);
    strcpy(Bases[3].name_string, "Delta");

    // The foreign "charlie" row would be the closest greater name after
    // bravo if candidate faction filtering were absent; Current is the next
    // same-faction candidate.
    reset_case(window, 1, 8, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 2);
    expect_normal(2, 1);

    // At either lexical edge, wrap to the opposite end of the same faction.
    reset_case(window, 0, 8, 1, 0, 0);
    window.prev();
    assert((int)window.focusedBaseID_ == 7);
    expect_normal(7, 0);
    strcpy(Bases[7].name_string, "eCho");
    reset_case(window, 0, 8, 1, 0, 0);
    window.prev();
    assert((int)window.focusedBaseID_ == 4);
    expect_normal(4, 0);
    strcpy(Bases[7].name_string, "Zulu");
    reset_case(window, 7, 8, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 0);
    expect_normal(0, 7);
    reset_case(window, 7, 8, 1, 0, 0);
    window.prev();
    assert((int)window.focusedBaseID_ == 4);
    expect_normal(4, 7);

    // Names equal to the focus are not directional candidates; next proceeds
    // to Zulu, while previous from ECHO chooses Delta.
    reset_case(window, 4, 8, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 7);
    expect_normal(7, 4);
    reset_case(window, 5, 8, 1, 0, 0);
    window.prev();
    assert((int)window.focusedBaseID_ == 3);
    expect_normal(3, 5);

    // One-base traversal keeps its selection but still performs zoom.
    reset_case(window, 0, 1, 1, 0, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 0);
    expect_normal(0, 0);

    // Foreign current base is refreshed first, then denied without selection.
    reset_case(window, 2, 8, 1, 0, 0);
    Bases[2].faction_id_current = 2;
    window.prev();
    assert((int)window.focusedBaseID_ == 2);
    assert(event_count == 1);
    expect_event(0, 'S', 2, 0, 2);
    Bases[2].faction_id_current = 1;

    // Omniscient view permits the same foreign current-base traversal.
    reset_case(window, 2, 8, 1, 0, STATE_OMNISCIENT_VIEW);
    Bases[2].faction_id_current = 2;
    strcpy(Bases[2].name_string, "delta");
    window.prev();
    assert((int)window.focusedBaseID_ == 6);
    expect_normal(6, 2);
    Bases[2].faction_id_current = 1;
    strcpy(Bases[2].name_string, "Current");

    // Upkeep exits after initial set_base and ARROWUPKEEP pop, before zoom.
    reset_case(window, 2, 8, 1, 1, 0);
    window.next();
    assert((int)window.focusedBaseID_ == 2);
    assert(event_count == 2);
    expect_event(0, 'S', 2, 0, 2);
    expect_event(1, 'P', 0, 0, 2);

    // The eight-row table is initialized without changing unrelated rows.
    for (i = 8; i < 16; ++i) Bases[i].faction_id_current = 0;
    puts("BaseWin pointer traversal actual-source controls passed");
    return 0;
}
'''


def compile_and_run(body: str, companion: str, compiler: str):
    prelude = PRELUDE.replace("@STATE_OMNISCIENT_VIEW@", omniscient_state_value())
    program = prelude + body + "\n" + companion + SUFFIX
    with tempfile.TemporaryDirectory(prefix="basewin-pointer-traversal-") as temp:
        cpp = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        cpp.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def damage_mutants(body: str):
    # These anchors express semantic faults independently of pointer/index
    # spelling. Each mutant must compile and fail the same behavior oracle.
    direction = re.search(r"(comparison\s*)([<>])(\s*0)", body)
    assert direction, "direction comparison anchor missing from actual body"
    reverse = body[:direction.start(2)] + (">" if direction.group(2) == "<" else "<") + body[direction.end(2):]

    faction = re.search(
        r"\|\|\s*[^;{}]*?faction_id_current\s*!=\s*[^;{}]*?faction_id_current",
        body, re.S,
    )
    assert faction, "same-faction filter anchor missing from actual body"
    no_filter = body[:faction.start()] + body[faction.end():]

    wrap = re.search(r"if\s*\(\s*nearest\s*>=\s*0\s*\)", body)
    assert wrap, "nearest-versus-wrap selection anchor missing from actual body"
    wrong_wrap = body[:wrap.start()] + "if (nearest < 0)" + body[wrap.end():]
    return {"reversed name comparison": reverse,
            "removed faction filter": no_filter,
            "incorrect wrap choice": wrong_wrap}


@pytest.mark.parametrize("method", ["prev", "next"])
def test_actual_method_body_and_damage_controls(method):
    source = current_source()
    body = method_body(source, method)
    companion = method_body(source, "next" if method == "prev" else "prev")
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    for compiler in compilers:
        good = compile_and_run(body, companion, compiler)
        assert good.returncode == 0, compiler + ": " + good.stdout + good.stderr
        assert "controls passed" in good.stdout
        for label, mutant in damage_mutants(body).items():
            failed = compile_and_run(mutant, companion, compiler)
            assert failed.returncode != 0, compiler + ": " + label + " mutant survived"


def test_native_actual_methods_and_damage_controls(tmp_path):
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native compilation requires the coordinator resource lease")
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    settings = Measurement.read(ROOT)

    def build(name, flags, body, companion):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        prelude = PRELUDE.replace("@STATE_OMNISCIENT_VIEW@", omniscient_state_value())
        source.write_text(prelude + body + "\n" + companion + SUFFIX, encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo", "/subsystem:console",
             "/incremental:no", "/out:" + str(exe), str(obj), "kernel32.lib"],
            cwd=tmp_path, timeout=60,
        )
        assert linked.returncode == 0, linked.stdout + linked.stderr
        return exe

    def run(exe):
        with tempfile.TemporaryFile() as output:
            result = subprocess.run(["wine32", str(exe)], env=environment(), stdout=output,
                                    stderr=output, timeout=30)
            output.seek(0)
            return result.returncode, output.read().decode(errors="replace")

    source = current_source()
    for method in ("prev", "next"):
        body = method_body(source, method)
        companion = method_body(source, "next" if method == "prev" else "prev")
        for profile, flags in enumerate(settings.flag_sets):
            code, output = run(build("basewin-%s-good-%d" % (method, profile), flags,
                                     body, companion))
            assert code == 0, "native %s profile %d: %s" % (method, profile, output)
            for mutant_index, (label, mutant) in enumerate(damage_mutants(body).items()):
                code, output = run(build("basewin-%s-bad-%d-%d" %
                                         (method, profile, mutant_index), flags,
                                         mutant, companion))
                assert code != 0, "native %s %s mutant survived: %s" % (method, label, output)
