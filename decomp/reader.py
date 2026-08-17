"""The parser of the source map: file in, records out.

The whole reading machinery lives here - the lesson tokens, region
extraction, the proved-store region rule and the tree walk. `read` is the
entry point: a FILE reads that file, a DIRECTORY is globbed recursively
and every annotation under it comes back, and two arguments read in-memory
text - the migrator's dry-run surface. `read_file` is the explicit
single-file form. NOTHING HERE CACHES: a call reads what is on disk at
the moment of the call, and a caller that asks repeatedly memoises at its
own layer. The patterns and the marker-recognition rule live in `grammar`;
deciding BETWEEN records - one claimant per address - lives in
`annotation_scan`. This module produces the claimants.
"""

from __future__ import annotations

import re
from pathlib import Path

from .grammar import (EXCLUSION_TOKEN, LESSON_CONTINUED, LESSON_DEFERRED,
                      LESSON_LEVER, LESSON_RULED_OUT, LESSON_UNRECOVERABLE,
                      MARKER, MARKER_KEYWORD, MARKER_MATCHED, NEXT_MARKER,
                      SENTINEL, _MANGLED_BASE, _NAME_FIELD, block_state_after,
                      parse_marker)
from .model import DecompilationState, Mode, State

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_ROOT = REPO_ROOT / "src"


def _lessons(lines: list[str], index: int) -> tuple:
    """(levers, ruled_out, unrecoverable) from the comment run after `index`.

    THE THREE TOKENS CARRY DIFFERENT DURABILITY, and that is the whole design.

      LEVER: <fingerprint> <what worked>   on a body that MATCHED. The
          divergence is gone, so the fingerprint is historical and has to be
          written down or the lesson cannot be filed against anything.
      RULED-OUT: <what did not work>       on a body that has NOT matched. The
          divergence is still live, so the key is MEASURED rather than written -
          and a key that is never written can never be stale.
      UNRECOVERABLE: <evidence>            on a PLACEHOLDER, and only there.
          NO C BODY CAN EXIST for this piece. It is a decision, and
          `recovery_frontier` filters it out of batch selection the way it
          filters EXCLUDED.
      DEFERRED: <evidence>                 also on a PLACEHOLDER only.
          A body CAN exist; nobody has had the budget to write one. This does
          NOT filter the address out of selection - the whole point is that it
          comes back.

    THE THIRD TOKEN EXISTS BECAUSE THE FIRST TWO COULD NOT SAY IT. The grammar
    used to hold that a RULED-OUT belongs only on a landed body - "you cannot
    rule a spelling out of a body that does not exist" - which is right for an
    ordinary function and wrong for one no C body can express. A function that
    reads three registers live-in with no prologue has no best attempt to land;
    demanding one asks for a fabrication so the note has somewhere to sit.

    Batch 12 hit this and it was patched by re-wording the note to `NOT
    ATTEMPTED`, which was false - it HAD been attempted, thoroughly. Batch 13
    hit it three times independently (0x005CF2F0, 0x005D0D82, 0x005D2037, all
    in the 0x005CB000-0x005D2000 neighbourhood). A vocabulary that cannot
    record a real finding gets lied to; this is the token that stops that.

    It is self-refuting like the other two: it sits on a placeholder, so if
    that address ever lands a body the gate fails until somebody deletes the
    claim or explains it.

    Both are read only from the comment run immediately after the marker, so a
    mention of either word in ordinary prose further down a file is not a claim.
    A continuation line is an indented comment line carrying no token of its own,
    which is how a long RULED-OUT list stays readable.
    """
    levers, ruled, unrecoverable, deferred, current = [], [], [], [], None
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        lever = LESSON_LEVER.match(line)
        out = LESSON_RULED_OUT.match(line)
        if lever:
            levers.append((lever.group("key"), lever.group("prose")))
            current = ("lever", len(levers) - 1)
            continue
        if out:
            ruled.append(out.group("prose"))
            current = ("ruled", len(ruled) - 1)
            continue
        dead = LESSON_UNRECOVERABLE.match(line)
        if dead:
            unrecoverable.append(dead.group("prose"))
            current = ("unrecoverable", len(unrecoverable) - 1)
            continue
        later = LESSON_DEFERRED.match(line)
        if later:
            deferred.append(later.group("prose"))
            current = ("deferred", len(deferred) - 1)
            continue
        joined = LESSON_CONTINUED.match(line)
        if joined and current:
            kind, position = current
            if kind == "lever":
                key, prose = levers[position]
                levers[position] = (key, prose + " " + joined.group("prose"))
            elif kind == "unrecoverable":
                unrecoverable[position] += " " + joined.group("prose")
            elif kind == "deferred":
                deferred[position] += " " + joined.group("prose")
            else:
                ruled[position] = ruled[position] + " " + joined.group("prose")
            continue
        current = None
    return tuple(levers), tuple(ruled), tuple(unrecoverable), tuple(deferred)


