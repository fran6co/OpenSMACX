"""Actual vehicle-renderer opening gates; not full rendering or native ABI proof."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2])).resolve()
SOURCE = ROOT / "src" / "mapwin.cpp"
SIGNATURE = "void MapWin::draw_vehicles(int tile_x, int tile_y, int pixel_x, int pixel_y,"


def opening_block() -> str:
    source = SOURCE.read_text()
    start = source.index(SIGNATURE)
    body = source.index("{", start) + 1
    end = source.index("const int half_x = tile_x >> 1;", body)
    end = source.index(";", end) + 1
    return source[body:end]


def actual_method() -> str:
    source = SOURCE.read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated actual MapWin::draw_vehicles")


FULL_PRELUDE = r"""
#include <stdint.h>
#include <cassert>
#include <cstdlib>

enum { STATE_OMNISCIENT_VIEW = 1 };
struct GameDataStateFieldsProjection {
    uint32_t game_rules_;
    uint32_t game_state_;
    struct { struct { uint32_t human_mask_; } bytes_; } lock_masks_;
    uint32_t game_more_preferences_;
};
struct GameDataStateProjection { GameDataStateFieldsProjection fields_; };
static GameDataStateProjection GameDataStateStorage;
static int MapBaseDisplayOverride;
static int ControlTurnPhase;
struct PlayerDataProjection { uint32_t flags; };
static PlayerDataProjection PlayersData[8];
struct Map { uint32_t visibility, bit, val2; };
static Map tile;
static Map *map_loc(int, int) { return &tile; }
static void pixel_to_tile(int, int, int *x, int *y) { *x = 0; *y = 0; }
struct Veh {
    int x, y, proto_id, next_veh_id_stack, home_base_id, faction_id;
    uint32_t flags, visibility, state;
    int order;
};
static Veh Vehs[8];
struct VehiclePrototype { int chassis_id; };
static VehiclePrototype VehPrototypes[8];
static int VehDrawSuppressedVehicle = -1;
static int VehDrawSelection689F28 = -1;
static int VehDrawSelection689F2C = -1;
struct ConsoleProjection {
    int selected_vehicle_, cursor_active_, field_23BF8_, field_23BE4_;
    int survey_mode_, faction_id_;
};
static ConsoleProjection ConsoleGlobal;
static int IsMultiplayerNet, ConsoleControlTurnActive;
static int MapIsFlat = 1, MapLongitude = 64, MapLongitudeBounds = 64;
struct TutWinBaseProjection { int focused_base_id() { return 0; } };
static TutWinBaseProjection TutWinBaseWindow;
static int draw_base_calls, veh_at_calls, vehicle_draw_calls, last_vehicle_drawn;
class MapWin;
static bool mutate_base_flags, mutate_first_veh_at_flags;
static uint32_t base_flags_after_callback;
class Buffer {};
class Win { public: void get_mouse_pos(int *x, int *y) { *x = *y = 0; } };
class MapWin : public Win, public Buffer {
public:
    uint32_t field_1DD70_;
    int zoom_offset_;
    MapWin() : field_1DD70_(0), zoom_offset_(1) {}
    void draw_bases(int, int, int, int, int) {
        ++draw_base_calls;
        if (mutate_base_flags) field_1DD70_ = base_flags_after_callback;
    }
    void draw_vehicles(int tile_x, int tile_y, int pixel_x, int pixel_y,
                       int faction_id);
};
static int veh_at(int, int) {
    ++veh_at_calls;
    if (mutate_first_veh_at_flags && veh_at_calls == 1)
        static_cast<MapWin *>(0)->field_1DD70_ = 0;
    return 3;
}
static int veh_selectable(int) { return 1; }
static int veh_ready(int) { return 1; }
static void veh_draw(Buffer *, int id, int, int, int, int, int) {
    ++vehicle_draw_calls;
    last_vehicle_drawn = id;
}
"""


def full_harness(method: str) -> str:
    # The callback writes through the actual fixture MapWin instance. Keep that
    # receiver explicit so the callback changes the same field the body reloads.
    prelude = FULL_PRELUDE.replace(
        "static bool mutate_base_flags, mutate_first_veh_at_flags;",
        "static bool mutate_base_flags, mutate_first_veh_at_flags;\nstatic MapWin *active_window;"
    ).replace(
        "static_cast<MapWin *>(0)->field_1DD70_ = 0;",
        "active_window->field_1DD70_ &= ~0x00200000U;"
    )
    return prelude + "\n" + method + r"""
static void reset_case(MapWin &window, uint32_t flags) {
    window.field_1DD70_ = flags;
    active_window = &window;
    window.zoom_offset_ = 1;
    MapBaseDisplayOverride = 1;
    ControlTurnPhase = 0;
    GameDataStateStorage.fields_.game_rules_ = 0;
    GameDataStateStorage.fields_.game_state_ = STATE_OMNISCIENT_VIEW;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    ConsoleGlobal.selected_vehicle_ = 0;
    ConsoleGlobal.cursor_active_ = 0;
    ConsoleGlobal.field_23BF8_ = 0;
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.survey_mode_ = 0;
    ConsoleGlobal.faction_id_ = 0;
    Vehs[0].x = 100; Vehs[0].y = 100;
    Vehs[0].faction_id = 0; Vehs[0].flags = 0; Vehs[0].visibility = 0;
    Vehs[0].state = 0; Vehs[0].order = 0;
    Vehs[3].x = 4; Vehs[3].y = 6; Vehs[3].faction_id = 0;
    Vehs[3].flags = 0; Vehs[3].visibility = 0; Vehs[3].state = 0;
    VehDrawSuppressedVehicle = -1;
    VehDrawSelection689F28 = -1;
    VehDrawSelection689F2C = -1;
    tile.visibility = 0; tile.bit = 0; tile.val2 = 0;
    draw_base_calls = veh_at_calls = vehicle_draw_calls = 0;
    last_vehicle_drawn = -1;
    mutate_base_flags = false;
    mutate_first_veh_at_flags = false;
    base_flags_after_callback = 0;
}

