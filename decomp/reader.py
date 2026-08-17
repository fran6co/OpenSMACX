"""Reading entry points for the source map: file in, records out.

`read_file` reads one file, `read` reads in-memory text - the migrator's
dry-run surface - and `scan_tree` reads the whole tree, memoised on the
files themselves. The engine underneath - marker recognition, region
cutting, lesson tokens, precedence - lives in `annotation_scan`; this
module is its entry point, and nothing here decides what a marker MEANS,
only how the reading is arranged.
"""

from __future__ import annotations

import re
from pathlib import Path

from .annotation_scan import (_abs, _exclusion_citation, _parse_marker,
                              _state_of, extract_backward,
                              extract_forward_text, lessons)
from .annotation_scan import SRC_ROOT
from .grammar import LEGACY_BLOCK, LEGACY_OPENING, LEGACY_TRAILING
from .model import DecompilationState, Mode, State



def read_file(path: Path) -> list:
    """Every annotation one file declares, in line order."""
    path = Path(path)
    return read(path.read_text(), path)


def read(text: str, path) -> list:
    """Scan TEXT attributed to `path` - the migrator's dry-run surface.

    An explicit marker always wins. The legacy-store adapters fire only when a
    file under `src/recovered/` carries NO marker at all, so a migrated file
    is read by the new grammar even before the adapter is retired.
    """
    path = Path(path)
    lines = text.splitlines()
    kind = store_kind(path)
    found = []

    in_block = False
    for index, line in enumerate(lines):
        # Parse with the state the line OPENS in, then advance the state, so
        # a marker on the line that opens a comment is still inside it.
        parsed = _parse_marker(line, in_block)
        in_block = _block_state_after(line, in_block)
        if parsed is None:
            continue
        address, keyword, rest, matched = parsed
        found_levers, found_ruled, found_dead, found_later = \
            lessons(lines, index)
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
            if kind == "proved":
                # Proved bodies keep the writeback semantics even once an
                # explicit marker lands in the header: the body is the
                # definition after the leading `//` run, never a brace count
                # from the marker line (the header prose could carry braces).
                region, error = _proved_body(lines), ""
            else:
                try:
                    region = extract_forward_text(lines, index + 1)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
            recipe = "writeback" if kind == "proved" else "census"
            found.append(DecompilationState(
                address=address, mode=Mode.BODY,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, region=region, extract_error=error,
                recipe=recipe, byte_exact=matched,
                levers=found_levers, ruled_out=found_ruled,
                unrecoverable=found_dead, deferred=found_later))

    if not found:
        found.extend(_legacy_file_annotations(path, text, lines, kind))

    # A file may legitimately map several pieces (the thunk files do, once
    # migrated); but the SAME address twice in one file is a defect the
    # cross-reference reports, so nothing is dropped here.
    return found


