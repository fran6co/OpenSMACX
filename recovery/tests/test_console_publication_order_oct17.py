"""Focused Console publication-order controls.

The host harness executes extracted current C++ against logging doubles; it
does not establish the native Console/GameDataState ABI, full application
behavior, or acceptance. Pinned instructions independently establish the
original read/store order for the two complete mapped spans.
"""

from hashlib import sha256
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _body(source: str, signature: str) -> str:
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", source)
    assert match, "missing actual Console body: " + signature
    opening = source.index("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated Console body: " + signature)


def _actual_bodies() -> tuple[str, str]:
    editor_path = ROOT / "src/console_commands.cpp"
    ui_path = ROOT / "src/console.cpp"
    editor_source = editor_path.read_text(encoding="utf-8")
    ui_source = ui_path.read_text(encoding="utf-8")
    records = {record.address: record for path in (editor_path, ui_path)
               for record in read_file(path)}
    assert records[0x4E1290].image_spans == ((0x4E1290, 0x4E12FB),)
    assert records[0x51D760].image_spans == ((0x51D760, 0x51D7B5),)
    return (_body(editor_source, "void Console::editor_clear()"),
            _body(ui_source, "void Console::set_screen_geometry()"))


def _pinned_image() -> PeImage:
    path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned original executable unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    return PeImage(raw, PIN)


def _instructions(image: PeImage, start: int, end: int):
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    code = image.read(Span(start, end))
    decoded = list(decoder.disasm(code, start))
    assert sum(instruction.size for instruction in decoded) == end - start
    assert decoded[0].address == start
    assert decoded[-1].address + decoded[-1].size == end
    return decoded


def _absolute_memory(instruction, operand_index: int, address: int) -> bool:
    from capstone.x86 import X86_OP_MEM

    operand = instruction.operands[operand_index]
    return (operand.type == X86_OP_MEM and operand.mem.base == 0 and
            operand.mem.index == 0 and operand.mem.disp == address)


def _based_memory(instruction, operand_index: int, base: int, displacement: int) -> bool:
    from capstone.x86 import X86_OP_MEM

    operand = instruction.operands[operand_index]
    return (operand.type == X86_OP_MEM and operand.mem.base == base and
            operand.mem.index == 0 and operand.mem.disp == displacement)


def test_pinned_editor_clear_captures_then_publishes_game_state_after_resets():
    from capstone.x86 import X86_OP_IMM, X86_OP_REG
    from capstone.x86_const import X86_REG_EAX, X86_REG_ECX

    instructions = _instructions(_pinned_image(), 0x4E1290, 0x4E12FB)
    auto_undo = [i for i in instructions if i.mnemonic == "call" and
                 i.operands[0].type == X86_OP_IMM and i.operands[0].imm == 0x5ABF20]
    state_reads = [i for i in instructions if i.mnemonic == "mov" and
                   len(i.operands) == 2 and i.operands[0].type == X86_OP_REG and
                   i.operands[0].reg == X86_REG_ECX and
                   _absolute_memory(i, 1, 0x9A64C0)]
    state_or = [i for i in instructions if i.mnemonic == "or" and
                i.op_str == "ecx, 4"]
    zero_eax = [i for i in instructions if i.mnemonic == "xor" and
                len(i.operands) == 2 and i.operands[0].type == X86_OP_REG and
                i.operands[0].reg == X86_REG_EAX and i.operands[1].type == X86_OP_REG and
                i.operands[1].reg == X86_REG_EAX]
    vehicle_zero = [i for i in instructions if i.mnemonic == "mov" and
                    _absolute_memory(i, 0, 0x9A64C8) and
                    i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX]
    base_zero = [i for i in instructions if i.mnemonic == "mov" and
                 _absolute_memory(i, 0, 0x9A64CC) and
                 i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX]
    cursor_zero = [i for i in instructions if i.mnemonic == "mov" and
                   _absolute_memory(i, 0, 0x939288) and
                   i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX]
    selected_reset = [i for i in instructions if i.mnemonic == "mov" and
                      _absolute_memory(i, 0, 0x93928C)]
    state_stores = [i for i in instructions if i.mnemonic == "mov" and
                    len(i.operands) == 2 and
                    _absolute_memory(i, 0, 0x9A64C0) and
                    i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_ECX]
    assert all(len(group) == 1 for group in (auto_undo, state_reads, state_or, zero_eax,
                                               vehicle_zero, base_zero, cursor_zero,
                                               selected_reset, state_stores))
    order = [auto_undo[0].address, state_reads[0].address, zero_eax[0].address,
             state_or[0].address,
             vehicle_zero[0].address, base_zero[0].address, cursor_zero[0].address,
             selected_reset[0].address, state_stores[0].address]
    assert order == sorted(order)
    assert selected_reset[0].operands[1].type == X86_OP_IMM
    assert selected_reset[0].operands[1].imm == 0xFFFFFFFF


