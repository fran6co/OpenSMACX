"""Original bounds and live consumer references, not FX/runtime closure."""

import os
from pathlib import Path
import shutil
import struct

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_AC_READ, CS_AC_WRITE
from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_REG_ESI

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import object_targets, read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
UNIT = "src/autosound.cpp"
SYMBOL = "?AutoSoundDefaults@@3PAIA"
EXTENT = Span(0x9BC080, 0x9BC114)
CONSUMERS = ((0x62BA80, 0x62BBE8), (0x62BBF0, 0x62BD3D),
             (0x62BDD0, 0x62BF1D))


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    return PeImage(path.read_bytes(), image_pin(ROOT))


@pytest.fixture(scope="module")
def compiled():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    return compiler, settings, objects


@pytest.mark.parametrize("start,end,access", [
        *((start, end, CS_AC_READ) for start, end in CONSUMERS),
    (0x445CD0, 0x4468E1, CS_AC_WRITE),
])
def test_independent_original_consumers_and_producer_cover_37_words(original, start, end, access):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    observed = []
    for instruction in decoder.disasm(original.read(Span(start, end)), start):
        for operand in instruction.operands:
            if (operand.type == X86_OP_MEM and not operand.mem.base and not operand.mem.index
                    and 0x9BC080 <= operand.mem.disp < 0x9BC118 and operand.access & access):
                assert operand.size == 4
                observed.append(operand.mem.disp)
    assert sorted(observed) == list(range(EXTENT.start, EXTENT.end, 4))
    assert original.read_initial(EXTENT) == bytes(148)


def defaults_binding():
    binding, = (b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol == SYMBOL)
    assert binding.unit == UNIT and binding.span == EXTENT
    assert binding.evidence.address == CONSUMERS[0][0]
    return binding


def test_original_reset_stores_the_zeroed_nonvolatile_register(original):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(original.read(Span(0x445D00, 0x446470)), 0x445D00))
    assert (instructions[0].mnemonic, instructions[0].op_str) == ("xor", "esi, esi")
    # Calls preserve ESI under the original x86 ABI; this is not a proof of
    # those callees or of control-flow reachability/startup ordering.
    assert all(X86_REG_ESI not in op.regs_access()[1] for op in instructions[1:])
    stores = [op for op in instructions if op.mnemonic == "mov"
              and op.operands[0].type == X86_OP_MEM
              and EXTENT.start <= op.operands[0].mem.disp < EXTENT.end]
    assert len(stores) == 37
    assert all(op.operands[1].type == X86_OP_REG and op.operands[1].reg == X86_REG_ESI
               for op in stores)


def test_storage_extent_and_initial_values_reject_compiled_damage(compiled, original):
    compiler, settings, objects = compiled
    binding = defaults_binding()
    path = ROOT / UNIT
    source = path.read_bytes()
    declaration = b"AutoSoundDefaultsBlock AutoSoundDefaults;"
    assert source.count(declaration) == 1
    header_path = ROOT / "src/autosound_defaults.h"
    header = header_path.read_bytes()
    block_tail = (b"  Sound *val_33_;\n  Sound *event_window_;\n  Sound *val_35_;\n"
                  b"  Sound *val_36_;\n  Sound *val_37_;\n};")
    assert header.count(block_tail) == 1
    block_start = header.index(b"struct AutoSoundDefaultsBlock {")
    block_end = header.index(b"};", block_start) + 2
    block = header[block_start:block_end]

    def check(obj):
        units = {UNIT: obj}
        targets, definitions = object_targets([binding], units)
        assert definitions[0]["definition"] == "pass", definitions
        row, = check_initial_data([binding], units, original, targets)
        return row

    for flags, obj in zip(settings.flag_sets, objects):
        row = check(obj)
        assert row["content"] == row["proof"]["boundary"] == "pass"
        assert row["compiler_storage"]["size"] == 148
        small_block = block.replace(block_tail, b"  union { Sound *val_33_; Sound *event_window_; "
                                     b"Sound *val_35_; Sound *val_36_; Sound *val_37_; };\n};")
        large_block = block.replace(b"  Sound *val_37_;\n};",
                                    b"  Sound *val_37_;\n  uint32_t extra_;\n};")
        layout_assert = b'static_assert(sizeof(void *) != 4 || sizeof(AutoSoundDefaultsBlock) == 0x94,\n              "AutoSound defaults must occupy the original 37-word block");'
        for altered_block, proof in ((small_block, "boundary"), (large_block, "boundary")):
            damaged_header = header.replace(block, altered_block).replace(layout_assert, b"")
            damaged = Coff(compiler.overlay(path, {"src/autosound_defaults.h": damaged_header},
                                            (), flags + settings.layout_flags))
            bad = check(damaged)
            assert bad["content"] == bad["proof"][proof] == "fail"
        nonzero = Coff(compiler.overlay(path, {
            UNIT: source.replace(declaration, b"AutoSoundDefaultsBlock AutoSoundDefaults = {1};")},
            (), flags + settings.layout_flags))
        bad = check(nonzero)
        assert bad["content"] == bad["proof"]["bytes"] == "fail"
    assert path.read_bytes() == source


@pytest.mark.parametrize("address", [start for start, _ in CONSUMERS])
def test_consumers_match_actual_targets_and_reject_wrong_tail(compiled, original, address):
    compiler, settings, objects = compiled
    record, = (r for r in read_file(ROOT / UNIT) if r.address == address)
    binding = FunctionBinding(Origin(image_pin(ROOT), "pe:va", address),
                              tuple(Span(a, b) for a, b in record.image_spans),
                              record.symbol or record.name, UNIT)
    defaults = defaults_binding()

    def check(obj):
        units = {UNIT: obj}
        targets, _ = object_targets([defaults], units)
        return check_objects([binding], units, original, external_targets=targets)[0]

    def offsets(obj):
        code, relocations, _ = obj.subject(obj.function(binding.symbol))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(code, 0))
        values = []
        for relocation in relocations:
            if obj.symbols[int(relocation.target)].name != defaults.emitted_symbol:
                continue
            op, = (op for op in instructions
                   if op.address <= relocation.offset < op.address + op.size)
            memory, = (operand for operand in op.operands if operand.type == X86_OP_MEM)
            assert memory.size == 4 and memory.access & CS_AC_READ
            assert not memory.mem.base and not memory.mem.index
            values.append(struct.unpack_from("<I", code, relocation.offset)[0])
        return sorted(values)

    path = ROOT / UNIT
    source = path.read_bytes()
    assignment = b"val_37_ = AutoSoundDefaults.val_37_;"
    assert source.count(assignment) == 3
    for flags, obj in zip(settings.flag_sets, objects):
        assert offsets(obj) == list(range(0, 148, 4))
        if address != 0x62BA80:
            row = check(obj)
            assert row["boundary"] == row["bytes"] == row["references"] == "pass", row
        # The constructor also stores its compiler-generated vftable. Its
        # defaults loads are covered here without inventing a table binding.
        damaged = Coff(compiler.overlay(path, {UNIT: source.replace(
            assignment, b"val_37_ = AutoSoundDefaults.val_36_;")},
            (), flags + settings.layout_flags))
        assert offsets(damaged) != list(range(0, 148, 4))
        if address != 0x62BA80:
            bad = check(damaged)
            assert bad["boundary"] == "pass"
            assert bad["bytes"] == bad["references"] == bad["result"] == "fail", bad
    assert path.read_bytes() == source
