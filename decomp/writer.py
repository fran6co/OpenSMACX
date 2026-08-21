"""Write annotations back onto source: the inverse of `reader`.

TWO OPERATIONS, EACH EXPLICIT. `write` takes a file's text and the records
that describe its annotations, and rewrites each record's marker canonically
at `record.line`, appending a record one past the last line; `remove`
deletes the annotations the records name, marker line and lesson run.
`write_file` and `remove_file` apply them on disk, grouping the records by
the paths the records themselves carry.

THE WRITER TRUSTS THE RECORDS. They are the description of the annotations
the text carries - the output of reading it, possibly edited, plus any new
ones being added - so nothing here re-reads the text to check them. The one
exception is a single-line look at each record's own line, because
replace-or-insert cannot be decided from the record alone. A marker the
records do not describe passes through `write` untouched; deleting it is a
`remove`, stated deliberately, not an omission.

THE ANNOTATION LAYER ONLY. Code lines pass through untouched, and the fact
block under a marker passes through as it is - except the `name` and
`spans` facts, which ride the marker line itself: the marker names the
piece and says where it ends in the image, and an annotation that cannot
say both cannot be added.

THE SPELLINGS COME FROM `grammar`, nowhere else. A written marker is
`// ORIGINAL: 0x{address:08X}` with ` FILE`, ` EXCLUDED <citation>` and
` BYTE_EXACT` appended as the record requires - exactly the forms
`grammar.MARKER` and `grammar.MARKER_KEYWORD` accept - and lessons are
spelled the way `grammar.LESSON_*` accept them. That is what makes the
read -> write -> read loop closed by construction; `python -m decomp` is
the measurement that says so over the whole tree.

STATE IS NEVER WRITTEN. It is derived from the region on read, which is
what makes the loop a test: the rewritten file must re-derive the same
state from the same code.
"""

from __future__ import annotations

from pathlib import Path

from .grammar import (FACT_LINE, LESSON_CONTINUED, LESSON_DEFERRED,
                      LESSON_LEVER, LESSON_RULED_OUT, LESSON_UNRECOVERABLE,
                      MARKER)
from .model import DecompilationState, Mode, State

# ------------------------------------------------------------------ spelling


def _marker_line(record: DecompilationState) -> str:
    """The canonical marker line for a record: the address, then what the
    piece is - its name and its image spans - then how to read it."""
    line = f"// ORIGINAL: 0x{record.address:08X}"
    if record.name:
        line += f" {record.name}"
    if record.image_spans:
        line += " " + ";".join(f"0x{low:08X}-0x{high:08X}"
                               for low, high in record.image_spans)
    if record.mode is Mode.FILE:
        line += " FILE"
    if record.state is State.EXCLUDED:
        line += f" EXCLUDED {record.exclusion or 'unspecified'}"
    if record.byte_exact:
        line += " BYTE_EXACT"
    if record.semantic:
        line += " SEMANTIC"
    return line


def _fact_drop(lines: list[str], index: int) -> set[int]:
    """The 0-based line indices of the `name` and `spans` fact lines after
    the marker at `index` - those facts ride the marker line now, so the
    lines come out wherever they still exist."""
    drop = set()
    for offset, line in enumerate(lines[index + 1:], start=index + 1):
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        match = FACT_LINE.match(line)
        if match and match.group(1) in ("name", "spans"):
            drop.add(offset)
    return drop


def _lesson_lines(record: DecompilationState) -> list[str]:
    """The record's lessons, one canonical line each.

    Continuation lines are a display form: the reader joins them with a
    space, so one line per lesson re-reads to the identical tuple.
    """
    out = [f"// LEVER: {key} {prose}" for key, prose in record.levers]
    out += [f"// RULED-OUT: {prose}" for prose in record.ruled_out]
    out += [f"// UNRECOVERABLE: {prose}" for prose in record.unrecoverable]
    out += [f"// DEFERRED: {prose}" for prose in record.deferred]
    return out


