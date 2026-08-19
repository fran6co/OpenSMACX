"""The parser of the source map: file in, records out.

The whole reading machinery lives here - the lesson tokens, region
extraction, the proved-store region rule and the tree walk. `read` is the
entry point: a FILE reads that file, a DIRECTORY is globbed recursively
and every annotation under it comes back, and two arguments read in-memory
text. `read_file` is the explicit single-file form. NOTHING HERE CACHES:
a call reads what is on disk at
the moment of the call, and a caller that asks repeatedly memoises at its
own layer. The patterns live in `grammar`; deciding BETWEEN records - one
claimant per address - lives in `annotation_scan`. This module produces
the claimants.
"""

from __future__ import annotations

import re
from pathlib import Path

from .grammar import (DEFINITION_HEAD, DEFINITION_KEYWORDS,
                      EXCLUSION_TOKEN, FACT_LINE, LESSON_CONTINUED,
                      LESSON_DEFERRED,
                      LESSON_LEVER, LESSON_RULED_OUT, LESSON_UNRECOVERABLE,
                      MARKER, MARKER_KEYWORD, MARKER_MATCHED, MARKER_TAIL,
                      NEXT_MARKER, SENTINEL, _MANGLED_BASE)
from .model import DecompilationState, Mode, Recipe, State

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_ROOT = REPO_ROOT / "src"


# --------------------------------------------------------------- recognition

# The grammar APPLIED: the rule for where a marker is valid. A hit in code
# or data is prose, not a map entry, and this is the ONE implementation of
# that rule - everything that reads a marker reads it through here.
def _block_state_after(line: str, in_block: bool) -> bool:
    """Cheap block-comment tracking: count the transitions on the line.

    Deliberately naive about strings containing comment marks - a false
    positive there only makes the scanner MORE willing to read a marker, and
    a marker at an address the catalogue does not know is reported as
    uncatalogued rather than believed, so the error cannot fabricate a map
    entry.
    THE SCAN IS BY `find`, NOT BY CHARACTER. This ran once per line of every
    file under `src/`, and every tool in the loop reads the tree at
    startup: the catalogue, the brief, the verifier and the ratchet all pay
    it. Measured 2026-08-14, the character walk was 730,000 calls making 30
    MILLION `startswith` probes - 8.8 s of the 12 s `load_functions` took, and
    the largest single cost in the whole critical path. `find` does the same
    search in C and skips everything between the marks.

    The states are the same three: outside, inside a block, and stopped at a
    line comment. Depth never exceeds one - the character version only ever
    incremented from zero - so `/*` inside a block is text, exactly as before.
    """
    index = 0
    if in_block:
        end = line.find("*/")
        if end < 0:
            return True
        index = end + 2
    while True:
        start = line.find("/*", index)
        if start < 0:
            return False
        line_comment = line.find("//", index)
        if 0 <= line_comment < start:
            return False
        end = line.find("*/", start + 2)
        if end < 0:
            return True
        index = end + 2


def _parse_marker(line: str, in_block: bool) -> tuple | None:
    """(address, keyword, rest, matched, name, spans) for a marker IN A
    COMMENT.

    The marker names the piece before saying how to read it: name and
    image spans ride the tail ahead of the keywords. A tail without them
    is the pre-migration spelling; it yields empty name and spans, and
    the construction falls back to the fact block.
    """
    match = MARKER.search(line)
    if not match:
        return None
    # The marker only counts inside a comment: either the line opens `//`
    # before it, or the scanner is inside a `/* */` block. A hit in code or
    # data is prose, not a map entry.
    prefix = line[:match.start()]
    if not in_block and "//" not in prefix and "/*" not in prefix:
        return None
    tail = match.group("tail") or ""
    tail_match = MARKER_TAIL.match(tail)
    if tail_match:
        name = tail_match.group("name")
        spans = _spans_of(tail_match.group("spans"))
        tail = tail_match.group("rest")
    else:
        name, spans = "", ()
    matched = bool(MARKER_MATCHED.search(tail))
    keyword_hit = MARKER_KEYWORD.match(MARKER_MATCHED.sub("", tail))
    keyword = keyword_hit.group("kw") if keyword_hit else None
    rest = keyword_hit.group("rest") if keyword_hit else ""
    return int(match.group("addr"), 16), keyword, rest, matched, name, spans


def _lessons(lines: list[str], index: int,
             ) -> tuple[tuple[tuple[str, str], ...], tuple[str, ...],
                        tuple[str, ...], tuple[str, ...]]:
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


