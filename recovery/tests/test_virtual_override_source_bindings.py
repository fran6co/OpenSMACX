"""Keep original identities separate from current emitted override symbols."""
from pathlib import Path

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("source,address,original,emitted,end", [
    ("netwin.cpp", 0x00483310, "?on_key_down@NetWin@@QAEHH@Z",
     "?on_key_down@NetWin@@UAEHH@Z", 0x00483414),
    ("prodpicker.cpp", 0x00493C20, "?on_mousewheel@ProdPicker@@QAEXH@Z",
     "?on_mousewheel@ProdPicker@@UAEXH@Z", 0x00493C68),
])
def test_virtual_override_retains_original_identity(source, address, original, emitted, end):
    rows = [row for row in read_file(ROOT / "src" / source) if row.address == address]
    assert len(rows) == 1
    row = rows[0]
    assert row.name == original
    assert row.symbol == emitted
    assert row.image_spans == ((address, end),)
    assert row.byte_exact  # Existing historical obligation, not a fresh measurement.
