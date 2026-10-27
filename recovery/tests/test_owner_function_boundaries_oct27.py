"""Pinned function ends exclude alignment bytes before neighboring entries."""
from dataclasses import fields, replace
import csv
from difflib import SequenceMatcher
import re
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.source import Mode, read_file, read_text
from match_recovery.source.writer import write_bytes
from test_original_function_boundaries_oct18 import PIN, _entry_flow, _original


ROOT = Path(__file__).resolve().parents[2]
IDA_EXPORT = ROOT / "docs/recovery/ida9-functions.csv"
GHIDRA_EXPORT = ROOT / "docs/recovery/ghidra-functions.csv"

TARGETS = {
    "src/caviar.cpp": {
        0x00617F20: {
            "name": "?add_object@Caviar@@QAEHPAUCaviarData@@H@Z",
            "span": (0x00617F20, 0x00618079),
            "old_padding_end": 0x0061807D,
            "next": 0x00618080,
            "instructions": 111,
            "returns": (0x00617F39, 0x00617F4D, 0x00618069, 0x00618076),
            "mode": Mode.BODY,
        },
        0x006181D0: {
            "name": "?set_shadow_vector@Caviar@@QAEXMM@Z",
            "span": (0x006181D0, 0x0061824B),
            "old_padding_end": 0x0061824F,
            "next": 0x00618250,
            "instructions": 44,
            "returns": (0x00618248,),
            "mode": Mode.FILE,
        },
    },
    "src/datalink.cpp": {
        0x0042B190: {
            "name": "?set_cat_tech@Datalink@@QAEXXZ",
            "span": (0x0042B190, 0x0042B2F3),
            "old_padding_end": 0x0042B2FA,
            "next": 0x0042B300,
            "instructions": 116,
            "returns": (0x0042B2F2,),
            "mode": Mode.FILE,
        },
    },
}

NEIGHBORS = {
    "src/caviar.cpp": {
        0x00618080: ("?add_object@Caviar@@QAEHPAUCaviarData@@PAEH@Z",
                     (0x00618080, 0x006181D0)),
        0x00618250: ("?UNK5@Caviar@@QAEXHH@Z", (0x00618250, 0x00618253)),
    },
    "src/datalink.cpp": {
        0x0042B300: ("?set_cat_concept@Datalink@@QAEXXZ",
                     (0x0042B300, 0x0042B48C)),
    },
}

EXPECTED_IDA_SOURCE_IDC_SHA256 = (
    "42032fc2807f7dec6de1ed6efb9d18a70668a015f23e32d58149c9d9952bc0a7"
)


def _rows(relative: str, raw: bytes | None = None):
    path = ROOT / relative
    if raw is None:
        return {record.address: record for record in read_file(path)}
    return {
        record.address: record
        for record in read_text(raw.decode("utf-8", errors="surrogateescape"), relative)
    }


def _metadata(record):
    """All parsed metadata except the extent and raw comment/body region."""
    return tuple((field.name, getattr(record, field.name))
                 for field in fields(record)
                 if field.name not in ("image_spans", "region"))


def _marker_change_lines(before: bytes, after: bytes, addresses: set[int]):
    old_lines = before.decode("utf-8", errors="surrogateescape").splitlines(keepends=True)
    new_lines = after.decode("utf-8", errors="surrogateescape").splitlines(keepends=True)
    removed, added = [], []
    for tag, left, right, new_left, new_right in SequenceMatcher(
            None, old_lines, new_lines, autojunk=False).get_opcodes():
        if tag != "equal":
            removed.extend(old_lines[left:right])
            added.extend(new_lines[new_left:new_right])

    marker = re.compile(
        r"^(?P<prefix>.*ORIGINAL:\s+(?P<address>0x[0-9A-Fa-f]+)\s+\S+\s+"
        r"0x[0-9A-Fa-f]+-)(?P<end>0x[0-9A-Fa-f]+)(?P<suffix>.*)$"
    )

    def indexed(lines):
        result = {}
        for line in lines:
            match = marker.match(line.rstrip("\r\n"))
            assert match, f"non-marker source line changed: {line!r}"
            address = int(match.group("address"), 16)
            assert address in addresses
            assert address not in result
            result[address] = match
        return result

    old, new = indexed(removed), indexed(added)
    assert set(old) == set(new) == addresses
    for address in addresses:
        assert old[address].group("prefix") == new[address].group("prefix")
        assert old[address].group("suffix") == new[address].group("suffix")
    return old, new


def _assert_target_records(relative: str, records) -> None:
    for address, expected in TARGETS[relative].items():
        row = records[address]
        assert row.name == expected["name"]
        assert row.image_spans == (expected["span"],)
        assert row.mode is expected["mode"]


