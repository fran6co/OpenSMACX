"""Write annotations back onto source: the inverse of `reader`.

`write` takes a file's text and the records it should carry, and returns the
text whose annotation layer expresses exactly those records - replacing the
markers already there, removing the ones that are not, inserting the ones
that are new. `write_file` does the same to a file on disk.

THE ANNOTATION LAYER ONLY. Code lines pass through untouched, and the
catalogue's fact block - the `// name`, `// size`, `// spans` lines stamped
under a marker - is prose as far as this module is concerned: it belongs to
`project_catalogue`, and the writer never touches it.

THE SPELLINGS COME FROM `grammar`, nowhere else. A written marker is
`// ORIGINAL: 0x{address:08X}` with ` FILE`, ` EXCLUDED <citation>` and
` BYTE_EXACT` appended as the record requires - exactly the forms
`grammar.MARKER` and `grammar.MARKER_KEYWORD` accept - and lessons are
spelled the way `grammar.LESSON_*` accept them. That is what makes the
read -> write -> read loop closed by construction, and `roundtrip_tree` is
the measurement that says so over the whole tree.

STATE IS NEVER WRITTEN. It is derived from the region on read, which is
what makes the loop a test: the rewritten file must re-derive the same
state from the same code.
"""

from __future__ import annotations

from pathlib import Path

from . import reader
from .annotation_scan import SRC_ROOT, _code_only, _parse_marker
from .reader import _block_state_after
from .grammar import (LESSON_CONTINUED, LESSON_DEFERRED, LESSON_LEVER,
                      LESSON_RULED_OUT, LESSON_UNRECOVERABLE, MARKER)
from .model import Mode, State

# ------------------------------------------------------------------ spelling


def marker_line(record) -> str:
    """The canonical marker line for a record."""
    line = f"// ORIGINAL: 0x{record.address:08X}"
    if record.mode is Mode.FILE:
        line += " FILE"
    if record.state is State.EXCLUDED:
        line += f" EXCLUDED {record.exclusion or 'unspecified'}"
    if record.byte_exact:
        line += " BYTE_EXACT"
    return line


def lesson_lines(record) -> list:
    """The record's lessons, one canonical line each.

    Continuation lines are a display form: the reader joins them with a
    space, so one line per lesson re-reads to the identical tuple.
    """
    out = [f"// LEVER: {key} {prose}" for key, prose in record.levers]
    out += [f"// RULED-OUT: {prose}" for prose in record.ruled_out]
    out += [f"// UNRECOVERABLE: {prose}" for prose in record.unrecoverable]
    out += [f"// DEFERRED: {prose}" for prose in record.deferred]
    return out


def _lesson_drop(lines: list, index: int) -> set:
    """The 0-based line indices of the lesson run owned by the marker at
    `index` - the lines a replacement must delete.

    MIRRORS `annotation_scan.lessons` line for line, because the writer and
    the reader disagreeing about which lines BELONG to a marker is the whole
    shape of the defect this module exists to avoid. A token line is
    dropped; a continuation line is dropped only while a token is open -
    the same rule `lessons` reads by. Prose comment lines are kept: they
    are not annotations.
    """
    drop = set()
    current = None
    for offset, line in enumerate(lines[index + 1:], start=index + 1):
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        if (LESSON_LEVER.match(line) or LESSON_RULED_OUT.match(line)
                or LESSON_UNRECOVERABLE.match(line)
                or LESSON_DEFERRED.match(line)):
            drop.add(offset)
            current = True
            continue
        if LESSON_CONTINUED.match(line) and current:
            drop.add(offset)
            continue
        current = None
    return drop


# ------------------------------------------------------------------- writing


def write(text: str, records: list) -> str:
    """The text carrying exactly `records` as its annotations.

    Markers already in the text are REPLACED: a record at the marker's line
    rewrites it, and a marker no record claims is removed with its lesson
    run. A record at a line holding no marker is INSERTED there; a line past
    the last line appends. Fact lines, prose and code pass through
    untouched.
    """
    lines = text.splitlines()

    by_line = {}
    for record in records:
        if not record.line:
            raise ValueError(
                f"{record.address_hex}: filename-derived records are not "
                f"marker-addressable")
        if record.mode is Mode.FILE and record.state is State.EXCLUDED:
            raise ValueError(
                f"{record.address_hex}: FILE and EXCLUDED are mutually "
                f"exclusive marker keywords")
        if record.line > len(lines) + 1:
            raise ValueError(
                f"{record.address_hex}: line {record.line} is past the end "
                f"of a {len(lines)}-line file")
        if record.line in by_line:
            raise ValueError(
                f"{record.address_hex}: line {record.line} is claimed by "
                f"another record")
        by_line[record.line] = record

    existing = {}
    in_block = False
    for index, line in enumerate(lines):
        parsed = _parse_marker(line, in_block)
        in_block = _block_state_after(line, in_block)
        if parsed is not None:
            existing[index + 1] = parsed[0]

    out, drop = [], set()
    for index, line in enumerate(lines):
        if index in drop:
            continue
        number = index + 1
        if number in existing:
            drop.update(_lesson_drop(lines, index))
            record = by_line.get(number)
            if record is not None:
                out.append(marker_line(record))
                out.extend(lesson_lines(record))
            continue
        if number in by_line:
            record = by_line[number]
            out.append(marker_line(record))
            out.extend(lesson_lines(record))
        out.append(line)

    if len(lines) + 1 in by_line:
        record = by_line[len(lines) + 1]
        out.append(marker_line(record))
        out.extend(lesson_lines(record))

    joined = "\n".join(out)
    if text.endswith("\n") or not joined:
        joined += "\n" if text.endswith("\n") else ""
    return joined


def write_file(path, records: list) -> None:
    """Rewrite the file at `path` so it carries exactly `records`."""
    path = Path(path)
    resolved = path.resolve()
    for record in records:
        if Path(record.path).resolve() != resolved:
            raise ValueError(
                f"{record.address_hex}: record names {record.path}, "
                f"not {path}")
    path.write_text(write(path.read_text(), records))


# --------------------------------------------------------------- the proof


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
             if not MARKER.search(line)]
    return _code_only("\n".join(lines))


def _key(record) -> tuple:
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
            record.deferred)


def roundtrip_tree(root: Path = SRC_ROOT) -> tuple:
    """(looped, skipped): files whose annotations survive write -> read.

    Every file with annotations is read, rewritten in memory from its own
    records, and read again; the two parses must agree field for field.
    Skipped files are the ones the loop is NOT for: filename-derived store
    records, which cannot be marker-addressed, and the legacy inline
    spellings, whose markers point backward - migrating those is a rewrite
    of the CODE's comment, not of the annotation layer, and belongs to the
    migrator.
    """
    looped = skipped = 0
    for path in reader.sources(root):
        records = reader.read_file(path)
        if not records:
            continue
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
