"""Original Sprite global identities and paired CRT lifetime evidence."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import struct
import subprocess
from collections import Counter
import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, x86_const

from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import read, read_text
from match_recovery.source_inventory import source_files


ROOT = Path(__file__).resolve().parents[2]
IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PAIRS = (
    (0x404440, 0x6A7130, 0x404460, 2),
    (0x404470, 0x6A72E0, 0x404490, 11),
    (0x4044A0, 0x6A71F0, 0x4044C0, 6),
    (0x4044D0, 0x6A7280, 0x4044F0, 9),
    (0x404500, 0x6A74C0, 0x404520, 21),
    (0x404530, 0x6A71C0, 0x404550, 5),
    (0x404560, 0x6A7250, 0x404580, 8),
    (0x404590, 0x6A7190, 0x4045B0, 4),
    (0x4045C0, 0x6A7100, 0x4045E0, 1),
    (0x4045F0, 0x6A7430, 0x404610, 18),
    (0x404620, 0x6A7160, 0x404640, 3),
    (0x404650, 0x6A7490, 0x404670, 20),
    (0x404680, 0x6A73D0, 0x4046A0, 16),
    (0x4046B0, 0x6A7370, 0x4046D0, 14),
    (0x4046E0, 0x6A74F0, 0x404700, 22),
    (0x404710, 0x6A72B0, 0x404730, 10),
    (0x404740, 0x6A73A0, 0x404760, 15),
    (0x404770, 0x6A7340, 0x404790, 13),
    (0x4047A0, 0x6A7400, 0x4047C0, 17),
    (0x4047D0, 0x6A7460, 0x4047F0, 19),
    (0x404800, 0x6A7310, 0x404820, 12),
    (0x404830, 0x6A7220, 0x404850, 7),
)


def test_object_split_preserves_definitions_order_and_original_obligations():
    """Compare the entire move against its immutable pre-split source, not a count."""
    before = subprocess.check_output(
        ["git", "show", "e5e813abb85de1b6775cfa1a5ab759dd51a73241:src/sprite.cpp"],
        cwd=ROOT, text=True,
    )
    implementation = (ROOT / "src/sprite.cpp").read_text()
    residual_path = ROOT / "src/sprite_objects.cpp"
    assets_path = ROOT / "src/interface_assets.cpp"
    residual = residual_path.read_text()
    assets = assets_path.read_text()
    objects = residual + "\n" + assets
    # Compare typed declarations, not incidental trailing annotations/comments.
    definitions = r"(?m)^Sprite\s+(\w+(?:\[\w+\])*)\s*;"
    expected = re.findall(definitions, before)
    actual = re.findall(definitions, objects)
    assert len(expected) == 190
    assert Counter(actual) == Counter(expected)
    assert len(set(actual)) == 190
    assert not re.findall(definitions, implementation)
    assert "Sprite::Sprite()" in implementation
    assert "Sprite::Sprite()" not in objects

    old = read_text(before, ROOT / "src/sprite.cpp")
    kept = read_text(implementation, ROOT / "src/sprite.cpp")
    previous = {row.address: row for row in old}
    assert len(previous) == len(old)
    old_addresses = set(previous)
    # Several original Sprite identities were already rehomed to other
    # owners. Search configured live source units for the complete historical
    # identity set instead of limiting continuity to the three old owners.
    live = [row for path in source_files(ROOT) for row in read(path)
            if row.address in old_addresses]
    current = {row.address: row for row in live}
    assert len(current) == len(live) == len(old)
    assert set(current) == old_addresses
    identity = lambda row: (row.address, row.name, row.image_spans, row.semantic)
    assert sorted(map(identity, old)) == sorted(map(identity, live))
    for address, old_row in previous.items():
        row = current[address]
        assert not old_row.byte_exact or row.byte_exact
        assert not old_row.semantic or row.semantic
        assert row.mode == old_row.mode
        assert row.kind == old_row.kind
        assert row.exclusion == old_row.exclusion
        assert row.levers == old_row.levers
        assert row.ruled_out == old_row.ruled_out
        assert row.unrecoverable == old_row.unrecoverable
        assert row.deferred == old_row.deferred
        # File positions change when an owner moves; the complete authored
        # lesson text, including its wrapping, must remain intact.
        lesson_text = lambda item: tuple(
            (lesson.kind, lesson.key, lesson.prose,
             tuple(text for _line, text in lesson.lines))
            for lesson in (item.origin or ())
        )
        assert lesson_text(row) == lesson_text(old_row)
    for row in kept:
        assert row.body == previous[row.address].body


def test_moved_sprite_cohort_follows_pinned_crt_order():
    image_path = pinned_image_path()
    if not image_path.is_file():
        pytest.skip("pinned original unavailable for Sprite CRT order check")
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    image = PeImage(image_bytes, PIN)
    assets_path = ROOT / "src/interface_assets.cpp"
    assets = assets_path.read_text()
    asset_lines = assets.splitlines()
    asset_sprite_bindings = [
        binding for binding in read_objects(ROOT, PIN)
        if binding.unit == "src/interface_assets.cpp"
        and re.match(r"^Sprite\s+\w+(?:\[\w+\])*\s*;", asset_lines[binding.line])
    ]
    assert len(asset_sprite_bindings) == 166
    sprite_origins = {binding.origin.address for binding in asset_sprite_bindings}
    assert len(sprite_origins) == 166

    # Follow the pinned .CRT roots, then each mapped initializer's absolute
    # receiver operand back to the exact original Sprite object identity.
    asset_records = read_text(assets, assets_path)
    record_by_address = {record.address: record for record in asset_records}
    assert len(record_by_address) == len(asset_records)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    ordered_sprite_origins = []
    for slot in range(0x00682004, 0x006826CC, 4):
        root = int.from_bytes(image.read(Span(slot, slot + 4)), "little")
        record = record_by_address.get(root)
        if root == 0 or record is None:
            continue
        assert record.image_spans and record.image_spans[0][0] == root
        low, high = record.image_spans[0]
        code = image.read(Span(low, high))
        references = set()
        for instruction in decoder.disasm(code, low):
            for operand in instruction.operands:
                if operand.type == x86_const.X86_OP_IMM:
                    references.add(operand.imm & 0xFFFFFFFF)
                elif (operand.type == x86_const.X86_OP_MEM
                      and operand.mem.base == 0 and operand.mem.index == 0):
                    references.add(operand.mem.disp & 0xFFFFFFFF)
        receivers = references & sprite_origins
        assert len(receivers) <= 1, hex(root)
        if receivers:
            ordered_sprite_origins.extend(receivers)
    assert len(ordered_sprite_origins) == 166
    assert len(set(ordered_sprite_origins)) == 166
    assert ordered_sprite_origins == [binding.origin.address for binding in asset_sprite_bindings]


def pinned_image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        return Path(configured)
    if IMAGE.is_file():
        return IMAGE
    for parent in ROOT.parents:
        candidate = parent / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    return IMAGE


def relative_call(address: int, opcode: int, target: int) -> bytes:
    return bytes((opcode,)) + struct.pack("<i", target - (address + 5))


def test_sprite_globals_keep_original_starts_and_paired_lifetimes() -> None:
    image_bytes = pinned_image_path().read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    image = PeImage(image_bytes, PIN)
    source = (ROOT / "src/sprite_objects.cpp").read_text(encoding="utf-8")

    documented = {
        tuple(int(value, 16) for value in match)
        for match in re.findall(
            r"^// (0x[0-9A-Fa-f]+) -> (0x[0-9A-Fa-f]+) -> (0x[0-9A-Fa-f]+)$",
            source, re.MULTILINE,
        )
    }
    assert documented == {(init, obj, cleanup) for init, obj, cleanup, _ in PAIRS}

    bindings = parse_objects(source, "src/sprite_objects.cpp", PIN, address_space="pe:va")
    by_symbol = {binding.symbol: binding for binding in bindings}
    assert len(by_symbol) == len(bindings)
    for init, obj, cleanup, number in PAIRS:
        name = f"g_UNUSED_SPRITE_VAR{number:02d}"
        symbol = f"?{name}@@3VSprite@@A"
        declaration = f"Sprite {name};  // 0x{obj:08X}"
        binding = by_symbol[symbol]
        assert binding.origin.address == obj
        assert binding.span is None and binding.evidence is None
        assert source.splitlines()[binding.line].strip() == declaration

        init_head = b"\xB9" + struct.pack("<I", obj)
        init_bytes = image.read(Span(init, init + 0x16))
        assert init_bytes[:5] == init_head
        assert init_bytes[5:10] == relative_call(init + 5, 0xE8, 0x5E37E0)
        assert init_bytes[10:15] == b"\x68" + struct.pack("<I", cleanup)
        assert init_bytes[15:20] == relative_call(init + 15, 0xE8, 0x645398)
        assert init_bytes[20:] == b"\x59\xC3"

        cleanup_bytes = image.read(Span(cleanup, cleanup + 10))
        assert cleanup_bytes[:5] == init_head
        assert cleanup_bytes[5:] == relative_call(cleanup + 5, 0xE9, 0x5E3820)
