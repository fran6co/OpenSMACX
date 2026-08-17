"""`uv run python -m decomp` - are the reader and the writer sane?

THREE CHECKS. The first is the ground truth and would be enough on its own:
parse `src/` and prove the result against the tree itself - the counts, the
shapes every consumer reads, and one known-good recovered body. It catches
the failure this package can actually have in isolation, which is silence: a
reader that resolves to nothing returns `{}`, and every count computed from
it comes out zero looking like an answer.

The second closes the loop the writer opens: every annotated file is read,
rewritten in memory from its own records, and read again - the two parses
must agree in every fact they carry, and a second write from the rewritten
text must change nothing. The annotation layer is allowed to canonicalise;
the code underneath it is not.

The third is transitional. The same grammar also lives in
`tools/annotation_scan.py` and `tools/project_catalogue.py`, which the 61
scripts in `tools/` still import; while both copies exist, this holds the
package's parse against theirs and fails on any record, any catalogue row,
or any pattern of the grammar disagreeing. The shapes the two copies expose
are deliberately different - this package's records carry enums, absolute
paths and no parsing metadata - so the comparison projects both sides onto
the facts the grammar decides. It skips itself once those modules are gone,
which is what a finished refactor looks like.
"""

from __future__ import annotations

import sys
from collections.abc import Callable
from pathlib import Path

from decomp import (DecompilationState, State, from_source, grammar, resolve,
                    scan_tree, writer)
from decomp import project_catalogue

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"

FACTS = ("name", "size", "body_ranges", "prototype", "binary_kind",
         "recovery_state", "end_address")


def sanity() -> tuple:
    """The floor: `src/` parses, and to the shape consumers read."""
    records, _duplicates = resolve(scan_tree())
    assert len(records) > 5000, len(records)
    assert any(record.state is State.IMPLEMENTED for record in records)
    assert all(isinstance(record.path, Path) and record.path.is_absolute()
               for record in records)

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
    return records, rows


def _fingerprint(records: list[DecompilationState], claim_attr: str,
                 path_of: Callable[[DecompilationState], str]) -> list:
    """Everything about a record that the grammar decides.

    `claim_attr` and `path_of` project each copy's shape onto common facts:
    the ratchet claim is `byte_exact` here and `matched` in tools/, the path
    is absolute here and repo-relative there.
    """
    return sorted((a.address, a.mode, a.state, path_of(a), a.line,
                   a.exclusion, getattr(a, claim_attr), a.recipe,
                   a.extract_error, hash(a.region), a.levers, a.ruled_out,
                   a.unrecoverable, a.deferred) for a in records)


def drift(records: list[DecompilationState], rows: dict[int, dict]) -> bool:
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
    ours = _fingerprint(records, "byte_exact",
                        lambda a: str(a.path.relative_to(REPO_ROOT)))
    mine = _fingerprint(theirs, "matched", lambda a: a.path)
    assert len(theirs) == len(records), \
        f"record count: tools {len(theirs)} vs decomp {len(records)}"
    if ours != mine:
        for left, right in zip(ours, mine):
            assert left == right, f"record drift:\n  tools  {left}\n  decomp {right}"
    assert ours == mine, "record drift past the pairwise walk"

    their_rows = their_catalogue.from_source()
    assert set(their_rows) == set(rows), \
        (f"row addresses: tools {len(their_rows)} vs decomp {len(rows)}, "
         f"symmetric difference {len(set(their_rows) ^ set(rows))}")
    prefix = str(REPO_ROOT) + "/"
    for address, row in sorted(rows.items()):
        # `source_locations` carries this package's absolute paths; tools/
        # carries repo-relative ones. Project onto the common form.
        ours = dict(row)
        if ours.get("source_locations", "").startswith(prefix):
            ours["source_locations"] = ours["source_locations"][len(prefix):]
        assert their_rows[address] == ours, \
            f"row drift at 0x{address:08X}:\n  tools  {their_rows[address]}\n  decomp {ours}"

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


def loop() -> tuple:
    """(looped, skipped): every annotated file read, rewritten from its own
    records, and read again - the writer proved against the tree the sanity
    check just proved the reader against. Fails if nothing loops: a writer
    that cannot round-trip a single file is not a writer."""
    looped, skipped = writer.roundtrip_tree()
    assert looped > 0, "no file survives the read -> write -> read loop"
    return looped, skipped


def main() -> int:
    records, rows = sanity()
    checked = drift(records, rows)
    looped, skipped = loop()
    against = "agrees with tools/" if checked else "tools/ copies absent"
    print(f"ok: {len(records)} records, {len(rows)} catalogue rows "
          f"(proved against src/, loop closed on {looped} files, "
          f"{skipped} skipped, {against})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