def _block_state_after(line: str, in_block: bool) -> bool:
    """Cheap block-comment tracking: count the transitions on the line.

    Deliberately naive about strings containing comment marks - a false
    positive there only makes the scanner MORE willing to read a marker, and
    a marker at an address the catalogue does not know is reported as
    uncatalogued rather than believed, so the error cannot fabricate a map
    entry.
    THE SCAN IS BY `find`, NOT BY CHARACTER. This ran once per line of every
    file under `src/`, and every tool in the loop calls `scan_tree` at
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


def store_kind(path: Path) -> str:
    """Which legacy store a file sits in, by directory SHAPE, not repo path.

    "units" when the parent is `recovered/units`, "proved" when the parent is
    `recovered`, and "" anywhere else - so the mechanism works in a fixture
    tree as well as in the real one, and a store relocated during the
    migration keeps its semantics. The file must be named for an address.
    """
    resolved = Path(path).resolve()
    if not re.fullmatch(r"[0-9a-f]{8}", resolved.stem):
        return ""
    if resolved.parent.name == "units" \
            and resolved.parent.parent.name == "recovered":
        return "units"
    if resolved.parent.name == "recovered":
        return "proved"
    return ""


def _legacy_file_annotations(path: Path, text: str, lines: list,
                             kind: str) -> list:
    """Read-only recognition of the pre-migration stores and spellings."""
    found = []
    resolved = Path(path).resolve()

    # The two src/recovered/ stores, keyed by filename.
    if kind == "units":
        found.append(DecompilationState(
            address=int(resolved.stem, 16), mode=Mode.FILE,
            state=_state_of(text, ""), path=_abs(path), line=0, region=text, recipe="verbatim"))
        return found
    if kind == "proved":
        body = _proved_body(lines)
        found.append(DecompilationState(
            address=int(resolved.stem, 16), mode=Mode.BODY,
            state=_state_of(body, ""), path=_abs(path), line=0, region=body, recipe="writeback"))
        return found

    # The two deprecated inline spellings, anywhere under src/.
    in_block = False
    for index, line in enumerate(lines):
        block_now = _block_state_after(line, in_block)
        if in_block or line.lstrip().startswith("/*"):
            match = LEGACY_BLOCK.search(line)
            if match and "// ORIGINAL:" not in line:
                address = int(match.group("addr"), 16)
                try:
                    region = extract_forward_text(lines, index + 1)
                    error = ""
                except ValueError as problem:
                    region, error = "", str(problem)
                found.append(DecompilationState(
                    address=address, mode=Mode.BODY,
                    state=_state_of(region, ""), path=_abs(path),
                    line=index + 1, region=region,
                    extract_error=error, recipe="census"))
        trailing = LEGACY_TRAILING.search(line)
        if trailing:
            address = int(trailing.group("addr"), 16)
            try:
                region = extract_backward(lines, index)
                error = ""
            except ValueError as problem:
                region, error = "", str(problem)
            found.append(DecompilationState(
                address=address, mode=Mode.BODY,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, region=region,
                extract_error=error, recipe="census"))
            in_block = block_now
            continue
        opening = LEGACY_OPENING.search(line)
        if opening:
            address = int(opening.group("addr"), 16)
            try:
                region = extract_forward_text(lines, index + 1)
                error = ""
            except ValueError as problem:
                region, error = "", str(problem)
            found.append(DecompilationState(
                address=address, mode=Mode.BODY,
                state=_state_of(region, ""), path=_abs(path),
                line=index + 1, region=region,
                extract_error=error, recipe="census"))
        in_block = block_now
    return found


def _proved_body(lines: list) -> str:
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


# {root: (stamp, annotations)} for the process. Keyed on what the files ARE,
# not on having been asked before - see `scan_tree`.
_TREE_CACHE = {}


# BOTH SUFFIXES, and the stamp and the scan must agree on which. `.cpp` is
# what this tree is written in; `.c` arrived with `src/vendor/zlib-1.0.2/`,
# where the recovery for thirteen functions is upstream C that no C++ compiler
# will parse - zlib 1.0.2 uses K&R definitions. A scan that globbed only
# `.cpp` would report those annotations as missing while the files sat in the
# tree, and a STAMP that globbed only `.cpp` would be worse: the memo would
# never notice a `.c` landing changing.
def sources(root: Path) -> list:
    return sorted(path for suffix in ("*.cpp", "*.c")
                  for path in Path(root).rglob(suffix))


def tree_stamp(root: Path = SRC_ROOT) -> tuple:
    """(path, mtime, size) for every file `scan_tree` would read.

    Stat-ing 3,500 files costs about ten milliseconds; reading and parsing
    them costs two and a half seconds. That gap is the whole reason this
    exists, and the stamp is the same triple every build system trusts to
    decide whether a file changed.
    """
    out = []
    for path in sources(root):
        try:
            info = path.stat()
        except OSError:
            continue
        out.append((str(path), info.st_mtime_ns, info.st_size))
    return tuple(out)


def scan_tree(root: Path = SRC_ROOT) -> list:
    """Every annotation under `root`, deterministic order.

    MEMOISED ON THE FILES THEMSELVES, because the callers are layered and
    each one asks independently. `agent_brief` for a single address called
    this TEN times and `project_catalogue.from_source` nine, at 2.6 s a call -
    41 s for a brief whose own work is a few hundred milliseconds. The layers
    are right; asking again is not.

    Keyed on the stamp rather than on the argument, so a tool that WRITES a
    recovery and re-scans - which `writeback` does - sees its own edit. A
    plain `lru_cache` here would hand it the tree as it was before the write,
    and the verdict it published would describe code that no longer exists.
    """
    stamp = tree_stamp(root)
    hit = _TREE_CACHE.get(str(root))
    if hit is not None and hit[0] == stamp:
        return hit[1]
    found = []
    for path in sources(root):
        found.extend(read_file(path))
    found.sort(key=lambda ann: (ann.path, ann.line, ann.address))
    _TREE_CACHE[str(root)] = (stamp, found)
    return found
