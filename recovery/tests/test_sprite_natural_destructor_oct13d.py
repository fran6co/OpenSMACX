"""Pinned destructor contract; native retention is checked at publication."""
import os
from pathlib import Path
import re
import struct

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin

ROOT = Path(__file__).resolve().parents[2]


def test_original_sprite_destructor_is_a_close_tail_call():
    image_path = Path(os.environ.get(
        'OPENSMACX_IMAGE', str(ROOT / '.opensmacx/game/terranx_original.exe')))
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    raw = image.read(Span(0x406850, 0x406855))
    assert len(raw) == 5 and raw[0] == 0xE9
    assert 0x406855 + struct.unpack('<i', raw[1:])[0] == 0x5E3820
    row, = (row for path in ('src/sprite.cpp', 'src/sprite_objects.cpp')
            for row in read_file(ROOT / path)
            if row.address == 0x406850)
    assert row.path == ROOT / 'src/sprite_objects.cpp'
    assert row.image_spans == ((0x406850, 0x406855),)
    assert row.body == 'src/sprite.h'
    header = (ROOT / row.body).read_text()
    # Natural inline destruction still delegates to the real Sprite owner;
    # no exported trampoline or substitute cleanup body is introduced.
    assert re.search(r'^\s*~Sprite\(\)\s*\{\s*close\(\);\s*\}', header, re.M)
