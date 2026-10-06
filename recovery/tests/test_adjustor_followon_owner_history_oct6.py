"""Identity/history controls for the Console + residual adjustor migration.

These tests use the complete pre-transfer source at BASE and current source
records. They do not claim current native matching or final linker selection.
"""
from functools import lru_cache
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery import binding_source
from match_recovery.source import Mode, SourceRole, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "5f3476a12d0fbd1675e5a9f6ab47b4f5759f28db"
SPECS = (
    (0x0051DE60, "src/console.cpp", "?on_sys_close@Console@@$4PPPPPPPM@A@AEXXZ", 14),
    (0x0051DED0, "src/console.cpp", "?on_key_click@Console@@$4PPPPPPPM@A@AEHHH@Z", 25),
    (0x0051DF40, "src/console.cpp", "?on_nc_hittest@Console@@$4PPPPPPPM@A@AEHHH@Z", 75),
    (0x0060CF60, "src/listbox.cpp", "?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z", 15),
    (0x0060CFB0, "src/listbox.cpp", "?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z", 26),
    (0x0060FE80, "src/checkbox.cpp", "?on_key_down@CheckBox@@$4PPPPPPPM@A@AEHH@Z", 26),
    (0x00611850, "src/spritebox.cpp", "?on_left_click@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z", 19),
    (0x00611880, "src/spritebox.cpp", "?on_right_click@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z", 22),
    (0x006118B0, "src/spritebox.cpp", "?on_key_down@SpriteBox@@$4PPPPPPPM@A@AEHH@Z", 26),
    (0x006127D0, "src/editgroup.cpp", "?pass_dialog_focus@EditGroup@@$4PPPPPPPM@A@AEXXZ", 64),
)
MIGRATED = frozenset(address for address, *_ in SPECS)


@lru_cache(None)
def before_adjustors():
    raw = subprocess.check_output(
        ["git", "show", f"{BASE}:src/adjustor_thunks.cpp"], cwd=ROOT
    )
    return {row.address: row for row in read_text(raw.decode(), ROOT / "src/adjustor_thunks.cpp")}


@pytest.mark.parametrize("address,owner,symbol,slot", SPECS)
def test_ten_transfer_records_preserve_full_identity_and_history(address, owner, symbol, slot):
    prior = before_adjustors()[address]
    rows = read_file(ROOT / owner)
    matches = [row for row in rows if row.address == address]
    assert len(matches) == 1
    current, = matches
    assert current.mode is Mode.FILE
    assert current.source_role is SourceRole.UNCLASSIFIED
    assert current.symbol == symbol
    for field in ("name", "image_spans", "kind", "byte_exact", "semantic",
                  "levers", "ruled_out", "unrecoverable", "deferred"):
        assert getattr(current, field) == getattr(prior, field), (hex(address), field)
    assert address not in {row.address for row in read_file(ROOT / "src/adjustor_thunks.cpp")}
    binding = binding_source.parse((ROOT / owner).read_text(), rows)[current.line]
    text = "\n".join(binding["evidence"])
    assert "constructor" in text.lower()
    assert f"slot {slot}" in text or f"slot +0x{slot * 4:X}" in text
    if address == 0x0051DF40:
        assert "prototype says void" in text
        assert "sets EAX to 0 or 1" in text
        assert "int(int,int)" in text


def test_superseded_ten_adapter_helpers_are_absent_and_other_rows_remain():
    from test_virtual_adjustor_owners_oct6 import FOLLOW_ON_MIGRATED_ADDRESSES
    assert FOLLOW_ON_MIGRATED_ADDRESSES == MIGRATED
    old = before_adjustors()
    helpers = []
    for address in MIGRATED:
        symbol = old[address].symbol
        found = re.match(r"\?(adjust_[^@]+)@@", symbol or "")
        assert found, hex(address)
        helpers.append(found.group(1))
    for path in (ROOT / "src").rglob("*"):
        if path.suffix not in (".cpp", ".h"):
            continue
        text = path.read_text(errors="surrogateescape")
        for helper in helpers:
            assert not re.search(r"\b" + re.escape(helper) + r"\s*\(", text), (path, helper)
    current = {row.address: row for row in read_file(ROOT / "src/adjustor_thunks.cpp")}
    assert set(current) == set(old) - MIGRATED