# --------------------------------------------------------------- extraction


def _abs(path: Path) -> Path:
    """Absolute: the record carries a path it can be acted on directly."""
    return Path(path).resolve()


def _brace_delta(line: str, in_block: bool) -> tuple[int, bool, bool, str]:
    """(depth change, still inside a block comment, saw an open, code) for a line.

    `saw_open` is separate from a positive delta because `void g() { }` opens
    and closes on one line: its delta is zero, and testing the delta alone
    meant a single-line definition never counted as opened at all.

    COUNTED WHERE THE COMPILER COUNTS. `line.count("{")` reads a brace in a
    comment or a string literal as structure, so prose like
    `` `class Shim { ... };` `` inside a RULED-OUT note closed the region
    early and the body below it was dropped in silence. An agent reported
    exactly that after writing the idiom into a note.

    `code` is what survived that same stripping - the line with comment text
    and string/char contents removed. It exists because `region_end` has to
    ask whether the subject is DEFINED yet, and a note naming the subject in
    prose is not a definition: `0x0063FFE0`'s own PROPOSAL line spells
    `sub_63ffe0` three lines above the helper that used to end its region.
    Returned from here rather than recomputed so there stays ONE stripper.
    """
    out, index, length, saw_open = 0, 0, len(line), False
    code = []
    while index < length:
        if in_block:
            close = line.find("*/", index)
            if close < 0:
                return out, True, saw_open, "".join(code)
            index, in_block = close + 2, False
            continue
        char = line[index]
        if char == "/" and index + 1 < length:
            following = line[index + 1]
            if following == "/":
                # rest of the line is comment
                return out, False, saw_open, "".join(code)
            if following == "*":
                index, in_block = index + 2, True
                continue
        if char in "\"'":
            quote, index = char, index + 1
            while index < length:
                if line[index] == "\\":
                    index += 2
                    continue
                if line[index] == quote:
                    index += 1
                    break
                index += 1
            continue
        if char == "{":
            out += 1
            saw_open = True
        elif char == "}":
            out -= 1
        code.append(char)
        index += 1
    return out, in_block, saw_open, "".join(code)


def _subject_identifier(lines: list[str], start: int) -> str | None:
    """The identifier the marker at `start` names, or None if it has none.

    Read out of the annotation's own `// name` field, which every catalogued
    row carries, so `region_end` needs no new argument and every caller of it
    gets the benefit. The header is scanned only up to the first brace: past
    that the region is code, and a `name` in code is not this field.
    """
    for line in lines[start:]:
        if "{" in line:
            break
        matched = _NAME_FIELD.match(line)
        if matched is None:
            continue
        parts = _MANGLED_BASE.match(matched.group(1))
        if parts is None:
            return None
        return next(group for group in parts.groups() if group)
    return None


