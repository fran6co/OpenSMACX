"""Cross-check the two measured terrain table windows against source values.

These bounded reads establish only the pinned values at the stated starts;
they do not assert original symbol names, source constness, or allocation size.
"""
from pathlib import Path
import os
import re
import struct

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
CANYON = (0x6858D8, 0x685908, "WorldCanyonOffsetValues", 12,
          (26, 27, 9, 1, 2, 0, 3, 4, 17, 23, 36, 35))
RIDGE = (0x685908, 0x68593C, "world_ridge_offset_val", 13,
         (47, 44, 24, 20, 8, 7, 0, 5, 4, 17, 23, 35, 45))


def source_values(source, name, count):
    pattern = (r"(?:(?:const\s+)?uint32_t)\s+" + re.escape(name)
               + r"\s*\[\s*" + str(count) + r"\s*\]\s*=\s*\{([^}]*)\}")
    matches = re.findall(pattern, source)
    assert len(matches) == 1, "expected one source definition for " + name
    return tuple(int(value.strip(), 0) for value in matches[0].split(",") if value.strip())


def test_static_terrain_table_values_match_pinned_pe_windows():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required for table-window verification")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    source = SOURCE.read_text(encoding="utf-8")
    for start, end, name, count, expected in (CANYON, RIDGE):
        assert end - start == count * 4
        actual = struct.unpack("<" + "I" * count, image.read(Span(start, end)))
        assert actual == expected, (hex(start), actual)
        assert source_values(source, name, count) == actual, name
