"""Live project regression controls; no executable-derived fixtures are stored."""

from pathlib import Path
import os
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_AC_READ, CS_AC_WRITE
from capstone.x86 import X86_OP_MEM

from match_recovery.codeview import class_layout
from match_recovery.application_layout import check_member
from match_recovery.layout_source import read_members
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file
from match_recovery.import_source import read_imports, import_targets
from match_recovery.source_inventory import image_pin
from match_recovery.pe_coff import PeImage
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.object_source import read_objects, object_targets


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


def test_original_event_slots_and_editgroup_ownership(compiled):
    """Original slots/layout, not a claim about all class lifetimes or GUI flow."""
    compiler, settings, graphic_objects = compiled
    image_path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))

    def section(obj, symbol):
        entry, = (s for s in obj.symbols.values() if s.section > 0 and s.name == symbol)
        assert entry.value == 0
        return obj.sections[entry.section - 1]

    def slots(obj, symbol):
        table = section(obj, symbol)
        assert len(table.data) == 90 * 4
        return {offset // 4: obj.symbols[index].name for offset, index, _ in table.relocations}

    def original_slot(table, slot):
        from match_recovery.model import Span
        return int.from_bytes(image.read(Span(table + slot * 4, table + slot * 4 + 4)), 'little')

    from match_recovery.model import Span
    for trial, flags in enumerate(settings.flag_sets):
        graphic = slots(graphic_objects[trial], '??_7GraphicWin@@6BWin@@@')
        assert graphic[18] == '?on_mouse_leave@Win@@UAEXHH@Z'
        assert graphic[51] == '?on_dialog_focus@Win@@UAEXH@Z'
        assert graphic[88] == '?on_link_clicked@GraphicWin@@UAEHHPAD@Z'
        assert graphic[89] == '?on_draw_caption@GraphicWin@@UAEXPADPAVBuffer@@@Z'
        assert [original_slot(0x66FC50, i) for i in (18, 51, 88, 89)] == [
            0x404260, 0x404270, 0x4042A0, 0x404260]
        group = Coff(compiler(ROOT / 'src/editgroup.cpp', (), flags + settings.layout_flags))
        layout = class_layout(group, 'EditGroup')
        assert layout['size'] == 0xB98
        assert layout['members']['boxes_'] == 4
        assert layout['members']['text_buffers_'] == 0x2C
        assert layout['members']['limits_'] == 0x54
        assert layout['members']['label_width_'] == 0x7C
        assert layout['members']['edit_width_'] == 0x80
        assert 'kind_' not in layout['members']
        assert layout['member_types']['boxes_'] == {
            'kind': 'array', 'target': {'kind': 'pointer', 'width': 4,
            'target': {'kind': 'class_value', 'class': 'EditBox'}},
            'size': 40, 'element_size': 4, 'count': 10}
        # vbtable contents establish virtual-base offsets; sizeof alone cannot.
        assert section(group, '??_8EditGroup@@7B@').data == image.read(Span(0x670A3C, 0x670A48))
        group_slots = slots(group, '??_7EditGroup@@6BWin@@@')
        assert group_slots[51] == '?on_dialog_focus@EditGroup@@$4PPPPPPPM@A@AEXH@Z'
        assert calls(group, group_slots[51]) == [symbol_at('src/editgroup.cpp', 0x612670)]
        assert original_slot(0x6708D4, 51) == 0x6127C0
        assert group_slots[88] == graphic[88] and group_slots[89] == graphic[89]
        dialogs = Coff(compiler(ROOT / 'src/dialogs.cpp', (), flags + settings.layout_flags))
        layout = class_layout(dialogs, 'Dialogs')
        assert layout['size'] == 0xC94 and layout['members']['kind_'] == 0x180
        assert [(b['class'], b['offset']) for b in layout['bases'] if not b['virtual']] == [
            ('ListBox', 0), ('RadioButton', 0x44), ('CheckBox', 0x58), ('SpriteBox', 0x70), ('EditGroup', 0xF8)]
        for base, start in [('ListBox', 0x670A78), ('RadioButton', 0x670A6C),
                            ('CheckBox', 0x670A60), ('SpriteBox', 0x670A54), ('EditGroup', 0x670A48)]:
            assert section(dialogs, f'??_8Dialogs@@7B{base}@@@').data == image.read(Span(start, start + 12))
        dialog_slots = slots(dialogs, '??_7Dialogs@@6BWin@@@')
        assert dialog_slots[18] == '?on_mouse_leave@Dialogs@@$4PPPPPPPM@A@AEXHH@Z'
        assert dialog_slots[51] == '?on_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXH@Z'
        assert [original_slot(0x669BE8, i) for i in (18, 51)] == [0x4071A0, 0x407260]
        assert dialog_slots[88] == graphic[88] and dialog_slots[89] == graphic[89]


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
    item, = (item for item in read_members(ROOT, image_pin(ROOT))
              if item.class_name == "GraphicWin" and item.member == "owned_buffer_")
    assert item.class_pointer_type == "Buffer"
    for obj in objects:
        layout = class_layout(obj, "GraphicWin")
        assert layout["size"] == 0xA14
        assert layout["members"]["owned_buffer_"] == 0xA08
        assert layout["member_type_kinds"]["owned_buffer_"] == "pointer"
        assert "owned_buffer_" not in layout["class_value_members"]
        assert check_member(item, layout)["result"] == "pass"


def test_frame_sprites_are_typed_pointers_in_the_existing_window_layout(compiled):
    _, _, objects = compiled
    items = [item for item in read_members(ROOT, image_pin(ROOT))
             if item.class_name == "GraphicWin" and item.class_pointer_type == "Sprite"]
    assert len(items) == 15
    for obj in objects:
        layout = class_layout(obj, "GraphicWin")
        assert layout["size"] == 0xA14
        for item in items:
            assert check_member(item, layout)["result"] == "pass"


def test_frame_defaults_have_real_zero_initialized_storage(compiled):
    _, _, objects = compiled
    bindings = read_objects(ROOT, image_pin(ROOT))
    binding, = (item for item in bindings if item.origin.address == 0x009B3394)
    initializer = symbol_at("src/graphicwin.cpp", 0x005D4EF0)
    for obj in objects:
        targets, _ = object_targets(bindings, {"src/graphicwin.cpp": obj})
        assert targets[binding.symbol] == binding.origin.address
        assert binding.symbol in calls(obj, initializer)
        symbol, = (item for item in obj.symbols.values() if item.name == binding.symbol)
        section = obj.sections[symbol.section - 1]
        assert section.flags & 0x80  # IMAGE_SCN_CNT_UNINITIALIZED_DATA
        assert symbol.value + 11 * 4 <= obj.section_sizes[symbol.section - 1]
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if image_path.is_file():
        image = PeImage(image_path.read_bytes(), image_pin(ROOT))
        region, _, raw_size = next(item for item in image.sections
                                   if item[0].start <= binding.origin.address < item[0].end)
        assert region.start + raw_size <= binding.origin.address
        assert binding.origin.address + 11 * 4 <= region.end


def test_input_and_process_default_records_have_distinct_verified_layouts(compiled):
    _, _, objects = compiled
    members = read_members(ROOT, image_pin(ROOT))
    for name in ("NCSprites", "GraphicWinFrameDefaults"):
        items = [item for item in members if item.class_name == name]
        assert len(items) == 11
        for obj in objects:
            layout = class_layout(obj, name)
            assert layout["size"] == 44
            assert all(check_member(item, layout)["result"] == "pass" for item in items)


def test_default_publisher_and_refresh_match_real_data_targets_and_reject_swapped_fields(compiled):
    compiler, settings, objects = compiled
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    source = ROOT / "src/graphicwin.cpp"
    records = [r for r in read_file(source) if r.address in (0x005D6A50, 0x005D6EC0)]
    assert len(records) == 2
    bindings = [FunctionBinding(Origin(pin, "pe:va", r.address),
                                tuple(Span(start, end) for start, end in r.image_spans),
                                r.symbol or r.name, "src/graphicwin.cpp") for r in records]
    object_bindings = read_objects(ROOT, pin)

    def verify(obj):
        units = {"src/graphicwin.cpp": obj}
        targets, _ = object_targets(object_bindings, units)
        return check_objects(bindings, units, image, external_targets=targets)

    passing = [flags for flags, obj in zip(settings.flag_sets, objects)
               if all(row["result"] == "pass" for row in verify(obj))]
    assert passing  # A complete trial, not mixed partial proofs across flags.

    header = ROOT / "src/graphicwin.h"
    before = header.read_bytes()
    start = before.index(b"struct GraphicWinFrameDefaults {")
    end = before.index(b"};", start) + 2
    record = before[start:end]
    swapped = record.replace(b"Sprite *bottom_left;", b"Sprite *swap_control;")
    swapped = swapped.replace(b"Sprite *top_right;", b"Sprite *bottom_left;")
    swapped = swapped.replace(b"Sprite *swap_control;", b"Sprite *top_right;")
    damaged = Coff(compiler.overlay(source, {"src/graphicwin.h": before[:start] + swapped + before[end:]},
                                    (), passing[0] + settings.layout_flags))
    # The swapped record still has eleven pointers and compiles. Only the
    # relocated data-field addresses change; a relocation-masked byte check
    # cannot establish that the publisher and consumers reach the right slots.
    control = objects[settings.flag_sets.index(passing[0])]
    for binding, row in zip(bindings, verify(damaged)):
        original_code, relocations, _ = control.subject(control.function(binding.symbol))
        damaged_code, _, _ = damaged.subject(damaged.function(binding.symbol))
        masked = {offset for relocation in relocations
                  for offset in range(relocation.offset, relocation.offset + 4)}
        assert len(original_code) == len(damaged_code)
        assert original_code != damaged_code
        assert all(left == right for offset, (left, right) in enumerate(zip(original_code, damaged_code))
                   if offset not in masked)
        assert row["boundary"] == "pass"
        # The strict checker applies the bindings before comparing bytes.
        assert row["bytes"] == row["references"] == row["result"] == "fail"
    layout = class_layout(damaged, "GraphicWinFrameDefaults")
    item, = (item for item in read_members(ROOT, pin)
              if item.class_name == "GraphicWinFrameDefaults" and item.member == "top_right")
    assert layout["size"] == 44
    assert check_member(item, layout)["result"] == "fail"
    assert header.read_bytes() == before


def test_frame_overlay_uses_real_renderers_and_screen_coordinate_imports(compiled):
    compiler, settings, objects = compiled
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    overlay = symbol_at("src/graphicwin.cpp", 0x005D6AC0)
    renderers = {symbol_at("src/sprite.cpp", 0x005E4B4A),
                 symbol_at("src/buffer.cpp", 0x005DA9F0),
                 symbol_at("src/buffer.cpp", 0x005E2B00)}
    expected = {"__imp__GetWindowRect@8": 0x006692A8,
                "__imp__IntersectRect@12": 0x00669338}
    bindings = read_imports(ROOT)
    for obj in objects:
        targets = set(calls(obj, overlay))
        assert renderers <= targets
        assert expected.keys() <= targets
        resolved, _ = import_targets(bindings, {"src/graphicwin.cpp": obj}, image)
        assert {name: resolved[name] for name in expected} == expected

    # The seed called this GetClientRect, although the original IAT slot
    # is GetWindowRect. Both APIs type-check; compilation alone misses it.
    source = ROOT / "src/graphicwin.cpp"
    before = source.read_bytes()
    anchor = b"GetWindowRect(HandleMain, &window_rect);"
    assert before.count(anchor) == 1
    wrong = before.replace(anchor, b"GetClientRect(HandleMain, &window_rect);")
    for flags in settings.flag_sets:
        damaged = Coff(compiler.overlay(source, {"src/graphicwin.cpp": wrong}, (), flags))
        targets = set(calls(damaged, overlay))
        assert "__imp__GetClientRect@8" in targets
        assert "__imp__GetWindowRect@8" not in targets
        resolved, _ = import_targets(bindings, {"src/graphicwin.cpp": damaged}, image)
        assert "__imp__GetClientRect@8" not in resolved  # No invented alias.
    assert source.read_bytes() == before


@pytest.mark.parametrize("replacement, kind", [(b"Win *", "pointer"), (b"int ", "non_class_value"),
                                               (b"Buffer &", "reference")])
def test_wrong_owned_buffer_type_at_identical_offset_fails_native_layout_check(compiled, replacement, kind):
    compiler, settings, _ = compiled
    header = ROOT / "src/graphicwin.h"
    before = header.read_bytes()
    anchor = b"Buffer *owned_buffer_;"
    assert before.count(anchor) == 1
    # Compile the actual declaration with a layout witness. The original
    # method bodies need not type-check under deliberately wrong member types.
    getter = b"Buffer *owned_buffer() const { return owned_buffer_; }"
    getter_declaration = b"Buffer *owned_buffer() const;"
    assert before.count(getter) == 1
    projected = before.replace(getter, getter_declaration, 1)
    witness = b'#include "stdafx.h"\n#include "graphicwin.h"\nint layout_witness(GraphicWin *p) { return sizeof(*p); }\n'
    item, = (item for item in read_members(ROOT, image_pin(ROOT))
              if item.class_name == "GraphicWin" and item.member == "owned_buffer_")
    for flags in settings.flag_sets:
        def layout(text):
            obj = Coff(compiler.overlay(ROOT / "src/graphicwin.cpp",
                                         {"src/graphicwin.cpp": witness, "src/graphicwin.h": text},
                                         (), flags + settings.layout_flags))
            return class_layout(obj, "GraphicWin")
        # Project out the unused inline getter for both witnesses. Its return
        # expression is not part of this member-layout control; retaining it
        # would make a wrong member type fail compilation before layout checks.
        control = layout(projected)
        assert check_member(item, control)["result"] == "pass"
        damaged_header = projected.replace(anchor, replacement + b"owned_buffer_;")
        damaged = layout(damaged_header)
        assert damaged["size"] == control["size"] == 0xA14
        row = check_member(item, damaged)
        assert row["actual_offset"] == 0xA08 and row["offset_result"] == "pass"
        assert row["member_type_kind"] == kind
        assert row["result"] == row["type_result"] == "fail"
    assert header.read_bytes() == before


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
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
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
