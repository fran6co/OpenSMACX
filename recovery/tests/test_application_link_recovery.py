"""Live C++ link-closure controls, not a claim that the application runs.

All binary observations are recomputed from the current source. No original
executable-derived object, disassembly or fixture is stored in the repository.
"""

from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file
from match_recovery.source.model import State
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
                     "src/base.cpp", "src/basewin.cpp", "src/net_class.cpp", "src/general.cpp")
    }


def symbol_at(unit, address):
    record, = (row for row in read_file(ROOT / unit) if row.address == address)
    return record.symbol or record.name


def references(obj, symbol):
    _, relocations, _ = obj.subject(obj.function(symbol))
    return [obj.symbols[int(row.target)].name for row in relocations]


@pytest.mark.parametrize("unit,address", [
    ("src/base.cpp", 0x4EB560), ("src/base.cpp", 0x4E80B0),
    ("src/base.cpp", 0x4F5F30), ("src/base.cpp", 0x4E5FE0),
    ("src/basewin.cpp", 0x417040), ("src/basewin.cpp", 0x418DF0),
    ("src/console.cpp", 0x511520), ("src/console.cpp", 0x4DDA50),
    ("src/infowin.cpp", 0x458FF0),
])
def test_application_dependencies_have_mapped_product_definitions(compiled, unit, address):
    # Keep passing when a placeholder is genuinely implemented. This protects
    # definition ownership/link continuity, not the current empty spelling.
    record, = (row for row in read_file(ROOT / unit) if row.address == address)
    for obj in compiled[unit]:
        obj.function(record.symbol or record.name)
    if record.state == State.PLACEHOLDER:
        assert not record.byte_exact and not record.semantic


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


def test_net_allocation_node_is_not_a_second_receiver(compiled):
    # Original evidence: Net construction 0x62D763..0x62D817; deleting
    # destructor 0x402CC0 adjusts by -0x58; allocator 0x401ED3 requests 0x60.
    # These bounded constraints do not claim full list/receiver recovery.
    for obj in compiled["src/net_class.cpp"]:
        net = class_layout(obj, "Net")
        node = class_layout(obj, "NetAllocationNode")
        receiver = class_layout(obj, "JackalVoiceRx")
        assert net["members"]["allocation_node_"] == 0x700
        assert net["class_value_members"]["allocation_node_"] == "NetAllocationNode"
        assert node["size"] == 0x60
        assert node["members"]["entries_"] == 0x2C
        assert node["class_value_members"]["entries_"] == "NetUnk72C"
        base, = node["bases"]
        assert base["class"] == "StringAllocationBase" and base["virtual"]
        assert base["vbptr_offset"] == 0 and base["vbtable_index"] == 1
        receiver_base, = receiver["bases"]
        assert receiver["size"] == 0x24 and receiver_base["vbptr_offset"] == 4
        table, = (s for s in obj.symbols.values()
                  if s.name == "??_8NetAllocationNode@@7B@" and s.section > 0)
        section = obj.sections[table.section - 1]
        assert not section.relocations
        assert struct.unpack_from("<ii", section.data, table.value) == (0, 0x58)


def test_net_captures_outer_allocation_owner_before_contained_list(compiled):
    # Original owner stores at 0x62D77C and 0x62D7A2, respectively.
    # The old flattened declaration captured at +0x720 first and reversed
    # +0x754/+0x75C. A matching total sizeof(Net) could not catch that defect.
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for obj in compiled["src/net_class.cpp"]:
        code, _, _ = obj.subject(obj.function(symbol_at("src/net_class.cpp", 0x62D6A0)))
        owner_stores = [i.operands[0].mem.disp for i in decoder.disasm(code, 0)
                        if i.mnemonic == "mov" and i.operands[0].type == X86_OP_MEM
                        and i.operands[0].mem.base and not i.operands[0].mem.index
                        and i.operands[0].mem.disp in (0x720, 0x754, 0x75C)]
        assert owner_stores == [0x75C, 0x754]


def test_receiver_has_typed_list_state_and_one_argument_hooks(compiled):
    for obj in compiled["src/net_class.cpp"]:
        receiver = class_layout(obj, "JackalVoiceRx")
        assert receiver["size"] == 0x24
        assert receiver["members"] == {"head_": 8, "cursor_": 12, "count_": 16,
                                       "cursor_index_": 20, "entries_heap_": 24}
        assert all(receiver["member_type_kinds"][name] == "pointer"
                   for name in ("head_", "cursor_", "entries_heap_"))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        for symbol, expected in (("?on_insert@JackalVoiceRx@@UAEHPAX@Z", [("xor", "eax, eax"), ("ret", "4")]),
                                 ("?on_remove@JackalVoiceRx@@UAEXPAX@Z", [("ret", "4")])):
            code, _, _ = obj.subject(obj.function(symbol))
            # Original shared leaf ABI: zero/ret 4 and ret 4, respectively.
            # This does not assign the unrelated AlphaMovie catalogue names
            # to this receiver, or claim its entire lifetime is recovered.
            instructions = [(i.mnemonic, i.op_str) for i in decoder.disasm(code, 0)]
            assert instructions[:len(expected)] == expected


