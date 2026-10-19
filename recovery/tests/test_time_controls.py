"""Source-driven host controls for the integrated time-control pair.

This proposal deliberately reads the real annotated owner bodies on every run;
the C++ fixture supplies only readable host projections. It is not an ABI,
native matching, linked-preservation, or game-runtime check.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
from test_game_logic_seed_batch_behavior import _definition


HARNESS = Path(__file__).with_name("time_controls_harness.cpp")
FUNCTIONS = {
    0x00523C60: ("src/game.cpp", "?set_time_controls@@YAXXZ",
                 "void __cdecl set_time_controls(", "EXTRACTED_SET_TIME_CONTROLS"),
    0x00589330: ("src/popup.cpp", "?time_controls_dialog@@YAXPAUPopup@@@Z",
                 "void __cdecl time_controls_dialog(",
                 "EXTRACTED_TIME_CONTROLS_DIALOG"),
}


def _actual_definition(address):
    owner, decorated, signature, _marker = FUNCTIONS[address]
    records = [row for row in read_file(ROOT / owner)
               if row.address == address]
    assert len(records) == 1, f"{owner}: expected one {address:#x} record"
    record = records[0]
    assert record.mode == Mode.BODY
    assert record.name == decorated
    assert record.region and not record.extract_error
    definition = _definition(record.region, signature)
    assert "BODY GOES HERE." not in definition
    return definition


def _extract_sources():
    bodies = {address: _actual_definition(address) for address in FUNCTIONS}
    setter = bodies[0x00523C60]
    dialog = bodies[0x00589330]

    # The source uses the StringTable pointer overload; its host projection
    # maps that same pointer identity to the fixture's 32-bit key.
    assert dialog.count("StringTable.get(TimeControl[index].name)") == 1
    assert dialog.count("nullptr") == 2
    dialog = dialog.replace("nullptr", "0")  # C++98 host syntax only
    return setter, dialog


def _actual_console_time_controls():
    source = (ROOT / "src/console.cpp").read_text()
    header = (ROOT / "src/console_commands.h").read_text()
    method = _definition(source, "void Console::time_controls()")
    formatter = _definition(header, "static void parse_field_number(")
    clamp = _definition(source, "static int parse_turn_time_clamped(")
    assert "parse_field_number(6, TimeControl[5].accumulated);" in method
    assert "0x270F" in clamp
    return method, formatter, clamp


TIME_CONTROLS_HARNESS = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
using std::string;
using std::vector;

struct RulesTimeControl {
    int base, unit, event, turn, extra, refresh, accumulated;
} TimeControl[6];
struct char256 { char str[256]; };
char StringTemp[256];
char256 ParseStrBuffer[7];
static char *PopupStartCaption = const_cast<char *>("host caption");
static char ConsoleStrCustomTime[] = "CUSTOMTIME";
static char ConsoleStrHostTime[] = "HOSTTIME";
static int expected_fields[7] = {11, 12, 13, 14, 15, 16, 17};
static int parsed_fields[7];
static string parsed_values[7];
static int parse_calls, start_calls, dialog_calls, exec_calls, close_calls;
static int exec_results[2], accept_input, failures;
static int message_calls, message_result, sync_calls;

static char *_itoa(int value, char *buffer, int radix) {
    assert(radix == 10);
    std::sprintf(buffer, "%d", value);
    return buffer;
}
int parse_says(int index, char *text, int gender, int plural) {
    if (gender != -1 || plural != -1 || parse_calls >= 7) failures = 1;
    if (parse_calls < 7) {
        parsed_fields[parse_calls] = index;
        parsed_values[parse_calls] = text;
    }
    ++parse_calls;
    if (index < 0 || index >= 7) failures = 1;
    else std::strncpy(ParseStrBuffer[index].str, text, sizeof(ParseStrBuffer[index].str) - 1);
    return 0;
}
static int console_turns_assigned() { return 1; }
static int IsMultiplayerNet = 1;
struct NetMessage { void pop(char *, int, int, int) {} } NetMsgPrimary;
struct NetDaemon {
    void synch(int code, int a, int b, int c, void *data, int d, int e) {
        if (code != 0x85 || a || b || c || data || d != 1 || e != 0x2101) failures = 1;
        ++sync_calls;
    }
} NetDaemonNet;
struct TimeControlFields { int time_controls_selection_; };
struct { TimeControlFields fields_; } GameDataStateStorage;
static void time_controls_dialog(class Popup *popup);
static void message_data(int code, int a, int result, int c, int d, int e) {
    if (code != 0x4C02 || a || c || d || e) failures = 1;
    ++message_calls;
    message_result = result;
}

class Popup {
public:
    void close() { ++close_calls; }
    void set_value(int) {}
    int exec(int, int) {
        if (exec_calls >= 2) { failures = 1; return -99; }
        return exec_results[exec_calls++];
    }
    int start(char *caption, const char *label, int value, char *text,
              int flags, void *sprite) {
        ++start_calls;
        if (caption != PopupStartCaption || std::strcmp(label, ConsoleStrCustomTime) ||
            value != -1 || text || flags != 0x44 || sprite || parse_calls != 7)
            failures = 1;
        for (int i = 0; i < 7; ++i) {
            if (parsed_fields[i] != i || parsed_values[i] != std::to_string(expected_fields[i]))
                failures = 1;
        }
        if (accept_input) {
            const char *inputs[7] = {"10001", "-2", "33", "0", "5", "6", "10000"};
            for (int i = 0; i < 7; ++i)
                std::strncpy(ParseStrBuffer[i].str, inputs[i], sizeof(ParseStrBuffer[i].str) - 1);
        }
        return 0;
    }
};
static void time_controls_dialog(Popup *) { ++dialog_calls; }

/*ACTUAL_PARSE_FIELD_NUMBER*/
/*ACTUAL_PARSE_TURN_TIME_CLAMPED*/
struct Console { void time_controls(); };
/*ACTUAL_TIME_CONTROLS*/

static void reset_case(bool accept) {
    std::memset(TimeControl, 0, sizeof(TimeControl));
    TimeControl[5].base = 11; TimeControl[5].unit = 12;
    TimeControl[5].event = 13; TimeControl[5].turn = 14;
    TimeControl[5].extra = 15; TimeControl[5].refresh = 16;
    TimeControl[5].accumulated = 17;
    std::memset(ParseStrBuffer, 0, sizeof(ParseStrBuffer));
    parse_calls = start_calls = dialog_calls = exec_calls = close_calls = 0;
    message_calls = sync_calls = failures = 0;
    message_result = -99;
    accept_input = accept;
    exec_results[0] = 5;
    exec_results[1] = accept ? 0 : -1;
}

static void verify_custom_fields_before_popup() {
    reset_case(false);
    Console console;
    console.time_controls();
    assert(failures == 0);
    assert(parsed_fields[0] == 0 && parsed_fields[1] == 1 && parsed_fields[2] == 2 &&
           parsed_fields[3] == 3 && parsed_fields[4] == 4 && parsed_fields[5] == 5 &&
           parsed_fields[6] == 6);
    assert(parsed_values[0] == "11" && parsed_values[1] == "12" &&
           parsed_values[2] == "13" && parsed_values[3] == "14" &&
           parsed_values[4] == "15" && parsed_values[5] == "16" &&
           parsed_values[6] == "17");
    assert(start_calls == 1 && dialog_calls == 1 && close_calls == 1);
    assert(message_calls == 1 && message_result == 5 && sync_calls == 0);
    assert(TimeControl[5].base == 11 && TimeControl[5].unit == 12 &&
           TimeControl[5].event == 13 && TimeControl[5].turn == 14 &&
           TimeControl[5].extra == 15 && TimeControl[5].refresh == 16 &&
           TimeControl[5].accumulated == 17);
}

static void acceptance_keeps_clamps_and_synchronizes() {
    reset_case(true);
    Console console;
    console.time_controls();
    assert(failures == 0);
    assert(TimeControl[5].base == 9999 && TimeControl[5].unit == 0 &&
           TimeControl[5].event == 33 && TimeControl[5].turn == 0 &&
           TimeControl[5].extra == 5 && TimeControl[5].refresh == 6 &&
           TimeControl[5].accumulated == 9999);
    assert(sync_calls == 1 && message_calls == 1 && message_result == 5);
    assert(start_calls == 1 && close_calls == 1);
}

int main() {
    verify_custom_fields_before_popup();
    acceptance_keeps_clamps_and_synchronizes();
    std::puts("PASS: actual Console time_controls host control");
}
'''


def _run_actual_time_controls(tmp_path, method, formatter, clamp, compiler):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = TIME_CONTROLS_HARNESS.replace("/*ACTUAL_PARSE_FIELD_NUMBER*/", formatter)
    source = source.replace("/*ACTUAL_PARSE_TURN_TIME_CLAMPED*/", clamp)
    source = source.replace("/*ACTUAL_TIME_CONTROLS*/", method)
    unit = tmp_path / "actual_time_controls.cpp"
    executable = tmp_path / "actual_time_controls"
    unit.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(unit), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def _replace_once(text, old, new):
    assert text.count(old) == 1, f"mutation anchor not unique: {old!r}"
    return text.replace(old, new)


def _render(setter, dialog):
    source = HARNESS.read_text()
    for address, body in ((0x00523C60, setter), (0x00589330, dialog)):
        marker = "/*" + FUNCTIONS[address][3] + "*/"
        assert source.count(marker) == 1
        source = source.replace(marker, body)
    return source


def _compile_and_run(tmp_path, setter, dialog, compiler):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "time_controls_host.cpp"
    executable = tmp_path / "time_controls_host"
    source.write_text(_render(setter, dialog))
    command = [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
               "-pedantic", "-fsanitize=address,undefined",
               "-fno-sanitize-recover=all", str(source), "-o", str(executable)]
    built = subprocess.run(command, text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          env={**__import__("os").environ,
                               "ASAN_OPTIONS": "detect_leaks=0"})


def _mutated_cases(setter, dialog):
    cases = []
    cases.append(("base-weight", _replace_once(
        setter, "weighted_base_count += control.base *",
        "weighted_base_count += control.unit *"), dialog))
    cases.append(("active-divisor", _replace_once(
        setter, ") / active_factions +", ") / MaxPlayerNum +"), dialog))
    cases.append(("vehicle-order-one", _replace_once(
        setter, "vehicle.order == 1", "vehicle.order == 3"), dialog))
    cases.append(("vehicle-order-two", _replace_once(
        setter, "vehicle.order == 2", "vehicle.order == 3"), dialog))
    cases.append(("vehicle-scan-start", _replace_once(
        setter, "int vehicle_id = 0;", "int vehicle_id = 1;"), dialog))
    cases.append(("vehicle-state-200", _replace_once(
        setter, "vehicle.state & 0x200u", "vehicle.state & 0x400u"), dialog))
    cases.append(("vehicle-state-composite", _replace_once(
        setter, "(vehicle.state & 0x01000200u) == 0x01000200u",
        "(vehicle.state & 0x01000000u) == 0x01000000u"), dialog))
    cases.append(("vehicle-state-4000", _replace_once(
        setter, "vehicle.state & 0x4000u", "vehicle.state & 0x8000u"), dialog))
    cases.append(("network-slot", _replace_once(
        setter, "trailing_words_[8]", "trailing_words_[7]"), dialog))
    cases.append(("early-turn-boundary", _replace_once(
        setter, "GameDataStateStorage.fields_.turn_current_num_ < 6",
        "GameDataStateStorage.fields_.turn_current_num_ <= 6"), dialog))
    cases.append(("candidate-maximum", _replace_once(
        setter, "seconds <= candidate", "seconds >= candidate"), dialog))
    cases.append(("refresh-active-only", _replace_once(
        setter, "if (ConsoleControlTurnActive != 0 && ControlTurnPhase == 0 &&",
        "if ((active_mask & (1u << faction_id)) != 0 &&\n"
        "            ConsoleControlTurnActive != 0 && ControlTurnPhase == 0 &&"),
        dialog))
    cases.append(("refresh-cap", _replace_once(
        setter, "            if (next_bonus >= control.accumulated) {\n"
        "                next_bonus = control.accumulated;\n"
        "            }\n", ""), dialog))
    cases.append(("dialog-base-unit", setter, _replace_once(
        dialog, "_itoa(TimeControl[index].base, base_text, 10);",
        "_itoa(TimeControl[index].unit, base_text, 10);")))
    cases.append(("dialog-label-slot", setter, _replace_once(
        dialog, "label_get(0x1094 / 4)", "label_get(0x1090 / 4)")))
    cases.append(("dialog-plus-spacing", setter, _replace_once(
        dialog, 'strcat(StringTemp, " + ");',
        'strcat(StringTemp, "+");')))
    cases.append(("dialog-slot-order", setter, _replace_once(
        dialog, "popup->dialogs().item(StringTemp, index);",
        "popup->dialogs().item(StringTemp, 5 - index);")))
    cases.append(("dialog-six-rows", setter, _replace_once(
        dialog, "index < MaxTimeControlNum", "index < MaxTimeControlNum - 1")))
    cases.append(("dialog-row-zero-label", setter, _replace_once(
        dialog, "label_get(0x474 / 4)", "label_get(0x470 / 4)")))
    return cases


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_integrated_time_controls_actual_bodies_and_mutations(tmp_path, compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"host compiler unavailable: {compiler}")

    setter, dialog = _extract_sources()
    pristine = _compile_and_run(tmp_path / "pristine", setter, dialog,
                                compiler_path)
    assert pristine.returncode == 0, pristine.stdout + pristine.stderr
    assert "PASS: source-extracted time-control host controls" in pristine.stdout

    for index, (name, changed_setter, changed_dialog) in enumerate(
            _mutated_cases(setter, dialog)):
        case_dir = tmp_path / ("mutation-" + str(index))
        case_dir.mkdir()
        result = _compile_and_run(case_dir, changed_setter, changed_dialog,
                                  compiler_path)
        assert result.returncode != 0, f"mutation survived: {name}"


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_console_time_controls_prefills_all_seven_custom_fields(tmp_path, compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"host compiler unavailable: {compiler}")

    method, formatter, clamp = _actual_console_time_controls()
    actual = _run_actual_time_controls(tmp_path / "actual", method, formatter,
                                       clamp, compiler_path)
    assert actual.returncode == 0, actual.stdout + actual.stderr
    assert "PASS: actual Console time_controls host control" in actual.stdout

    damaged_method = _replace_once(
        method, "        parse_field_number(6, TimeControl[5].accumulated);\n", "")
    damaged = _run_actual_time_controls(tmp_path / "missing-seventh", damaged_method,
                                        formatter, clamp, compiler_path)
    assert damaged.returncode != 0, "missing accumulated prefill survived"


def test_pinned_native_time_control_operands_and_literals():
    """Independent original accesses; host projections do not establish ABI."""
    import os
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE",
                              ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    setter = [(i.mnemonic, i.op_str) for i in decoder.disasm(
        image.read(Span(0x523C60, 0x523DC9)), 0x523C60)]
    for operation in (
        ("mov", "esi, dword ptr [0x9a64e0]"),
        ("and", "eax, 0xff"),
        ("mov", "edi, 0x970b00"),
        ("imul", "ecx, dword ptr [edi - 0xff8]"),
        ("mov", "edx, 0x95282c"),
        ("mov", "al, byte ptr [edx + 0xa]"),
        ("mov", "al, byte ptr [edx + 0xd]"),
        ("test", "ah, 2"),
        ("and", "ebx, 0x1000200"),
        ("test", "ah, 0x40"),
        ("add", "edx, 0x34"),
        ("add", "edi, 0x20cc"),
        ("cmp", "edi, 0x97f094"),
        ("imul", "esi, dword ptr [0x93e968]"),
        ("idiv", "ebx"),
        ("cmp", "eax, 5"),
        ("mov", "dword ptr [0x9a64e4], edx"),
    ):
        assert operation in setter
    assert not any("0x93e964" in arg for _, arg in setter)

    dialog = [(i.mnemonic, i.op_str) for i in decoder.disasm(
        image.read(Span(0x589330, 0x58954A)), 0x589330)]
    assert ("lea", "ebx, [esi + 0x21d0]") in dialog
    assert ("cmp", "edi, 6") in dialog
    for address in (0x406380, 0x6169A0, 0x612A70):
        assert ("call", hex(address)) in dialog
    fields = [(op, arg) for op, arg in dialog
              if op == "mov" and "[esi + 0x94f1" in arg]
    assert fields == [("mov", "eax, dword ptr [esi + 0x94f1b8]"),
                      ("mov", "ecx, dword ptr [esi + 0x94f1bc]"),
                      ("mov", "eax, dword ptr [esi + 0x94f1c0]"),
                      ("mov", "ecx, dword ptr [esi + 0x94f1c4]")]
    for address, literal in ((0x68F57C, b"TIMECONTROLS\0"),
                             (0x68F58C, b"|\0"), (0x68F590, b" + \0"),
                             (0x682E9C, b"(\0"), (0x682E98, b")\0"),
                             (0x682820, b" \0")):
        assert image.read(Span(address, address + len(literal))) == literal

    custom_seed = {i.address: (i.mnemonic, i.op_str) for i in decoder.disasm(
        image.read(Span(0x515B86, 0x515BBE)), 0x515B86)}
    assert custom_seed[0x515B86] == ("mov", "eax, dword ptr [0x94f274]")
    assert custom_seed[0x515BB7] == ("push", "6")
    assert custom_seed[0x515BB9] == ("call", "0x625ec0")


def test_time_control_seed_cleanup_and_direct_caller_binding():
    assert not (ROOT / "src/recovered/units/00523c60.cpp").exists()
    assert not (ROOT / "src/recovered/units/00589330.cpp").exists()
    assert "time_controls_dialog(&pop_menu);" in (ROOT / "src/netwin.cpp").read_text()
    assert "sub_589330" not in (ROOT / "src/netwin.cpp").read_text()
