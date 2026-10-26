"""Assertions for tests whose transitional leaf collection was retired."""
from pathlib import Path

from match_recovery.source import SourceRole, read_file
from match_recovery.source_inventory import source_files


def assert_leaf_collection_retired(root: Path) -> None:
    seed = root / "src/leaf_recoveries.cpp"
    assert not seed.exists(), f"retired source collection unexpectedly exists: {seed}"


def assert_current_owner(root: Path, address: int, relative: str) -> None:
    owner = root / relative
    rows = [row for row in read_file(owner) if row.address == address]
    assert len(rows) == 1, f"expected one current owner record for 0x{address:08X} in {relative}"
    row, = rows
    assert row.path.resolve() == owner.resolve()
    assert row.source_role is not SourceRole.SCAFFOLD
    assert row.image_spans, f"current owner record lost original span: 0x{address:08X}"


def current_record_by_address(root: Path, address: int):
    """Find one current mapped identity without assuming its owning file."""
    matches = [row for path in source_files(root) for row in read_file(path)
               if row.address == address]
    assert len(matches) == 1, (
        f"expected one current mapped identity 0x{address:08X}, got {len(matches)}")
    row, = matches
    assert row.source_role is not SourceRole.SCAFFOLD
    assert row.image_spans, f"current owner record lost original span: 0x{address:08X}"
    return row