def test_net_initializes_receiver_state_after_its_own_table(compiled):
    expected_offsets = [0xB8, 0xBC, 0xC0, 0xC4, 0xC8]
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for obj in compiled["src/net_class.cpp"]:
        code, relocations, _ = obj.subject(obj.function(symbol_at("src/net_class.cpp", 0x62D6A0)))
        table, = (row for row in relocations
                  if obj.symbols[int(row.target)].name == "??_7JackalVoiceRx@@6B0@@")
        zero_registers, stores = set(), []
        for instruction in decoder.disasm(code, 0):
            operands = instruction.operands
            _, written = instruction.regs_access()
            zero_registers.difference_update(written)
            if (instruction.mnemonic == "xor" and len(operands) == 2
                    and operands[0].type == operands[1].type == X86_OP_REG
                    and operands[0].reg == operands[1].reg):
                zero_registers.add(operands[0].reg)
            if instruction.mnemonic == "call":
                # Ordinary calling convention preserves the other registers.
                zero_registers.difference_update({reg for reg in zero_registers
                                                 if instruction.reg_name(reg) in ("eax", "ecx", "edx")})
            if instruction.mnemonic != "mov" or len(operands) != 2:
                continue
            destination, value = operands
            if (destination.type == X86_OP_MEM and destination.mem.base and not destination.mem.index
                    and destination.mem.disp in expected_offsets):
                assert instruction.address > table.offset
                assert ((value.type == X86_OP_IMM and value.imm == 0)
                        or (value.type == X86_OP_REG and value.reg in zero_registers))
                stores.append(destination.mem.disp)
        assert stores == expected_offsets


def test_receiver_cleanup_uses_the_shared_layout_and_virtual_owners(compiled):
    # Original insertion bounds the node at 0x1C, distinct from the 0x60
    # payload. Cleanup dispatches through receiver slot 1, Sound slot 2,
    # and the node's virtual allocation-base destructor, respectively.
    for unit in ("src/general.cpp", "src/net_class.cpp", "src/netdaemon.cpp"):
        for obj in compiled[unit]:
            receiver = class_layout(obj, "JackalVoiceRx")
            assert receiver["size"] == 0x24
            assert receiver["members"] == {"head_": 8, "cursor_": 12, "count_": 16,
                                           "cursor_index_": 20, "entries_heap_": 24}
    for obj in compiled["src/general.cpp"]:
        node = class_layout(obj, "VoiceReceiverEntry")
        assert node["size"] == 0x1C
        assert node["members"] == {"id_": 4, "payload_": 8, "next_": 12, "previous_": 16}
        assert node["member_type_kinds"]["payload_"] == "pointer"
        base, = node["bases"]
        assert base["class"] == "StringAllocationBase" and base["virtual"]
        assert base["vbptr_offset"] == 0 and base["vbtable_index"] == 1
        code, relocations, _ = obj.subject(obj.function(symbol_at("src/general.cpp", 0x538EC0)))
        assert not relocations  # All three operations dispatch virtually.
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        calls = [i for i in decoder.disasm(code, 0) if i.mnemonic == "call"]
        assert len(calls) == 3
        assert all(i.operands[0].type == X86_OP_MEM for i in calls)
        assert [i.operands[0].mem.disp for i in calls] == [4, 8, 0]
    for obj in compiled["src/netdaemon.cpp"]:
        targets = references(obj, symbol_at("src/netdaemon.cpp", 0x538D10))
        assert symbol_at("src/general.cpp", 0x538EC0) not in targets
        assert not any("JackalVoiceRxShim" in target or "NetFifoShim" in target for target in targets)
        assert not any("VoiceTx" in target or "NetFifo" in target for target in targets)
        assert targets.count("??1Net@@QAE@XZ") == 1
    for obj in compiled["src/net_class.cpp"]:
        targets = references(obj, "??1Net@@QAE@XZ")
        # VC6 passes the destructor to its exception-safe array iterator;
        # one relocation is a callback for three elements, not one teardown.
        iterator = "??_M@YGXPAXIHP6EX0@Z@Z"
        cleanup = symbol_at("src/general.cpp", 0x538EC0)
        assert targets.count("??1NetFifo@@QAE@XZ") == targets.count(iterator) == 1
        assert targets.count(cleanup) == 1
        assert targets.index(iterator) < targets.index(cleanup) < targets.index("??1Sound@@UAE@XZ")
        code, relocations, _ = obj.subject(obj.function("??1Net@@QAE@XZ"))
        begin, = (r.offset for r in relocations if obj.symbols[int(r.target)].name == "??1NetFifo@@QAE@XZ")
        end, = (r.offset for r in relocations if obj.symbols[int(r.target)].name == iterator)
        setup = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code[begin + 4:end - 1], begin + 4))
        assert [(i.mnemonic, i.op_str) for i in setup[:4]] == [
            ("push", "3"), ("lea", "eax, [edi + 0xe8]"), ("push", "0x24"), ("push", "eax")]


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


