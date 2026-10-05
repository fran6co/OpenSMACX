"""Compile actual production key-handler excerpts and dispatch mutants.

The service objects are explicit test doubles, but each tested branch/tail is
extracted verbatim from the mapped, implemented Console::on_key_click body.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, State, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"


def actual_source():
    records = [record for record in read_file(SOURCE)
               if record.address == 0x5178C0]
    record, = records
    assert record.name == "?on_key_click@Console@@QAEHHH@Z"
    assert record.mode is Mode.BODY
    assert record.state is State.IMPLEMENTED
    assert record.image_spans == (
        (0x5178C0, 0x51B66F),
        (0x65D4D0, 0x65D4E2),
    )
    return _block(SOURCE.read_text(), "int Console::on_key_click(int a1, int a2)")


def _block(source: str, marker: str) -> str:
    start = source.index(marker)
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
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
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated proposal block: {marker}")


def _selected(source: str, marker: str) -> str:
    function = source.index("int Console::on_key_click(int a1, int a2)")
    return _block(source[function:], marker)


def _condition(source: str, marker: str) -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    condition = source[start + len("if ("):opening].strip()
    assert condition.endswith(")")
    return condition[:-1]


def _compile_and_run(tmp_path: Path, source: str, stem: str) -> int:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], check=False).returncode


def _chat_harness(branch: str, epilogue: str) -> str:
    return r'''
#include <assert.h>
int chat_calls, pan_calls, mode_calls;
void chat(int) { ++chat_calls; }
int dispatch(int a2, int native_command_consumed) {
BRANCH
goto native_command_done;
native_command_done:
EPILOGUE
native_done:
    return 1;
native_map_pan_route:
    ++pan_calls;
    return 1;
native_mode_command_tables:
    ++mode_calls;
    return 1;
}
int main() {
    dispatch(0x20043, 1);
    assert(chat_calls == 1 && pan_calls == 0 && mode_calls == 0);
    dispatch(0x20043, 0);
    assert(chat_calls == 2 && pan_calls == 1 && mode_calls == 0);
    dispatch(0x20044, 0);
    assert(chat_calls == 2 && pan_calls == 1 && mode_calls == 1);
    return 0;
}
'''.replace("BRANCH", branch).replace("EPILOGUE", epilogue)


def _quit_harness(route_10051: str, case_20051: str, tail: str, *, pbem: bool) -> str:
    return r'''
#include <assert.h>
#include <string.h>
int IsMultiplayerNet, IsMultiplayerPBEM, GameRules, ExitTurnLoop;
unsigned int LockEnableMask;
int hang_calls, reset_calls, save_calls, auto_save_calls, retire_calls, pop_calls, log_calls;
struct ConsoleState { int faction_id_; } ConsoleGlobal;
struct NetState {
    void hang_up() { ++hang_calls; }
    void reset() { ++reset_calls; }
} NetDaemonNet;
int popup_result = 1;
char prompt[32];
int bit_count(int value) { int n=0; for (; value; value &= value-1) ++n; return n; }
int X_pop(const char *text, int (*)()) { ++pop_calls; strcpy(prompt,text); return popup_result; }
int save_game(int) { ++save_calls; return 1; }
void auto_save() { ++auto_save_calls; }
void log_say(const char *, int, int, int) { ++log_calls; }
void retire_game() { ++retire_calls; }
void dispatch_quit(int a2) {
ROUTE_10051
    switch (a2) {
CASE_20051
    default: break;
    }
    return;
native_retirement_confirmation:
TAIL
}
int main() {
    ConsoleGlobal.faction_id_ = 2;
    IsMultiplayerPBEM = PBEM_VALUE;
    LockEnableMask = 0xFF;
    GameRules = 0x400;
    dispatch_quit(0x20051);
    assert(retire_calls == 0);
    assert(ExitTurnLoop == 1 && log_calls == 1);
    assert(pop_calls == 1 && strcmp(prompt, PBEM_VALUE ? "REALLYWIMPOUT" : "REALLYQUIT") == 0);
    if (PBEM_VALUE) {
        assert(save_calls == 1 && (LockEnableMask & (1u<<2)) == 0);
        assert(auto_save_calls == 0 && hang_calls == 0 && reset_calls == 0);
    } else {
        assert(save_calls == 0 && auto_save_calls == 1);
    }
    dispatch_quit(0x10051);
    assert(retire_calls == 1);
    return 0;
}
'''.replace("ROUTE_10051", route_10051).replace("CASE_20051", case_20051).replace(
        "TAIL", tail).replace("PBEM_VALUE", "1" if pbem else "0")


def _netmsg_activity_harness(first_gate: str, later_gate: str) -> str:
    return r'''
#include <assert.h>
int active_calls[8], active_count;
struct NetMsgStub {
    int id, active;
    int is_active() const { active_calls[active_count++] = id; return active; }
} NetMsgPrimary = {1, 0}, NetMsgSecondary = {2, 0};
int ConsoleModalState7CD2E8;
struct ButtonGroupStub { int field_84_; };
struct MainInterfaceStub {
    ButtonGroupStub group;
    ButtonGroupStub &button_group(int) { return group; }
} MainInterfaceGlobal = {{0x3E8}};
struct TutWindowStub { int is_visible() const { return 0; } } TutWinBaseWindow;
bool initial_gate(int a1, int a2) {
    bool enter_space_handoff = false;
FIRST_GATE
    return enter_space_handoff;
}
int later_gate() {
LATER_GATE
    return 0;
native_done:
    return 1;
}
int main() {
    // Original order is Primary (0x80A6BC), then Secondary (0x805304).
    active_count = 0; NetMsgPrimary.active = NetMsgSecondary.active = 0;
    assert(initial_gate(0, 0x0D));
    assert(active_count == 2 && active_calls[0] == 1 && active_calls[1] == 2);
    active_count = 0; NetMsgPrimary.active = 1; NetMsgSecondary.active = 0;
    assert(!initial_gate(0, 0x0D));
    assert(active_count == 1 && active_calls[0] == 1);
    active_count = 0; NetMsgPrimary.active = 0; NetMsgSecondary.active = 1;
    assert(!initial_gate(0, 0x0D));
    assert(active_count == 2 && active_calls[0] == 1 && active_calls[1] == 2);

    active_count = 0; NetMsgPrimary.active = NetMsgSecondary.active = 0;
    assert(later_gate() == 0);
    assert(active_count == 2 && active_calls[0] == 1 && active_calls[1] == 2);
    active_count = 0; NetMsgPrimary.active = 1; NetMsgSecondary.active = 0;
    assert(later_gate() == 1);
    assert(active_count == 1 && active_calls[0] == 1);
    active_count = 0; NetMsgPrimary.active = 0; NetMsgSecondary.active = 1;
    assert(later_gate() == 1);
    assert(active_count == 2 && active_calls[0] == 1 && active_calls[1] == 2);
    return 0;
}
'''.replace("FIRST_GATE", first_gate).replace("LATER_GATE", later_gate)


def _y_join_harness(route: str) -> str:
    return r'''
#include <assert.h>
int scenario_tail_calls, command_dispatch_calls;
unsigned int GameState;
const unsigned STATE_SCENARIO_EDITOR = 0x1000u;
const unsigned STATE_OMNISCIENT_VIEW = 0x80u;
const unsigned STATE_DEBUG_MODE = 0x40u;
struct Button { void show(int) {} void hide() {} };
struct MainInterface { Button flat_button_[3]; } MainInterfaceGlobal;
struct Dirty { void change(int) {} void redraw() {} } ConsoleInfoWin, ConsoleStatusWin;
struct Climate { void redraw() {} } WorldClimateWorldWin;
struct Map { void main_caption() {} } MapWinObject;
Map *MapWinTable[1] = {&MapWinObject};
void set_dirty() {}
void draw_map(int) {}
void desktop_update() {}
int dispatch(int normalized_character, unsigned state,
             int native_command_consumed) {
    GameState = state;
    if (normalized_character == -1) goto native_scenario_tail;
    if (normalized_character == -2) goto native_editor_command_dispatch;
ROUTE
native_scenario_tail:
    ++scenario_tail_calls;
native_editor_command_dispatch:
    ++command_dispatch_calls;
    return 0;
}
int main() {
    dispatch('y', 0x1000u, 0);
    assert(scenario_tail_calls == 0 && command_dispatch_calls == 1);
    return 0;
}
'''.replace("ROUTE", route)


def _control_turn_harness(lock_block: str) -> str:
    return r'''
#include <assert.h>
int IsMultiplayerNet, ConsoleControlTurnActive, UnkGlobal0093A934;
struct LockEntry {
    struct Payload { int flags; } payload_;
    int leading_words_[4];
};
struct NetDaemon { LockEntry lock_state_[3]; } NetDaemonNet;
int native_editor_lock;
int dispatch() {
    native_editor_lock = 0;
LOCK_BLOCK
    return native_editor_lock;
native_done:
    return -1;
}
int main() {
    IsMultiplayerNet = 0; ConsoleControlTurnActive = 1;
    assert(dispatch() == 0);
    IsMultiplayerNet = 1; ConsoleControlTurnActive = 1;
    assert(dispatch() == -1);
    ConsoleControlTurnActive = 0;
    assert(dispatch() == 0);
    NetDaemonNet.lock_state_[2].payload_.flags = 1;
    assert(dispatch() == 1);
    NetDaemonNet.lock_state_[2].payload_.flags = 0;
    NetDaemonNet.lock_state_[0].leading_words_[3] = 1;
    assert(dispatch() == 1);
    return 0;
}
'''.replace("LOCK_BLOCK", lock_block)


def _entry_guard_harness(entry_guard: str) -> str:
    return r'''
#include <assert.h>
int ExitTurnLoop;
int dispatch(int a2) {
ENTRY_GUARD
    return 1;
native_done:
    return 3;
}
int main() {
    ExitTurnLoop = 1; assert(dispatch(0) == 3);
    ExitTurnLoop = 0; assert(dispatch(0x12) == 3);
    assert(dispatch(0x10012) == 3);
    assert(dispatch(0x20012) == 3);
    assert(dispatch(0x30012) == 3);
    assert(dispatch(0x10000) == 1);
    return 0;
}
'''.replace("ENTRY_GUARD", entry_guard)


def _cursor_gate_harness(mode1_condition: str, mode8_condition: str) -> str:
    return r'''
#include <assert.h>
struct ConsoleState { int cursor_active_; } ConsoleGlobal;
int map_cursor_mode, selected_map_slot, selected_vehicle_id;
unsigned int field_23D10_, field_1DD80_;
int selected_vehicle_, normalized_character, a1, MapWinSelectedSlot;
int mode1_route() { return CONDITION1; }
int mode8_route() { return CONDITION8; }
int main() {
    map_cursor_mode = 1; selected_map_slot = 0; selected_vehicle_id = 0;
    ConsoleGlobal.cursor_active_ = 1;
    assert(mode1_route());
    ConsoleGlobal.cursor_active_ = 0;
    assert(!mode1_route());

    field_23D10_ = 8; field_1DD80_ = 1; selected_vehicle_ = 0;
    normalized_character = 'g'; a1 = 'g'; MapWinSelectedSlot = 0;
    ConsoleGlobal.cursor_active_ = 1;
    assert(mode8_route());
    ConsoleGlobal.cursor_active_ = 0;
    assert(!mode8_route());
    return 0;
}
'''.replace("CONDITION1", mode1_condition).replace("CONDITION8", mode8_condition)


def test_candidate_map_mode_cursor_gates_and_no_slot_gate_mutations(tmp_path):
    proposal = actual_source()
    mode1_condition = _condition(proposal, "if (map_cursor_mode == 1 &&")
    mode8_condition = _condition(proposal, "if (field_23D10_ == 8 &&")
    assert _compile_and_run(
        tmp_path,
        _cursor_gate_harness(mode1_condition, mode8_condition),
        "map-mode-cursor-gates",
    ) == 0

    no_mode1_cursor = mode1_condition.replace(
        "&& ConsoleGlobal.cursor_active_ != 0", "", 1
    )
    assert no_mode1_cursor != mode1_condition
    assert _compile_and_run(
        tmp_path,
        _cursor_gate_harness(no_mode1_cursor, mode8_condition),
        "map-mode1-cursor-mutant",
    ) != 0

    no_mode8_cursor = mode8_condition.replace(
        "&&\n        ConsoleGlobal.cursor_active_ != 0", "", 1
    )
    assert no_mode8_cursor != mode8_condition
    assert _compile_and_run(
        tmp_path,
        _cursor_gate_harness(mode1_condition, no_mode8_cursor),
        "map-mode8-cursor-mutant",
    ) != 0

    erroneous_slot_gate = mode8_condition.replace(
        "field_23D10_ == 8 &&",
        "field_23D10_ == 8 && MapWinSelectedSlot != 0 &&",
        1,
    )
    assert erroneous_slot_gate != mode8_condition
    assert _compile_and_run(
        tmp_path,
        _cursor_gate_harness(mode1_condition, erroneous_slot_gate),
        "map-mode8-slot-mutant",
    ) != 0


def test_candidate_chat_dispatch_consumed_edge_and_mutation(tmp_path):
    proposal = actual_source()
    chat = _selected(proposal, "if (a2 == 0x20043) {")
    greater = _selected(proposal, "if (a2 > 0x20043) {")
    done_start = proposal.index("native_command_done:\nif (native_command_consumed != 0)")
    done_start += len("native_command_done:\n")
    done_end = proposal.index("goto native_map_pan_route;", done_start) + len("goto native_map_pan_route;")
    done = proposal[done_start:done_end]
    assert _compile_and_run(tmp_path, _chat_harness(chat + "\n" + greater, done), "chat") == 0

    damaged = chat.replace("goto native_command_done;", "goto native_map_pan_route;", 1)
    assert damaged != chat
    assert _compile_and_run(tmp_path, _chat_harness(damaged + "\n" + greater, done), "chat-mutant") != 0


def test_candidate_quit_paths_and_mutations(tmp_path):
    proposal = actual_source()
    route_10051 = _selected(proposal, "if (a2 == 0x10051) {")
    case_start = proposal.index("case 0x20051:", proposal.index("native_mode_command_tables:"))
    case_end = proposal.index("case 0x20053:", case_start)
    case_20051 = proposal[case_start:case_end]
    tail = _selected(proposal, "native_retirement_confirmation:\n{")
    tail = tail.replace("native_retirement_confirmation:\n", "", 1)

    assert _compile_and_run(
        tmp_path, _quit_harness(route_10051, case_20051, tail, pbem=True), "quit-pbem"
    ) == 0
    assert _compile_and_run(
        tmp_path, _quit_harness(route_10051, case_20051, tail, pbem=False), "quit-single"
    ) == 0
    # Native retirement confirmation exits/logs after its optional network
    # hang-up. It must still do so when the confirming user is single-player.
    assert "ExitTurnLoop = 1;" in tail and "log_say(\"Quit***********************************\"" in tail

    network_guard = tail.index("if (IsMultiplayerNet != 0) {")
    exit_store = tail.index("ExitTurnLoop = 1;", network_guard)
    net_guard_end = tail.index("\n        }", network_guard)
    assert exit_store > net_guard_end

    damaged_case = case_20051.replace("goto native_retirement_confirmation;", "break;", 1)
    assert damaged_case != case_20051
    assert _compile_and_run(
        tmp_path, _quit_harness(route_10051, damaged_case, tail, pbem=True), "quit-dispatch-mutant"
    ) != 0

    damaged_tail = tail.replace("if (IsMultiplayerPBEM != 0) {", "if (IsMultiplayerPBEM == 0) {", 1)
    assert damaged_tail != tail
    assert _compile_and_run(
        tmp_path, _quit_harness(route_10051, case_20051, damaged_tail, pbem=True), "quit-state-mutant"
    ) != 0

    damaged_network_guard = tail.replace(
        "ExitTurnLoop = 1;", "if (IsMultiplayerNet != 0) ExitTurnLoop = 1;", 1
    ).replace(
        'log_say("Quit***********************************", 0, 0, 0);',
        'if (IsMultiplayerNet != 0) log_say("Quit***********************************", 0, 0, 0);', 1
    )
    assert damaged_network_guard != tail
    assert _compile_and_run(
        tmp_path,
        _quit_harness(route_10051, case_20051, damaged_network_guard, pbem=False),
        "quit-network-guard-mutant",
    ) != 0


def test_candidate_netmsg_activity_uses_existing_receiver_members_and_order(tmp_path):
    proposal = actual_source()
    first_gate = _selected(proposal, "if (a2 == 0x0D || a1 == 0x20) {")
    later_gate = _selected(
        proposal,
        "if (NetMsgPrimary.is_active() != 0 ||\n            NetMsgSecondary.is_active() != 0) {",
    )
    assert _compile_and_run(
        tmp_path, _netmsg_activity_harness(first_gate, later_gate), "netmsg-active"
    ) == 0

    wrong_owner = first_gate.replace("NetMsgPrimary.is_active()", "NetMsgSecondary.is_active()", 1)
    assert wrong_owner != first_gate
    assert _compile_and_run(
        tmp_path, _netmsg_activity_harness(wrong_owner, later_gate), "netmsg-active-mutant"
    ) != 0


def test_candidate_modal_visible_guard_and_scenario_y_join(tmp_path):
    proposal = actual_source()
    modal_guard_start = proposal.index("// Native 0x517FB8 tests TutWinBaseWindow::is_visible")
    visibility_guard = _block(proposal[modal_guard_start:],
                              "if (TutWinBaseWindow.is_visible() == 0) {")
    modal_start = proposal.index("// Modal BaseWin event routing")
    modal_switch_start = proposal.index("switch (a2) {", modal_start)
    modal_switch = _block(proposal[modal_switch_start:], "switch (a2) {")

    modal_switch = modal_switch.replace("TutWinBaseWindow.exit();", "exit_window();").replace(
        "TutWinBaseWindow.prev();", "prev_window();").replace("TutWinBaseWindow.next();", "next_window();")
    prefix = r'''
#include <assert.h>
struct TutWindowStub { int visible; int is_visible() const { return visible; } } TutWinBaseWindow;
int button_clicks;
struct ButtonGroupStub {
    int field_84_;
    void button_click(int id) { assert(id == 0x3E9); ++button_clicks; }
};
struct MainInterfaceStub {
    ButtonGroupStub group;
    ButtonGroupStub &button_group(int) { return group; }
} MainInterfaceGlobal;
int exit_calls, prev_calls, next_calls, raw_and_numeric_routes;
void exit_window() { ++exit_calls; }
void prev_window() { ++prev_calls; }
void next_window() { ++next_calls; }
'''
    # Compile the actual guard/table blocks after binding their source-owned
    # calls to counters; the harness keeps the false edge separate from later
    # command routing just as the native 0x518179 target does.
    route = r'''
int dispatch(int visible, int a2, int button_state) {
    TutWinBaseWindow.visible = visible;
    MainInterfaceGlobal.group.field_84_ = button_state;
    (void)a2;
VISIBILITY
    ++raw_and_numeric_routes;
    return 1;
native_after_basewindow_raw_map:
    return 2;
}
int modal(int a2) {
MODAL
    return 1;
native_done:
    return 3;
}
int main() {
    assert(dispatch(0, 0x30, 0) == 2 && raw_and_numeric_routes == 0);
    assert(dispatch(0, 0x30, 0x3E8) == 2 && button_clicks == 1);
    assert(dispatch(1, 0x30, 0x3E8) == 1 && raw_and_numeric_routes == 1);
    assert(modal(0x0D) == 3 && exit_calls == 1);
    assert(modal(0x21) == 3 && prev_calls == 1);
    assert(modal(0x27) == 3 && next_calls == 1);
    assert(modal(0x30) == 1);
    return 0;
}
'''.replace("VISIBILITY", visibility_guard).replace("MODAL", modal_switch)
    source = prefix + route
    assert _compile_and_run(tmp_path, source, "modal-visible") == 0

    damaged_guard = visibility_guard.replace(
        "if (TutWinBaseWindow.is_visible() == 0) {",
        "if (TutWinBaseWindow.is_visible() != 0) {", 1
    )
    assert damaged_guard != visibility_guard
    assert _compile_and_run(
        tmp_path, prefix + route.replace(visibility_guard, damaged_guard), "modal-guard-mutant"
    ) != 0

    damaged_handoff = visibility_guard.replace(
        "if (MainInterfaceGlobal.button_group(0).field_84_ == 0x3E8) {\n"
        "        MainInterfaceGlobal.button_group(0).button_click(0x3E9);\n"
        "    }\n",
        "",
        1,
    )
    assert damaged_handoff != visibility_guard
    assert _compile_and_run(
        tmp_path, prefix + route.replace(visibility_guard, damaged_handoff),
        "modal-button-handoff-mutant",
    ) != 0

    y_route = _selected(proposal, "if (normalized_character == 'y' &&")
    assert _compile_and_run(tmp_path, _y_join_harness(y_route), "scenario-y-join") == 0
    damaged_y = y_route.replace(
        "goto native_editor_command_dispatch;", "goto native_scenario_tail;", 1
    )
    assert damaged_y != y_route
    assert _compile_and_run(tmp_path, _y_join_harness(damaged_y), "scenario-y-join-mutant") != 0


def test_candidate_turn_control_lock_uses_its_real_global(tmp_path):
    proposal = actual_source()
    prelude_start = proposal.index("// Native 0x517BA7..0x517BDF")
    lock_block = _block(proposal[prelude_start:], "if (IsMultiplayerNet != 0) {")
    assert "ConsoleControlTurnActive != 0" in lock_block
    assert "UnkGlobal0093A934 != 0" not in lock_block
    assert _compile_and_run(
        tmp_path, _control_turn_harness(lock_block), "turn-control-lock"
    ) == 0
    damaged = lock_block.replace(
        "ConsoleControlTurnActive != 0", "UnkGlobal0093A934 != 0", 1
    )
    assert damaged != lock_block
    assert _compile_and_run(
        tmp_path, _control_turn_harness(damaged), "turn-control-lock-mutant"
    ) != 0


def test_candidate_entry_exit_and_masked_command_guards(tmp_path):
    proposal = actual_source()
    guard = _selected(proposal, "if (ExitTurnLoop != 0 ||")
    assert _compile_and_run(
        tmp_path, _entry_guard_harness(guard), "entry-guards"
    ) == 0
    damaged = guard.replace("ExitTurnLoop != 0 ||", "0 ||", 1)
    assert damaged != guard
    assert _compile_and_run(
        tmp_path, _entry_guard_harness(damaged), "entry-exit-mutant"
    ) != 0
    # Keep a well-typed mask/equality expression so the mutant reaches the
    # behavioral assertion instead of being rejected as a tautological test.
    damaged = guard.replace("0xFFFCFFFFu", "0xFFFDFFFFu", 1)
    assert damaged != guard
    assert _compile_and_run(
        tmp_path, _entry_guard_harness(damaged), "entry-mask-mutant"
    ) != 0


def test_candidate_full_function_is_lexically_balanced():
    proposal = actual_source()
    assert _selected(proposal, "int Console::on_key_click(int a1, int a2)").endswith("}")


def test_netmsg_active_field_native_header_offset_witness():
    netmsg_header = (ROOT / "src/netmsg.h").read_text()
    popup_header = (ROOT / "src/popup.h").read_text()
    netmsg_source = (ROOT / "src/netmsg.cpp").read_text()

    assert "static_assert(sizeof(Popup) == 0x537C" in popup_header
    members = (
        "uint32_t field_537C_;",
        "uint32_t field_5380_;",
        "int32_t field_5384_;",
        "int32_t field_5388_;",
    )
    positions = [netmsg_header.index(member) for member in members]
    assert positions == sorted(positions)
    assert "bool is_active() const;" in netmsg_header
    assert "return field_5384_ != 0;" in netmsg_source
    assert "RECOVERY_OBJECT: v1 0x00805338 ?NetMsgPrimary@@3VNetMsg@@A" in netmsg_source
    assert "RECOVERY_OBJECT: v1 0x007FFF80 ?NetMsgSecondary@@3VNetMsg@@A" in netmsg_source
    assert 0x7FFF80 + 0x5384 == 0x805304
    assert 0x805338 + 0x5384 == 0x80A6BC


def _pan_harness(block):
    return r'''
#include <assert.h>
int MapLatitudeBounds, MapLongitudeBounds, MapIsFlat;
struct MapWin {
    int field_1DD9C_, field_1DDA0_, field_1DDA8_;
    int field_1DDCC_, field_1DDD0_, field_1DDD8_, calls;
    void set_center(int x, int y, int mode) {
        assert(mode == 1);
        field_1DD9C_ = x; field_1DDA0_ = y; ++calls;
    }
} map_window;
MapWin *MapWinTable[] = {&map_window};
void pan(int a2) {
PAN_BLOCK
}
void reset(int x=20, int y=20) {
    MapLatitudeBounds=40; MapLongitudeBounds=80; MapIsFlat=0;
    map_window.field_1DD9C_=x; map_window.field_1DDA0_=y;
    map_window.field_1DDA8_=18; map_window.field_1DDD8_=10;
    map_window.field_1DDCC_=16; map_window.field_1DDD0_=16;
    map_window.calls=0;
}
int main() {
    const int north[]={0x10026,0x10068}, south[]={0x10028,0x10062};
    const int west[]={0x10025,0x10064}, east[]={0x10027,0x10066};
    for (int i=0;i<2;++i) {
        reset(); pan(north[i]);
        assert(map_window.field_1DDA0_==14 && map_window.calls==1);
        reset(); pan(south[i]);
        assert(map_window.field_1DDA0_==23 && map_window.calls==1);
        reset(); MapLatitudeBounds=10; pan(south[i]);
        assert(map_window.field_1DDA0_==20 && map_window.calls==0);
        reset(); map_window.field_1DDA8_=-1; pan(north[i]);
        assert(map_window.field_1DDA0_==20 && map_window.calls==0);
        reset(1); MapLatitudeBounds=10; pan(west[i]);
        assert(map_window.field_1DD9C_==75 && map_window.field_1DDA0_==20);
        reset(79); MapLatitudeBounds=10; pan(east[i]);
        assert(map_window.field_1DD9C_==5 && map_window.field_1DDA0_==20);
        reset(1); MapIsFlat=1; pan(west[i]);
        assert(map_window.field_1DD9C_==-5);
        reset(79); MapIsFlat=1; pan(east[i]);
        assert(map_window.field_1DD9C_==85);
        reset(); map_window.field_1DDCC_=-8; pan(west[i]);
        assert(map_window.field_1DD9C_==22);
        reset(); map_window.field_1DDCC_=-8; pan(east[i]);
        assert(map_window.field_1DD9C_==18);
    }
    reset(); pan(0x12345);
    assert(map_window.calls==0);
    return 0;
}
'''.replace("PAN_BLOCK", block)


def test_actual_owner_pan_geometry_and_mutants(tmp_path):
    source = actual_source()
    block = _selected(source, "// The 0x51BB5C byte map's input domain")
    assert _compile_and_run(tmp_path, _pan_harness(block), "pan-owner") == 0
    south = block.replace("step > maximum", "step > map_window->field_1DDA8_", 1)
    south = south.replace("step = maximum;", "step = map_window->field_1DDA8_;", 1)
    assert south != block
    assert _compile_and_run(tmp_path, _pan_harness(south), "pan-south-mutant") != 0
    anchor = "int step = (map_window->field_1DDCC_ + 2) / 3;"
    assert block.count(anchor) == 1
    horizontal = block.replace(anchor, anchor + "\n"
        "int wrong_maximum=MapLatitudeBounds-2*map_window->field_1DDD8_"
        "-map_window->field_1DDA8_+1;\n"
        "if (step<0 || wrong_maximum<0) step=0;\n"
        "else if (step>wrong_maximum) step=wrong_maximum;", 1)
    assert _compile_and_run(tmp_path, _pan_harness(horizontal), "pan-horizontal-mutant") != 0


def test_actual_owner_editor_load_argument_order(tmp_path):
    source = actual_source()
    matches = re.findall(r"case 0x30076:.*?break;", source, re.S)
    assert len(matches) == 1
    route, = matches
    harness = r'''
#include <assert.h>
int calls, selected_slot, selected_faction;
void load_proc(int slot, int faction) {
    ++calls; selected_slot=slot; selected_faction=faction;
}
void dispatch(int a2) {
    switch (a2) { ROUTE default: break; }
}
int main() {
    dispatch(0x30076);
    assert(calls==1 && selected_slot==1 && selected_faction==0);
    dispatch(0x12345); assert(calls==1);
    return 0;
}
'''
    assert _compile_and_run(tmp_path, harness.replace("ROUTE", route), "editor-load") == 0
    mutant = route.replace("load_proc(1, 0)", "load_proc(-1, 1)", 1)
    assert mutant != route
    assert _compile_and_run(tmp_path, harness.replace("ROUTE", mutant), "editor-load-mutant") != 0
