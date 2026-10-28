"""Opt-in real VC6 virtual-base constructor cleanup control.

This test-owned program has no game code or data. Run under a coordinator-owned
compiler lease with OPENSMACX_GUARDED_EH_CONTROL=1.
"""

import os
import struct

import pytest

from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source.eh_regions import bind_eh_regions
from recovery.tests import test_subtracting_eh_compiler as control


@pytest.mark.skipif(os.environ.get("OPENSMACX_GUARDED_EH_CONTROL") != "1",
                    reason="requires an explicitly leased VC6 compiler control")
def test_real_guarded_constructor_regions_and_damage(tmp_path, monkeypatch):
    symbol = "??0ExtendedList@@QAE@XZ"
    monkeypatch.setattr(control, "SYMBOL", symbol)
    monkeypatch.setattr(control, "HEADER", control.HEADER.replace(
        "    ~ExtendedList();", "    ExtendedList();\n    ~ExtendedList();"))
    monkeypatch.setattr(control, "OWNER", control.OWNER +
                        "\nExtendedList::ExtendedList() { observed(this); }\n")
    raw, owner, _root, child, image_path, image, spans, record = control.compile_fixture(tmp_path)
    regions = bind_eh_regions(raw, symbol, image, spans)
    guarded = [action for action in regions[0].ownership.compiled_actions if action.guard]
    assert len(guarded) == 1
    assert guarded[0].guard.mask == 1
    assert compare_subject(record, image_path, subject_asm(raw, record)).verdict.value == "BYTE_EXACT"

    # A distinct valid frame flag remains bindable but cannot compare exact.
    pointer, = struct.unpack_from("<I", raw, 20 + (child - 1) * 40 + 20)
    damaged = bytearray(raw)
    flag_site = pointer + guarded[0].start + 2
    damaged[flag_site] = (damaged[flag_site] - 4) & 0xFF
    result = compare_subject(record, image_path, subject_asm(bytes(damaged), record))
    assert result.verdict.value == "MISMATCH"
    assert "frame guard" in result.diagnostic

    # A reversed condition is not part of the supported cleanup grammar.
    damaged = bytearray(raw)
    branch_site = pointer + guarded[0].start + 9
    assert damaged[branch_site] == 0x84
    damaged[branch_site] = 0x85
    with pytest.raises(ValueError):
        bind_eh_regions(bytes(damaged), symbol, image, spans)
