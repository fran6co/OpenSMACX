"""Pinned scalar identities, including the shared MultiWin open-state member."""
from pathlib import Path
import hashlib
import os

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
import pytest
from match_recovery.model import Span
from match_recovery.object_source import parse_objects, read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe",
))
SCALARS = {
    0x0074DA9C: ("?DiploRedrawFlag@@3HA", 0x0055B870),
    0x009BBFEC: ("?GenderDefault@@3HA", 0x0055D190),
    0x009BBFF0: ("?PluralityDefault@@3HA", 0x0055D190),
    0x006A721C: ("?PopupModalState@@3HA", 0x004E0EA0),
    0x009B7B1C: ("?WinScreenWidth@@3HA", 0x004E0EA0),
}
CHILD = (0x007FE06C, "?MultiWindowOpen@@3HA", "?MultiWindow@@3VMultiWin@@A", ("panel_open_",))
PINNED_ACCESS = {
    0x0055B8AF: ("mov", "eax, dword ptr [0x74da9c]"),
    0x0055B8B6: ("mov", "dword ptr [0x74da9c], eax"),
    0x0055BA5E: ("mov", "eax, dword ptr [0x7fe06c]"),
    0x0055D19C: ("mov", "dword ptr [0x9bbff0], 0"),
    0x0055D1C3: ("mov", "dword ptr [0x9bbfec], eax"),
    0x004E0EAF: ("mov", "dword ptr [0x6a721c], 6"),
    0x004E0EC1: ("mov", "dword ptr [0x6a721c], 0"),
    0x004E0EF2: ("cmp", "dword ptr [0x9b7b1c], 0x320"),
    0x0045D251: ("mov", "eax, dword ptr [0x7fe06c]"),
    0x004799AB: ("cmp", "dword ptr [esi + 0xa24], edi"),
    0x00479A2B: ("mov", "dword ptr [esi + 0xa24], ebx"),
    0x00479C3B: ("cmp", "dword ptr [esi + 0xa24], ebx"),
    0x00479F19: ("mov", "dword ptr [esi + 0xa24], 0"),
    0x0047A3C1: ("mov", "dword ptr [esi + 0xa24], 0"),
    0x00625E67: ("mov", "ecx, dword ptr [0x9bbfec]"),
    0x00625E7C: ("mov", "ecx, dword ptr [0x9bbff0]"),
    0x00404BED: ("mov", "edi, dword ptr [0x6a721c]"),
    0x005F02D4: ("mov", "dword ptr [0x9b7b1c], eax"),
}


def test_native_object_reader_binds_five_scalars_and_multiwin_member_without_overlap():
    rows = read_objects(ROOT, PIN)
    selected = [row for row in rows if row.symbol in
                {symbol for symbol, _ in SCALARS.values()} | {CHILD[1]}]
    assert len(selected) == 6
    for address, (symbol, evidence) in SCALARS.items():
        row, = [item for item in selected if item.symbol == symbol]
        assert row.origin.address == address
        assert row.span == Span(address, address + 4)
        assert row.evidence.address == evidence
        assert row.parent_symbol is None
    child, = [item for item in selected if item.symbol == CHILD[1]]
    assert child.origin.address == CHILD[0]
    assert child.span is None
    assert child.parent_symbol == CHILD[2]
    assert child.member_path == CHILD[3]

    faction = (ROOT / "src/faction.cpp").read_text(encoding="utf-8")
    faction_h = (ROOT / "src/faction.h").read_text(encoding="utf-8")
    assert "int MultiWindowOpen;" not in faction
    assert "extern int MultiWindowOpen" not in faction_h
    for path in ("src/faction.cpp", "src/tutwin.cpp", "src/maininterface.cpp", "src/console.cpp"):
        assert "MultiWindowOpen" not in (ROOT / path).read_text(encoding="utf-8")

    header = (ROOT / "src/multiwin.h").read_text(encoding="utf-8")
    assert "RECOVERY_MEMBER: v1 MultiWin panel_open_ 0xA24 0x00479970" in header
    assert "unsigned int panel_open() const { return panel_open_; }" in header
    assert "unsigned int panel_open_;" in header
    assert "RECOVERY_OBJECT: v3 0x007FE06C ?MultiWindowOpen@@3HA member ?MultiWindow@@3VMultiWin@@A panel_open_" in (ROOT / "src/game.cpp").read_text(encoding="utf-8")


def test_multwin_open_state_preserves_native_solo_and_multiplayer_branches():
    source = (ROOT / "src/multiwin.cpp").read_text(encoding="utf-8")
    assert "if (IsMultiplayerNet == 0 && panel_open_ != 0) return;" in source
    assert "if (IsMultiplayerNet == 0 && panel_open_ == 0) return;" in source
    assert "panel_open_ = 1;" in source
    assert "panel_open_ = 0;" in source
    assert "CouncilWindow.field_A24_" in source


def test_pinned_writable_storage_and_direct_reference_instructions():
    data = IMAGE.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    data_sections = [(span, size, flags) for name, (span, _, size), flags
                     in zip(image.section_names, image.sections, image.section_flags)
                     if name == ".data"]
    assert len(data_sections) == 1
    data_span, raw_size, data_flags = data_sections[0]
    assert data_span.start == 0x00682000
    assert data_span.start + raw_size == 0x006A8000
    assert data_span.end == 0x009C21F8
    assert data_flags & 0x80000000 and data_flags & 0x40
    assert not data_flags & 0x20000000
    spans = {address: Span(address, address + 4) for address in
             (*SCALARS.keys(), CHILD[0])}
    # PopupModalState lies within .data's 0x00682000..0x006A8000 raw bytes.
    assert image.read(spans[0x006A721C]) == bytes(4)
    # The four later objects and MultiWin child are writable-tail loader zeros.
    assert image.read_initial(spans[0x0074DA9C]) == bytes(4)
    for address in (0x009BBFEC, 0x009BBFF0, 0x009B7B1C, CHILD[0]):
        assert image.read_initial(spans[address]) == bytes(4)

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for address, expected in PINNED_ACCESS.items():
        code = image.read(Span(address, address + 16))
        instruction, = decoder.disasm(code, address, count=1)
        assert (instruction.mnemonic, instruction.op_str) == expected


def test_binding_and_image_pin_damage_controls_are_rejected():
    path, address = "src/faction.cpp", 0x0074DA9C
    symbol, evidence = SCALARS[address]
    source = (ROOT / path).read_text(encoding="utf-8")
    marker = (f"// RECOVERY_OBJECT: v2 0x{address:08X} "
              f"0x{address + 4:08X} {symbol} 0x{evidence:08X}")
    damaged = source.replace(marker, marker.replace("0074DA9C", "0074DA98"), 1)
    parsed = parse_objects(damaged, path, PIN, address_space="pe:va")
    assert not any(row.origin.address == address and row.symbol == symbol for row in parsed)
    with pytest.raises(ValueError, match="digest"):
        PeImage(IMAGE.read_bytes(), "0" * 64)

