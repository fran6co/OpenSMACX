"""Behavior controls for action_gate's native live vehicle rereads.

The compiled function is extracted from the worker's actual src/veh.cpp. The
pin test separately qualifies the original argument and field-load order; the
host fixture checks that callbacks which mutate the real Veh records remain
observable at the later focus call.
"""
from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"
PIN = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe",
))
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ACTION_GATE_SPAN = Span(0x004CF380, 0x004CF477)
SIGNATURE = "void __cdecl action_gate(int veh_id, int base_id) {"


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def _actual_action_gate() -> str:
    body = _definition(SOURCE.read_text(), SIGNATURE)
    assert "base_at(veh_x, veh_y)" in body
    assert "veh_drop(lifted, Bases[base_id].x, Bases[base_id].y)" in body
    return body.replace("void __cdecl action_gate", "void action_gate", 1)


def _actual_struct(header: Path, signature: str) -> str:
    text = header.read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    in_line_comment = False
    in_block_comment = False
    in_string = False
    escaped = False
    i = opening
    while i < len(text):
        char = text[i]
        next_char = text[i + 1] if i + 1 < len(text) else ""
        if in_line_comment:
            if char == "\n":
                in_line_comment = False
        elif in_block_comment:
            if char == "*" and next_char == "/":
                in_block_comment = False
                i += 1
        elif in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
        elif char == "/" and next_char == "/":
            in_line_comment = True
            i += 1
        elif char == "/" and next_char == "*":
            in_block_comment = True
            i += 1
        elif char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1] + ";"
        i += 1
    raise AssertionError("unterminated declaration: " + signature)


PREFIX = r"""
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#define __cdecl

VEH_DECLARATION
BASE_DECLARATION
static_assert(sizeof(Veh) == 0x34, "actual Veh layout");
static_assert(offsetof(Veh, x) == 0x00 && offsetof(Veh, y) == 0x02,
              "actual Veh coordinate members");
static_assert(offsetof(Veh, faction_id) == 0x0e, "actual Veh faction member");
static_assert(sizeof(Base) == 0x134, "actual Base layout");
static_assert(offsetof(Base, x) == 0x00 && offsetof(Base, y) == 0x02
              && offsetof(Base, state) == 0x30, "actual Base members");

Veh Vehs[8];
Base Bases[8];
enum Event { BASE_AT=1, LIFT=2, DROP=3, DRAW=4, FOCUS=5 };
int events[16], event_count;
int base_at_x, base_at_y, base_at_result;
int lifted_result, dropped_id, dropped_x, dropped_y;
int draw_x[4], draw_y[4], draw_mode[4], draw_count;
int selected_after_draws;
int focus_count, focus_x, focus_y, focus_faction;
int callback_contract_failed;

struct Console {
    int selected_vehicle_;
    int focus(int x, int y, int faction) {
        events[event_count++] = FOCUS;
        ++focus_count;
        focus_x = x; focus_y = y; focus_faction = faction;
        return 0;
    }
} ConsoleGlobal;

int base_at(int x, int y) {
    events[event_count++] = BASE_AT;
    base_at_x = x; base_at_y = y;
    return base_at_result;
}
int veh_lift(int vehicle_id) {
    events[event_count++] = LIFT;
    if (vehicle_id != 2) callback_contract_failed = 1;
    Vehs[vehicle_id].x = 11; Vehs[vehicle_id].y = 12;
    Vehs[vehicle_id].faction_id = 3;
    return lifted_result;
}
void veh_drop(int vehicle_id, int x, int y) {
    events[event_count++] = DROP;
    dropped_id = vehicle_id; dropped_x = x; dropped_y = y;
    if (vehicle_id != lifted_result || x != Bases[4].x || y != Bases[4].y)
        callback_contract_failed = 1;
    Vehs[2].x = 21; Vehs[2].y = 22; Vehs[2].faction_id = 5;
}
void draw_tile(int x, int y, int mode) {
    events[event_count++] = DRAW;
    if (draw_count < 4) {
        draw_x[draw_count] = x; draw_y[draw_count] = y;
        draw_mode[draw_count] = mode;
    }
    ++draw_count;
    // Model callback effects on the same actual typed record. The final draw
    // also changes the selection, so action_gate must test selection after it.
    Vehs[2].x = static_cast<int16_t>(31 + 10 * draw_count);
    Vehs[2].y = static_cast<int16_t>(32 + 10 * draw_count);
    Vehs[2].faction_id = static_cast<uint8_t>(4 + 2 * draw_count);
    if (draw_count == 2) ConsoleGlobal.selected_vehicle_ = selected_after_draws;
}
"""


