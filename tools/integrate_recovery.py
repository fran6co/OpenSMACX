#!/usr/bin/env python3
"""Move a recovery out of the store and into a file the build compiles.

`src/recovered/` was always staging. Its README says so: the bodies there are
the emitter's verification style, they are not in `OPENSMACX_SOURCES`, and
"rewriting it in the tree's own style is a later phase". This is that phase.

WHAT INTEGRATION ACTUALLY REQUIRES. A body in the store compiles against
GENERATED SCAFFOLDING - opaque class shells, globals named `g_009b2068` after
their address, vtable shims invented on the spot. A body in `src/` compiles
against the project's real headers, where none of those names exist. And a
body in `src/` must ALSO still verify, because `byte_match_census.py` re-scores
every catalogued row through the scaffolding. So an integrated body has to
compile under BOTH, and the two vocabularies barely overlap.

The pattern that satisfies both, measured on `Console::on_sys_close`:

    int *const exit_turn_loop = reinterpret_cast<int *>(0x009B2068);
    *exit_turn_loop = 1;

A named local bound to the literal address. The project's headers do not have
to know the scaffolding's name for it and the scaffolding does not have to
know the project's, because neither name appears - and it stays byte-exact.
`0x009B2068` is `ExitTurnLoopAddress` in `src/scenario.cpp`, and saying so in
a comment beside the local is how the reader gets that back.

This tool does the MECHANICAL half: append a body to a chosen file, catalogue
where it landed, re-verify from disk, and revert completely if the verdict is
not BYTE_EXACT. Choosing the file, and rewriting the body into a shape that
compiles against real headers, stays a human decision - the same line
`mizuchi_writeback.py` draws, for the same reason.

    tools/integrate_recovery.py 0x0051D7C0 --into src/console.cpp
    tools/integrate_recovery.py 0x0051D7C0 --into src/console.cpp --body fixed.cpp
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as writeback  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent


class Refused(Exception):
    """Nothing was changed."""


def stored_body(address: int) -> str:
    path = writeback.MATCHED_DIR / f"{address:08x}.cpp"
    if not path.is_file():
        raise Refused(f"no stored body at {path.relative_to(REPO_ROOT)}")
    return writeback.read_matched_body(path)


def with_doc_comment(address: int, body: str) -> str:
    """The body, guaranteed to carry the doc comment the catalogue needs.

    A body harvested out of `build/byte-match/<address>/unit.cpp` has NO doc
    comment - the emitter's units never carried one; only hand-written
    candidates do. But `source_locations` has to name a line inside one, and
    `export_recovery_inventory` reads `Status: Complete` through it to set
    `recovery_state`. So the required lines are synthesised here.

    `Purpose:` is deliberately NOT synthesised. It is the one line that says
    what the function is for, nobody can derive it from an address, and a
    generated placeholder would read exactly like a real one.
    """
    if any(l.strip().startswith("Original Offset:") for l in body.splitlines()):
        return body
    return (f"/*\nOriginal Offset: {address:08X}\nStatus: Complete\n*/\n"
            + body.lstrip("\n"))


def catalogue_line(body: str) -> int:
    """Which line of the body `source_locations` should name.

    The convention is a line INSIDE the doc comment: `byte_match_census`
    re-opens the extract with `/*` when it sees a `*/` before the first brace,
    so the location must land after the comment opens and before it closes.
    `Original Offset:` is the one line every body's comment carries.
    """
    for index, line in enumerate(body.splitlines(), start=1):
        if line.strip().startswith("Original Offset:"):
            return index
    raise Refused("body has no `Original Offset:` line to catalogue")


DEFINITION = re.compile(
    r"^(?P<ret>[\w:<>*&\s]+?)\b(?P<klass>\w+)::(?P<method>~?\w+)\s*"
    r"\((?P<params>[^)]*)\)\s*(?P<trail>const\s*)?\{", re.M)


def class_span(text: str, klass: str):
    """(start, end) of this class's body in `text`, or None.

    EVERY placement question has to be scoped to this span, and both of them
    were wrong before it existed. Asking "is the method already declared"
    against the whole FILE answers yes because a sibling class in the same
    header also has a `destroy`; inserting at the first `public:` in the file
    puts the declaration INTO that sibling. One reports nothing to do, the
    other quietly does the wrong thing, and neither fails loudly.
    """
    opening = re.search(rf"\b(?:class|struct)\s+(?:DLLEXPORT\s+)?"
                        rf"{re.escape(klass)}\b\s*(?::[^{{;]*)?{{", text)
    if opening is None:
        return None
    end = text.find("\n};", opening.end())   # this tree closes at column zero
    return (opening.end(), end) if end > 0 else None


def header_declaring(klass: str) -> Path | None:
    """The header that declares this class, wherever it lives.

    NOT `src/<lowercased>.h`. That convention holds for most classes and fails
    for 61 of them - `StringList` is not in `stringlist.h`, `FactionAmbience`
    is not in `factionambience.h` - so a backfill keyed on the filename
    reported "no header" and declared nothing, while the compiler went on
    reporting the member as undeclared. Search for the declaration instead.

    `hypothesis_layouts.h` is skipped deliberately: it is GENERATED, and a
    declaration added there is erased by the next regeneration. A class that
    lives only there needs promoting into a header of its own first, which is
    a decision rather than a splice.
    """
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        if header.name == "hypothesis_layouts.h":
            continue
        if class_span(header.read_text(errors="ignore"), klass):
            return header
    return None


def declaration_for(body: str) -> tuple:
    """(header path, declaration line) a member definition needs, or (None, why).

    A DEFINITION WITHOUT A DECLARATION COMPILES NOWHERE. `void FX::stop() {}`
    with no `stop` in `fx.h` is not a VC6 quirk - the compiler cannot find a
    member to define, demotes it to a global function, and then every `this`
    inside cascades. VC6 reports it as `C2039` plus a pile of `C2673`, which is
    confusing enough that it reads like a dialect problem and is not.

    This is the second half of what the splice has to carry. The first
    integration pass moved 38 bodies and left 39 undeclared, because the
    verification unit gets the declaration from the CATALOGUE and so never
    notices the header is missing it.
    """
    match = DEFINITION.search(body)
    if match is None:
        return None, "no member definition found (a free function needs none)"
    klass, method = match.group("klass"), match.group("method")
    header = header_declaring(klass)
    if header is None:
        return None, (f"no header declares class {klass}, so "
                      f"{klass}::{method} has nowhere to be declared")
    text = header.read_text(errors="ignore")
    start, end = class_span(text, klass)
    if re.search(rf"\b{re.escape(method)}\s*\(", text[start:end]):
        return None, "already declared"
    returns = " ".join(match.group("ret").split())
    params = " ".join(match.group("params").split())
    trail = " const" if match.group("trail") else ""
    lead = f"{returns} " if returns and not method.startswith("~") else ""
    return header, (f"  {lead}{method}({params}){trail};", klass)


def declare_in_header(header: Path, declaration: str, klass: str) -> str:
    """Insert into THIS class's public section, and return the old text.

    The class has to be located first. An earlier version took the first
    `public:` in the FILE, which is only right when the header declares one
    class - and `StringList` shares `stringstruct.h` with `StringStruct`,
    `FactionAmbience` shares `ambience.h` with five siblings. Sixty-one
    declarations would have gone into the wrong classes, every one of them
    compiling perfectly and describing a member that class does not have.
    """
    text = header.read_text()
    span = class_span(text, klass)
    if span is None:
        raise Refused(f"{header.name} does not declare {klass}")
    start, end = span
    inside = re.search(r"^\s*public:\s*$", text[start:end], re.M)
    if inside is None:
        raise Refused(f"{klass} in {header.name} has no `public:` section")
    at = start + inside.end()
    writeback.write_atomically(header, text[:at] + "\n" + declaration + text[at:])
    return text


SCAFFOLDING_INCLUDES = (
    # name the body may use -> the header in `src/` that declares it
    ("VCall", "vtable_shim.h"),
)


def required_includes(body: str, target: Path) -> list:
    """Headers this body needs that the target does not already pull in.

    This exists because integration is the ONE step where verification cannot
    see the mistake. `verify()` compiles the body against the emitter's
    scaffolding, and the scaffolding declares `VCall` itself - so a body that
    lands in `src/` without the include verifies BYTE_EXACT and breaks the
    build at the same time. That is not hypothetical: eight compiled files
    referenced a `VCall` that did not exist after the first integration pass,
    and the ratchet stayed green through all of it.

    Matching on the bare identifier is deliberately loose. A false positive
    costs one unused `#include`; a false negative costs a broken build that
    nothing downstream will report.
    """
    text = target.read_text()
    wanted = []
    for name, header in SCAFFOLDING_INCLUDES:
        if not re.search(rf"\b{re.escape(name)}\b", body):
            continue
        if re.search(rf'^\s*#\s*include\s*"{re.escape(header)}"', text, re.M):
            continue
        wanted.append(header)
    return wanted


def add_includes(target: Path, headers: list) -> str:
    """Insert after the last existing `#include`, and return the old text."""
    text = target.read_text()
    last = None
    for match in re.finditer(r'^\s*#\s*include\s*[<"].*$', text, re.M):
        last = match
    if last is None:
        raise Refused(f"{target.name} has no #include to insert after")
    block = "".join(f'\n#include "{header}"' for header in headers)
    writeback.write_atomically(target, text[:last.end()] + block
                               + text[last.end():])
    return text


def set_source_location(address: int, location: str) -> None:
    """Point this row at its new home, in `functions.csv`."""
    import csv
    path = emit.FUNCTIONS_CSV
    with path.open(newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        fields, rows = reader.fieldnames, list(reader)
    wanted = f"0x{address:08X}"
    for row in rows:
        if row["address"].upper() == wanted.upper():
            row["source_locations"] = location
            break
    else:
        raise Refused(f"{wanted} is not in the catalogue")
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def integrate(address: int, target: Path, body: str | None = None) -> dict:
    # Accept either a repo-relative path or an absolute one; `source_locations`
    # is always recorded repo-relative.
    target = target if target.is_absolute() else (REPO_ROOT / target)
    if not target.is_file():
        raise Refused(f"{target} does not exist")
    functions = emit.load_functions()
    row = functions.get(address)
    if row is None:
        raise Refused(f"0x{address:08X} is not in the catalogue")
    existing = (row.get("source_locations") or "").strip()
    if existing:
        raise Refused(f"0x{address:08X} already lives at {existing}")

    text = with_doc_comment(address, body if body is not None
                            else stored_body(address))
    offset = catalogue_line(text)

    original = target.read_text()

    # Includes go in FIRST and are then treated as part of the file the body
    # is appended to. Adding them afterwards would insert lines ABOVE the
    # definition and silently move it, leaving `source_locations` pointing a
    # line or two short - and `census.extract_body` reads that line, so the
    # row would stop being scored against its own body.
    needed = required_includes(text, target)
    if needed:
        add_includes(target, needed)
    base = target.read_text()

    appended = base.rstrip("\n") + "\n\n" + text.strip() + "\n"
    relative = str(target.relative_to(REPO_ROOT))
    line = len(base.rstrip("\n").splitlines()) + 2 + offset - 1
    location = f"{relative}:{line}"

    header, declaration = declaration_for(text)
    header_before = None

    writeback.write_atomically(target, appended)
    try:
        if header is not None:
            header_before = declare_in_header(
                header, declaration[0], declaration[1])
        set_source_location(address, location)
        # Re-extract from DISK. What is proved is the text now in the file,
        # not the text this tool was handed - the same rule the writeback
        # follows, and the reason a bad splice cannot pass.
        extracted = census.extract_body(location)
        verdict = writeback.verify(address, extracted)
        if verdict.get("tier") != "BYTE_EXACT":
            raise Refused(
                f"after integration it verifies as {verdict.get('tier')}: "
                f"{verdict.get('note') or verdict.get('refusal_reason') or ''}")
    except Exception:
        writeback.write_atomically(target, original)
        if header_before is not None:
            writeback.write_atomically(header, header_before)
        set_source_location(address, existing)
        raise

    stored = writeback.MATCHED_DIR / f"{address:08x}.cpp"
    if body is None and stored.is_file():
        stored.unlink()                    # the store was staging; it landed
    return {"address": f"0x{address:08X}", "name": row.get("name", ""),
            "source_location": location, "removed_from_store": body is None,
            "declared_in": header.name if header is not None else "",
            "includes_added": needed}


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("address")
    parser.add_argument("--into", required=True, type=Path,
                        help="the src/ file that should own this function")
    parser.add_argument("--body", type=Path,
                        help="use this rewritten body instead of the stored "
                             "one; the store is then left alone")
    arguments = parser.parse_args(argv)

    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2

    body = arguments.body.read_text() if arguments.body else None
    try:
        result = integrate(address, arguments.into, body)
    except Refused as error:
        print(f"refused: {error}", file=sys.stderr)
        return 1
    print(f"integrated {result['name']} into {result['source_location']} "
          f"(verified BYTE_EXACT)")
    if result["removed_from_store"]:
        print(f"  removed src/recovered/{address:08x}.cpp")
    if result["declared_in"]:
        print(f"  declared it in src/{result['declared_in']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
