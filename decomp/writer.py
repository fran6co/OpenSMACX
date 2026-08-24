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

THE ANNOTATION LAYER ONLY, and less of it than there used to be. Code
lines pass through untouched and so does the fact block - including any
`name` or `spans` line, which this module used to sweep out on the theory
that those facts ride the marker now. Measured: no real ones remain, and
the sweep's only live effect was deleting a WRAPPED PROSE line that began
with the word "name". A marker still carries the name and the spans, and an
annotation that cannot say both cannot be added.

THE RECORD CARRIES ITS OWN LINES. "The writer trusts the records" used to
mean "the writer discards what the record does not hold", because the
record was not a complete description of the annotation: a lesson came back
as joined prose and went out as one canonical line. `record.origin` closes
that gap, so an unedited lesson is put back exactly as its author wrote it,
and only the marker is re-spelled canonically.

THE SPELLINGS COME FROM `grammar`, nowhere else. A written marker is
`// ORIGINAL: 0x{address:08X}` with ` FILE`, ` EXCLUDED <citation>` and
` BYTE_EXACT` appended as the record requires - exactly the forms
`grammar.MARKER` and `grammar.MARKER_KEYWORD` accept - and lessons are
spelled the way `grammar.LESSON_*` accept them. That is what makes the
read -> write -> read loop closed by construction. What MEASURES that over
the whole tree is `tools/annotation_identity.py`, wired into `osmx check`;
`python -m decomp` has not existed since `__main__.py` was folded into
`decomp/tests/test_roundtrip.py`, and naming a dead entry point is how this
module's central claim went unenforced long enough to reflow 434 records.

