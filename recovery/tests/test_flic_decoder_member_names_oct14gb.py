"""Evidence-backed Flic member names must preserve storage and layout."""
from __future__ import annotations

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_flic_decoder_members_keep_dword_storage_and_original_offsets():
    header = (ROOT / "src/flic.h").read_text()
    source = (ROOT / "src/flic.cpp").read_text()
    fixture = (ROOT / "recovery/tests/test_flic_word_transfers_actual_source_oct14fj.py").read_text()

    members = {
        "decoded_width_": "0x59C",
        "decoded_height_": "0x5A0",
        "origin_x_": "0x5AC",
        "origin_y_": "0x5B0",
        "decoder_flags_": "0x5B4",
    }
    for name, offset in members.items():
        anchor = "0x0043F190" if name in ("origin_x_", "origin_y_") else "0x00629E50"
        assert re.search(
            rf"RECOVERY_MEMBER: v1 Flic {name} {offset} {anchor}", header
        ), f"missing preserved member identity: {name}"
        assert re.search(rf"uint32_t\s+{name};", header)
        assert re.search(rf"\b{re.escape(name)}\b", source)

    # This focused decoder fixture models only the members used by the two
    # extracted transfer bodies; keep those fixture spellings in sync.
    for name in ("decoded_width_", "origin_x_", "origin_y_"):
        assert re.search(rf"\b{re.escape(name)}\b", fixture)

    for old_name in ("field_59C_", "field_5A0_", "field_5AC_", "field_5B0_", "field_5B4_"):
        assert old_name not in header
        assert old_name not in source
        assert old_name not in fixture

    assert "static_assert(sizeof(Flic) == 0xae4" in header
    assert "uint16_t width_;" in header and "uint16_t height_;" in header


def test_named_accessors_and_state_bit_expressions_are_unchanged():
    header = (ROOT / "src/flic.h").read_text()
    source = (ROOT / "src/flic.cpp").read_text()

    assert "static_cast<int>(origin_x_)" in header
    assert "static_cast<int>(origin_y_)" in header
    assert "static_cast<int>(decoded_width_)" in header
    assert "static_cast<int>(decoded_height_)" in header
    assert "decoder_flags_ |= 1;" in source
    assert "decoder_flags_ |= 2;" in source
    assert "if (!(decoder_flags_ & 1))" in source
    assert "if (decoder_flags_ & 2)" in source
