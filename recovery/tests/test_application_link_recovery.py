"""Live C++ link-closure controls, not a claim that the application runs.

All binary observations are recomputed from the current source. No original
executable-derived object, disassembly or fixture is stored in the repository.
"""

from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file


ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture(scope="module")
def compiled():
    from recovery.environment.wine import VC6_ROOT
    if (not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None
            or not (ROOT / "build/compile_commands.json").is_file()):
        pytest.skip("project compiler and prepared build database are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    return {
        unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
        for unit in ("src/infowin.cpp", "src/pulldown.cpp", "src/graphicwin.cpp",
                     "src/menu.cpp", "src/popmenu.cpp", "src/temp.cpp", "src/console.cpp")
    }


def symbol_at(unit, address):
    record, = (row for row in read_file(ROOT / unit) if row.address == address)
    return record.symbol or record.name


def references(obj, symbol):
    _, relocations, _ = obj.subject(obj.function(symbol))
    return [obj.symbols[int(row.target)].name for row in relocations]


def test_info_text_rows_preserve_the_original_parallel_array_layout(compiled):
    expected = {"pane_rects_": 0x10, "font_": 0x58, "active_pane_": 0x80,
                "pane_states_": 0x88, "row_text_": 0xB8, "row_x_": 0x838,
                "row_y_": 0x898, "row_color_": 0x8F8,
                "row_reveal_color_": 0x958, "row_count_": 0x9B8,
                "text_lengths_": 0x9C0, "visible_characters_": 0x9C8,
                "button_": 0x9D0}
    for obj in compiled["src/infowin.cpp"]:
        layout = class_layout(obj, "InfoWin")
        assert {name: layout["members"][name] for name in expected} == expected
        for name in ("row_text_", "row_x_", "row_y_", "row_color_",
                     "row_reveal_color_", "row_count_"):
            assert layout["member_type_kinds"][name] == "array"


def test_info_draw_reaches_real_update_and_tutorial_definitions(compiled):
    function = symbol_at("src/infowin.cpp", 0x458420)
    update = symbol_at("src/graphicwin.cpp", 0x5D5630)
    arrow = symbol_at("src/tutwin.cpp", 0x4BDEA0)
    for obj in compiled["src/infowin.cpp"]:
        targets = references(obj, function)
        assert update in targets and arrow in targets
        assert "?MainInterfaceGlobal@@3VMainInterface@@A" in targets
        assert "?TutorialWindow@@3VTutWin@@A" in targets
        assert not any("Original" in target or "pending" in target.lower() for target in targets)


def test_pulldown_base_is_constructed_and_destroyed_once(compiled):
    constructor = symbol_at("src/pulldown.cpp", 0x5F8800)
    destructor = symbol_at("src/pulldown.cpp", 0x5F88A0)
    base_constructor = symbol_at("src/graphicwin.cpp", 0x5D4CF0)
    base_destructor = symbol_at("src/graphicwin.cpp", 0x5D4DD0)
    for obj in compiled["src/pulldown.cpp"]:
        assert references(obj, constructor).count(base_constructor) == 1
        assert references(obj, destructor).count(base_destructor) == 1
        assert "_free" in references(obj, destructor)
        layout = class_layout(obj, "PullDown")
        assert layout["size"] == 0xF40
        assert layout["members"]["poll_"] == 0xF1C
        # CodeView currently leaves pointers-to-functions undecoded. This
        # checks the offset, not an independently verified callback signature.


def test_menu_candidates_use_typed_source_definitions(compiled):
    for unit, address in (("src/pulldown.cpp", 0x5F9170),
                          ("src/menu.cpp", 0x5FB830),
                          ("src/popmenu.cpp", 0x59D250),
                          ("src/temp.cpp", 0x5FC9F0)):
        for obj in compiled[unit]:
            targets = references(obj, symbol_at(unit, address))
            assert not any("Original" in target or "pending" in target.lower()
                           or "vtable_slot" in target for target in targets)
    for obj in compiled["src/menu.cpp"]:
        layout = class_layout(obj, "AutoSound")
        assert layout["members"]["event_window_"] == 0x88
        assert layout["member_type_kinds"]["event_window_"] == "pointer"


def test_rectangle_update_keeps_all_three_operations(compiled):
    update = symbol_at("src/graphicwin.cpp", 0x5D5630)
    expected = ("?update_window@Win@@QAEHPAUtagRECT@@@Z",
                "?client_to_screen@Win@@QAEXPAUtagRECT@@@Z",
                "?flip@Win@@SAXPAUtagRECT@@@Z")
    for obj in compiled["src/graphicwin.cpp"]:
        targets = references(obj, update)
        assert all(target in targets for target in expected), targets


def test_survey_loads_the_real_tutorial_receiver_directly(compiled):
    function = symbol_at("src/console.cpp", 0x515530)
    for obj in compiled["src/console.cpp"]:
        code, relocations, _ = obj.subject(obj.function(function))
        receiver, = (row for row in relocations
                     if obj.symbols[int(row.target)].name == "?TutorialWindow@@3VTutWin@@A")
        instruction, = (item for item in Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0)
                         if item.address <= receiver.offset < item.address + item.size)
        # 0x005155EA is MOV ECX, immediate. Taking the object's address
        # through the old pointer alias produced a longer LEA instruction.
        assert instruction.mnemonic == "mov" and instruction.op_str.startswith("ecx,")
