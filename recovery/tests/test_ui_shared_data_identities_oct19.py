"""Source-start identities with independent witnesses in the pinned PE.

These controls exercise the actual source annotations and the pinned image.
They are not compiled COFF, provider-version, linked-selection or runtime proof.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

import pytest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    ROOT / ".opensmacx/game/terranx_original.exe",
))
EXPECTED = {
    "?WinHdcRefCount@@3HA": (0x009B3AB0, "src/win.cpp"),
    "?WinSharedHdc@@3PAUHDC__@@A": (0x009B7B2C, "src/win.cpp"),
    "?PopupStartCaption@@3PADA": (0x009B8AA8, "src/basepop.cpp"),
}
PINNED = {
    # The count is loaded/stored and incremented/decremented as a dword.
    0x005EC690: ("mov", "eax, dword ptr [0x9b3ab0]"),
    0x005EC69A: ("mov", "dword ptr [0x9b3ab0], eax"),
    0x005EC6F0: ("mov", "eax, dword ptr [0x9b3ab0]"),
    0x005EC6F6: ("mov", "dword ptr [0x9b3ab0], eax"),
    # GetDC receives the cell address (HDC *); ReleaseDC receives its value.
    0x005EC6B0: ("push", "0x9b7b2c"),
    0x005EC6B6: ("call", "dword ptr [ecx + 0x44]"),
    0x005EC706: ("mov", "edx, dword ptr [0x9b7b2c]"),
    0x005EC710: ("call", "dword ptr [ecx + 0x68]"),
    0x005EC713: ("mov", "dword ptr [0x9b7b2c], 0"),
    # set_def_file clears the first caption byte and passes the array address
    # to strcat; BasePop::start uses that address as the null-file fallback.
    0x006043E8: ("push", "0x9b8aa8"),
    0x006043ED: ("mov", "byte ptr [0x9b8aa8], 0"),
    0x006043F4: ("call", "0x645470"),
    0x00601CE1: ("mov", "eax, 0x9b8aa8"),
    0x00601CE8: ("call", "0x5fd550"),
}


def test_actual_source_bindings_name_the_three_existing_declarations():
    rows = read_objects(ROOT, PIN)
    selected = {row.symbol: row for row in rows if row.symbol in EXPECTED}
    assert set(selected) == set(EXPECTED)
    for symbol, (address, source_path) in EXPECTED.items():
        row = selected[symbol]
        assert row.origin.address == address
        assert row.origin.image_sha256 == PIN
        assert row.span is None  # v1 binds starts only; no original capacity claim.
        assert row.evidence is None
        assert row.unit == source_path

    win = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    basepop = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    assert "int WinHdcRefCount;        // 0x009B3AB0" in win
    assert "HDC WinSharedHdc;          // 0x009B7B2C" in win
    assert "char PopupStartCaption[256];  // 0x009B8AA8" in basepop
    assert "GetDC(&WinSharedHdc)" in win
    assert "ReleaseDC(WinSharedHdc)" in win
    assert "PopupStartCaption[0] = 0;" in basepop
    assert "strcat(PopupStartCaption, text);" in basepop
    assert "text_file != 0 ? text_file : (char *)PopupStartCaption" in basepop


def test_pinned_image_witnesses_support_cell_roles_and_caption_context():
    if not IMAGE.is_file():
        pytest.skip("pinned original image required")
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for address, expected in PINNED.items():
        instruction, = decoder.disasm(image.read(Span(address, address + 16)),
                                      address, count=1)
        assert (instruction.mnemonic, instruction.op_str) == expected

    # Both Win cells and PopupStartCaption lie in virtual .data, but beyond
    # its raw bytes. This is not a file-backed initial-value assertion.
    sections = [(span, raw_size) for name, (span, _virtual, raw_size), _flags
                in zip(image.section_names, image.sections, image.section_flags)
                if name == ".data"]
    data_span, raw_size = sections[0]
    assert data_span.start == 0x00682000
    raw_end = data_span.start + raw_size
    assert raw_end == 0x006A8000
    assert data_span.end == 0x009C21F8
    for address in EXPECTED.values():
        cell, _source_path = address
        assert raw_end <= cell < data_span.end
