#!/usr/bin/env -S uv run python
"""Does an annotation survive being rewritten? Four invariants, all zero.

THE WRITER USED TO REGENERATE PROSE, and regenerated prose does not come
back the way it went in. `osmx record` rewrote each lesson run from a lossy
parse, so 434 of the tree's records changed their own file the moment they
were recorded - a net 3,998 lines of measured LEVER/TRIED history reflowed,
truncated, hoisted out of the paragraph that explained them, or deleted
outright. None of it was visible: the roundtrip test compared POST-PARSE
tuples, which makes read -> write -> read a perfect fixed point over data
that was already lossy.

The four invariants below are what that test could not ask. They are
pinned at ZERO, not behind a falling ceiling: the fix did not rewrite the
tree to match the writer, it made the writer agree with what 5,968
annotations already said.

  I   a no-op rewrite is byte-identical
  II  flipping the ratchet keyword changes exactly ONE line, and that line
      differs only by the keyword
  III a demotion preserves the file's line count and the lessons it re-reads
  IV  clearing a record's lessons deletes exactly the lines `origin`
      attributes to them - no more, no fewer

II and III derive their edits from `decomp.record.stamped` rather than
re-spelling its rule, because a checker carrying its own copy of the rule
it checks is the defect it exists to catch.

    uv run tools/annotation_identity.py           # the census
    uv run tools/annotation_identity.py --check   # exit 1 if any broke
"""
from __future__ import annotations

import collections
import sys
from dataclasses import replace
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))

from decomp import read, write                        # noqa: E402
from decomp.asm import AsmComparison, Tier            # noqa: E402
from decomp.record import stamped                     # noqa: E402


def by_file(root: Path) -> dict:
    grouped = collections.defaultdict(list)
    for record in read(root):
        grouped[Path(record.path)].append(record)
    return grouped


def _one_keyword_flip(text: str, after: str) -> str:
    """Empty if the two texts differ by one line, and that line differs
    only by a ratchet keyword; otherwise what went wrong."""
    before_lines, after_lines = text.splitlines(), after.splitlines()
    if len(before_lines) != len(after_lines):
        return (f"line count moved {len(before_lines)} -> "
                f"{len(after_lines)}")
    moved = [i for i, (a, b) in enumerate(zip(before_lines, after_lines))
             if a != b]
    if len(moved) != 1:
        return f"{len(moved)} lines changed, expected 1"
    a, b = before_lines[moved[0]], after_lines[moved[0]]
    for word in (" BYTE_EXACT", " SEMANTIC"):
        if a.replace(word, "") == b.replace(word, ""):
            return ""
    return f"the changed line differs by more than a keyword:\n  {a}\n  {b}"


def census(root: Path):
    failures = collections.defaultdict(list)
    records = 0
    for path, group in sorted(by_file(root).items()):
        text = path.read_text(errors="replace")
        records += len(group)

        # I - a no-op rewrite must not touch a byte.
        if write(text, group) != text:
            failures["I no-op rewrite"].append(str(path))

        for record in group:
            where = f"{path.name}:{record.line} {record.address_hex}"

            # II - the ratchet keyword is the only thing a flip may move.
            flipped = replace(record, byte_exact=not record.byte_exact)
            problem = _one_keyword_flip(text, write(text, [flipped]))
            if problem:
                failures["II keyword flip"].append(f"{where}: {problem}")

            # III - a demotion may not change how many lines the file has,
            # nor which lessons it re-reads.
            down = stamped(record, AsmComparison(verdict=Tier.MISMATCH),
                           demote=True)
            after = write(text, [down])
            if len(after.splitlines()) != len(text.splitlines()):
                failures["III demotion line count"].append(where)
            elif (collections.Counter(_reread(after, record.line).ruled_out)
                  != collections.Counter(down.ruled_out)):
                failures["III demotion lessons"].append(where)

            # IV - clearing the lessons deletes exactly their own lines.
            if record.origin:
                bare = replace(record, levers=(), ruled_out=(),
                               unrecoverable=(), deferred=())
                owned = sum(len(lesson.lines) for lesson in record.origin)
                lost = (len(text.splitlines())
                        - len(write(text, [bare]).splitlines()))
                if lost != owned:
                    failures["IV lesson deletion"].append(
                        f"{where}: dropped {lost} lines, owns {owned}")

    return records, failures


def _reread(text: str, line: int):
    """The record at `line` in `text`, read back from the text itself."""
    from decomp.reader import _read_text
    for candidate in _read_text(text, REPO / "src" / "reread.cpp"):
        if candidate.line == line:
            return candidate
    raise AssertionError(f"no record re-read at line {line}")


def main() -> int:
    check = "--check" in sys.argv
    root = REPO / "src"
    records, failures = census(root)

    if failures:
        for name, rows in sorted(failures.items()):
            print(f"ANNOTATION IDENTITY BROKE: {name} - {len(rows)} case(s)")
            for row in rows[:10]:
                print(f"    {row}")
            if len(rows) > 10:
                print(f"    ... and {len(rows) - 10:,} more")
        return 1

    print(f"{'annotation identity: ' if check else ''}{records:,} annotation(s) "
          f"survive rewrite, demotion and deletion unchanged"
          f"{', all four invariants at zero' if check else ''}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