def test_pinned_geometry_reads_height_before_publishing_iface_width():
    from capstone.x86 import X86_OP_REG
    from capstone.x86_const import X86_REG_EAX, X86_REG_ECX, X86_REG_EDX

    instructions = _instructions(_pinned_image(), 0x51D760, 0x51D7B5)
    def one(predicate):
        found = [i for i in instructions if predicate(i)]
        assert len(found) == 1
        return found[0]

    width_read = one(lambda i: i.mnemonic == "mov" and len(i.operands) == 2 and
                      i.operands[0].type == X86_OP_REG and i.operands[0].reg == X86_REG_EAX and
                      _absolute_memory(i, 1, 0x9B7B1C))
    console_width = one(lambda i: i.mnemonic == "mov" and len(i.operands) == 2 and
                        _based_memory(i, 0, X86_REG_ECX, 0x23BCC) and
                        i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX)
    height_read = one(lambda i: i.mnemonic == "mov" and len(i.operands) == 2 and
                      i.operands[0].type == X86_OP_REG and i.operands[0].reg == X86_REG_EAX and
                      _absolute_memory(i, 1, 0x9B7B20))
    iface_width = one(lambda i: i.mnemonic == "mov" and
                      _absolute_memory(i, 0, 0x939E5C) and
                      i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EDX)
    subtract_panel = one(lambda i: i.mnemonic == "sub" and len(i.operands) == 2 and
                         i.operands[0].type == X86_OP_REG and i.operands[0].reg == X86_REG_EAX and
                         _absolute_memory(i, 1, 0x7AF59C))
    console_height = one(lambda i: i.mnemonic == "mov" and len(i.operands) == 2 and
                         _based_memory(i, 0, X86_REG_ECX, 0x23BD0) and
                         i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX)
    iface_height = one(lambda i: i.mnemonic == "mov" and
                       _absolute_memory(i, 0, 0x939E58) and
                       i.operands[1].type == X86_OP_REG and i.operands[1].reg == X86_REG_EAX)
    order = [width_read.address, console_width.address, height_read.address,
             iface_width.address, subtract_panel.address, console_height.address,
             iface_height.address]
    assert order == sorted(order)