def _assert_neighbor_records(relative: str, records) -> None:
    for address, (name, span) in NEIGHBORS[relative].items():
        row = records[address]
        assert row.name == name
        assert row.image_spans == (span,)


def test_pinned_full_entry_flows_end_before_padding_and_next_mappings():
    image = _original()
    for relative, targets in TARGETS.items():
        records = _rows(relative)
        for start, expected in targets.items():
            end = expected["span"][1]
            instructions, exits, escaped = _entry_flow(image, start, end)
            assert min(instructions) == start
            assert len(instructions) == expected["instructions"]
            assert tuple(address for address, kind, _ in exits if kind == "return") == expected["returns"]
            assert all(kind == "return" for _, kind, _ in exits)
            assert escaped == ()

            old_end = expected["old_padding_end"]
            assert image.read(Span(end, old_end)) == b"\x90" * (old_end - end)
            with pytest.raises(AssertionError):
                _entry_flow(image, start, old_end)

            next_entry = expected["next"]
            padding = image.read(Span(end, next_entry))
            assert padding == b"\x90" * (next_entry - end)
            assert end < next_entry

        _assert_target_records(relative, records)
        _assert_neighbor_records(relative, records)


def test_ida_and_ghidra_exports_independently_agree_on_corrected_ends():
    with IDA_EXPORT.open(newline="", encoding="utf-8") as stream:
        ida = {int(row["address"], 16): row for row in csv.DictReader(stream)}
    with GHIDRA_EXPORT.open(newline="", encoding="utf-8") as stream:
        ghidra = {int(row["address"], 16): row for row in csv.DictReader(stream)}

    assert {row["source_idc_sha256"] for row in ida.values()} == {
        EXPECTED_IDA_SOURCE_IDC_SHA256
    }
    for targets in TARGETS.values():
        for address, expected in targets.items():
            end = expected["span"][1]
            assert int(ida[address]["end_address"], 16) == end
            assert int(ghidra[address]["end_address"], 16) == end
            assert ghidra[address]["program_sha256"] == PIN


def test_native_writer_padding_and_neighbor_tail_damage_controls():
    for relative, targets in TARGETS.items():
        path = ROOT / relative
        raw = path.read_bytes()
        before = _rows(relative, raw)
        _assert_target_records(relative, before)
        _assert_neighbor_records(relative, before)
        all_before = before
        target_addresses = set(targets)

        for address, expected in targets.items():
            old_end = expected["old_padding_end"]
            row = before[address]
            damaged = write_bytes(raw, [replace(
                row, image_spans=((address, old_end),)
            )])
            damaged_rows = _rows(relative, damaged)
            with pytest.raises(AssertionError):
                _assert_target_records(relative, damaged_rows)
            _marker_change_lines(raw, damaged, {address})
            for record_address, old_record in all_before.items():
                if record_address != address:
                    assert _metadata(damaged_rows[record_address]) == _metadata(old_record)

        # A source writer mutation that truncates a neighboring identity must
        # be rejected even when the target's corrected span remains untouched.
        for address, (name, span) in NEIGHBORS[relative].items():
            sibling = before[address]
            damaged = write_bytes(raw, [replace(
                sibling, image_spans=((span[0], span[1] - 1),)
            )])
            damaged_rows = _rows(relative, damaged)
            with pytest.raises(AssertionError):
                _assert_neighbor_records(relative, damaged_rows)
            _marker_change_lines(raw, damaged, {address})
            for record_address, old_record in all_before.items():
                if record_address != address:
                    assert _metadata(damaged_rows[record_address]) == _metadata(old_record)

        # Reapplying the corrected spans through the native writer changes
        # only these markers and leaves every sibling's metadata intact.
        old_ends = write_bytes(raw, [
            replace(before[address],
                    image_spans=((address, expected["old_padding_end"]),))
            for address, expected in targets.items()
        ])
        old_end_rows = _rows(relative, old_ends)
        corrected = write_bytes(old_ends, [
            replace(old_end_rows[address], image_spans=(expected["span"],))
            for address, expected in targets.items()
        ])
        _marker_change_lines(old_ends, corrected, target_addresses)
        corrected_rows = _rows(relative, corrected)
        _assert_target_records(relative, corrected_rows)
        _assert_neighbor_records(relative, corrected_rows)
        for address, old_record in all_before.items():
            if address not in target_addresses:
                assert _metadata(corrected_rows[address]) == _metadata(old_record)
            else:
                assert _metadata(corrected_rows[address]) == _metadata(old_record)
                assert corrected_rows[address].region.replace(
                    next(line for line in corrected_rows[address].region.splitlines()
                         if f"ORIGINAL: 0x{address:08X}" in line), ""
                ) == old_record.region.replace(
                    next(line for line in old_record.region.splitlines()
                         if f"ORIGINAL: 0x{address:08X}" in line), ""
                )
