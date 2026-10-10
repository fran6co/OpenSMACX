"""Complete source bindings and original tail witnesses, not full acceptance."""
from dataclasses import replace, fields
from pathlib import Path
import csv

import pytest
from match_recovery.source import read_text, write
from match_recovery.source.asm import Listing
from match_recovery.source_history import revision_files
from match_recovery.source_inventory import image_pin
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span

ROOT = Path(__file__).resolve().parents[2]
BASE = "44f4141de73d0ddbaa0df731ee0cc0f7cd0a0651"
ENDS = {
    0x42BAB0: ("src/datalink.cpp", 0x42BCB1),
    0x481DA0: ("src/netwin.cpp", 0x481E8D),
    0x483190: ("src/netwin.cpp", 0x483234),
    0x483420: ("src/netwin.cpp", 0x4834C5),
    0x4C59A0: ("src/sounddevice.cpp", 0x4C59D7),
    0x4E7E50: ("src/base.cpp", 0x4E7F64),
    0x604300: ("src/basepop.cpp", 0x6043CA),
    0x614E50: ("src/editbox.cpp", 0x614F2C),
    0x618430: ("src/caviar.cpp", 0x618468),
}
RECORDED = {0x483420, 0x4C59A0, 0x614E50, 0x618430}


def test_only_native_writer_extent_and_recorder_claim_changes():
    files = tuple(sorted({unit for unit, end in ENDS.values()}))
    historical = revision_files(ROOT, BASE, ("src",))
    added_bytes = 0
    new_claims = set()
    for unit in files:
        path = ROOT / unit
        old = historical[unit].decode()
        current = path.read_text()
        before = read_text(old, path)
        expected = []
        for row in before:
            updated = row
            if row.address in ENDS:
                end = ENDS[row.address][1]
                assert len(row.image_spans) == 1
                start, previous_end = row.image_spans[0]
                assert start == row.address and end > previous_end
                added_bytes += end - previous_end
                updated = replace(updated, image_spans=((start, end),))
                if row.address in RECORDED:
                    if not row.byte_exact:
                        new_claims.add(row.address)
                    updated = replace(updated, byte_exact=True, semantic=False)
            expected.append(updated)
        assert current == write(old, expected), "body, identity or unrelated annotation changed"
        after = read_text(current, path)
        assert write(current, after) == current
        previous = {r.address: r for r in before}
        assert {r.address for r in after} == set(previous)
        for row in after:
            baseline = previous[row.address]
            for field in fields(row):
                if field.name not in {"line", "region", "origin", "image_spans", "byte_exact", "semantic"}:
                    assert getattr(row, field.name) == getattr(baseline, field.name)
            def lessons(r):
                return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
            assert lessons(row) == lessons(baseline)
            assert not baseline.byte_exact or row.byte_exact
            assert not baseline.semantic or row.semantic or row.byte_exact
            if row.address in ENDS:
                assert row.image_spans == ((row.address, ENDS[row.address][1]),)
                assert row.byte_exact == (row.address in RECORDED)
    assert added_bytes == 44
    assert new_claims == RECORDED - {0x4C59A0}


def test_independent_pinned_catalog_has_the_full_function_endpoints():
    pin = image_pin(ROOT)
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as stream:
        rows = [row for row in csv.DictReader(stream) if row["program_sha256"] == pin
                and int(row["address"], 16) in ENDS]
    assert len(rows) == len(ENDS)
    for row in rows:
        address = int(row["address"], 16)
        assert int(row["end_address"], 16) == ENDS[address][1]


def test_actual_original_complete_ranges_end_with_their_own_returns():
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    for address, (unit, end) in ENDS.items():
        listing = Listing(image.read(Span(address, end)), address)
        assert "ret" in listing.lines[-1]
    wave = Listing(image.read(Span(0x4C59A0, 0x4C59D7)), 0x4C59A0)
    lines = "\n".join(wave.lines)
    assert "0x004C59C5" in lines and "0x004C59CE" in lines
    assert "mov eax, 0x14" in lines and "mov eax, 0xc" in lines
    # Neither catalog coalescence nor five bytes of padding justify extending
    # Heap over the independently owned ServiceList::on_insert.
    heap = next(r for r in read_text((ROOT / "src/heap.cpp").read_text(), ROOT / "src/heap.cpp")
                if r.address == 0x5D4680)
    assert heap.image_spans == ((0x5D4680, 0x5D47CB),)
