"""Host control for enemy_veh's entry-faction snapshot across do_all_non_input."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/general.cpp"
SIGNATURE = "int __cdecl enemy_veh(int veh_id) {"


def _definition(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError("unterminated enemy_veh definition")


def _actual_body() -> str:
    return _definition(SOURCE.read_text()).replace(
        "int __cdecl enemy_veh", "int enemy_veh", 1
    )


def _actual_veh_declaration() -> str:
    header = (ROOT / "src/veh.h").read_text()
    signature = "struct Veh {"
    start = header.index(signature)
    opening = header.index("{", start)
    depth = 0
    in_line_comment = in_block_comment = in_string = escaped = False
    index = opening
    while index < len(header):
        char = header[index]
        following = header[index + 1] if index + 1 < len(header) else ""
        if in_line_comment:
            if char == "\n":
                in_line_comment = False
        elif in_block_comment:
            if char == "*" and following == "/":
                in_block_comment = False
                index += 1
        elif in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
        elif char == "/" and following == "/":
            in_line_comment = True
            index += 1
        elif char == "/" and following == "*":
            in_block_comment = True
            index += 1
        elif char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return header[start : index + 1] + ";"
        index += 1
    raise AssertionError("unterminated Veh declaration")


def _pre_snapshot_body(body: str) -> str:
    """Recreate the prior source's live field reload as a damage control."""
    cached = "    const uint8_t unit_faction = Vehs[unit_id].faction_id;\n"
    if cached in body:
        body = body.replace(cached, "", 1)
        body = body.replace(
            "if (unit_faction == ConsoleGlobal.faction_id_ && IsMultiplayerNet != 0)",
            "if (Vehs[unit_id].faction_id == ConsoleGlobal.faction_id_ && IsMultiplayerNet != 0)",
            1,
        )
        body = body.replace(
            "                    unit_faction)",
            "                    Vehs[unit_id].faction_id)",
            1,
        )
    return body


HARNESS = r"""
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#define __cdecl

VEH_DECLARATION
static_assert(sizeof(Veh) == 0x34, "actual Veh size");
static_assert(offsetof(Veh, faction_id) == 0x0e, "actual Veh faction offset");
struct VehPrototype { int plan; };
Veh Vehs[8];
VehPrototype VehPrototypes[8];

struct ConsoleState { int faction_id_; } ConsoleGlobal;
struct Channel { int leading_words_[4]; };
struct NetDaemon {
    Channel channels_[1];
    int awaits;
    int unlocks;
    int actions;
    void await_synch() { ++awaits; }
    void unlock_veh() { ++unlocks; }
    int action(int vehicle_id, int mode) {
        if (vehicle_id != 0 || mode != 1) return -1;
        ++actions;
        return 1;
    }
} NetDaemonNet;

const int ORDER_MOVE_TO = 18;
const unsigned int VSTATE_EXPLORE = 0x40;
int IsMultiplayerNet = 1;
int callback_count;
int zoc_count, zoc_x, zoc_y, zoc_faction;
int synch_count, enemy_move_count, action_count, skip_count;
int speed(int, int) { return 0; }
void do_all_non_input() {
    ++callback_count;
    // The callback mutates live coordinates and faction on the same vehicle.
    Vehs[0].x = 37;
    Vehs[0].y = 41;
    Vehs[0].faction_id = 5;
}
int zoc_any(int x, int y, int faction) {
    ++zoc_count;
    zoc_x = x; zoc_y = y; zoc_faction = faction;
    return 0; // Follow the original after_move edge.
}
int enemy_move(int) { ++enemy_move_count; return 0; }
void log_say(char *, int, int, int) {}
void log_say(char *, char *, int, int, int) {}
void synch_veh(int) { ++synch_count; }
void veh_skip(int) { ++skip_count; }
int action(int) { ++action_count; return 1; }
char *_itoa(int value, char *buffer, int radix) {
    if (radix != 10) return buffer;
    std::snprintf(buffer, 0x50, "%d", value);
    return buffer;
}
char LogEnemyVehicleDone[] = "Enemy Vehicle DONE";
char LogEnemyVehicleAction[] = "Enemy Vehicle ACTION";
char LogEnemyVehicle[] = "Enemy Vehicle";

BODY

int main() {
    Vehs[0].x = 10;
    Vehs[0].y = 11;
    Vehs[0].faction_id = 2;
    Vehs[0].moves_expended = 1;
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].proto_id = 0;
    Vehs[0].state = 0;
    VehPrototypes[0].plan = 0;
    ConsoleGlobal.faction_id_ = 2;

    const int result = enemy_veh(0);
    // The local-turn decision and zoc faction use the pre-callback faction;
    // zoc coordinates are read after the callback and remain live.
    if (callback_count != 1 || zoc_count != 1) return 1;
    if (zoc_faction != 2 || zoc_x != 37 || zoc_y != 41) return 2;
    if (result != 1 || synch_count != 1 || NetDaemonNet.awaits != 1
            || NetDaemonNet.actions != 1) return 3;
    if (enemy_move_count != 0 || action_count != 0 || skip_count != 0) return 4;
    return 0;
}
"""


def _run_host(tmp_path: Path, body: str, compiler: str) -> subprocess.CompletedProcess:
    harness = HARNESS.replace("VEH_DECLARATION", _actual_veh_declaration())
    harness = harness.replace("BODY", body)
    executable = tmp_path / "enemy-veh-faction-snapshot"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_enemy_veh_preserves_entry_faction_and_reloads_coordinates(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")

    actual = _actual_body()
    result = _run_host(tmp_path, actual, compiler)
    assert result.returncode == 0, result.stderr.decode()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_pre_snapshot_live_faction_reload_fails_mutation_control(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")

    body = _pre_snapshot_body(_actual_body())
    assert body != _actual_body() or "unit_faction" not in body
    result = _run_host(tmp_path, body, compiler)
    assert result.returncode != 0, "post-callback faction reload survived the control"