HARNESS = r'''
#include <assert.h>
#include <stdint.h>

enum Event { AUTO_UNDO=1, STATE_READ, VEHICLE_ZERO, BASE_ZERO, CURSOR_ZERO,
             SELECTED_RESET, STATE_WRITE, MAP_WIPE_1, PLANET_SIZE, MAP_SHUTDOWN,
             MAP_INIT, PATHS_INIT, MAP_WIPE_2, DRAW_MAP, WORLD_REDRAW };
static int events[64], event_count;
static int map_wipes;
static void record(int event) { events[event_count++] = event; }

struct StateWord {
    uint32_t value;
    operator uint32_t() const { record(STATE_READ); return value; }
    StateWord &operator=(uint32_t next) {
        value = next; record(STATE_WRITE); return *this;
    }
    StateWord &operator|=(uint32_t bits) {
        record(STATE_READ); value |= bits; record(STATE_WRITE); return *this;
    }
};
struct TraceInt {
    int value;
    int event;
    TraceInt(int initial=0, int write_event=0) : value(initial), event(write_event) {}
    TraceInt &operator=(int next) { value=next; record(event); return *this; }
    operator int() const { return value; }
};
struct GameFields { StateWord game_state_; TraceInt vehicle_current_count_, base_current_count_; };
struct GameStorage { GameFields fields_; } GameDataStateStorage;
struct Console {
    TraceInt cursor_active_, selected_vehicle_;
    void editor_clear();
} ConsoleGlobal;
struct PathState { void init() { record(PATHS_INIT); } } Paths;
struct GraphicWin { void redraw() { record(WORLD_REDRAW); } };
struct WorldWin : GraphicWin {} WorldClimateWorldWin;
static const uint32_t STATE_UNK_4 = 4;
static void auto_undo() { record(AUTO_UNDO); GameDataStateStorage.fields_.game_state_.value = 0x100; }
static void map_wipe() { record(map_wipes++ == 0 ? MAP_WIPE_1 : MAP_WIPE_2); }
static int size_of_planet(int value) { assert(value == 2); record(PLANET_SIZE); return 0; }
static void map_shutdown() { record(MAP_SHUTDOWN); }
static void map_init() { record(MAP_INIT); }
static void draw_map(int value) { assert(value == 1); record(DRAW_MAP); }

// ACTUAL_EDITOR_CLEAR_BODY

int main() {
    GameDataStateStorage.fields_.game_state_.value = 0;
    GameDataStateStorage.fields_.vehicle_current_count_.value = 9;
    GameDataStateStorage.fields_.base_current_count_.value = 7;
    ConsoleGlobal.cursor_active_.value = 1;
    ConsoleGlobal.selected_vehicle_.value = 23;
    GameDataStateStorage.fields_.vehicle_current_count_.event = VEHICLE_ZERO;
    GameDataStateStorage.fields_.base_current_count_.event = BASE_ZERO;
    ConsoleGlobal.cursor_active_.event = CURSOR_ZERO;
    ConsoleGlobal.selected_vehicle_.event = SELECTED_RESET;
    ConsoleGlobal.editor_clear();
    const int expected[] = {AUTO_UNDO, STATE_READ, VEHICLE_ZERO, BASE_ZERO,
        CURSOR_ZERO, SELECTED_RESET, STATE_WRITE, MAP_WIPE_1, PLANET_SIZE,
        MAP_SHUTDOWN, MAP_INIT, PATHS_INIT, MAP_WIPE_2, DRAW_MAP, WORLD_REDRAW};
    assert(event_count == (int)(sizeof(expected)/sizeof(expected[0])));
    for (int i=0; i<event_count; ++i) assert(events[i] == expected[i]);
    assert(GameDataStateStorage.fields_.game_state_.value == 0x104);
    assert(GameDataStateStorage.fields_.vehicle_current_count_.value == 0);
    assert(GameDataStateStorage.fields_.base_current_count_.value == 0);
    assert(ConsoleGlobal.cursor_active_.value == 0);
    assert(ConsoleGlobal.selected_vehicle_.value == -1);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["none", "publish_early", "drop_flag", "wrong_vehicle_count"])
def test_editor_clear_current_owner_order_and_state_value(tmp_path, compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    editor_clear, _ = _actual_bodies()
    replacements = {
        "drop_flag": ("pending_state |= STATE_UNK_4;", "pending_state |= (STATE_UNK_4 & 0U);"),
        "wrong_vehicle_count": ("vehicle_current_count_ = 0;", "vehicle_current_count_ = 1;"),
    }
    if mutation == "publish_early":
        publication = "    GameDataStateStorage.fields_.game_state_ = pending_state;\n"
        reset = "    GameDataStateStorage.fields_.vehicle_current_count_ = 0;\n"
        assert editor_clear.count(publication) == editor_clear.count(reset) == 1
        editor_clear = editor_clear.replace(publication, "", 1).replace(reset, publication + reset, 1)
    elif mutation != "none":
        old, new = replacements[mutation]
        assert old in editor_clear, "live mutation anchor changed; candidate source required"
        editor_clear = editor_clear.replace(old, new, 1)
    source = HARNESS.replace("// ACTUAL_EDITOR_CLEAR_BODY", editor_clear)
    executable = tmp_path / "editor-clear-order"
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                           "-x", "c++", "-", "-o", str(executable)],
                          input=source, text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)
    if mutation == "none":
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, mutation + " escaped actual-body order/value controls"
