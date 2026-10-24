"""Host controls for the actual MapWin::UNK4 body and its live scan inputs."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/mapwin.cpp").is_file())
SOURCE = ROOT / "src/mapwin.cpp"
SIGNATURE = "void MapWin::UNK4(int tile_x, int tile_y)"


def actual_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                               for char in match[0]), source)
    assert masked.count(SIGNATURE) == 1
    start = masked.index(SIGNATURE)
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for position in range(opening, len(masked)):
        if masked[position] == "{":
            depth += 1
        elif masked[position] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:position + 1]
                return body
    raise AssertionError("unterminated actual MapWin::UNK4 definition")


HARNESS = r'''#include <cassert>
#include <cstdio>
#include <vector>
#define nullptr 0

class MapWin;
typedef MapWin *MapWinSlots[8];
MapWinSlots MapWinTable;
static const unsigned int MapWinTableSlots = 8;
int MapEditorPaintId;
static int climate_calls;
static int editor_calls;
static int editor_x, editor_y, editor_mask, editor_field_value;
static int expected_field_during_climate;
static MapWin *active_receiver;
static std::vector<int> draws;
static std::vector<int> events;
static bool mutate_next_slots;
static MapWin *replacement_slot_one;

void world_climate();

class MapWin {
public:
    MapWin(int identity, int active)
        : field_1DD8C_(0), view_active_(active), identity_(identity) {}

    void editor(int x, int y, int flags) {
        ++editor_calls;
        editor_x = x;
        editor_y = y;
        editor_mask = flags;
        editor_field_value = field_1DD8C_;
    }

    void draw_map(int mode) {
        assert(mode == 1);
        draws.push_back(identity_);
        events.push_back(identity_);
        if (identity_ == 0 && mutate_next_slots) {
            MapWinTable[1] = replacement_slot_one;
            MapWinTable[2]->view_active_ = 1;
        }
    }

    void UNK4(int tile_x, int tile_y);

    int field_1DD8C_;
    int view_active_;
    int identity_;
};

void world_climate() {
    ++climate_calls;
    events.push_back(100);
    assert(active_receiver != 0);
    assert(active_receiver->field_1DD8C_ == expected_field_during_climate);
}
'''


def program(body: str, main: str) -> str:
    return (HARNESS + "\n" + body + "\n" + main)


def run_program(tmp_path: Path, compiler: str, source: str,
                label: str) -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    cpp.write_text(source, encoding="utf-8")

    def disable_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    built = subprocess.run(
        [executable, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)],
        cwd=tmp_path, capture_output=True, text=True, timeout=45,
        preexec_fn=disable_core_dumps)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], cwd=tmp_path, capture_output=True,
                          text=True, timeout=10, preexec_fn=disable_core_dumps)


POSITIVE_MAIN = r'''
int main() {
    MapWin primary(0, 0), inactive(1, 0), active(2, 1);
    MapWinTable[0] = &primary;
    MapWinTable[1] = &inactive;
    MapWinTable[2] = &active;
    MapWinTable[3] = 0;
    MapWinTable[4] = 0;
    MapWinTable[5] = 0;
    MapWinTable[6] = 0;
    MapWinTable[7] = 0;
    active_receiver = &primary;

    const int paints[] = { 0, 2, 0x12, 1, 3 };
    for (int i = 0; i < 5; ++i) {
        MapEditorPaintId = paints[i];
        primary.field_1DD8C_ = 0xA6;
        expected_field_during_climate = 0;
        draws.clear();
        const int old_climate_calls = climate_calls;
        const int old_editor_calls = editor_calls;
        primary.UNK4(17 + i, -9 - i);
        assert(editor_calls == old_editor_calls + 1);
        assert(editor_x == 17 + i && editor_y == -9 - i);
        assert(editor_mask == 2 && editor_field_value == 0xA6);
        assert(primary.field_1DD8C_ == 0);
        assert(climate_calls == old_climate_calls +
               ((paints[i] == 0 || paints[i] == 2 || paints[i] == 0x12) ? 1 : 0));
        assert(draws.size() == 2U && draws[0] == 0 && draws[1] == 2);
    }

    // Each iteration reloads its slot after the earlier draw callback, and
    // reloads the active flag when it reaches the secondary entry.
    MapWin replacement(9, 1);
    replacement_slot_one = &replacement;
    const int dynamic_paints[] = { 1, 0 };
    for (int i = 0; i < 2; ++i) {
        MapWinTable[0] = &primary;
        MapWinTable[1] = &inactive;
        MapWinTable[2] = &active;
        MapWinTable[3] = 0;
        MapWinTable[4] = 0;
        MapWinTable[5] = 0;
        MapWinTable[6] = 0;
        MapWinTable[7] = 0;
        active.view_active_ = 0;
        mutate_next_slots = true;
        MapEditorPaintId = dynamic_paints[i];
        primary.field_1DD8C_ = 2;
        expected_field_during_climate = 0;
        draws.clear();
        events.clear();
        const int old_climate_calls = climate_calls;
        primary.UNK4(70 + i, 80 + i);
        assert(climate_calls == old_climate_calls + (i == 1 ? 1 : 0));
        assert(draws.size() == 3U);
        assert(draws[0] == 0 && draws[1] == 9 && draws[2] == 2);
        if (i == 0) {
            assert(events.size() == 3U);
            assert(events[0] == 0 && events[1] == 9 && events[2] == 2);
        } else {
            assert(events.size() == 4U);
            assert(events[0] == 100 && events[1] == 0 &&
                   events[2] == 9 && events[3] == 2);
        }
    }

    // A null slot zero takes the null guard before the slot-zero bypass.
    MapWinTable[0] = 0;
    MapWinTable[1] = &inactive;
    MapWinTable[2] = 0;
    mutate_next_slots = false;
    draws.clear();
    inactive.UNK4(0, 0);
    assert(draws.empty());
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_mapwin_update_views_branch_order_and_live_scans(compiler, tmp_path):
    body = actual_body()
    positive = run_program(tmp_path / "positive", compiler,
                           program(body, POSITIVE_MAIN), "positive")
    assert positive.returncode == 0, positive.stdout + positive.stderr

    bypass_mutant = body.replace(
        "slot == 0 || window->view_active_ != 0",
        "slot != 0 && window->view_active_ != 0", 1)
    assert bypass_mutant != body
    damaged = run_program(tmp_path / "mutant", compiler,
                          program(bypass_mutant, POSITIVE_MAIN), "slot0_bypass")
    assert damaged.returncode != 0, "slot-zero-bypass damage survived actual witness"
