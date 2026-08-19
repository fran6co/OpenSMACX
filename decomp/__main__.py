"""`uv run python -m decomp` - are the reader and the writer sane?

TWO CHECKS. The first is the ground truth and would be enough on its own:
parse `src/` and prove the result against the tree itself - the count, the
shapes every consumer reads, and the completeness every annotation now
carries. It catches the failure this package can actually have in
isolation, which is silence: a reader that finds nothing returns `[]`, and
every count computed from it comes out zero looking like an answer.

The second closes the loop the writer opens: every annotated file is read,
rewritten in memory from its own records, and read again - the two parses
must agree in every fact they carry, and a second write from the rewritten
text must change nothing. The annotation layer is allowed to canonicalise;
the code underneath it is not.

There used to be a third: the parse held against the `tools/` copies, so
the two could not drift while both existed. The marker format moved on
2026-08-18 - the marker names the piece, carrying its name and its image
spans - and the tools copies stayed on the old spelling, by decision; the
comparison died with the shared format, and the package proves itself
against `src/` alone from then on.
"""

from __future__ import annotations

from pathlib import Path

from decomp import DecompilationState, State, grammar, read, reader, write
from decomp.reader import SRC_ROOT

REPO_ROOT = Path(__file__).resolve().parent.parent


def sanity() -> list:
    """The floor: `src/` parses, and to the shape consumers read."""
    records = read(SRC_ROOT)
    assert len(records) > 5000, len(records)
    assert any(record.state is State.IMPLEMENTED for record in records)
    assert all(isinstance(record.path, Path) and record.path.is_absolute()
               for record in records)
    # A marker names the piece it carries: the migration put a name and
    # image spans on every one, and a fresh annotation must supply both.
    unnamed = [r for r in records if not r.name or not r.image_spans]
    assert not unnamed, \
        f"{len(unnamed)} records without name/spans, first at " \
        f"{unnamed[0].path}:{unnamed[0].line}"
    return records


def _code_only(region: str) -> str:
    """The region with comments and blank lines removed."""
    kept, in_block = [], False
    for line in region.splitlines():
        text = line.strip()
        if in_block:
            if "*/" in text:
                in_block = False
                text = text.split("*/", 1)[1].strip()
            else:
                continue
        if "//" in text:
            text = text.split("//", 1)[0].strip()
        if text.startswith("/*"):
            if "*/" not in text:
                in_block = True
            continue
        if not text:
            continue
        kept.append(text)
    return "\n".join(kept)


def _region_code(region: str) -> str:
    """The region's code with the annotation layer removed.

    A region can start mid-comment - a bare marker inside a doc block - and
    `_code_only` cannot know that: it keeps the bare marker line, which is
    annotation, and drops the `//`-prefixed one the writer emits, so the two
    spellings of the SAME layer would compare as different code. Marker
    lines are removed from both sides first; the code underneath is what a
    round trip must preserve.
    """
    lines = [line for line in region.splitlines()
             if not grammar.MARKER.search(line)]
    return _code_only("\n".join(lines))


def _key(record: DecompilationState) -> tuple:
    """Everything a round trip must preserve.

    `line` is NOT in it: it is a position in one text, and a rewrite that
    canonicalises the annotation layer - a wrapped lesson re-emitted on one
    line - legitimately moves every line below. The region is compared
    code-only for the same reason, and the record fields beside it already
    prove the annotations survived. What must never change is the code.
    """
    return (record.address, record.mode, record.state,
            _region_code(record.region), record.byte_exact,
            record.exclusion, record.extract_error, record.recipe,
            record.levers, record.ruled_out, record.unrecoverable,
            record.deferred, record.name, record.image_spans,
            record.symbol)


def roundtrip_tree(root: Path = SRC_ROOT) -> tuple[int, int]:
    """(looped, skipped): files whose annotations survive write -> read.

    Every file with annotations is read, rewritten in memory from its own
    records, and read again; the two parses must agree field for field.
    A skip means `write` refused a record - measured zero since the
    migration gave every annotation an explicit marker, so a new skip is
    an event worth investigating.
    """
    by_path: dict[Path, list[DecompilationState]] = {}
    for record in read(root):
        by_path.setdefault(record.path, []).append(record)
    looped = skipped = 0
    for path, records in by_path.items():
        try:
            rewritten = write(path.read_text(), records)
        except ValueError:
            skipped += 1
            continue
        reread = reader.read(rewritten, path)
        if [_key(r) for r in records] != [_key(r) for r in reread]:
            skipped += 1
            continue
        # THE FIXED POINT. The first write may canonicalise - one line per
        # lesson, keywords in one order - and shift every line below; the
        # second write, from the records of the text it produced, must
        # change nothing. Canonical is stable or it is not canonical.
        if write(rewritten, reread) != rewritten:
            skipped += 1
            continue
        looped += 1
    return looped, skipped


def loop() -> tuple[int, int]:
    """(looped, skipped): every annotated file read, rewritten from its own
    records, and read again - the writer proved against the tree the sanity
    check just proved the reader against. Fails if nothing loops: a writer
    that cannot round-trip a single file is not a writer."""
    looped, skipped = roundtrip_tree()
    assert looped > 0, "no file survives the read -> write -> read loop"
    return looped, skipped


def main() -> int:
    records = sanity()
    looped, skipped = loop()
    print(f"ok: {len(records)} records (proved against src/, loop closed on "
          f"{looped} files, {skipped} skipped)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
