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

    text = body if body is not None else stored_body(address)
    offset = catalogue_line(text)

    original = target.read_text()
    appended = original.rstrip("\n") + "\n\n" + text.strip() + "\n"
    relative = str(target.relative_to(REPO_ROOT))
    line = len(original.rstrip("\n").splitlines()) + 2 + offset - 1
    location = f"{relative}:{line}"

    writeback.write_atomically(target, appended)
    try:
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
        set_source_location(address, existing)
        raise

    stored = writeback.MATCHED_DIR / f"{address:08x}.cpp"
    if body is None and stored.is_file():
        stored.unlink()                    # the store was staging; it landed
    return {"address": f"0x{address:08X}", "name": row.get("name", ""),
            "source_location": location, "removed_from_store": body is None}


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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
