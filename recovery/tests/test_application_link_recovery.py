"""Live C++ link-closure controls, not a claim that the application runs.

All binary observations are recomputed from the current source. No original
executable-derived object, disassembly or fixture is stored in the repository.
"""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file
from match_recovery.processes import run_bounded


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
                     "src/menu.cpp", "src/popmenu.cpp", "src/temp.cpp", "src/console.cpp",
                     "src/netdaemon.cpp", "src/statuswin.cpp", "src/popup.cpp", "src/scroll.cpp",
                     "src/base.cpp", "src/basewin.cpp")
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


def test_base_lock_references_shared_state_and_real_base_fields(compiled):
    function = symbol_at("src/netdaemon.cpp", 0x530D50)
    expected = {"?ConsoleControlTurnActive@@3HA", "?IsMultiplayerNet@@3HA",
                "?BaseIDCurrentSelected@@3HA", "?UnkGlobal0093A934@@3HA",
                "?Bases@@3PAUBase@@A", "?lock@NetDaemon@@QAEHHHHHHH@Z",
                "?message_data@@YAIHHHHHH@Z"}
    for obj in compiled["src/netdaemon.cpp"]:
        targets = references(obj, function)
        assert expected <= set(targets), targets
        layout = class_layout(obj, "NetDaemon")
        # The blocked flag is the fourth word of the first lock-state entry,
        # not a scalar reached through a literal receiver offset.
        assert layout["members"]["lock_state_"] == 0x1B2C
        base = class_layout(obj, "Base")
        assert base["size"] == 0x134
        assert {name: base["members"][name] for name in ("x", "y", "state")} == {
            "x": 0, "y": 2, "state": 0x30}
        assert not any("Original" in target or "vtable_slot" in target for target in targets)


def test_turn_control_is_defined_once_and_referenced_by_the_console(compiled):
    symbol = "?ConsoleControlTurnActive@@3HA"
    for obj in compiled["src/console.cpp"]:
        definitions = [row for row in obj.symbols.values()
                       if row.name == symbol and row.section > 0]
        assert len(definitions) == 1
        assert symbol in references(obj, symbol_at("src/console.cpp", 0x5108A0))


def test_status_handler_uses_source_actions_and_owned_status_fields(compiled):
    expected = {"?soft_zoom@BaseWin@@QAEXH@Z", "?production@BaseWin@@QAEXHH@Z",
                "?hurry@BaseWin@@QAEXXZ", "?gov_on@BaseWin@@QAEXH@Z",
                "?set_move@Console@@QAEXH@Z", "?editor_edit_veh@Console@@QAEXH@Z",
                "?redraw@StatusWin@@QAEXXZ", "?ConsoleControlTurnActive@@3HA"}
    for obj in compiled["src/statuswin.cpp"]:
        targets = references(obj, symbol_at("src/statuswin.cpp", 0x4B9010))
        assert expected <= set(targets), targets
        assert not any("Original" in target or "vtable_slot" in target for target in targets)
        layout = class_layout(obj, "StatusWin")
        expected_members = {"font1_": 0x1530, "font2_": 0x1558, "font3_": 0x1580,
                            "iUnitIndex_": 0x15BC, "iCityIndex_": 0x15C0, "spot_": 0x15E0}
        assert {name: layout["members"][name] for name in expected_members} == expected_members


def test_popup_family_destroys_each_owned_subobject_once(compiled):
    popup_destructor = symbol_at("src/popup.cpp", 0x406C00)
    menu_destructor = symbol_at("src/popmenu.cpp", 0x421400)
    pull_destructor = symbol_at("src/pulldown.cpp", 0x5F88A0)
    for obj in compiled["src/popmenu.cpp"]:
        targets = references(obj, menu_destructor)
        assert targets.count(pull_destructor) == 1
        assert targets.count(popup_destructor) == 1
    for obj in compiled["src/popup.cpp"]:
        targets = references(obj, popup_destructor)
        assert targets.count(symbol_at("src/scroll.cpp", 0x406E60)) == 1
        base_destructor = symbol_at("src/basepop.cpp", 0x4064D0)
        if base_destructor in targets:
            assert targets.count(base_destructor) == 1
        else:
            # BasePop's in-class destructor expands here. Check its close
            # and two uniquely owned members, not a nonexistent call edge.
            assert targets.count("?close@BasePop@@QAEXXZ") == 1
            assert targets.count("??1Spot@@QAE@XZ") == 1
            assert targets.count("??1Heap@@QAE@XZ") == 1
        assert not any("Original" in target for target in targets)
    for obj in compiled["src/scroll.cpp"]:
        targets = references(obj, symbol_at("src/scroll.cpp", 0x406E60))
        assert targets.count("?close@Scroll@@QAEIXZ") == 1
        assert targets.count("??1GraphicWin@@UAE@XZ") == 1
        assert not any("?destroy@" in target or "Original" in target for target in targets)