STATE IS NEVER WRITTEN. It is derived from the region on read, which is
what makes the loop a test: the rewritten file must re-derive the same
state from the same code.
"""

from __future__ import annotations

import re
from pathlib import Path

from .grammar import (FACT_LINE, LESSON_CONTINUED, LESSON_DEFERRED,
                      LESSON_LEVER, LESSON_RULED_OUT, LESSON_UNRECOVERABLE,
                      MARKER, NEXT_MARKER)
from .model import DecompilationState, Mode, State
from .reader import _lesson_run

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


# `_fact_drop` IS GONE, and its absence is the point. It deleted any line
# under a marker whose first word was `name` or `spans`, on the theory that
# those facts ride the marker line now. Measured over the whole tree: ZERO
# real `// name` / `// spans` fact lines remain, and its only live effect was
# deleting src/win.cpp:2473 - a WRAPPED PROSE line reading "name in early
# transcriptions guessed: ..." - which silently rewrote the sentence above it
# into something its author never said. A sweep with no work left to do and
# one way to corrupt is not a sweep worth keeping; `address_index` and
# `marker_symbols` already police marker identity.


def _lesson_lines(record: DecompilationState) -> list[str]:
    """The record's lessons, one canonical line each.

    Continuation lines are a display form: the reader joins them with a
    space, so one line per lesson re-reads to the identical tuple.
    """
    out = [f"// LEVER: {key} {prose}" for key, prose in record.levers]
    out += [f"// TRIED: {prose}" for prose in record.ruled_out]
    # UNRECOVERABLE is no longer emitted - the reader folds it into
    # `ruled_out`, so it comes back out as `// TRIED:` above.
    out += [f"// TRIED: {prose}" for prose in record.unrecoverable]
    out += [f"// DEFERRED: {prose}" for prose in record.deferred]
    return out


def _reads_back_as(candidate: list[str], kind: str, key: str,
                   prose: str) -> bool:
    """Would these lines re-read as exactly this lesson?

    THE CONTRACT IS THAT THE RECORD AND THE FILE AGREE. Keeping an author's
    line verbatim is the whole point of provenance, but not at the cost of
    the file saying something the record does not. `// LEVER: jl/jge  flipped
    ...` - two spaces - reads as prose `flipped ...` with the key split off,
    and `stamped` rejoins it with ONE space; re-spelling that line in place
    would leave the file re-reading a prose the record never stated. So the
    reader itself is asked, and a line that would not come back the same is
    spelled canonically instead. Faithful where it can be, honest always.
    """
    run = _lesson_run(["// ORIGINAL: 0x00000000 x 0x0-0x1", *candidate], 0)
    return (len(run) == 1 and run[0].kind == kind
            and run[0].key == key and run[0].prose == prose)


def _wanted(record: DecompilationState) -> list[tuple[str, str, str]]:
    """(kind, key, prose) for every lesson the record currently states."""
    out = [("lever", key, prose) for key, prose in record.levers]
    out += [("ruled", "", prose) for prose in record.ruled_out]
    out += [("unrecoverable", "", prose) for prose in record.unrecoverable]
    out += [("deferred", "", prose) for prose in record.deferred]
    return out


def _origin_is_live(record: DecompilationState, lines: list[str]) -> bool:
    """Whether `record.origin`'s offsets still describe THIS text.

    THE ONE THING THE WRITER CHECKS. `origin` carries each lesson line's
    text as well as its offset precisely so that a caller who edited
    `record.line`, or who read the record from different text, is caught
    by a string compare instead of corrupting a file. On any mismatch the
    record falls back to canonical spelling, which is what the writer did
    for every record before provenance existed - degraded, never wrong.
    """
    if record.origin is None:
        return False
    for lesson in record.origin:
        for offset, original in lesson.lines:
            if offset >= len(lines) or lines[offset] != original:
                return False
    return True


def _retoken(line: str, word: str) -> str:
    """`line` with its lesson keyword replaced by `word`, nothing else moved.

    This is the ONLY edit made to an existing lesson line, and it exists for
    exactly one transition: `record.stamped` demotes a landed body and moves
    its LEVER into `ruled_out`. Re-spelling one word in place is what lets
    the prose beside it - the wrapping, the parenthetical qualifier, the
    measurement - survive a demotion.
    """
    return re.sub(r"\b(?:LEVER|TRIED|RULED-OUT|UNRECOVERABLE|DEFERRED)\b",
                  word, line, count=1)


_TOKEN_WORD = {"lever": "LEVER", "ruled": "TRIED",
               "unrecoverable": "TRIED", "deferred": "DEFERRED"}


def _reconcile(record: DecompilationState, lines: list[str], index: int):
    """(drop, respell, additions, insert_after) for one record's lesson run.

    THE RECORD SAYS WHAT ITS LESSONS MEAN; `origin` SAYS WHAT THEY LOOK
    LIKE. A lesson whose (kind, key, prose) is unchanged is left exactly
    where its author put it, byte for byte. A lesson the record no longer
    states has its lines dropped. A lesson the record states that no origin
    line accounts for is spelled canonically and inserted after the run.

    That ordering is what makes the whole thing byte-identical on a no-op
    rewrite, and it is why this module stopped regenerating prose: prose
    regenerated from a joined string cannot come back the way it went in.
    """
    wanted = _wanted(record)
    if not _origin_is_live(record, lines):
        # No usable provenance: the historical behaviour, which is to
        # replace the run wholesale.
        return (_lesson_drop(lines, index), {}, _lesson_lines(record), index)

    remaining = list(wanted)
    drop: set = set()
    respell: dict = {}
    last = index

    for lesson in record.origin:
        exact = (lesson.kind, lesson.key, lesson.prose)
        demoted = ("ruled", "", f"{lesson.key} {lesson.prose}".strip())
        if exact in remaining:
            remaining.remove(exact)
            last = max(last, lesson.lines[-1][0])
            continue
        # The two in-place re-spellings: `stamped`'s demotion of a LEVER,
        # and the retired `UNRECOVERABLE:` spelling migrating to `TRIED:`.
        # Both change ONE WORD on ONE line, which is what lets the prose
        # beside them - the wrapping, the parenthetical, the measurement -
        # survive an edit that used to regenerate the whole run.
        migrating = None
        if lesson.kind == "lever" and demoted in remaining:
            migrating = demoted
        elif lesson.kind == "unrecoverable" and \
                ("ruled", "", lesson.prose) in remaining:
            migrating = ("ruled", "", lesson.prose)
        if migrating is not None:
            offset, text_ = lesson.lines[0]
            respelled = _retoken(text_, "TRIED")
            candidate = [respelled] + [t_ for _, t_ in lesson.lines[1:]]
            if _reads_back_as(candidate, "ruled", "", migrating[2]):
                remaining.remove(migrating)
                respell[offset] = respelled
                last = max(last, lesson.lines[-1][0])
                continue
        drop.update(offset for offset, _ in lesson.lines)

    additions = [f"// {_TOKEN_WORD[kind]}: "
                 + (f"{key} {prose}" if kind == "lever" else prose)
                 for kind, key, prose in remaining]
    return drop, respell, additions, last


def _lesson_drop(lines: list[str], index: int) -> set[int]:
    """The 0-based line indices of the lesson run owned by the marker at
    `index` - the lines a replacement must delete.

    MIRRORS `reader._lessons` line for line, because the writer and
    the reader disagreeing about which lines BELONG to a marker is the whole
    shape of the defect this module exists to avoid. A token line is
    dropped; a continuation line is dropped only while a token is open -
    the same rule `_lessons` reads by. Prose comment lines are kept: they
    are not annotations.

    IT STOPPED MIRRORING, AND THAT CORRUPTED FILES. `_lessons` gained three
    rules - a marker inside a `/* */` block keeps reading past the closing
    `*/`, and inside a block an unprefixed line is still tried as a lesson -
    and this function kept the old one, breaking at the first line not
    starting with `//` or `*`. The two then disagreed about which lines a
    marker owns, so `write` deleted FEWER lines than it re-emitted and every
    rewrite added a second copy of the same lesson. Thirteen files in src/
    were failing the roundtrip fixed-point check this way, wave.cpp among
    them, and each `osmx record` touching one of them duplicated a line.

    The block-detection below is `_lessons`'s, verbatim in behaviour: scan
    BACKWARDS for whether this marker sits inside a block comment, then apply
    the same continue/break rules going forward.
    """
    inside = False
    for earlier in reversed(lines[:index]):
        head = earlier.strip()
        if head.startswith("*/"):
            break
        if head.startswith("/*"):
            inside = True
            break

    drop = set()
    current = None
    for offset, line in enumerate(lines[index + 1:], start=index + 1):
        stripped = line.strip()
        # Mirrors reader._lessons: a marker owns only up to the next marker.
        if NEXT_MARKER.match(line):
            break
        if inside and "*/" in stripped:
            inside = False
            if stripped not in ("*/", "*/;"):
                continue
        prefixed = stripped.startswith("//") or stripped.startswith("*")
        if not prefixed and not inside:
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

    drop: set = set()
    respell: dict = {}
    add_after: dict = {}
    for line_number, record in by_line.items():
        index = line_number - 1
        if index < len(lines) and _holds_marker(lines[index], record):
            gone, spelled, additions, after = _reconcile(record, lines, index)
            for offset in gone & set(respell):
                respell.pop(offset, None)
            drop |= gone
            respell.update(spelled)
            if additions:
                add_after.setdefault(after, []).extend(additions)

    out = []
    for index, line in enumerate(lines):
        if index in drop:
            continue
        record = by_line.get(index + 1)
        if record is not None:
            if _holds_marker(line, record):
                out.append(_marker_line(record))
                if index in add_after:
                    out.extend(add_after.pop(index))
                continue
            # A new annotation: it goes above this line, which is kept.
            if not record.name or not record.image_spans:
                raise ValueError(
                    f"{record.address_hex}: an annotation being added must "
                    f"carry its name and its spans")
            out.append(_marker_line(record))
            out.extend(_lesson_lines(record))
        out.append(respell.get(index, line))
        if index in add_after:
            out.extend(add_after.pop(index))

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