def _fixture_prefix() -> str:
    veh = _actual_struct(ROOT / "src/veh.h", "struct Veh {")
    base = _actual_struct(ROOT / "src/base.h", "struct Base {")
    return PREFIX.replace("VEH_DECLARATION", veh).replace("BASE_DECLARATION", base)


MAIN = r"""
void reset(int selected_before, int selected_after) {
    std::memset(Vehs, 0, sizeof(Vehs));
    std::memset(Bases, 0, sizeof(Bases));
    for (int i = 0; i != 8; ++i) {
        Vehs[i].x = static_cast<int16_t>(i);
        Vehs[i].y = static_cast<int16_t>(i + 20);
        Vehs[i].faction_id = static_cast<uint8_t>(i + 1);
        Bases[i].x = static_cast<int16_t>(i * 10 + 1);
        Bases[i].y = static_cast<int16_t>(i * 10 + 2);
    }
    Vehs[2].x = 7; Vehs[2].y = 9; Vehs[2].faction_id = 1;
    Vehs[5].x = 101; Vehs[5].y = 102; Vehs[5].faction_id = 9;
    Bases[1].x = 3; Bases[1].y = 4; Bases[1].state = 0x10;
    Bases[4].x = 20; Bases[4].y = 30; Bases[4].state = 0x40;
    ConsoleGlobal.selected_vehicle_ = selected_before;
    base_at_x = base_at_y = -999; base_at_result = 1;
    lifted_result = 5;
    dropped_id = dropped_x = dropped_y = -999;
    draw_count = event_count = focus_count = 0;
    focus_x = focus_y = focus_faction = -999;
    callback_contract_failed = 0;
    selected_after_draws = selected_after;
    for (int i = 0; i != 4; ++i) draw_x[i] = draw_y[i] = draw_mode[i] = -999;
}

int expect_event_order(bool focused) {
    const int expected[] = { BASE_AT, LIFT, DROP, DRAW, DRAW, FOCUS };
    const int expected_without_focus[] = { BASE_AT, LIFT, DROP, DRAW, DRAW };
    const int count = focused ? 6 : 5;
    if (event_count != count) return 1;
    for (int i = 0; i != count; ++i) {
        const int value = focused ? expected[i] : expected_without_focus[i];
        if (events[i] != value) return 2;
    }
    return 0;
}

int selected_after_callbacks_uses_latest_record() {
    reset(-1, 2); // draw callback selects this vehicle after all earlier work
    action_gate(2, 4);
    if (callback_contract_failed || expect_event_order(true)) return 10;
    if (base_at_x != 7 || base_at_y != 9) return 11;
    if (dropped_id != 5 || dropped_x != 20 || dropped_y != 30) return 12;
    if (draw_count != 2 || draw_x[0] != 3 || draw_y[0] != 4
        || draw_mode[0] != 2 || draw_x[1] != 20 || draw_y[1] != 30
        || draw_mode[1] != 2) return 13;
    if (Bases[1].state != (0x10 | 0x20000000)
        || Bases[4].state != (0x40 | 0x20000000)) return 14;
    // Both callbacks and the final draw mutated the source vehicle. Focus is
    // for original veh_id 2, not veh_lift's returned id 5, and observes the
    // final live x/y/faction values after the last draw callback.
    if (Vehs[2].x != 51 || Vehs[2].y != 52 || Vehs[2].faction_id != 8) return 15;
    if (focus_count != 1 || focus_x != 51 || focus_y != 52 || focus_faction != 8)
        return 16;
    return 0;
}

int selection_is_rechecked_after_callbacks() {
    reset(2, 7); // initially selected, then deselected by the final draw
    action_gate(2, 4);
    if (callback_contract_failed || expect_event_order(false)) return 20;
    if (ConsoleGlobal.selected_vehicle_ != 7 || focus_count != 0) return 21;
    if (base_at_x != 7 || base_at_y != 9 || dropped_id != 5) return 22;
    return 0;
}

int main() {
    const int first = selected_after_callbacks_uses_latest_record();
    if (first) return first;
    return selection_is_rechecked_after_callbacks();
}
"""


