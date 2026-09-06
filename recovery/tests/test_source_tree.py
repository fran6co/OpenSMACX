"""Application-wide controls for the installed source reader and writer."""
from pathlib import Path
from match_recovery.source import read, State
from test_roundtrip import _raw_marker_sites, roundtrip_tree

REPO_ROOT = Path(__file__).resolve().parents[2]

def test_tree_roundtrip():
    """Every annotated file in src/ survives read -> write -> read.

    Measured zero skips when this landed: a new skip is an event worth
    investigating - the writer and the reader disagreeing about what an
    annotation owns.
    """
    looped, skipped = roundtrip_tree(REPO_ROOT / "src")
    expected = {path for path, _, _ in _raw_marker_sites(REPO_ROOT / "src")}
    assert expected, "the source inventory unexpectedly contains no annotations"
    assert looped == len(expected), (looped, len(expected))
    assert skipped == 0, skipped



def test_the_tree_parses_to_the_shape_consumers_read():
    """The floor, and the failure this package can actually have in
    isolation: SILENCE. A reader that finds nothing returns `[]`, and every
    count computed from it comes out zero looking like an answer.
    """
    records = read(REPO_ROOT / "src")
    expected = _raw_marker_sites(REPO_ROOT / "src")
    assert expected, "the source inventory unexpectedly contains no annotations"
    actual = {(record.path, record.line, record.address) for record in records}
    assert len(actual) == len(records), "the parser duplicated an annotation"
    assert actual == expected, {"missing": expected - actual, "unexpected": actual - expected}
    assert any(record.state is State.IMPLEMENTED for record in records)
    assert all(isinstance(record.path, Path) and record.path.is_absolute()
               for record in records)
    # A marker names the piece it carries: the migration put a name and
    # image spans on every one, and a fresh annotation must supply both.
    unnamed = [r for r in records if not r.name or not r.image_spans]
    assert not unnamed, \
        f"{len(unnamed)} records without name/spans, first at " \
        f"{unnamed[0].path}:{unnamed[0].line}"
