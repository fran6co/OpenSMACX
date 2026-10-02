"""Service-double controls for the actual 0x476A50 monument trigger.

This compiles the definition extracted from src/monuwin.cpp. It checks function-level
control flow and source-owned indexing only; it is not UI, ABI, or game-runtime
proof.
"""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/monuwin.cpp"


def braced(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated monument definition")


def actual_definition():
    text = PRODUCT.read_text()
    rows = [row for row in read_file(PRODUCT) if row.address == 0x476A50]
    assert len(rows) == 1, "expected one source mapping for native 0x476A50"
    assert rows[0].image_spans[0][0] == 0x476A50
    match = re.search(r"void\s+__cdecl\s+monument\s*\(\s*int\s+from_load\s*\)", text)
    assert match, "actual monument(int) definition not found"
    return braced(text, match.start())


HARNESS = r"""
#include <cstdio>
#include <cstdlib>
#define __cdecl
struct ConsoleInfo { int faction_id_; } ConsoleGlobal;
struct Player { int rule_flags; } Players[8];
struct MonumentRecord { int achieved_; };
struct MonumentFaction { MonumentRecord records_[21]; } FactionMonuments[8];
int IsMultiplayerNet, GameMorePreferences, GameState, MonumentWindowCount;
int init_result;
bool first_event = true;
void event(const char *name, int a=0, int b=0, int c=0, int d=0) {
    if (!first_event) std::putchar(',');
    first_event = false;
    std::printf("[\"%s\",%d,%d,%d,%d]", name, a, b, c, d);
}
struct GraphicWin { void close() { event("base_close"); } };
struct MonuWin : GraphicWin {
    int selection_pending_;
    MonuWin() : selection_pending_(43) {}
    int init() { event("init", MonumentWindowCount); return init_result; }
    void set_modal(int result, int (*poll)(), void *owner) {
        event("modal", result, poll != 0, owner != 0, selection_pending_);
    }
} MonumentWindowGlobal;
void do_all_draws() { event("draw_all"); }
// ACTUAL DEFINITION
int main(int argc, char **argv) {
    if (argc != 11) return 2;
    const int faction = std::atoi(argv[1]);
    ConsoleGlobal.faction_id_ = faction;
    const int from_load = std::atoi(argv[2]);
    Players[faction].rule_flags = std::atoi(argv[3]);
    IsMultiplayerNet = std::atoi(argv[4]);
    GameMorePreferences = std::atoi(argv[5]);
    GameState = std::atoi(argv[6]);
    const unsigned current = (unsigned)std::strtoul(argv[7], 0, 0);
    const unsigned peer0 = (unsigned)std::strtoul(argv[8], 0, 0);
    const unsigned peer7 = (unsigned)std::strtoul(argv[9], 0, 0);
    init_result = std::atoi(argv[10]);
    MonumentWindowCount = 42;
    for (int f=0; f<8; ++f) {
        unsigned mask = f == faction ? current : f == 0 ? peer0 : f == 7 ? peer7 : 0;
        for (int i=0; i<21; ++i)
            FactionMonuments[f].records_[i].achieved_ = ((mask >> i) & 1u) != 0;
    }
    std::printf("{\"events\":[");
    monument(from_load);
    std::printf("],\"count\":%d,\"selection\":%d}\n",
                MonumentWindowCount, MonumentWindowGlobal.selection_pending_);
    return 0;
}
"""


def compile_actual(tmp_path):
    compiler = os.environ.get("CXX") or shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("GCC/Clang host compiler unavailable")
    source = HARNESS.replace("// ACTUAL DEFINITION", actual_definition())
    cpp, exe = tmp_path / "monument.cpp", tmp_path / "monument"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    return exe


def run(exe, *, faction=3, from_load=0, flags=0, net=0, prefs=0, state=0,
        current=0, peer0=0, peer7=0, init_result=0):
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    result = subprocess.run([str(exe), str(faction), str(from_load), str(flags), str(net),
                             str(prefs), str(state), hex(current), hex(peer0), hex(peer7),
                             str(init_result)], check=True, capture_output=True, text=True,
                            env=env, timeout=10)
    return json.loads(result.stdout)


@pytest.fixture(scope="module")
def executable(tmp_path_factory):
    return compile_actual(tmp_path_factory.mktemp("monuwin-trigger"))


def test_trigger_gates_preserve_early_state(executable):
    for kwargs in ({"flags": 0x80}, {"net": 1},
                   {"from_load": 1, "prefs": 0x1000000},
                   {"from_load": 1, "state": 0x10},
                   {"from_load": 1, "state": 0x9}):
        out = run(executable, current=0x7FFF, **kwargs)
        assert out == {"events": [], "count": 42, "selection": 43}
    # Load-only restrictions do not suppress a normal (non-load) call.
    out = run(executable, from_load=0, prefs=0x1000000, state=0x19, current=1)
    assert [row[0] for row in out["events"]] == ["init", "modal", "base_close", "draw_all"]


def test_only_first_fifteen_records_of_selected_faction_count(executable):
    # Bits 15..20 deliberately set; none contributes to the native 15-entry scan.
    out = run(executable, faction=3, current=0x7FFF | (0x3F << 15),
              peer0=0x7FFF, peer7=0x7FFF)
    assert out["count"] == 15
    assert out["events"][0] == ["init", 15, 0, 0, 0]
    assert out["selection"] == 0
    peers = run(executable, faction=3, current=0x5, peer0=0x7FFF, peer7=0x7FFF)
    assert peers["count"] == 2, "records must come from the selected faction only"


def test_empty_count_and_init_failure_do_not_show_modal(executable):
    empty = run(executable, current=0x3F8000, peer0=0x7FFF, peer7=0x7FFF)
    assert empty == {"events": [], "count": 0, "selection": 43}
    failed = run(executable, current=0x15, init_result=5, from_load=1)
    assert failed["events"] == [["init", 3, 0, 0, 0]]
    assert failed["count"] == 3 and failed["selection"] == 43


def test_success_sets_load_selection_then_modal_close_and_draw(executable):
    out = run(executable, faction=3, current=0x45, peer0=0x7FFF, peer7=0x7FFF,
              from_load=1)
    assert out["count"] == 3 and out["selection"] == 1
    assert out["events"] == [["init", 3, 0, 0, 0], ["modal", 0, 0, 0, 1],
                              ["base_close", 0, 0, 0, 0], ["draw_all", 0, 0, 0, 0]]
