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


def test_time_control_seed_cleanup_and_direct_caller_binding():
    assert not (ROOT / "src/recovered/units/00523c60.cpp").exists()
    assert not (ROOT / "src/recovered/units/00589330.cpp").exists()
    assert "time_controls_dialog(&pop_menu);" in (ROOT / "src/netwin.cpp").read_text()
    assert "sub_589330" not in (ROOT / "src/netwin.cpp").read_text()