def _region_end(lines: list[str], start: int) -> int | None:
    """Index of the line that ends the region opened at `start`, or None.

    THE LAST TOP-LEVEL CLOSE BEFORE THE NEXT MARKER, not the first. A body
    that needs something ahead of its definition - a helper class expressing a
    `__thiscall` receiver, an edited `VCall` shim whose auto-generated form is
    nullary, an inline `operator new` for a placement-new constructor - has its
    region closed at that helper's own `};`, and everything after it,
    including the function the marker names, was silently dropped. The piece
    then scores `expected one external .text symbol, found 0` while the same
    text compiles standalone.

    THREE AGENTS FOUND THIS INDEPENDENTLY in one batch and four addresses were
    converted to FILE mode to route around it. It is also a structural
    conflict with the emitter's own pattern: `vtable_shim` is deliberately a
    separate top-level class, which survives `--dir` and could not survive the
    committed-file path the gate reads.

    THE PUNCTUATION WAS STANDING IN FOR A NAME THE MARKER ALREADY CARRIES.
    Closing on `};` versus `}` distinguishes a helper CLASS from the next
    definition, but a helper FUNCTION closes `}` like anything else, so the
    subject below it was dropped just the same - reported on `0x0063FFE0`,
    which paid for it by inlining a `round_nearest` helper three times, and
    routed around again on the sibling `0x0063FE00`. So the region now
    extends while the subject is not yet DEFINED and stops at the first
    top-level close once it is. Where the name cannot be read, or is never
    defined, the punctuation rule still decides, which is why every one of
    the 5,066 committed regions ends exactly where it did before.
    """
    depth, opened, end, in_block = 0, False, None, False
    # PERMISSIVE ON PURPOSE, because the two errors are not symmetric.
    # Calling the subject defined too early only restores the old answer;
    # calling it defined too late extends the region over a neighbour. So a
    # bare mention in code counts, and `punctuated` keeps the old answer as
    # the floor for a subject that never appears at all.
    subject = _subject_identifier(lines, start)
    defined = subject is None
    wanted = None if defined else re.compile(rf"\b{re.escape(subject)}\b")
    punctuated = None
    for offset, line in enumerate(lines[start:]):
        # ONLY ONCE A REGION HAS OPENED. 20 annotations sit inside a doc
        # comment that is followed by another marker before any brace - the
        # bulk-annotated files put several markers in one header block - and
        # breaking there ended the scan before the definition was reached,
        # turning every one of them into "no closing brace within the file".
        if offset and opened and NEXT_MARKER.match(line):
            break
        delta, in_block, saw_open, code = _brace_delta(line, in_block)
        # BEFORE this line's braces are judged, since a single-line
        # definition names the subject and closes the region on one line.
        if not defined and wanted.search(code):
            defined = True
        depth += delta
        if saw_open:
            opened = True
        # ONLY A LINE THAT CARRIES A BRACE can end the region. Without this a
        # blank line after a balanced definition kept extending it, since the
        # depth is still zero there.
        if opened and depth <= 0 and (saw_open or delta):
            end = start + offset
            # A TYPE DECLARATION CLOSES WITH `};`, A FUNCTION BODY WITH `}`.
            # That is the only signal available for telling "a helper this
            # piece needs" from "the next definition, which happens to carry
            # no marker of its own" - and both appear in this tree. Absorbing
            # the second is how an over-eager region swallowed a neighbouring
            # function in `test_writeback`.
            stripped = line.rstrip()
            if not stripped.endswith((";", ",")):
                if punctuated is None:
                    punctuated = end
                if defined:
                    break
    return end if defined else punctuated


def _extract_forward_text(lines: list[str], marker_line: int) -> str:
    """The region claimed by a marker at `marker_line` (1-based), in LINES.

    The brace counter, the inclusive closing-brace line, and the `/*`
    re-opening when the extract starts mid-comment are the census rule -
    duplicated line for line here only so the scanner can run on in-memory
    text (the migrator's dry-run), and pinned to
    `byte_match_census.body_span` over the whole tree by
    test_annotation_scan so the copy cannot drift from the canonical
    extractor it mirrors.
    """
    start = marker_line - 1
    end = _region_end(lines, start)
    if end is None:
        raise ValueError("no closing brace within the file")
    text = "\n".join(lines[start:end + 1]) + "\n"
    if "*/" in text.split("{", 1)[0]:
        text = "/*\n" + text
    return text


def _is_placeholder_region(text: str) -> bool:
    if SENTINEL not in text:
        return False
    after = text.split(SENTINEL, 1)[1]
    for line in after.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        if stripped.startswith("}"):
            continue
        if stripped.startswith("return") and "PLACEHOLDER" in stripped:
            continue
        return False
    return True


def _state_of(region: str, excluded: str) -> str:
    if excluded:
        return State.EXCLUDED
    if _is_placeholder_region(region):
        return State.PLACEHOLDER
    return State.IMPLEMENTED


def _exclusion_citation(rest: str) -> str:
    rest = rest.strip()
    match = EXCLUSION_TOKEN.match(rest)
    if match:
        return f"S{match.group('section')}"
    return rest or "unspecified"

def read_file(path: Path) -> list[DecompilationState]:
    """Every annotation one file declares, in line order."""
    path = Path(path)
    return _read_text(path.read_text(), path)


