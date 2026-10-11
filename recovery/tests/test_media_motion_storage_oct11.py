"""Typed decoder storage and exact bounded migration; not whole-game proof."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "8f65307007e7ba275ae9b4e42c78a42f1095cdd3"
FILES = ("src/general.cpp", "src/general.h", "src/movie_media.cpp", "src/movie_media.h")


def migration(text, relative, *, reverse=False):
    fixture = Path(__file__).with_name("media-motion-storage-oct11.patch").read_text()
    section = next(p for p in fixture.split("diff --git ")[1:]
                   if p.startswith("a/" + relative + " b/" + relative + "\n"))
    hunks = section.split("\n@@ ")[1:]
    for hunk in reversed(hunks) if reverse else hunks:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(l[1:] for l in lines if l.startswith((" ", "-")))
        after = "".join(l[1:] for l in lines if l.startswith((" ", "+")))
        if reverse:
            before, after = after, before
        assert before and text.count(before) == 1
        text = text.replace(before, after, 1)
    return text


def restore_motion_storage(text, relative):
    from test_media_stereo_state_owner_oct11 import restore_stereo_state
    if relative in ("src/movie_media.cpp", "src/movie_media.h"):
        text = restore_stereo_state(text, relative)
    # Earlier migration controls compare their own immutable checkpoints.
    # Project out this exact later migration only when its fields are present.
    present = "struct MediaMotionCoordinate {" if relative.endswith(".h") else "decoded_pixel_blocks"
    return migration(text, relative, reverse=True) if present in text else text


@pytest.mark.parametrize("relative", FILES)
def test_bounded_source_and_identity_claim_continuity(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT, text=True)
    from test_media_stereo_state_owner_oct11 import restore_stereo_state
    new = path.read_text()
    if relative in ("src/movie_media.cpp", "src/movie_media.h"):
        new = restore_stereo_state(new, relative)
    assert migration(old, relative) == new
    assert migration(new, relative, reverse=True) == old
    before, after = read_text(old, path), read_text(new, path)
    assert len(before) == len(after)
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(a, field.name) == getattr(b, field.name)
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)


def test_storage_types_and_remaining_casts():
    header = (ROOT / "src/movie_media.h").read_text()
    assert "int *motion_row_offsets;" in header
    assert "MediaMotionCoordinate *motion_coordinates;" in header
    assert "int *motion_pixel_offsets;" in header
    assert "MediaDecodedPixelBlock4x4 *decoded_pixel_blocks;" in header
    for relative in FILES:
        text = (ROOT / relative).read_text()
        for offset in ("644", "648", "64c", "650"):
            assert "owned_heap_" + offset not in text
    source = (ROOT / "src/general.cpp").read_text()
    assert "row_offsets[iVar13 + coordinate.y] + coordinate.x" in source
    assert "memcpy(&component, &uVar9, sizeof(component));" in source


def test_pinned_array_strides_signed_unpack_and_block_rows():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / '.opensmacx/game/terranx_original.exe'
    if not path.is_file():
        pytest.skip('pinned original unavailable')
    image = PeImage(path.read_bytes(), '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions = {}
    for start, end in ((0x5D3DF0, 0x5D3F48), (0x5D3980, 0x5D3DF0), (0x5D4448, 0x5D44FC)):
        instructions.update({i.address: (i.mnemonic, i.op_str) for i in
            Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(start, end)), start)})
    for address, instruction in {
        0x5D3E98: ('shl', 'edx, 3'),
        0x5D3EA1: ('mov', 'dword ptr [esi + 0x644], eax'),
        0x5D3EEF: ('neg', 'eax'),
        0x5D3EFD: ('mov', 'dword ptr [edx + ecx*4], eax'),
        0x5D3AA0: ('lea', 'edx, [edi*8]'),
        0x5D3AC6: ('lea', 'eax, [edi*4]'),
        0x5D3BC4: ('and', 'eax, 0x3ff'),
        0x5D3BCE: ('or', 'eax, 0xfffffc00'),
        0x5D3BD6: ('sub', 'ecx, 4'),
        0x5D3C07: ('lea', 'ecx, [ecx + eax*8]'),
        0x5D3C0D: ('mov', 'ecx, dword ptr [ecx + 4]'),
        0x5D3C16: ('add', 'ecx, dword ptr [esi]'),
        0x5D3C1E: ('mov', 'dword ptr [esi + eax*4], ecx'),
        0x5D4491: ('mov', 'ebx, dword ptr [ebx + eax*4]'),
        0x5D44CC: ('lea', 'ebx, [ebx + eax*2]'),
        0x5D44CF: ('mov', 'eax, dword ptr [ebx]'),
        0x5D44D3: ('mov', 'eax, dword ptr [ebx + 4]'),
        0x5D44D9: ('mov', 'eax, dword ptr [ebx + 8]'),
        0x5D44DF: ('mov', 'eax, dword ptr [ebx + 0xc]'),
    }.items():
        assert instructions[address] == instruction
