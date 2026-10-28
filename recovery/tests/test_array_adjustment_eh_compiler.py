"""Opt-in test-owned VC6 controls for unadjusted/subtracting array cleanup."""

import os
import struct

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source.eh_actions import EHActionKind
from match_recovery.source.eh_regions import bind_eh_regions
from recovery.tests import test_array_eh_compiler as control


@pytest.mark.skipif(os.environ.get("OPENSMACX_ARRAY_ADJUSTMENT_CONTROL") != "1",
                    reason="requires an explicitly leased VC6 compiler control")
@pytest.mark.parametrize("subtract", [False, True])
def test_real_array_adjustment_shapes_and_damage(tmp_path, monkeypatch, subtract):
    if subtract:
        header = control.HEADER.replace("struct ArrayOwner {", """
struct VirtualRoot {
    VirtualRoot();
    virtual ~VirtualRoot();
    int value;
};
struct ArrayOwner : virtual VirtualRoot {""").replace("Element elements[3];", "Element elements[96];")
        monkeypatch.setattr(control, "HEADER", header)
        monkeypatch.setattr(control, "PIECE", control.PIECE + """
VirtualRoot::VirtualRoot() : value(0) {}
VirtualRoot::~VirtualRoot() { destroyed(value); }
""")
        monkeypatch.setattr(control, "SYMBOL", "??1ArrayOwner@@UAE@XZ")
    else:
        monkeypatch.setattr(control, "HEADER", control.HEADER.replace("    char prefix[320];\n", ""))

    raw, owner, _root, child, image_path, image, spans, record = control._compile_array_fixture(
        tmp_path, local_callback=False)
    regions = bind_eh_regions(raw, control.SYMBOL, image, spans)
    arrays = [a for a in regions[0].ownership.compiled_actions
              if a.kind is EHActionKind.ARRAY_DESTRUCTOR]
    assert len(arrays) == 1
    action, = arrays
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(decoder.disasm(owner.sections[child - 1].data[action.start:action.end],
                                       action.start))
    assert len(instructions) == (8 if subtract else 7)
    assert action.element_count == (96 if subtract else 3)
    assert action.element_size == 4
    if subtract:
        assert instructions[4].mnemonic == "sub" and instructions[4].bytes[0] == 0x2D
        assert action.base_adjustment == -384
    else:
        assert instructions[4].mnemonic == "push" and action.base_adjustment == 0
    assert compare_subject(record, image_path, subject_asm(raw, record)).verdict.value == "BYTE_EXACT"

    # Alter the real element-count push without changing its relocation graph.
    pointer, = struct.unpack_from("<I", raw, 20 + (child - 1) * 40 + 20)
    count = instructions[1]
    assert count.bytes[0] == 0x6A and count.size == 2
    damaged = bytearray(raw)
    damaged[pointer + count.address + 1] -= 1
    result = compare_subject(record, image_path, subject_asm(bytes(damaged), record))
    assert result.verdict.value == "MISMATCH"
    assert "array-cleanup semantics" in result.diagnostic