def _subject_of(name: str) -> str | None:
    """The identifier a mangled name is built around, or None."""
    parts = _MANGLED_BASE.match(name)
    if parts is None:
        return None
    return next(group for group in parts.groups() if group)


def _region_end(lines: list[str], start: int, name: str = "") -> int | None:
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
    subject = _subject_of(name) if name else None
    defined = subject is None
    wanted = (None if subject is None
              else re.compile(rf"\b{re.escape(subject)}\b"))
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
        if wanted is not None and not defined and wanted.search(code):
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


def _extract_forward_text(lines: list[str], marker_line: int,
                          name: str = "") -> str:
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
    end = _region_end(lines, start, name)
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


def _state_of(region: str, excluded: str) -> State:
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
    `path`. No caching anywhere: a call reads what is on disk at the moment
    of the call. A caller that wants a different set of files reads them
    itself and keeps the results.
    """
    if path is not None:
        # The two-argument form is TEXT, and a `Path` here is a caller that
        # meant the one-argument form: `str(path)` would parse the filename
        # as if it were a source file and return nothing, which is the silent
        # empty answer this package is built to avoid.
        if not isinstance(source, str):
            raise TypeError(
                "read(text, path) takes the text to scan, not "
                f"{type(source).__name__}; read(path) reads a file")
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


def function_line(source: Path | str, name: str) -> int:
    """The 1-based line of the DEFINITION of `name`.

    `source` is a path (it is read) or the text itself; `name` is the C++
    spelling as it appears in code - `StringStruct::close`, `mem_get`,
    `BasePop::BasePop`. Matching happens on CODE ONLY, stripped of comments
    and string contents by the same walk the scanner uses, so a name never
    matches inside a comment, a fact block, or a string literal. A
    candidate is a definition only when its parameter list closes and a
    body opens before anything else - declarations and call sites are
    refused. Raises ValueError, listing the candidate lines, when zero or
    several definitions match.
    """
    if isinstance(source, Path):
        text = source.read_text(errors="ignore")
    else:
        as_path = Path(source)
        text = as_path.read_text(errors="ignore") if as_path.is_file() else source

    pattern = re.compile(rf"(?<![\w:]){re.escape(name)}\s*\(")
    lines = text.splitlines()
    definitions = []
    in_block = False
    for index, line in enumerate(lines):
        state = in_block
        code = _brace_delta(line, state)[3]
        in_block = _block_state_after(line, state)
        if pattern.search(code) and _is_definition(lines, index, state, pattern):
            definitions.append(index + 1)

    if len(definitions) != 1:
        raise ValueError(
            f"{name}: {len(definitions)} definitions found"
            f"{', lines ' + ', '.join(map(str, definitions)) if definitions else ''}")
    return definitions[0]


def _is_definition(lines: list[str], start: int, state: bool,
                   pattern: re.Pattern[str]) -> bool:
    """The `name(` on line `start` opens a parameter list that closes and
    is followed by `{` - a definition, not a declaration or a call.

    From the opening paren the balance is tracked over comment-stripped
    code; when it returns to zero, the next code must be the body's brace
    (an optional `const` in between is allowed). A `;` or anything else
    first is a declaration; a balance that closes INSIDE a larger
    expression - `if (name(x)) {` - is a call site.
    """
    code = _brace_delta(lines[start], state)[3]
    match = pattern.search(code)
    if match is None:
        return False
    stream, depth, closed = code[match.end():], 1, False
    index, line_state = start, state
    while True:
        if closed:
            # The parens are shut; the body must be the next code to come.
            rest = stream.lstrip()
            if rest.startswith("const"):
                rest = rest[5:].lstrip()
            if rest.startswith("{"):
                return True
            if rest:
                return False
        else:
            for pos, ch in enumerate(stream):
                if ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
                    if depth == 0:
                        rest = stream[pos + 1:].lstrip()
                        if rest.startswith("const"):
                            rest = rest[5:].lstrip()
                        if rest.startswith("{"):
                            return True
                        if rest:
                            return False
                        closed = True
                        break
        # The next line opens in the state the current line ends in.
        line_state = _block_state_after(lines[index], line_state)
        index += 1
        if index >= len(lines):
            return False
        stream = _brace_delta(lines[index], line_state)[3]


def _read_text(text: str, path: Path | str,
               ) -> list[DecompilationState]:
    """Scan TEXT attributed to `path`.

    Only EXPLICIT markers are read - the tree's single annotation grammar.
    """
    path = Path(path)
    lines = text.splitlines()
    proved = _proved_store(path)
    found = []

    in_block = False
    for index, line in enumerate(lines):
        # Parse with the state the line OPENS in, then advance the state, so
        # a marker on the line that opens a comment is still inside it.
        parsed = _parse_marker(line, in_block)
        in_block = _block_state_after(line, in_block)
        if parsed is None:
            continue
        address, keyword, rest, matched, name, spans = parsed
        found_levers, found_ruled, found_dead, found_later = \
            _lessons(lines, index)
        emitted = _symbol_fact(lines, index)
        if keyword == "FILE":
            region = text
            found.append(DecompilationState(
                address=address, mode=Mode.FILE,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, name=name, image_spans=spans, region=region,
                symbol=emitted, recipe=Recipe.VERBATIM,
                byte_exact=matched, levers=found_levers,
                ruled_out=found_ruled, unrecoverable=found_dead,
                deferred=found_later))
        elif keyword == "EXCLUDED":
            found.append(DecompilationState(
                address=address, mode=Mode.BODY, state=State.EXCLUDED,
                path=_abs(path), line=index + 1, name=name, image_spans=spans,
                symbol=emitted,
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
                    region = _extract_forward_text(lines, index + 1, name)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
            recipe = Recipe.WRITEBACK if proved else Recipe.CENSUS
            found.append(DecompilationState(
                address=address, mode=Mode.BODY,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, name=name, image_spans=spans, region=region,
                symbol=emitted, extract_error=error, recipe=recipe,
                byte_exact=matched,
                levers=found_levers, ruled_out=found_ruled,
                unrecoverable=found_dead, deferred=found_later))

    return found


def region_identifiers(record: DecompilationState) -> tuple[str, ...]:
    """Identifiers the record's own source region names, in source order.

    HOW THE `symbol` FACT GETS WRITTEN, and nothing else. The source does not
    always carry the original's name: `src/delegation_thunks.cpp` claims
    `?on_scrolling@BaseWin@@QAEXHH@Z` over a body it spells
    `base_win_on_scrolling_redirect`, because a redirect shim is a different
    symbol by design. A migration guesses the identifier from the definition
    head, compiles, and records the symbol the compiler ACTUALLY emitted.

    Nothing that measures calls this. Inference is fine for writing a fact
    down once; inside the loop that measures against the fact, a regex over
    definition heads would quietly pick a neighbouring function as bodies are
    edited.
    """
    seen: list[str] = []
    for match in DEFINITION_HEAD.finditer(record.region or ""):
        identifier = match.group("identifier")
        if identifier not in DEFINITION_KEYWORDS and identifier not in seen:
            seen.append(identifier)
    return tuple(seen)


def _symbol_fact(lines: list[str], index: int) -> str:
    """The `// symbol <name>` fact under the marker at `index`, or "".

    THE ONE FACT THE RECORD TAKES OUT OF THE BLOCK. `name` and `spans` ride
    the marker; the rest of the fact block is the catalogue's and passes
    through untouched. This one is here because the measurement needs it: it
    says what the COMPILER emits for the piece, which a redirect shim makes
    different from what the image calls it.

    Read from the comment run immediately after the marker, the same window
    the lessons come from, so a fact belonging to the NEXT annotation cannot
    be picked up.
    """
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        match = FACT_LINE.match(stripped)
        if match and match.group(1) == "symbol":
            value = (match.group(2) or "").strip()
            # A SYMBOL IS ONE TOKEN. Wrapped prose is re-flowed onto lines
            # that begin `// ` plus a word, and `symbol` is a word people
            # write - `src/` carries 2,037 lines that start `// symbol ` and
            # every one of them is a sentence. The key alone cannot tell
            # them apart; the value can, because a mangled name has no
            # spaces in it. This is the same lesson `FACT_LINE`'s own
            # comment records for `calls`, applied before it can bite.
            return "" if not value or " " in value else value
    return ""


def _spans_of(value: str) -> tuple[tuple[int, int], ...]:
    """(low, high) pairs out of a spans fact; the body's span is first."""
    spans = []
    for part in value.split(";"):
        low, _sep, high = part.partition("-")
        if low and high:
            spans.append((int(low, 16), int(high, 16)))
    return tuple(spans)


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
