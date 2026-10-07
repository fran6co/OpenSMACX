"""Assertions for tests whose transitional leaf collection was retired."""
from pathlib import Path

from match_recovery.source import SourceRole, read_file


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