def read(source: Path | str,
         path: Path | str | None = None) -> list[DecompilationState]:
    """The annotations under a path, or in a text.

    ONE ARGUMENT is a path: a FILE reads that file; a DIRECTORY is globbed
    recursively for source files and every annotation under it comes back,
    in deterministic order. TWO ARGUMENTS are in-memory TEXT attributed to
    `path` - the migrator's dry-run surface. No caching anywhere: a call
    reads what is on disk at the moment of the call. A caller that wants a
    different set of files reads them itself and keeps the results.
    """
    if path is not None:
        return _read_text(source, Path(path))
    source = Path(source)
    if source.is_dir():
        found: list[DecompilationState] = []
        # BOTH SUFFIXES. `.cpp` is what this tree is written in; `.c` arrived
        # with `src/vendor/zlib-1.0.2/`, where the recovery for thirteen
        # functions is upstream C that no C++ compiler will parse - zlib
        # 1.0.2 uses K&R definitions. A glob that took only `.cpp` would
        # report those annotations as missing while the files sat in the
        # tree.
        for suffix in ("*.cpp", "*.c"):
            for file in source.rglob(suffix):
                found.extend(read_file(file))
        found.sort(key=lambda record: (record.path, record.line,
                                       record.address))
        return found
    return read_file(source)


def _read_text(text: str, path: Path | str) -> list[DecompilationState]:
    """Scan TEXT attributed to `path` - the migrator's dry-run surface.

    Only EXPLICIT markers are read. The deprecated spellings and the
    marker-less stores were rewritten with markers by the migration, and
    the tree's rule is that recognition of those forms is deleted once the
    rewrite lands, not maintained.
    """
    path = Path(path)
    lines = text.splitlines()
    proved = _proved_store(path)
    found = []

    in_block = False
    for index, line in enumerate(lines):
        # Parse with the state the line OPENS in, then advance the state, so
        # a marker on the line that opens a comment is still inside it.
        parsed = parse_marker(line, in_block)
        in_block = block_state_after(line, in_block)
        if parsed is None:
            continue
        address, keyword, rest, matched = parsed
        found_levers, found_ruled, found_dead, found_later = \
            _lessons(lines, index)
        if keyword == "FILE":
            region = text
            found.append(DecompilationState(
                address=address, mode=Mode.FILE,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, region=region, recipe="verbatim",
                byte_exact=matched, levers=found_levers, ruled_out=found_ruled,
                unrecoverable=found_dead, deferred=found_later))
        elif keyword == "EXCLUDED":
            found.append(DecompilationState(
                address=address, mode=Mode.BODY, state=State.EXCLUDED,
                path=_abs(path), line=index + 1,
                exclusion=_exclusion_citation(rest), byte_exact=matched,
                levers=found_levers, ruled_out=found_ruled,
                unrecoverable=found_dead, deferred=found_later))
        else:
            if proved:
                # Proved bodies keep the writeback semantics even once an
                # explicit marker lands in the header: the body is the
                # definition after the leading `//` run, never a brace count
                # from the marker line (the header prose could carry braces).
                region, error = _proved_body(lines), ""
            else:
                try:
                    region = _extract_forward_text(lines, index + 1)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
            recipe = "writeback" if proved else "census"
            found.append(DecompilationState(
                address=address, mode=Mode.BODY,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, region=region, extract_error=error,
                recipe=recipe, byte_exact=matched,
                levers=found_levers, ruled_out=found_ruled,
                unrecoverable=found_dead, deferred=found_later))

    return found


def _proved_store(path: Path) -> bool:
    """True for files in the proved writeback store, by directory SHAPE,
    not repo path: the parent is `recovered` and the name is an address.

    These files carry explicit markers like everything else; the store
    still decides how their BODIES are cut - the definition after the
    leading `//` run, never a brace count from the marker line.
    """
    resolved = Path(path).resolve()
    return bool(re.fullmatch(r"[0-9a-f]{8}", resolved.stem)) \
        and resolved.parent.name == "recovered"


def _proved_body(lines: list[str]) -> str:
    """The definition out of a proved file, sans header - writeback semantics.

    Mirrors `writeback.read_matched_body`: the header is the leading
    run of `//` lines and blanks, and everything from the first other line is
    the body. Kept local because importing writeback drags in the PE
    and writeback machinery a scanner must not need.
    """
    index = 0
    while index < len(lines) and (not lines[index].strip()
                                  or lines[index].lstrip().startswith("//")):
        index += 1
    return "\n".join(lines[index:]).strip() + "\n"
