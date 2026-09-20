"""Real generated WorldWin deletion family, not full lifetime/game acceptance."""

from dataclasses import replace
import hashlib
import os
from pathlib import Path
import shutil
import struct

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage, weak_external
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references


PRIMARY = "??_GWorldWin@@UAEPAXI@Z"
WEAK_PRIMARY = "??_EWorldWin@@UAEPAXI@Z"
ADJUSTOR = "??_EWorldWin@@WEEE@AEPAXI@Z"
COMPLETE = "??1WorldWin@@UAE@XZ"
DELETE = "??3@YAXPAX@Z"
UNIT = "src/worldwin.cpp"


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not image_path.is_file():
        pytest.skip("VC6, Wine32 and the pinned image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    return settings, objects, image


def binding(image, address):
    record, = (row for row in read_file(ROOT / UNIT) if row.address == address)
    return FunctionBinding(Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", address),
                           tuple(Span(a, b) for a, b in record.image_spans),
                           record.symbol or record.name, UNIT)


def test_primary_uses_natural_thiscall_body_and_original_delete_target(family):
    settings, objects, image = family
    selected = binding(image, 0x4C4C70)
    assert selected.symbol == PRIMARY
    for flags, obj in zip(settings.flag_sets, objects):
        assert references(obj, PRIMARY) == [COMPLETE, DELETE]
        # These are independently established source/runtime identities. This
        # caller check does not prove the complete destructor or CRT behavior.
        row, = check_objects([selected], {UNIT: obj}, image,
                             external_targets={COMPLETE: 0x4C4A70, DELETE: 0x64557F})
        if "/Oy-" in flags:
            assert row["boundary"] == row["bytes"] == row["references"] == "pass"
        else:
            assert row["result"] == "fail"  # Original retains its frame.
        code, _, _ = obj.subject(obj.function(PRIMARY))
        returns = [op for op in Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0) if op.mnemonic == "ret"]
        assert len(returns) == 1 and returns[0].op_str == "4"
        assert not any("scalar_delete_world_win" in s.name for s in obj.symbols.values())


def assert_adjustor(obj):
    code, relocs, _ = obj.subject(obj.function(ADJUSTOR))
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0))
    assert len(code) == 11 and len(instructions) == 2
    assert (instructions[0].mnemonic, instructions[0].op_str) == ("sub", "ecx, 0x444")
    assert instructions[1].mnemonic == "jmp"
    relocation, = relocs
    assert relocation.offset == 7
    weak = obj.symbols[int(relocation.target)]
    assert weak.name == WEAK_PRIMARY and weak.storage == 105
    fallback, mode = weak_external(obj, weak)
    assert mode == 2 and fallback.name == PRIMARY
    obj.function(fallback.name)  # A fallback record alone is not a definition.


def test_adjustor_preserves_flags_and_exposes_unproved_weak_selection(family):
    _, objects, image = family
    selected = binding(image, 0x4C4CA0)
    for obj in objects:
        assert_adjustor(obj)
        row, = check_objects([selected], {UNIT: obj}, image,
                             external_targets={PRIMARY: 0x4C4C70})
        assert row["boundary"] == "pass"
        # SEARCH_LIBRARY may be overridden. The COFF-only native comparator
        # must not turn the fallback edge into a linked-selection proof.
        assert row["references"] == row["result"] == "unknown"


@pytest.mark.parametrize("damage", ["adjustment", "fallback_mode"])
def test_generated_family_controls_reject_wrong_adjustment_or_fallback(family, damage):
    _, objects, _ = family
    for original in objects:
        obj = Coff(original.raw_data)
        if damage == "adjustment":
            number = obj.function(ADJUSTOR).section
            section = obj.sections[number - 1]
            code = bytearray(section.data)
            assert struct.unpack_from("<I", code, 2)[0] == 0x444
            struct.pack_into("<I", code, 2, 0x448)
            obj.sections = tuple(replace(value, data=bytes(code)) if i == number - 1 else value
                                 for i, value in enumerate(obj.sections))
        else:
            _, relocs, _ = obj.subject(obj.function(ADJUSTOR))
            index = int(relocs[0].target)
            auxiliary = bytearray(obj.auxiliary[index])
            struct.pack_into("<I", auxiliary, 4, 3)
            obj.auxiliary[index] = bytes(auxiliary)
        with pytest.raises(AssertionError):
            assert_adjustor(obj)


def test_constructor_tables_and_ordinary_buffer_base_select_the_family(family):
    _, objects, image = family
    assert struct.unpack("<I", image.read(Span(0x66DF30, 0x66DF34)))[0] == 0x4C4C70
    assert struct.unpack("<I", image.read(Span(0x66DF28, 0x66DF2C)))[0] == 0x4C4CA0
    tables = {"??_7WorldWin@@6BWin@@@": WEAK_PRIMARY,
              "??_7WorldWin@@6BBuffer@@@": ADJUSTOR}
    for obj in objects:
        layout = class_layout(obj, "WorldWin")
        assert [(b["class"], b["offset"], b["virtual"]) for b in layout["bases"]] == [
            ("GraphicWin", 0, False)]
        graphic = class_layout(obj, "GraphicWin")
        assert ("Buffer", 0x444, False) in [
            (b["class"], b["offset"], b["virtual"]) for b in graphic["bases"]]
        constructor = references(obj, "??0WorldWin@@QAE@XZ")
        for name, target in tables.items():
            assert name in constructor
            symbol, = (s for s in obj.symbols.values() if s.name == name and s.section > 0)
            first, = ((offset, index, kind) for offset, index, kind in
                       obj.sections[symbol.section - 1].relocations if offset == symbol.value)
            assert first[2] == 6 and obj.symbols[first[1]].name == target
