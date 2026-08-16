"""Read the catalogue's facts back out of the annotations `src/` carries.

THE READING HALF ONLY. `tools/project_catalogue.py` both STAMPS the fact block
onto an annotation - from `docs/recovery/functions.csv`, through
`emit_translation_unit` - and reads it back. Stamping is what pulls in the
export, pefile and the whole emitter, so it stays in `tools/` and only the
reader is here: `FACT_LINE`, `stamped`, and `from_source`. That is what keeps
this package standard-library-only.

A COPY, AND KNOWINGLY SO - see the same note in `annotation_scan`. `python -m
decomp` holds this parse against the one in `tools/` and fails if they diverge.

`docs/DECOMP_MAP.md` has said since it was written that "the decompilation is
tracked in exactly one place: the source tree… The CSVs under `docs/recovery/`
are derived views with a retirement path, not the record." That was true of the
MAP - which addresses exist and what state they are in - and false of everything
else about a function. Its name, its size, the spans it occupies and its
prototype lived only in `docs/recovery/functions.csv`, an IDA export, and every
tool that wanted them read the CSV.

Except for placeholders. All 1,863 of those already carry the facts, stamped by
`decomp_status.py --generate-placeholders`:

    // ORIGINAL: 0x00401100 FILE
    // name      ?add@StringStruct@@QAEHH@Z
    // size      1043 bytes
    // spans     0x00401100-0x00401513
    // prototype int (__thiscall ?add@StringStruct@@QAEHH@Z)(StringStruct*, int)
    // callers   21   call targets   3

Measured 2026-08-12: 1,863 of 3,478 annotated files carry that block, and it is
exactly the placeholder third. Product source, `src/recovered/` and
`src/recovered/units/` carry none of it. The projection was built for the
population that needed it and never finished, so the catalogue stayed load
bearing for the 4,108 pieces already recovered.

That projection is finished, and `tools/project_catalogue.py --check` holds it
to the export in both directions. Once every annotation carries its own facts, a
reader wanting the name of a function can ask `src/` - which is what this module
does - and the CSV becomes what the map already is, a derived view.

WHAT IS NOT PROJECTED, and why. `segment`, `comments` and `priority` have no
reader at all (measured: 0 tool references between them, and `priority` is empty
in every one of the 6,000 rows), so copying them into `src/` would carry dead
weight across. `body_ranges` IS projected, because a function's spans are how
the ratchet knows which bytes to compare and 402 functions carry more than one.
"""

from __future__ import annotations

from pathlib import Path

from . import annotation_scan
from .annotation_scan import REPO_ROOT
from .grammar import CONTINUABLE, CONTINUED, FACT_LINE


# {root: (stamp, rows)}, keyed the same way `annotation_scan.scan_tree` is.
_SOURCE_CACHE = {}


