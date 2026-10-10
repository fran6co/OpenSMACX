"""Current-symbol corrections must preserve every original binding and body."""
from dataclasses import asdict
from pathlib import Path
import subprocess

from match_recovery.source import read_text, write_symbol

ROOT = Path(__file__).resolve().parents[2]
BASE = "1aa75f00d946a4165c6fba4374fddc37b879666a"
SYMBOLS = {
    0x45C700: "?on_left_click@MainInterface@@UAEXHH@Z",
    0x45C740: "?on_right_click@MainInterface@@UAEXHH@Z",
}


def original(path):
    return subprocess.check_output(["git", "-C", str(ROOT), "show", f"{BASE}:{path}"])


def test_only_two_current_symbol_facts_change():
    path = "src/maininterface.cpp"
    before = original(path)
    expected = before
    for address, symbol in SYMBOLS.items():
        row = next(row for row in read_text(expected.decode(), path) if row.address == address)
        expected = write_symbol(expected, row, symbol)
    after = (ROOT / path).read_bytes()
    assert after == expected
    assert (ROOT / "src/maininterface.h").read_bytes() == original("src/maininterface.h")
    old = {row.address: row for row in read_text(before.decode(), path)}
    new = {row.address: row for row in read_text(after.decode(), path)}
    assert old.keys() == new.keys()
    for address, row in old.items():
        left, right = asdict(row), asdict(new[address])
        left.pop("line"); right.pop("line")
        # Reader regions include surrounding annotation text. Whole-file
        # equality to the two writer operations above proves their preservation.
        left.pop("region"); right.pop("region")
        for facts in (left, right):
            for provenance in facts["origin"]:
                provenance["lines"] = tuple(text for _, text in provenance["lines"])
        if address in SYMBOLS:
            assert row.name == SYMBOLS[address].replace("@@UAEX", "@@QAEX")
            assert new[address].symbol == SYMBOLS[address]
            left.pop("symbol"); right.pop("symbol")
        assert left == right