def _lesson_drop(lines: list[str], index: int) -> set[int]:
    """The 0-based line indices of the lesson run owned by the marker at
    `index` - the lines a replacement must delete.

    MIRRORS `reader._lessons` line for line, because the writer and
    the reader disagreeing about which lines BELONG to a marker is the whole
    shape of the defect this module exists to avoid. A token line is
    dropped; a continuation line is dropped only while a token is open -
    the same rule `_lessons` reads by. Prose comment lines are kept: they
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


def _holds_marker(line: str, record: DecompilationState) -> bool:
    """Whether `line` carries this record's marker - the difference
    between rewriting an annotation and inserting a new one."""
    match = MARKER.search(line)
    return match is not None and int(match.group("addr"), 16) == record.address


def write(text: str, records: list[DecompilationState]) -> str:
    """The text with each of `records` written at its own line.

    `records` describes the annotations the text carries - plus, if it
    wants, new ones. A record whose line already holds its marker is
    rewritten canonically in place: the marker line carries the name and
    the image spans, the lesson run is replaced, and any leftover
    `// name` / `// spans` fact lines come out - those facts ride the
    marker now. The rest of the fact block passes through untouched. A
    record whose line holds NO marker is an annotation being ADDED above
    that line - an addition that cannot say what it names and where it
    ends is refused - and the line itself stays untouched underneath. A
    record one past the last line appends. Everything the records do not
    describe - code, prose, fact lines, other markers - passes through as
    it was.
    """
    lines = text.splitlines()

    by_line = {}
    for record in records:
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

    out, drop = [], set()
    for index, line in enumerate(lines):
        if index in drop:
            continue
        record = by_line.get(index + 1)
        if record is not None:
            if _holds_marker(line, record):
                drop.update(_lesson_drop(lines, index))
                drop.update(_fact_drop(lines, index))
                out.append(_marker_line(record))
                out.extend(_lesson_lines(record))
                continue
            # A new annotation: it goes above this line, which is kept.
            if not record.name or not record.image_spans:
                raise ValueError(
                    f"{record.address_hex}: an annotation being added must "
                    f"carry its name and its spans")
            out.append(_marker_line(record))
            out.extend(_lesson_lines(record))
        out.append(line)

    if len(lines) + 1 in by_line:
        record = by_line[len(lines) + 1]
        if not record.name or not record.image_spans:
            raise ValueError(
                f"{record.address_hex}: an annotation being added must "
                f"carry its name and its spans")
        out.append(_marker_line(record))
        out.extend(_lesson_lines(record))

    joined = "\n".join(out)
    if text.endswith("\n"):
        joined += "\n"
    return joined


def remove(text: str, records: list[DecompilationState]) -> str:
    """The text with the annotations described by `records` deleted.

    Deletion is a statement, not an omission: each named record's marker
    line and the lesson run owned by it are dropped; everything else -
    code, prose, fact lines, the markers of records not named - passes
    through untouched.
    """
    lines = text.splitlines()

    drop: set[int] = set()
    for record in records:
        if record.line > len(lines):
            raise ValueError(
                f"{record.address_hex}: line {record.line} is past the end "
                f"of a {len(lines)}-line file")
        index = record.line - 1
        drop.add(index)
        drop.update(_lesson_drop(lines, index))

    out = [line for index, line in enumerate(lines) if index not in drop]
    joined = "\n".join(out)
    if text.endswith("\n"):
        joined += "\n"
    return joined


def write_file(records: list[DecompilationState]) -> None:
    """Rewrite each record's own file with its annotations canonical.

    Records carry their paths: they are grouped by file, and each file is
    read, rewritten and saved. The text comes from disk because a record
    carries only what ITS annotation claims - the region - and a file is
    also everything between and around its annotations, which no record
    describes.
    """
    by_path: dict[Path, list[DecompilationState]] = {}
    for record in records:
        by_path.setdefault(Path(record.path), []).append(record)
    for path, group in by_path.items():
        path.write_text(write(path.read_text(), group))


def remove_file(records: list[DecompilationState]) -> None:
    """Delete the named annotations from each record's own file."""
    by_path: dict[Path, list[DecompilationState]] = {}
    for record in records:
        by_path.setdefault(Path(record.path), []).append(record)
    for path, group in by_path.items():
        path.write_text(remove(path.read_text(), group))