def run_recovered_harness(tmp_path, monkeypatch, harness_name, handlers, declarations=()):
    """Compile actual source bodies with doubles; not full-game acceptance."""
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("the project compiler and Wine32 are required")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    harness = Path(__file__).with_name(harness_name).read_text()
    for anchor, unit, start, end in declarations:
        text = (ROOT / unit).read_text()
        assert text.count(start) == text.count(end) == harness.count(anchor) == 1
        first, last = text.index(start), text.index(end)
        assert first < last
        harness = harness.replace(anchor, text[first:last])
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


def test_receiver_cleanup_respects_payload_and_allocation_ownership(tmp_path, monkeypatch):
    # The historical FILE marker intentionally exposes the complete TU as
    # record.region. Select its live definition using unique neighboring anchors.
    run_recovered_harness(tmp_path, monkeypatch, "receiver_cleanup_harness.cpp", [], declarations=[
        ("// LIVE_CLEAR_BODY", "src/general.cpp", "void JackalVoiceRx::clear() {",
         "// ORIGINAL: 0x005D271D"),
        ("// LIVE_ALLOCATION_BASE", "src/stringstruct.h", "class StringAllocationBase {",
         "// REAL VIRTUAL INHERITANCE, like StringStructPayload"),
        ("// LIVE_OWNER_PUBLICATION", "src/stringstruct.cpp", "StringAllocationBase::~StringAllocationBase() {",
         "void StringStruct::unk_slot00()"),
        ("// LIVE_RECEIVER_DECLARATIONS", "src/net_class.h", "struct VoiceReceiverEntry :",
         "// The list subobject at Net+0x72C"),
        ("// LIVE_QUEUE_DECLARATIONS", "src/hypothesis_layouts.h", "class NetFifo {",
         "/* 0x80 bytes, 18 member(s), 18 named. From Thinker. */"),
        ("// LIVE_QUEUE_CONSTRUCTOR", "src/net_class.cpp", "NetFifo::NetFifo() {",
         "// ---------------------------------------------------------------------------\n// THE FIFO QUEUE"),
        ("// LIVE_QUEUE_CLOSE", "src/net_class.cpp", "void NetFifo::close() {",
         "// ORIGINAL: 0x00633AB0"),
        ("// LIVE_QUEUE_DESTRUCTOR", "src/net_class.cpp", "NetFifo::~NetFifo() {",
         "// The wrapped device's own interface"),
    ])


def test_status_menu_runtime_branches_use_the_actual_recovered_handler(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "status_menu_harness.cpp", [
        ("// RECOVERED_HANDLER is replaced with read_file(...).region, never a copy.",
         "src/statuswin.cpp", 0x4B9010)])


def test_soft_zoom_and_computation_runtime_use_actual_recovered_bodies(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "base_compute_harness.cpp", [
        ("// RECOVERED_COMPUTE", "src/base.cpp", 0x4EC3B0),
        ("// RECOVERED_SOFT_ZOOM", "src/basewin.cpp", 0x41AE40)])


def test_governor_runtime_uses_typed_state_and_preserves_dispatch(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "governor_harness.cpp", [
        ("// RECOVERED_GOVERNOR", "src/basewin.cpp", 0x415F70)])


def test_governor_reaches_real_owners_and_base_fields(compiled):
    for obj in compiled["src/basewin.cpp"]:
        targets = references(obj, symbol_at("src/basewin.cpp", 0x415F70))
        for symbol in ("?BaseCurrent@@3PAUBase@@A", "?LockEnableMask@@3IA",
                       "?NetDaemonNet@@3VNetDaemon@@A",
                       "?MainInterfaceGlobal@@3VMainInterface@@A",
                       "?base_reset@@YAXHH@Z", "?base_doctors@@YAXXZ",
                       "?redraw@GraphicWin@@UAEXXZ"):
            assert symbol in targets, targets
        layout = class_layout(obj, "Base")
        expected = {"state": 0x30, "governor": 0x38, "queue_size": 0x4C,
                    "resource_sqr_radius": 0x78, "specialist_total": 0x7C,
                    "specialist_unk_1": 0x80}
        assert {key: layout["members"][key] for key in expected} == expected
