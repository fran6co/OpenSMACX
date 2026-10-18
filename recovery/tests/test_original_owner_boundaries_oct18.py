"""Original control-flow and neighbor controls, not linked/runtime acceptance."""
from dataclasses import replace

import pytest

from match_recovery.model import Span
from match_recovery.source import read_file
from match_recovery.source.reader import read_text
from match_recovery.source.writer import write_bytes

from test_original_function_boundaries_oct18 import ROOT, _entry_flow, _original


CASES = (
    ("src/net_class.cpp", 0x62E010, 0x62E24A, 0x62E2CF, 184,
     (0x62E249,), ((0x62E24A, 0x62E250), (0x62E299, 0x62E2A0)),
     ((0x62E250, 0x62E299), (0x62E2A0, 0x62E41C))),
    ("src/faction.cpp", 0x55D2A0, 0x55D425, 0x55D444, 128,
     (0x55D33C, 0x55D3D1, 0x55D40B, 0x55D424),
     ((0x55D425, 0x55D430),), ((0x55D430, 0x55D753),)),
)


@pytest.mark.parametrize("unit,start,end,old_end,count,returns,padding,neighbors", CASES)
def test_original_flow_and_neighbor_identity(unit, start, end, old_end, count,
                                            returns, padding, neighbors):
    image = _original()
    instructions, exits, escaped = _entry_flow(image, start, end)
    assert len(instructions) == count
    assert escaped == ()
    assert sorted(address for address, kind, _ in exits if kind == "return") == list(returns)
    assert max(stop for _, _, stop in exits) == end
    rows = {row.address: row for row in read_file(ROOT / unit)}
    assert rows[start].image_spans == ((start, end),)
    assert old_end > end
    for low, high in padding:
        assert image.read(Span(low, high)) == b"\x90" * (high - low)
    for address, stop in neighbors:
        assert rows[address].image_spans == ((address, stop),)
        assert address >= end
    # The old interval consumes part of a distinct neighbor, not only fill.
    assert any(address < old_end < stop for address, stop in neighbors)


@pytest.mark.parametrize("unit,start,end,old_end,count,returns,padding,neighbors", CASES)
def test_native_writer_preserves_other_identities_and_rejects_old_extent(
        unit, start, end, old_end, count, returns, padding, neighbors):
    raw = (ROOT / unit).read_bytes()
    rows = {row.address: row for row in read_text(raw.decode(), unit)}
    current = rows[start]
    rewritten = write_bytes(raw, [replace(current, image_spans=((start, end),))])
    assert rewritten == raw
    damaged = write_bytes(raw, [replace(current, image_spans=((start, old_end),))])
    changed = {row.address: row for row in read_text(damaged.decode(), unit)}
    assert changed[start].image_spans != ((start, end),)
    assert (changed[start].byte_exact, changed[start].semantic) == (
        current.byte_exact, current.semantic)
    for address in rows.keys() - {start}:
        before, after = rows[address], changed[address]
        assert (before.name, before.symbol, before.image_spans,
                before.mode, before.byte_exact, before.semantic) == (
                    after.name, after.symbol, after.image_spans,
                    after.mode, after.byte_exact, after.semantic)
