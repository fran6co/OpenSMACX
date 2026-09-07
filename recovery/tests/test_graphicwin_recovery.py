"""Live project regression controls; no executable-derived fixtures are stored."""

from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_AC_READ, CS_AC_WRITE
from capstone.x86 import X86_OP_MEM

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file
from match_recovery.import_source import read_imports, import_targets
from match_recovery.source_inventory import image_pin
from match_recovery.pe_coff import PeImage
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span


ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture(scope="module")
def compiled():
    from recovery.environment.wine import VC6_ROOT
    if (not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None
            or not (ROOT / "build/compile_commands.json").is_file()):
        pytest.skip("project compiler and build database are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / "src/graphicwin.cpp", (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    return compiler, settings, objects


def symbol_at(path, address):
    record, = (record for record in read_file(ROOT / path) if record.address == address)
    return record.symbol or record.name


def calls(obj, symbol):
    _, relocations, _ = obj.subject(obj.function(symbol))
    return [obj.symbols[int(row.target)].name for row in relocations]


def global_accesses(obj, function, symbol):
    code, relocations, _ = obj.subject(obj.function(function))
    offsets = {row.offset for row in relocations if obj.symbols[int(row.target)].name == symbol}
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return [operand.access for instruction in decoder.disasm(code, 0)
            if any(instruction.address <= offset < instruction.address + instruction.size for offset in offsets)
            for operand in instruction.operands if operand.type == X86_OP_MEM]


def test_window_initializer_reaches_its_base_and_uses_native_allocation(compiled):
    _, _, objects = compiled
    derived = symbol_at("src/graphicwin.cpp", 0x005D4EF0)
    base = symbol_at("src/win.cpp", 0x005EBD80)
    for obj in objects:
        targets = calls(obj, derived)
        assert base in targets
        assert derived not in targets  # The previous unqualified call recursed.
        assert "??2@YAPAXI@Z" in targets
        assert not any("WaveOperatorNew" in target for target in targets)


def test_owned_buffer_is_pointer_storage_not_an_embedded_buffer_or_integer(compiled):
    _, _, objects = compiled
    for obj in objects:
        layout = class_layout(obj, "GraphicWin")
        assert layout["size"] == 0xA14
        assert layout["members"]["owned_buffer_"] == 0xA08
        assert layout["member_type_kinds"]["owned_buffer_"] == "pointer"
        assert "owned_buffer_" not in layout["class_value_members"]


def test_layout_override_delegates_to_base_and_occupies_the_original_slot(compiled):
    _, _, objects = compiled
    derived = symbol_at("src/graphicwin.cpp", 0x005D63E0)
    base = symbol_at("src/win.cpp", 0x005F2940)
    size_handler = symbol_at("src/graphicwin.cpp", 0x005D6210)
    redraw = symbol_at("src/graphicwin.cpp", 0x005D5A70)
    for obj in objects:
        assert calls(obj, derived) == [base]  # Qualified call, no self-recursion.
        table, = (s for s in obj.symbols.values() if s.name == "??_7GraphicWin@@6BWin@@@")
        section = obj.sections[table.section - 1]
        entries = {offset // 4: obj.symbols[index].name
                   for offset, index, kind in section.relocations if kind == 6}
        # Original slot observations do not establish the complete table extent.
        assert entries[62] == redraw
        assert entries[66] == derived
        assert entries[71] == size_handler


def test_original_direct_redraw_call_survives_virtual_recovery(compiled):
    compiler, settings, _ = compiled
    source = ROOT / "src/time.cpp"
    before = source.read_bytes()
    timer = symbol_at("src/time.cpp", 0x0050EF10)
    redraw = symbol_at("src/graphicwin.cpp", 0x005D5A70)
    for flags in settings.flag_sets:
        obj = Coff(compiler(source, (), flags))
        assert redraw in calls(obj, timer)

    # The original timer calls GraphicWin directly at 0x0050EF30. Dropping
    # qualification still compiles, but changes it into virtual dispatch.
    anchor = b"TurnRedrawWindow->GraphicWin::redraw();"
    assert before.count(anchor) == 1
    damaged = before.replace(anchor, b"TurnRedrawWindow->redraw();")
    obj = Coff(compiler.overlay(source, {"src/time.cpp": damaged}, (), settings.flag_sets[0]))
    assert redraw not in calls(obj, timer)
    assert source.read_bytes() == before


def test_redraw_publishes_its_receiver_in_real_global_storage(compiled):
    _, _, objects = compiled
    redraw = symbol_at("src/graphicwin.cpp", 0x005D5A70)
    for obj in objects:
        assert global_accesses(obj, redraw, "?WinCallbackWindow@@3PAVWin@@A") == [CS_AC_WRITE]


def test_sdk_import_pointer_is_verified_despite_vc6_function_type_hint(compiled):
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original image is required")
    _, _, objects = compiled
    obj = objects[0]  # The primary trial has the original redraw extent.
    symbol = "__imp__InvalidateRect@12"
    emitted, = (item for item in obj.symbols.values() if item.name == symbol)
    assert (emitted.section, emitted.value, emitted.storage, emitted.type) == (0, 0, 2, 0x20)
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    compiled_units = {"src/graphicwin.cpp": obj}
    targets, reports = import_targets(read_imports(ROOT), compiled_units, image)
    assert next(row for row in reports if row["symbol"] == symbol)["identity"] == "pass"
    record, = (row for row in read_file(ROOT / "src/graphicwin.cpp") if row.address == 0x005D5A70)
    binding = FunctionBinding(Origin(pin, "pe:va", record.address),
                              tuple(Span(start, end) for start, end in record.image_spans),
                              record.symbol or record.name, "src/graphicwin.cpp")
    row, = check_objects([binding], compiled_units, image, import_targets=targets)
    assert next(item for item in row["relocations"] if item["symbol"] == symbol)["result"] == "pass"
    unresolved, = check_objects([binding], compiled_units, image)
    assert next(item for item in unresolved["relocations"] if item["symbol"] == symbol)["result"] == "unknown"


def test_system_close_assigns_the_callback_pointer_instead_of_overwriting_the_window(compiled):
    compiler, settings, _ = compiled
    path = ROOT / "src/win.cpp"
    source = path.read_bytes()
    callback = symbol_at("src/win.cpp", 0x005F2680)
    global_symbol = "?WinCallbackWindow@@3PAVWin@@A"
    for flags in settings.flag_sets:
        obj = Coff(compiler(path, (), flags))
        assert global_accesses(obj, callback, global_symbol) == [CS_AC_WRITE]

    # The bad form also references the same symbol, but loads its pointer
    # and then overwrites the old window instead of assigning the global.
    anchor = b"WinCallbackWindow = self;\n    Callback404 cb"
    assert source.count(anchor) == 1
    damaged = source.replace(anchor, b"*reinterpret_cast<Win **>(WinCallbackWindow) = self;\n    Callback404 cb")
    obj = Coff(compiler.overlay(path, {"src/win.cpp": damaged}, (), settings.flag_sets[0]))
    assert global_accesses(obj, callback, global_symbol) == [CS_AC_READ]
    assert path.read_bytes() == source


def test_unqualified_initializer_damage_really_changes_the_compiled_call_target(compiled):
    compiler, settings, _ = compiled
    path = ROOT / "src/graphicwin.cpp"
    source = path.read_bytes()
    anchor = b"const int base_result = Win::init("
    assert source.count(anchor) == 1
    damaged = source.replace(anchor, b"const int base_result = init(")
    obj = Coff(compiler.overlay(path, {"src/graphicwin.cpp": damaged}, (), settings.flag_sets[0]))
    derived = symbol_at("src/graphicwin.cpp", 0x005D4EF0)
    targets = calls(obj, derived)
    assert derived in targets
    assert symbol_at("src/win.cpp", 0x005EBD80) not in targets
    assert path.read_bytes() == source