int main() {
    MapWin window;

    // The pinned 0x00200040 mask admits the filter route. The first veh_at
    // callback clears 0x00200000; the subsequent test must observe that write.
    reset_case(window, 0x00200000U);
    mutate_first_veh_at_flags = true;
    window.draw_vehicles(4, 6, 8, 10, 0);
    assert(draw_base_calls == 1);
    assert(veh_at_calls == 1);
    assert(vehicle_draw_calls == 1 && last_vehicle_drawn == 3);

    // draw_bases adds the live 0x80000000 stop bit. The original exits before
    // vehicle lookup even though the opening snapshot lacked that bit.
    reset_case(window, 0x00000040U);
    mutate_base_flags = true;
    base_flags_after_callback = 0x80000040U;
    window.draw_vehicles(4, 6, 8, 10, 0);
    assert(draw_base_calls == 1);
    assert(veh_at_calls == 0 && vehicle_draw_calls == 0);
}
"""


def compile_and_run(method: str, compiler: str, expect_success: bool) -> None:
    harness = full_harness(method)
    with tempfile.TemporaryDirectory(prefix="mapwin-live-flags-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        exe = Path(tmp) / "control"
        cpp.write_text(harness)
        subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(exe)],
                       check=True, timeout=60, capture_output=True, text=True)
        result = subprocess.run([str(exe)], check=False, timeout=30,
                                capture_output=True, text=True)
        if expect_success:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0, "causal mutant unexpectedly passed"


def replace_once(text: str, old: str, new: str) -> str:
    assert text.count(old) == 1, f"mutation anchor count for {old!r}: {text.count(old)}"
    return text.replace(old, new, 1)


def test_actual_full_body_reloads_mutable_flags_and_uses_pinned_mask():
    method = actual_method()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "no host C++ compiler available"
    for compiler in compilers:
        compile_and_run(method, compiler, True)


def test_full_body_rejects_cached_reload_and_wrong_mask_mutants():
    method = actual_method()
    base_cached = replace_once(
        method,
        "if ((field_1DD70_ & 0x80000000U) != 0) return;",
        "if ((window_flags & 0x80000000U) != 0) return;",
    )
    filter_cached = replace_once(
        method,
        "if ((field_1DD70_ & 0x00200000U) != 0) {",
        "if ((window_flags & 0x00200000U) != 0) {",
    )
    wrong_mask = replace_once(
        method,
        "if ((field_1DD70_ & 0x00200040U) == 0) return;",
        "if ((field_1DD70_ & 0x00020040U) == 0) return;",
    )
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "no host C++ compiler available"
    for compiler in compilers:
        for mutant in (base_cached, filter_cached, wrong_mask):
            compile_and_run(mutant, compiler, False)


def test_actual_draw_vehicles_opening_gate():
    block = opening_block()
    harness = f"""
#include <stdint.h>
#include <cassert>

static int MapBaseDisplayOverride;
static int ControlTurnPhase;
struct GameDataStateFieldsProjection {{ uint32_t game_rules_; }};
struct GameDataStateProjection {{ GameDataStateFieldsProjection fields_; }};
static GameDataStateProjection GameDataStateStorage;
static uint32_t field_1DD70_;
static int reached_after_gate;

static void run_gate(uint32_t flags, int override_value, int phase, uint32_t rules) {{
    const int tile_x = 4;
    MapBaseDisplayOverride = override_value;
    ControlTurnPhase = phase;
    GameDataStateStorage.fields_.game_rules_ = rules;
    field_1DD70_ = flags;
    reached_after_gate = 0;
    {block}
    reached_after_gate = 1;
}}

int main() {{
    // AH,0x10 in the pinned prologue is the 0x1000 mask.
    run_gate(0x40U | 0x1000U, 1, 0, 0); assert(reached_after_gate == 0);
    // Bit 20 is not the native rejection bit and must not reject the route.
    run_gate(0x40U | 0x00100000U, 1, 0, 0); assert(reached_after_gate == 1);
    run_gate(0x1000U, 1, 0, 0); assert(reached_after_gate == 0); // required group absent
    run_gate(0x40U | 0x01000000U, 1, 0, 0); assert(reached_after_gate == 0);
    run_gate(0x40U, 0, 0, 0); assert(reached_after_gate == 0); // activation gates off
    run_gate(0x40U, 1, 0, 0); assert(reached_after_gate == 1); // display override
    run_gate(0x40U, 0, 0, 0x80U); assert(reached_after_gate == 1); // scenario rule
    run_gate(0x40U, 0, 1, 0); assert(reached_after_gate == 1); // active phase
}}
"""
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "no host C++ compiler available"
    with tempfile.TemporaryDirectory(prefix="mapwin-gate-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        cpp.write_text(harness)
        for compiler in compilers:
            exe = Path(tmp) / compiler.replace("+", "p")
            subprocess.run(
                [compiler, "-std=c++98", str(cpp), "-o", str(exe)],
                check=True,
                timeout=60,
            )
            subprocess.run([str(exe)], check=True, timeout=30)
