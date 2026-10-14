"""Original Sprite global identities and paired CRT lifetime evidence."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import struct
import subprocess

from match_recovery.model import Span
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text


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
    objects = (ROOT / "src/sprite_objects.cpp").read_text()
    definitions = r"^Sprite \w+(?:\[\d+\])?;[^\n]*$"
    expected = re.findall(definitions, before, re.MULTILINE)
    assert len(expected) == 190
    assert re.findall(definitions, objects, re.MULTILINE) == expected
    assert not re.findall(definitions, implementation, re.MULTILINE)
    assert "Sprite::Sprite()" in implementation
    assert "Sprite::Sprite()" not in objects

    old = read_text(before, ROOT / "src/sprite.cpp")
    kept = read_text(implementation, ROOT / "src/sprite.cpp")
    moved = read_text(objects, ROOT / "src/sprite_objects.cpp")
    identity = lambda row: (row.address, row.name, row.image_spans, row.semantic)
    assert sorted(map(identity, old)) == sorted(map(identity, kept + moved))
    assert len(moved) == 381  # 380 lifetime helpers plus the natural inline destructor.
    previous = {row.address: row for row in old}
    # Recording may add freshly measured obligations, never withdraw old ones.
    for row in kept + moved:
        assert not previous[row.address].byte_exact or row.byte_exact
    for row in kept:
        assert row.body == previous[row.address].body


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
