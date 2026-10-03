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


def test_actual_draw_vehicles_opening_gate():
    block = opening_block()
    harness = f"""
#include <stdint.h>
#include <cassert>

static int MapBaseDisplayOverride;
static int ControlTurnPhase;
static uint32_t GameRules;
static uint32_t field_1DD70_;
static int reached_after_gate;

static void run_gate(uint32_t flags, int override_value, int phase, uint32_t rules) {{
    const int tile_x = 4;
    MapBaseDisplayOverride = override_value;
    ControlTurnPhase = phase;
    GameRules = rules;
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