def _run(body: str, compiler: str, optimization: str, expected: int) -> None:
    with tempfile.TemporaryDirectory(prefix="vehicle-action-gate-") as directory:
        cpp = Path(directory) / "action_gate.cpp"
        exe = Path(directory) / "action_gate"
        cpp.write_text(_fixture_prefix() + "\n" + body + "\n" + MAIN)
        subprocess.run([
            compiler, "-std=c++11", optimization, "-Wall", "-Wextra", "-Werror",
            str(cpp), "-o", str(exe)
        ], check=True)
        result = subprocess.run([str(exe)], check=False)
        assert result.returncode == expected, (
            f"{compiler} {optimization}: expected {expected}, got {result.returncode}"
        )


def test_actual_action_gate_observes_post_callback_vehicle_state() -> None:
    body = _actual_action_gate()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither g++ nor clang++ is available"
    for compiler in compilers:
        for optimization in ("-O0", "-O2"):
            _run(body, compiler, optimization, expected=0)


def test_stale_coordinate_snapshot_mutant_fails_callback_control() -> None:
    body = _actual_action_gate()
    live_focus = (
        "ConsoleGlobal.focus(Vehs[veh_id].x, Vehs[veh_id].y, "
        "Vehs[veh_id].faction_id);"
    )
    stale_focus = "ConsoleGlobal.focus(veh_x, veh_y, Vehs[veh_id].faction_id);"
    assert body.count(live_focus) == 1, (
        "the minimal live-field correction has not been published into src/veh.cpp"
    )
    mutant = body.replace(live_focus, stale_focus, 1)
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler):
            for optimization in ("-O0", "-O2"):
                _run(mutant, compiler, optimization, expected=16)


def test_pinned_gate_snapshots_lookup_then_reloads_focus_fields_after_draws() -> None:
    assert PIN.is_absolute() and PIN.is_file()
    image_data = PIN.read_bytes()
    assert hashlib.sha256(image_data).hexdigest() == PIN_SHA256
    image = PeImage(image_data, PIN_SHA256)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(ACTION_GATE_SPAN), ACTION_GATE_SPAN.start
    ))
    assert instructions and instructions[0].address == ACTION_GATE_SPAN.start
    assert sum(instruction.size for instruction in instructions) == (
        ACTION_GATE_SPAN.end - ACTION_GATE_SPAN.start
    )
    native = {instruction.address: (instruction.mnemonic, instruction.op_str)
              for instruction in instructions}

    # Initial vehicle coordinates feed base_at before lift/drop/draw callbacks.
    assert native[0x004CF395] == ("movsx", "ecx, word ptr [eax + 0x95282a]")
    assert native[0x004CF39C] == ("movsx", "edx, word ptr [eax + 0x952828]")
    assert native[0x004CF3A5][0] == "call" and native[0x004CF3A5][1].endswith("0x4e3a50")
    assert native[0x004CF3EE][0] == "call" and native[0x004CF3EE][1].endswith("0x5bffa0")
    assert native[0x004CF3F7][0] == "call" and native[0x004CF3F7][1].endswith("0x5c0080")
    assert native[0x004CF421][0] == "call" and native[0x004CF421][1].endswith("0x46af40")
    assert native[0x004CF43B][0] == "call" and native[0x004CF43B][1].endswith("0x46af40")

    # Selection is tested after both draw calls. On the selected path the
    # original vehicle's current faction/y/x are then loaded from Vehs before
    # the focus call (Veh member offsets 0x0e/0x02/0x00).
    assert native[0x004CF440] == ("mov", "eax, dword ptr [0x93928c]")
    assert native[0x004CF448] == ("cmp", "ebx, eax")
    assert native[0x004CF44D] == ("jne", "0x4cf475")
    assert native[0x004CF44F] == ("mov", "eax, dword ptr [ebp + 8]")
    assert native[0x004CF454] == ("mov", "cl, byte ptr [eax + 0x952836]")
    assert native[0x004CF45A] == ("movsx", "edx, word ptr [eax + 0x95282a]")
    assert native[0x004CF461] == ("movsx", "eax, word ptr [eax + 0x952828]")
    assert native[0x004CF470][0] == "call" and native[0x004CF470][1].endswith("0x5108a0")
