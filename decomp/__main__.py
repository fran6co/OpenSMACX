"""`uv run python -m decomp` - is the reader sane, and has the copy drifted?

TWO CHECKS. The first is the ground truth and would be enough on its own:
parse `src/` and prove the result against the tree itself - the counts, the
shapes every consumer reads, and one known-good recovered body. It catches
the failure this package can actually have in isolation, which is silence: a
reader that resolves to nothing returns `{}`, and every count computed from
it comes out zero looking like an answer.

The second is transitional. The same grammar also lives in
`tools/annotation_scan.py` and `tools/project_catalogue.py`, which the 61
scripts in `tools/` still import; while both copies exist, this holds the
package's parse against theirs and fails on any annotation, any catalogue
row, or any pattern of the grammar disagreeing. It skips itself once those
modules are gone, which is what a finished refactor looks like.
"""

from __future__ import annotations

import sys
from pathlib import Path

from decomp import (STATE_IMPLEMENTED, from_source, grammar, resolve,
                    scan_tree)
from decomp import project_catalogue

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"

FACTS = ("name", "size", "body_ranges", "prototype", "binary_kind",
         "recovery_state", "end_address")


def sanity() -> tuple:
    """The floor: `src/` parses, and to the shape consumers read."""
    annotations, _duplicates = resolve(scan_tree())
    assert len(annotations) > 5000, len(annotations)
    assert any(a.state == STATE_IMPLEMENTED for a in annotations)

    rows = from_source()
    assert len(rows) > 5000, len(rows)
    assert all(isinstance(address, int) for address in rows)
    sample = next(iter(rows.values()))
    for key in FACTS:
        assert key in sample, key

    # Buffer::Buffer, src/buffer.cpp - a recovered body carrying a full fact
    # block, so it exercises both readers at once.
    row = rows[0x005D7210]
    assert row["name"] == "??0Buffer@@QAE@XZ", row["name"]
    assert row["recovery_state"] == "source_complete", row["recovery_state"]
    return annotations, rows


def _fingerprint(annotations: list) -> list:
    """Everything about an annotation that the grammar decides."""
    return sorted((a.address, a.mode, a.state, a.path, a.line, a.deprecated,
                   a.exclusion, a.matched, a.recipe, a.extract_error,
                   hash(a.region), a.levers, a.ruled_out, a.unrecoverable,
                   a.deferred) for a in annotations)


def drift(annotations: list, rows: dict) -> bool:
    """True if the check ran. Raises if the two parsers disagree."""
    if not (TOOLS / "annotation_scan.py").is_file():
        print("skip: tools/annotation_scan.py is gone - the copies are now one")
        return False
    # THE ONE PLACE `tools/` IS TOUCHED, and only to be disagreed with. The
    # package itself never does this; a check that compares two things has to
    # reach both.
    sys.path.insert(0, str(TOOLS))
    try:
        import annotation_scan as their_scan
        import project_catalogue as their_catalogue
    except ImportError as missing:            # their deps, not ours
        print(f"skip: tools/ copies not importable ({missing})")
        return False
    finally:
        sys.path.remove(str(TOOLS))

    theirs, _ = their_scan.resolve(their_scan.scan_tree())
    ours, mine = _fingerprint(theirs), _fingerprint(annotations)
    assert len(theirs) == len(annotations), \
        f"annotation count: tools {len(theirs)} vs decomp {len(annotations)}"
    if ours != mine:
        for left, right in zip(ours, mine):
            assert left == right, f"annotation drift:\n  tools  {left}\n  decomp {right}"
    assert ours == mine, "annotation drift past the pairwise walk"

    their_rows = their_catalogue.from_source()
    assert set(their_rows) == set(rows), \
        (f"row addresses: tools {len(their_rows)} vs decomp {len(rows)}, "
         f"symmetric difference {len(set(their_rows) ^ set(rows))}")
    for address, row in sorted(rows.items()):
        assert their_rows[address] == row, \
            f"row drift at 0x{address:08X}:\n  tools  {their_rows[address]}\n  decomp {row}"

    # The grammar itself, EXHAUSTIVELY - every pattern `grammar` declares, so
    # a tightened pattern is caught even on a tree where no annotation
    # happens to exercise the difference.
    for name in grammar.SCAN_PATTERNS:
        assert getattr(their_scan, name).pattern == \
            getattr(grammar, name).pattern, f"{name} drift"
    for name in grammar.CATALOGUE_PATTERNS:
        assert getattr(their_catalogue, name).pattern == \
            getattr(grammar, name).pattern, f"{name} drift"
    assert their_catalogue.CONTINUABLE == grammar.CONTINUABLE, \
        "CONTINUABLE drift"
    return True


def main() -> int:
    annotations, rows = sanity()
    checked = drift(annotations, rows)
    against = "agrees with tools/" if checked else "tools/ copies absent"
    print(f"ok: {len(annotations)} annotations, {len(rows)} catalogue rows "
          f"(proved against src/, {against})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
