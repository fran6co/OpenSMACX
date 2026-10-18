"""Pinned source/data anchors for General's two bounded switch tables.

The source reader proves explicit source interpretation and the inverse test
proves exact source transport. These controls inspect the pinned original
guards, instruction anchors, table extent and relocations; they do not prove
candidate compiler output, linked selection or runtime behavior.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import struct

import pytest

from match_recovery.associated_source import read_associated
from match_recovery.model import Span


ROOT = Path(__file__).resolve().parents[2]
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXPECTED_BINDINGS = {
    (0x00628950, Span(0x00628A00, 0x00628A10), "switch_targets", 0x00628976,
     "src/general.cpp"),
    (0x00628950, Span(0x00628A10, 0x00628A47), "switch_selector", 0x00628970,
     "src/general.cpp"),
    (0x00628B70, Span(0x00628D94, 0x00628DAC), "switch_targets", 0x00628C65,
     "src/general.cpp"),
}


def _pinned_image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        return Path(configured)
    for root in (ROOT, *ROOT.parents):
        candidate = root / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    return ROOT / ".opensmacx/game/terranx_original.exe"


def _original():
    from match_recovery.pe_coff import PeImage

    path = _pinned_image_path()
    if not path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN_SHA256
    return PeImage(raw, PIN_SHA256)


def test_live_associated_source_population_is_exact_and_mapped():
    rows = tuple(row for row in read_associated(ROOT, PIN_SHA256)
                 if row.unit == "src/general.cpp")
    actual = {
        (row.owner.address, row.span, row.kind, row.evidence.address, row.unit)
        for row in rows
    }
    assert actual == EXPECTED_BINDINGS
    assert len(rows) == 3


def test_pinned_switch_guards_anchors_data_extents_and_relocations():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    image = _original()
    engine = Cs(CS_ARCH_X86, CS_MODE_32)

    def body(start: int, end: int):
        code = image.read(Span(start, end))
        rows = list(engine.disasm(code, start))
        assert rows and rows[0].address == start
        assert sum(row.size for row in rows) == len(code)
        assert rows[-1].address + rows[-1].size == end
        assert rows[-1].mnemonic.startswith("ret")
        return {row.address: row for row in rows}

    stoi = body(0x00628950, 0x006289FE)
    assert (stoi[0x00628965].mnemonic, stoi[0x00628965].op_str) == ("cmp", "ecx, 0x36")
    assert (stoi[0x00628968].mnemonic, stoi[0x00628968].op_str) == ("ja", "0x6289f4")
    assert (stoi[0x0062896E].mnemonic, stoi[0x0062896E].op_str) == ("xor", "edx, edx")
    assert (stoi[0x00628970].mnemonic, stoi[0x00628970].op_str) == (
        "mov", "dl, byte ptr [ecx + 0x628a10]")
    assert (stoi[0x00628976].mnemonic, stoi[0x00628976].op_str) == (
        "jmp", "dword ptr [edx*4 + 0x628a00]")
    assert image.absolute_sites(Span(0x00628972, 0x00628976)) == frozenset((0,))
    assert image.absolute_sites(Span(0x00628979, 0x0062897D)) == frozenset((0,))

    stoi_targets_span = Span(0x00628A00, 0x00628A10)
    stoi_selector_span = Span(0x00628A10, 0x00628A47)
    stoi_targets = image.read(stoi_targets_span)
    stoi_selector = image.read(stoi_selector_span)
    target_values = struct.unpack("<4I", stoi_targets)
    assert target_values == (0x0062897D, 0x006289F3, 0x00628998, 0x006289F4)
    assert set(target_values) <= set(stoi)
    assert len(stoi_selector) == 0x37
    assert set(stoi_selector) <= {0, 1, 2, 3}
    assert image.absolute_sites(stoi_targets_span) == frozenset((0, 4, 8, 12))
    assert not image.absolute_sites(stoi_selector_span)
    assert image.read(Span(0x006289FE, 0x00628A00)) == bytes.fromhex("8b ff")

    hsv = body(0x00628B70, 0x00628D92)
    assert (hsv[0x00628C12].mnemonic, hsv[0x00628C12].op_str) == ("cmp", "eax, 5")
    assert (hsv[0x00628C5F].mnemonic, hsv[0x00628C5F].op_str) == (
        "ja", "0x628d56")
    assert (hsv[0x00628C65].mnemonic, hsv[0x00628C65].op_str) == (
        "jmp", "dword ptr [eax*4 + 0x628d94]")
    assert image.absolute_sites(Span(0x00628C68, 0x00628C6C)) == frozenset((0,))

    hsv_table_span = Span(0x00628D94, 0x00628DAC)
    hsv_targets = struct.unpack("<6I", image.read(hsv_table_span))
    assert hsv_targets == (
        0x00628C6C, 0x00628C95, 0x00628CBE,
        0x00628CE4, 0x00628D0A, 0x00628D30,
    )
    assert set(hsv_targets) <= set(hsv)
    assert image.absolute_sites(hsv_table_span) == frozenset((0, 4, 8, 12, 16, 20))
    assert image.read(Span(0x00628D92, 0x00628D94)) == bytes.fromhex("8b ff")