def test_base_computation_owns_its_shared_state_and_orders_resource_passes(compiled):
    pointer = "?BaseCurrent@@3PAUBase@@A"
    cache = "?BaseComputedID@@3HA"
    stages = ["?base_support@@YAXXZ", "?base_yield@@YAXXZ", "?base_nutrient@@YAXXZ",
              "?base_minerals@@YAXXZ", "?base_energy@@YAXXZ"]
    for obj in compiled["src/base.cpp"]:
        for symbol in (pointer, cache):
            definitions = [row for row in obj.symbols.values() if row.name == symbol and row.section > 0]
            assert len(definitions) == 1
        targets = references(obj, symbol_at("src/base.cpp", 0x4EC3B0))
        assert [target for target in targets if target in stages] == stages
        assert cache in targets and "?BaseIDCurrentSelected@@3HA" in targets
        assert pointer in references(obj, symbol_at("src/base.cpp", 0x4E39D0))
    for obj in compiled["src/popup.cpp"]:
        assert pointer in references(obj, symbol_at("src/popup.cpp", 0x4456B0))
    for obj in compiled["src/basewin.cpp"]:
        layout = class_layout(obj, "BaseWin")
        assert layout["members"]["displayed_production_id_"] == 0x40B34
        targets = references(obj, symbol_at("src/basewin.cpp", 0x41AE40))
        assert "?base_compute@@YAXH@Z" in targets and cache in targets
        assert "?set_base@@YAXH@Z" in targets
        assert "?is_visible@Win@@QAEHXZ" in targets
        assert "?lock_base@NetDaemon@@QAEHHHHH@Z" in targets
        assert "?unlock_base@NetDaemon@@QAEXH@Z" in targets


def run_recovered_harness(tmp_path, monkeypatch, harness_name, handlers):
    """Compile actual source bodies with doubles; not full-game acceptance."""
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("the project compiler and Wine32 are required")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    harness = Path(__file__).with_name(harness_name).read_text()
    for anchor, unit, address in handlers:
        record, = (row for row in read_file(ROOT / unit) if row.address == address)
        assert record.region and not record.extract_error
        assert harness.count(anchor) == 1
        harness = harness.replace(anchor, record.region)
    source = tmp_path / "menu.cpp"
    source.write_text(harness)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-")
    (tmp_path / "menu.obj").write_bytes(compile_object(source, command, flags))
    result = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                          "/subsystem:console", "/incremental:no", "/out:menu.exe",
                          "menu.obj", "kernel32.lib"], cwd=tmp_path, timeout=60)
    assert result.returncode == 0, (result.stdout + result.stderr).decode(errors="replace")
    with tempfile.TemporaryFile() as diagnostics:
        result = subprocess.run(["wine32", str(tmp_path / "menu.exe")], env=environment(),
                                stdout=diagnostics, stderr=diagnostics, timeout=30)
        diagnostics.seek(0)
        output = diagnostics.read().decode(errors="replace")
    assert result.returncode == 0, output


def test_status_menu_runtime_branches_use_the_actual_recovered_handler(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "status_menu_harness.cpp", [
        ("// RECOVERED_HANDLER is replaced with read_file(...).region, never a copy.",
         "src/statuswin.cpp", 0x4B9010)])


def test_soft_zoom_and_computation_runtime_use_actual_recovered_bodies(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "base_compute_harness.cpp", [
        ("// RECOVERED_COMPUTE", "src/base.cpp", 0x4EC3B0),
        ("// RECOVERED_SOFT_ZOOM", "src/basewin.cpp", 0x41AE40)])