def from_source(src: Path = None) -> dict:
    """The catalogue, read back out of `src/` - the same shape `emit` returns.

    MEMOISED ON THE FILES, for the reason `scan_tree` gives at length: this is
    the layer every tool starts from, and one `agent_brief` invocation reached
    it nine times. The stamp is `annotation_scan.tree_stamp`, so a caller that
    writes a recovery and reads the catalogue back gets its own edit; nothing
    here is keyed on merely having been asked before.

    This is the direction that makes the export deletable. Every annotation
    carries its own facts and `--check` holds them to the export, so reading
    them back is not a second opinion: it is the same data with `src/` as the
    store instead of a CSV.
    """
    root = src or (REPO_ROOT / "src")
    stamp = annotation_scan.tree_stamp(root)
    hit = _SOURCE_CACHE.get(str(root))
    if hit is not None and hit[0] == stamp:
        return hit[1]
    rows = {}
    # `recovery_state` was never a fact ABOUT a function: the exporter derived
    # it from `src/` and wrote it back into the CSV. Deriving it HERE, once,
    # keeps every consumer agreeing rather than each re-deriving it - and it is
    # what the round trip through the export used to get wrong, leaving 651
    # addresses called `unrecovered` while `src/` held a proof for them.
    # RESOLVED, NOT RAW. 31 addresses are annotated in two places - a
    # `src/recovered/` writeback beside the `src/recovered/units/` unit that
    # RECORDS its measurement, which `annotation_scan._precedence` settles
    # deliberately, and a handful commented twice in one file. Walking the
    # unresolved list let whichever came last win, so this row's
    # `source_locations` named one file while `cross_reference` - which does
    # resolve - named the other, and the gate reported 19 "stale
    # source_locations" every run against a tree where nothing was stale.
    #
    # Two resolvers giving two answers to one question is the shape this tree
    # keeps finding defects in; there is one resolver and it lives in
    # `annotation_scan`.
    for annotation in annotation_scan.resolve(annotation_scan.scan_tree(root))[0]:
        path = REPO_ROOT / annotation.path
        try:
            lines = path.read_text(errors="ignore").splitlines()
        except OSError:
            continue
        if not annotation.line:
            continue
        present = stamped(lines, annotation.line - 1)
        if not present:
            continue
        size = present.get("size", "").replace(" bytes", "").strip()
        calls = present.get("calls", "")
        rows[annotation.address] = {
            "address": f"0x{annotation.address:08X}",
            "name": present.get("name", ""),
            "size": size,
            "body_ranges": present.get("spans", ""),
            "prototype": present.get("prototype", ""),
            "binary_kind": present.get("kind", ""),
            "flags": present.get("flags", ""),
            "notes": present.get("notes", ""),
            "caller_count": present.get("callers", "").split()[0]
                            if present.get("callers") else "0",
            "call_target_count": present.get("callers", "").split()[-1]
                                 if present.get("callers") else "0",
            "_calls": {int(part, 16) for part in calls.split()
                       if part.startswith("0x")},
            # THE RECORDED SPELLING, kept beside the parsed set because the
            # line carries more than addresses: `0x005CEB12 (16x)` says
            # sixteen sites reach one target, and a repair that rebuilds the
            # line from the set alone silently drops what an agent measured.
            "_calls_text": calls,
            "_indirect": {int(part, 16) for part in
                          present.get("indirect", "").split()
                          if part.startswith("0x")},
            # THE FIRST SPAN'S END, not the last. 402 functions carry a second
            # span, and MSVC outlined those cold blocks to 0x0065xxxx - so
            # taking the last one puts `end_address` in the funclet range and
            # `byte_match` reads the wrong bytes. Measured: 416 rows differed
            # that way and 241 BYTE_EXACT claims stopped reproducing, which is
            # how this was found rather than reasoned.
            "end_address": (present.get("spans", "").split(";")[0].split("-")[-1]
                            if "-" in present.get("spans", "") else ""),
            # BY CITATION, not by "is it excluded". S1 is the MSVC 6 CRT, which
            # is what `external_library` means; S2a is the C++ EH unwind
            # funclets, which are game code the project declines to express.
            # Collapsing them moved 15,970 bytes out of `eh_funclets` and into
            # `external_library` and failed `exclusions-current` - the two
            # populations are measured separately and must stay separate.
            "recovery_state": (
                "external_library"
                if annotation.state == annotation_scan.STATE_EXCLUDED
                and (annotation.exclusion or "").upper().startswith("S1") else
                "unrecovered"
                if annotation.state == annotation_scan.STATE_EXCLUDED else
                "source_complete"
                if annotation.state == annotation_scan.STATE_IMPLEMENTED else
                "unrecovered"),
            "source_locations": annotation.location if
                annotation.state == annotation_scan.STATE_IMPLEMENTED else "",
            "segment": ".text",
            "original_dependencies": "",
            "redirect_exports": "",
            "source_statuses": "",
            "priority": "",
            "comments": "",
        }
    _SOURCE_CACHE[str(root)] = (stamp, rows)
    return rows


def stamped(lines: list, index: int) -> dict:
    """{key: value} already recorded in the comment run after a marker.

    THE FIRST SPELLING OF A KEY WINS. The block is written once, by `facts`,
    and anything further down the comment run is prose that happened to parse
    - so a second match is never a correction, and letting it overwrite is how
    a RULED-OUT sentence became a function's call-edge list.
    """
    found = {}
    open_key, column = None, None
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        match = FACT_LINE.match(line)
        if match:
            key, value = match.group(1), match.group(2) or ""
            fresh = key not in found
            found.setdefault(key, value)
            open_key = key if fresh and key in CONTINUABLE and value else None
            column = line.index(value) if open_key else None
            continue
        carried = CONTINUED.match(line)
        if open_key and carried and len(carried.group(1)) + 2 == column:
            found[open_key] = f"{found[open_key]} {carried.group(2).strip()}"
            continue
        open_key, column = None, None
    return found
