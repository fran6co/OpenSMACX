#!/usr/bin/env python3
"""The command line over `decomp` - what state the source map is in.

    uv run tools/osmx.py status

WHY THIS IS A TOOL AND NOT PART OF THE PACKAGE. `decomp` names no location,
no compiler and no flag set, on purpose: it is a library and those are facts
about a checkout. Something has to know them, and this is that something.
Every path below is an option with an environment-variable default, so the
knowledge lives in one file that a person can read instead of being spread
through the code that uses it.
"""

from __future__ import annotations

import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import sys
import subprocess
import statistics
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from dataclasses import replace

from decomp import DecompilationState, State, mangled, read, write_file
from decomp.mangled import qualified_name
from decomp.calls import CallSite, call_sites, imported_names
from decomp.record import stamped
from decomp.asm import (AsmComparison, CompileFailed, build_command,
                        build_inputs, compare_record, compare_source,
                        compare_subject, compile_unit, flag_dependent,
                        original_asm, shared_spans, span_refusal,
                        subject_asm)

REPO_ROOT = Path(__file__).resolve().parent.parent

# THE CLAIM FLOOR IS A FILE, NOT A RECOUNT. `check` recomputes the claim
# count from the tree on every run, so "the floor is the number of claims"
# only held against honest edits: delete a marker - or typo it, `ORIGINAl:`
# reads as prose - and the claim did not fail, it CEASED TO EXIST, and the
# recount just followed it down (measured 2026-08-23: both corruptions read
# as 0 records, silently). This file is what the recount is compared TO.
# Only `osmx record` and `osmx semantic` rewrite it, in the same breath as
# the annotations; the gate fails on ANY mismatch, in either direction - a
# live count above the floor is a BYTE_EXACT token something other than
# `record` wrote. Collected agent patches can carry their worktree's floor
# line; the coordinator's re-measure (`osmx record`, batch step 4) is what
# reconciles it, exactly as it reconciles the markers themselves.
#
# THE FLOOR LIVES IN THE TREE IT COUNTS. The first version pinned the path
# from __file__ while counting the `--src` it was handed - the mixed
# location mechanism the 2026-08-24 audit named as a hazard, rebuilt hours
# later by its own author. It bit the same day: a record run against a
# small src wrote `claims 3` into MAIN's floor while main held 3,018.
# Deriving the path from `src` makes counting and writing the same tree by
# construction, and gives every worktree its own self-consistent floor.


def _floor_path(src: Path) -> Path:
    return Path(src) / "CLAIM_FLOOR"


def _read_floor(src: Path) -> int | None:
    """The persisted claim count, or None when the file is missing/garbled."""
    try:
        return int(_floor_path(src).read_text().split()[-1])
    except (OSError, ValueError, IndexError):
        return None


def _refresh_floor(src: Path) -> int:
    """Recount the tree and persist BESIDE IT. Called ONLY by
    record/semantic, which have just rewritten annotations - the gate itself
    never writes."""
    live = sum(1 for r in read(src) if r.byte_exact or r.semantic)
    _floor_path(src).write_text(
        "# The claim floor. Maintained by `osmx record` / `osmx semantic`;\n"
        "# `osmx check` fails when the tree's live claim count differs in\n"
        "# EITHER direction. Never edit by hand - bank or demote through\n"
        "# `osmx record`, which rewrites this file with what it measured.\n"
        f"claims {live}\n")
    return live

# More matches than this and a listing is more use than a dump.
SHOW_IN_FULL = 4

app = typer.Typer(add_completion=False, no_args_is_help=True,
                  help="Drive the OpenSMACX source map.")


@app.callback()
def main() -> None:
    """Read and measure the `ORIGINAL: 0x...` map in `src/`.

    The callback is what keeps this a GROUP. Typer folds a one-command app
    into its root, so `osmx status` would be `osmx` until the second command
    landed and every invocation written down would break at that moment.
    """


class Population(StrEnum):
    """What kind of work a record represents.

    `State` says what condition a region is IN; this says what should be
    DONE about it, which is a different question and the one a person asks.
    The difference is the ratchet claim: an implemented body that reproduces
    the shipped bytes is finished, and one that does not is the work.
    """
    NO_BODY = "NO_BODY"            # a placeholder; nothing written yet
    NOT_MATCHING = "NOT_MATCHING"  # a body exists and does not reproduce
    CLAIMED = "CLAIMED"            # carries BYTE_EXACT
    EXCLUDED = "EXCLUDED"          # a decision, not a gap


def population(record: DecompilationState) -> Population:
    if record.state is State.EXCLUDED:
        return Population.EXCLUDED
    if record.byte_exact:
        return Population.CLAIMED
    if record.state is State.PLACEHOLDER:
        return Population.NO_BODY
    return Population.NOT_MATCHING


def _build_inputs(compile_commands: Path) -> set[Path]:
    """The source files the build names, or an empty set if it names none.

    Empty is not the same as "none reachable": a checkout with no configured
    build simply cannot say, and `status` reports the column as unknown
    rather than as zero.
    """
    if not compile_commands.is_file():
        return set()
    table = json.loads(compile_commands.read_text())
    found = set()
    for entry in table:
        file = Path(entry["file"])
        if not file.is_absolute():
            file = Path(entry.get("directory", ".")) / file
        found.add(file.resolve())
    return found


@app.command()
def status(
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC",
        help="The annotated tree to read.")] = REPO_ROOT / "src",
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
        help="The build database, for which records are reachable.",
    )] = REPO_ROOT / "build" / "compile_commands.json",
    as_json: Annotated[bool, typer.Option(
        "--json", help="Machine-readable output.")] = False,
) -> None:
    """What state the source map is in.

    A REPORT, NOT A GATE: it always exits 0. `check` is the one that fails.

    Reads the tree and nothing else - no compiler, no image - so it answers
    on a checkout where neither is installed.
    """
    records = read(src)
    if not records:
        typer.secho(f"no annotations under {src}", fg=typer.colors.RED)
        raise typer.Exit(1)

    inputs = _build_inputs(compile_commands)
    rows = {}
    for name in Population:
        mine = [r for r in records if population(r) is name]
        sizes = sorted(r.size for r in mine) or [0]
        rows[str(name)] = {
            "count": len(mine),
            "reachable": (sum(1 for r in mine if r.path.resolve() in inputs)
                          if inputs else None),
            "median_bytes": int(statistics.median(sizes)),
            "under_128_bytes": sum(1 for s in sizes if s < 128),
        }

    if as_json:
        typer.echo(json.dumps({
            "records": len(records),
            "files": len({r.path for r in records}),
            "build_inputs": len(inputs),
            "populations": rows,
        }, indent=2))
        return

    files = len({r.path for r in records})
    typer.echo(f"{len(records):,} annotations across {files:,} files")
    if inputs:
        typer.echo(f"{len(inputs):,} of those files are build inputs; "
                   f"'reachable' counts records the build can compile today")
    else:
        typer.secho(f"no build database at {compile_commands} - "
                    f"'reachable' is unknown, not zero",
                    fg=typer.colors.YELLOW)
    typer.echo("")
    head = f"{'population':14}{'count':>8}{'reachable':>11}{'median B':>10}{'<128 B':>9}"
    typer.echo(head)
    typer.echo("-" * len(head))
    for name, row in rows.items():
        reach = "?" if row["reachable"] is None else f"{row['reachable']:,}"
        typer.echo(f"{name:14}{row['count']:>8,}{reach:>11}"
                   f"{row['median_bytes']:>10,}{row['under_128_bytes']:>9,}")




def _looks_like_an_address(text: str) -> bool:
    """Is `text` an address rather than a name?

    `0x...` always is. Otherwise it takes six hex digits or more, because
    the short ones are ambiguous and silently winning that argument is how
    someone asking about a function called `add` or `face` gets handed the
    bytes at 0x000ADD instead. Every address in this image is six to eight
    digits, so nothing real is lost.
    """
    lowered = text.lower()
    if lowered.startswith("0x"):
        return True
    return len(text) >= 6 and all(c in "0123456789abcdef" for c in lowered)


def _in_file(claimants: list, fragment: str) -> list:
    """The subset of `claimants` whose file path contains `fragment`.

    THIRTY-SIX ADDRESSES ARE ANNOTATED TWICE - the body in `src/` and the
    same piece preserved in `src/recovered/` or `src/unrecovered/` - and
    they are two claims measured in two translation units. Without a way to
    name one, `measure` and `record` could not be pointed at either of
    them; they refused with "2 pieces match" and there was no answer.
    """
    if not fragment:
        return claimants
    matched = [r for r in claimants if fragment in str(r.path)]
    # MOST SPECIFIC WINS. A plain substring makes `--in src/` match both
    # `src/text.cpp` and `src/unrecovered/00608c00.cpp`, so the fragment that
    # names one file exactly still refused with "2 pieces match". A fragment
    # that ends a path is the caller naming a FILE; one that appears in the
    # middle is naming a directory, and the first reading wins where both
    # apply.
    exact = [r for r in matched if str(r.path).endswith(fragment)]
    return exact or matched


def _matching(records: list, target: str) -> list:
    """Every record `target` names, by address or by name.

    THREE SPELLINGS, because a mangled name is not what anyone types.
    `WinMain` is `_WinMain@16` in this map, `set_font` is
    `?set_font@Buffer@@QAEHPAVFont@@000@Z`, and someone who knows the class
    writes `Buffer::set_font`. All three find it; an exact match on the
    whole mangled name or on the emitted symbol wins outright.
    """
    if _looks_like_an_address(target):
        wanted = int(target, 16)
        return [r for r in records if r.address == wanted]

    exact = [r for r in records if target and target in (r.name, r.symbol)]
    if exact:
        return exact

    if "::" in target:
        owner, _sep, member = target.rpartition("::")
        # A CONSTRUCTOR AND A DESTRUCTOR ARE NOT SPELLED LIKE A METHOD.
        # `?member@Owner@@` is the shape for a named member; `Text::Text` and
        # `Text::~Text` mangle to `??0Text@@` and `??1Text@@`, with no member
        # name at all. Without this, the two spellings a person is most
        # likely to type for the 199 header-defined pieces found nothing.
        if member == owner:
            scoped = f"??0{owner}@@"
        elif member == f"~{owner}":
            scoped = f"??1{owner}@@"
        else:
            scoped = f"?{member}@{owner}@@"
        return [r for r in records if r.name.startswith(scoped)
                or (r.symbol and r.symbol.startswith(scoped))]

    return [r for r in records
            if target in (mangled.identifier(r.name),
                          mangled.identifier(r.symbol))]


def _ambiguous(claimants: list, target: str, command: str,
               in_file: str) -> None:
    """Say why one target names several pieces, and how to say which.

    THE ADVICE HAS TO FIT THE CASE. `--in` disambiguates a piece annotated
    in two FILES - the body in `src/` and the copy preserved beside it - and
    it cannot separate two overloads that share one. `prefs_get` is two
    functions on lines 1326 and 1439 of `src/alpha.cpp`, so telling someone
    to pass `--in` there sends them somewhere with no answer in it. Both are
    reachable by address, and by the mangled name that spells the arguments
    out, so those are what get offered - printed in full, ready to copy.
    """
    files = {piece.path for piece in claimants}
    advice = (" - --in <path fragment> picks one"
              if len(files) > 1 and not in_file else
              " - name one of these exactly, or its address")
    typer.secho(f"{len(claimants)} pieces match {target!r}; {command} "
                f"names one{advice}", fg=typer.colors.RED)
    for piece in sorted(claimants, key=lambda r: (str(r.path), r.address)):
        typer.echo(f"  {piece.address_hex}  {piece.name}")
        typer.echo(f"  {'':10}  {piece.location}")
    raise typer.Exit(2)


def _lessons(record: DecompilationState) -> list[tuple[str, str]]:
    out = [("LEVER", f"{key}  {prose}") for key, prose in record.levers]
    out += [("TRIED", p) for p in record.ruled_out]
    out += [("TRIED", p) for p in record.unrecoverable]
    out += [("DEFERRED", p) for p in record.deferred]
    return out


@app.command()
def show(
    target: Annotated[str, typer.Argument(
        help="An address in hex, or a name: WinMain, set_font, "
             "Buffer::set_font.")],
    in_file: Annotated[str, typer.Option(
        "--in",
        help="Disambiguate by file when one address is annotated twice, "
             "e.g. --in src/text.cpp or --in unrecovered.")] = "",
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
        help="The pinned original executable.",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    as_json: Annotated[bool, typer.Option(
        "--json", help="Machine-readable output.")] = False,
) -> None:
    """Everything the map and the image say about one piece.

    What the annotation claims, what state it is in, and the shipped bytes
    it claims - which is the first thing anyone recovering a function looks
    at. No compiler: `measure` is the one that asks whether a body
    reproduces this.
    """
    records = read(src)
    claimants = _in_file(_matching(records, target), in_file)
    if not claimants:
        typer.secho(f"nothing under {src} matches {target!r}",
                    fg=typer.colors.RED)
        raise typer.Exit(1)

    # SEVERAL MATCHES IS A REAL CONDITION, not a corner case. 34 addresses in
    # this tree are annotated twice - usually a body in `src/` and the same
    # piece in the proved store - and a name like `init` names dozens of
    # methods. Choosing one silently would answer a question nobody asked.
    if len(claimants) > SHOW_IN_FULL:
        typer.secho(f"{len(claimants)} pieces match {target!r}; "
                    f"naming one of these exactly, or its address, shows it",
                    fg=typer.colors.YELLOW)
        for record in sorted(claimants, key=lambda r: r.address):
            typer.echo(f"  {record.address_hex}  {population(record):13} "
                       f"{record.name}")
        raise typer.Exit(1)
    if len(claimants) > 1 and not as_json:
        typer.secho(f"{len(claimants)} annotations match {target!r}; "
                    f"all are shown", fg=typer.colors.YELLOW)

    payload = []
    for record in claimants:
        entry = {
            "address": record.address_hex,
            "name": record.name,
            "symbol": record.symbol,
            "kind": record.kind,
            "body": record.body,
            "location": str(record.location),
            "mode": str(record.mode),
            "recipe": str(record.recipe),
            "population": str(population(record)),
            "spans": [[f"0x{lo:08X}", f"0x{hi:08X}"]
                      for lo, hi in record.image_spans],
            "size": record.size,
            "lessons": [[kind, text] for kind, text in _lessons(record)],
            "extract_error": record.extract_error,
        }
        entry["refusal"], entry["disassembly"] = _image_side(record, records,
                                                            exe)
        payload.append(entry)

    if as_json:
        typer.echo(json.dumps(payload, indent=2))
        return
    for entry in payload:
        _print_record(entry)


def _image_side(record: DecompilationState, records: list,
                exe: Path) -> tuple[str, list[str]]:
    """(refusal, lines) for a record, or ("", []) with no image to read."""
    if not exe.is_file():
        return "", []
    refusal = span_refusal(record, exe, shared_spans(records))
    if refusal is not None:
        return str(refusal), []
    try:
        return "", list(original_asm(record, exe).lines)
    except ValueError as problem:
        return "", [f"({problem})"]


def _print_record(entry: dict) -> None:
    typer.echo("")
    typer.secho(f"{entry['address']}  {entry['name']}", bold=True)
    typer.echo(f"  {entry['location']}")
    typer.echo(f"  {entry['mode']} / {entry['recipe']} / "
               f"{entry['population']}")
    spans = "  ".join(f"{lo}-{hi}" for lo, hi in entry["spans"])
    typer.echo(f"  spans      {spans}   ({entry['size']} bytes)")
    if len(entry["spans"]) > 1:
        typer.echo(f"  {'':10} the first is the body; the rest is cold code "
                   f"the image lays elsewhere")
    if entry["symbol"]:
        typer.echo(f"  symbol     {entry['symbol']}")
    if entry["kind"]:
        typer.echo(f"  kind       {entry['kind']}")
    if entry["body"]:
        # THE WHOLE POINT OF THE FACT. Without it a marker with no definition
        # under it reports "no closing brace within the file", which is true
        # and useless: the body is in a header, on purpose, and the reader
        # wants to be told where rather than told the file looks broken.
        typer.echo(f"  body       {entry['body']}   (defined in-class; the "
                   f"marker is here because a header cannot be compiled)")
    elif entry["extract_error"]:
        typer.secho(f"  no region  {entry['extract_error']}",
                    fg=typer.colors.YELLOW)
    for kind, text in entry["lessons"]:
        typer.echo(f"  {kind:9}  {text}")
    typer.echo("")
    if entry["refusal"]:
        typer.secho(f"  {entry['refusal']} - no verdict is defined for this "
                    f"span, so there is nothing to compare",
                    fg=typer.colors.YELLOW)
    elif entry["disassembly"]:
        for line in entry["disassembly"]:
            typer.echo(f"  {line}")
    else:
        typer.secho("  (no image to read; pass --exe)",
                    fg=typer.colors.YELLOW)


# The invocations this image was built with, crossed: `/O2` implies `/Oy`,
# which omits the frame pointer, and the shipped image is MIXED - 1,518
# functions open with `push ebp; mov ebp, esp` and 1,544 do not. Asking one
# answers about half the image. They live here and not in `decomp` because
# they are a fact about this build, and they are not in the CMake
# configuration either: that is a Debug database of `/Od /Ob0`.
FLAG_SETS = (
    "/c /O2 /Gy /GR- /Oy- /GX",
    "/c /O2 /Gy /GR- /GX",
    "/c /O1 /Gy /GR- /Oy- /GX",
    "/c /O1 /Gy /GR- /GX",
    # AND INLINE EXPANSION IS THE THIRD AXIS, for the same reason as the
    # frame pointer: it is a property of the FUNCTION, and this image is
    # mixed. `??1TextIndex` tail-calls its member's destructor where /O2
    # inlines it - 4 instructions against 21 - and `??1Text` needs the
    # opposite, because `??__ETxt` only reproduces with the constructor
    # folded in. Neither answer is right for the whole image, which is
    # exactly why the caller searches instead of choosing.
    "/c /O2 /Ob0 /Gy /GR- /Oy- /GX",
    "/c /O2 /Ob0 /Gy /GR- /GX",
    "/c /O1 /Ob0 /Gy /GR- /Oy- /GX",
    "/c /O1 /Ob0 /Gy /GR- /GX",
    # AND INTRINSICS ARE THE FOURTH, which a previous measurement retired
    # for the wrong reason: `/O2 /Oi-` reproduces the same bodies as `/O2`
    # for bodies that never call the CRT's block functions, and that was most
    # of the sample. It is decisive for the ones that do. `Buffer::init`
    # measures 121 of 286 instructions with intrinsics on - VC6 expands its
    # `memset` to `rep stosd` - and 280 of 286 with them off, because the
    # image CALLS memset at 0x006465F0. The alternative was
    # `#pragma function(memset)` in the source, which is a compiler switch
    # written into the program.
    "/c /O2 /Oi- /Gy /GR- /Oy- /GX",
    "/c /O2 /Oi- /Gy /GR- /GX",
    # AND EXCEPTION HANDLING IS THE FIFTH, found the same way /Oi- was: the
    # axis was retired on a sample that could not show it. Every set above
    # carried /GX, so no body was ever measured without it - and /GX is what
    # makes a constructor whose BASE has a destructor open with an EH frame
    # (`mov eax, 0; call __EH_prolog`). ??0Win@@QAE@XZ constructing its
    # AutoSound base is 0 of 107 with /GX and 12 of 107 without, because the
    # image's constructor has no EH frame at all. So each set above gets its
    # no-/GX twin rather than a flag being chosen for the whole image.
    "/c /O2 /Gy /GR- /Oy-",
    "/c /O2 /Gy /GR-",
    "/c /O1 /Gy /GR- /Oy-",
    "/c /O1 /Gy /GR-",
    "/c /O2 /Ob0 /Gy /GR- /Oy-",
    "/c /O2 /Ob0 /Gy /GR-",
    "/c /O1 /Ob0 /Gy /GR- /Oy-",
    "/c /O1 /Ob0 /Gy /GR-",
    "/c /O2 /Oi- /Gy /GR- /Oy-",
    "/c /O2 /Oi- /Gy /GR-",

)


# THE CEILING IS THE WINE PREFIX, not the machine. Every VC6 compile runs
# against the one prefix at ~/opt/vc6/.wineprefix, whose wineserver
# serialises, and eight concurrent `CL` is where this tree measured the
# knee. More workers than that queue on the server and win nothing; I also
# proved the point by accident, running the test suite beside a sweep and
# getting a compile failure that vanished when they were not competing.
WINE_CEILING = 8


@app.command()
def measure(
    target: Annotated[str, typer.Argument(
        help="An address in hex, or a name.")],
    in_file: Annotated[str, typer.Option(
        "--in",
        help="Disambiguate by file when one address is annotated twice, "
             "e.g. --in src/text.cpp or --in unrecovered.")] = "",
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
    )] = REPO_ROOT / "build" / "compile_commands.json",
    borrow: Annotated[str, typer.Option(
        help="Compile a file the build does not name with THIS file's "
             "command. Needed for src/recovered and src/unrecovered.",
    )] = "src/buffer.cpp",
    body: Annotated[Path | None, typer.Option(
        "--body",
        help="Score THIS file's spelling of the piece instead of the "
             "tree's. The tree is not touched.")] = None,
    directory: Annotated[Path | None, typer.Option(
        "--dir",
        help="Score every *.cpp in this directory and rank them, best "
             "first. The tree's own body is scored alongside as the "
             "baseline.")] = None,
    jobs: Annotated[int, typer.Option(
        help=f"Concurrent compiles for --dir. Capped at {WINE_CEILING}: "
             f"one shared wine prefix.")] = 0,
    reconfigure: Annotated[bool, typer.Option(
        "--reconfigure",
        help="Clear the cmake cache and regenerate the build database "
             "first, whether or not it looks stale.")] = False,
    every_flag_set: Annotated[bool, typer.Option(
        "--all-flags",
        help="Score EVERY flag set instead of reporting the best. A source "
             "change can improve one set while the winner comes from "
             "another; this is where that shows.")] = False,
    as_json: Annotated[bool, typer.Option("--json")] = False,
) -> None:
    """Does this body reproduce the shipped bytes?

    EXITS 0 ONLY FOR BYTE_EXACT. That is the contract every other tier is
    measured against, and it is what lets this be a loop condition:

        until osmx measure 0x0045F950; do  ...edit...  done

    It does not write anything. `record` is the one that stamps a claim,
    and keeping them apart is what lets an agent be given this and not that.

    `--body` AND `--dir` SCORE A SPELLING THAT IS NOT IN THE TREE, which is
    what makes this a loop rather than a report. A match is found by
    searching over source form, and the compiler is the only thing that can
    say which form is right; without a candidate to hand it, the only way to
    pose the question is to edit `src/`, measure, and edit back - so a
    failed experiment leaves a dirty checkout, one spelling can be in flight
    at a time, and an agent that may not write cannot ask at all.

        osmx measure 0x004E0F80 --dir /tmp/variants

    ranks a whole directory in one run against an unchanged tree. Each
    candidate is a translation unit that DEFINES the piece under the name
    the annotation records; it is compiled with the record's own invocation,
    so it sees the include path the real unit sees.
    """
    if body and directory:
        typer.secho("--body names one candidate and --dir a directory of "
                    "them; pass one", fg=typer.colors.RED)
        raise typer.Exit(2)
    _fresh_compile_commands(compile_commands, reconfigure)
    records = read(src)
    claimants = _in_file(_matching(records, target), in_file)
    if len(claimants) != 1:
        _ambiguous(claimants, target, "measure", in_file)
    record = claimants[0]
    command = _command_for(compile_commands, record.path, borrow)
    shared = shared_spans(records)

    if directory is not None:
        _rank_candidates(record, exe, directory, command, shared, jobs,
                         as_json)
        return

    source = body if body is not None else record.path
    if every_flag_set:
        # EVERY SET, NOT THE WINNER. `compare_source` ranks on similarity
        # first and agreement second, across flag sets that answer different
        # questions - so a source change can improve the set where the image's
        # helpers are inlined while the winner comes from the set where they
        # are not, and the verdict does not move. That reads as "nothing
        # happened" when something did.
        for flags in FLAG_SETS:
            try:
                one = compare_source(record, exe, source, command, (flags,),
                                     shared)
            except (ValueError, CompileFailed) as problem:
                typer.echo(f"  {flags:34s} {str(problem).splitlines()[0][:60]}")
                continue
            typer.echo(f"  {flags:34s} {str(one.verdict):14s} "
                       f"{one.matching_instructions}/"
                       f"{one.original_instructions} instructions, "
                       f"{one.mnemonic_similarity:.3f} similar")
        raise typer.Exit(0)
    try:
        result = compare_source(record, exe, source, command, FLAG_SETS,
                                shared)
    except (ValueError, CompileFailed) as problem:
        typer.secho(f"{record.address_hex}: {problem}", fg=typer.colors.RED)
        raise typer.Exit(2) from None

    if as_json:
        typer.echo(json.dumps(
            {"source": str(source), **_verdict_row(record, result)}, indent=2))
    else:
        _print_verdict(record, result, source)
    raise typer.Exit(0 if str(result.verdict) == "BYTE_EXACT" else 1)


def _command_for(compile_commands: Path, source: Path,
                 borrow: str) -> list[str]:
    """The build's invocation for `source`, or a borrowed one.

    A BUILD ENTRY IS A COMMAND, NOT A PERMISSION. `src/recovered/` and
    `src/unrecovered/` hold translation units CMake has no reason to build,
    and all they are missing is an invocation - but WHICH one is a judgement
    about include paths, which is why `decomp` refuses to guess and this
    borrows deliberately.
    """
    try:
        return build_command(compile_commands, source)
    except ValueError:
        return build_command(compile_commands, REPO_ROOT / borrow)


def _verdict_row(record: DecompilationState, result: AsmComparison) -> dict:
    return {
        "address": record.address_hex,
        "name": record.name,
        "verdict": str(result.verdict),
        "flags": result.flags,
        "instructions": [result.original_instructions,
                         result.compiled_instructions],
        "matching": result.matching_instructions,
        "similarity": round(result.mnemonic_similarity, 4),
        "first_divergence": result.first_divergence,
        "compared_bytes": result.compared_bytes,
        "masked_bytes": result.masked_bytes,
        "differing_constants": [list(c) for c in result.differing_constants],
        "data_bytes": result.data_bytes,
        "data_divergence": result.data_divergence,
        "diagnostic": result.diagnostic,
    }


def _score_candidate(job: tuple) -> tuple:
    """Score one candidate file. Runs in a worker process.

    A CANDIDATE THAT DOES NOT DEFINE THE PIECE IS A ROW, NOT A CRASH. Half
    the point of ranking a directory is that some of it is wrong; a
    misnamed subject or a missing `extern "C"` would otherwise take the
    other eight answers down with it.
    """
    source, record, exe, command, flag_sets, shared = job
    try:
        return str(source), compare_source(record, exe, source, command,
                                           flag_sets, shared), ""
    except (ValueError, CompileFailed) as problem:
        return str(source), None, str(problem)


def _rank_candidates(record: DecompilationState, exe: Path, directory: Path,
                     command: list[str], shared: frozenset, jobs: int,
                     as_json: bool) -> None:
    """Score every candidate in `directory` and report them best first."""
    candidates = sorted(directory.glob("*.cpp")) + sorted(
        directory.glob("*.c"))
    if not candidates:
        typer.secho(f"no *.cpp under {directory}", fg=typer.colors.RED)
        raise typer.Exit(2)

    # THE TREE'S OWN BODY IS ALWAYS IN THE FIELD. "Best" with no baseline
    # is not an answer anyone can act on: what a candidate has to beat is
    # what is committed, and one extra compile is what it costs to say so.
    work = [(source, record, exe, command, FLAG_SETS, shared)
            for source in [record.path, *candidates]]
    jobs = max(1, min(jobs or (os.cpu_count() or 1), WINE_CEILING))
    if jobs == 1:
        scored = [_score_candidate(job) for job in work]
    else:
        pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
        with pool:
            scored = list(pool.map(_score_candidate, work))

    def rank(row: tuple) -> tuple:
        _source, result, _note = row
        if result is None:
            return (99, 0.0)
        return (result.verdict.rank, -result.mnemonic_similarity)

    ordered = sorted(scored, key=rank)
    baseline = next(r for r in scored if r[0] == str(record.path))
    best = ordered[0]

    if as_json:
        typer.echo(json.dumps({
            "address": record.address_hex,
            "name": record.name,
            "baseline": str(record.path),
            "candidates": [
                {"source": source, "error": note,
                 **({} if result is None else _verdict_row(record, result))}
                for source, result, note in ordered],
        }, indent=2))
    else:
        typer.secho(f"\n{record.address_hex}  {record.name}", bold=True)
        typer.echo(f"  {len(candidates)} candidate(s) against the tree's own "
                   f"body")
        typer.echo(f"  {'tier':14}{'simil':>7} {'agree':>10}\n")
        for source, result, note in ordered:
            mark = "  <- the tree" if source == str(record.path) else ""
            if result is None:
                typer.secho(f"  {'ERROR':14}{Path(source).name}{mark}",
                            fg=typer.colors.RED)
                typer.echo(f"  {'':14}{note}")
                continue
            colour = (typer.colors.GREEN
                      if str(result.verdict) == "BYTE_EXACT" else None)
            # SIMILARITY FIRST, BECAUSE IT IS WHAT THE RANKING USES and
            # because `matching` is POSITIONAL: once a candidate's length
            # differs from the image's, every instruction after the first
            # divergence is compared against the wrong one, so the count
            # collapses and misranks. Measured on 0x005FEBB0, the best
            # candidate of five showed 33/99 against the tree's 34/99 while
            # aligning three MORE instructions than the tree did.
            typer.secho(f"  {str(result.verdict):14}"
                        f"{result.mnemonic_similarity:>7.3f} "
                        f"{result.matching_instructions:>4}/"
                        f"{result.original_instructions:<5} "
                        f"{Path(source).name}{mark}", fg=colour)
        if best[1] is not None and best[0] != baseline[0]:
            _print_verdict(record, best[1], Path(best[0]))

    verdict = best[1] and str(best[1].verdict)
    raise typer.Exit(0 if verdict == "BYTE_EXACT" else 1)


def _print_verdict(record: DecompilationState, result: AsmComparison,
                   source: Path | None = None) -> None:
    typer.secho(f"\n{record.address_hex}  {record.name}", bold=True)
    if source is not None and source != record.path:
        typer.echo(f"  candidate  {source}")
    colour = (typer.colors.GREEN if str(result.verdict) == "BYTE_EXACT"
              else typer.colors.YELLOW)
    typer.secho(f"  {result.verdict}", fg=colour, bold=True)
    if result.diagnostic:
        typer.echo(f"  {result.diagnostic}")
        return
    if result.flags:
        typer.echo(f"  flags      {result.flags}")
    typer.echo(f"  agreeing   {result.matching_instructions} of "
               f"{result.original_instructions} instructions"
               f"   (compiled has {result.compiled_instructions})")
    typer.echo(f"  bytes      {result.compared_bytes} compared, "
               f"{result.masked_bytes} discounted as relocations")
    if result.data_bytes:
        where = ("agrees" if result.data_divergence is None
                 else f"differs at +0x{result.data_divergence:X}")
        typer.echo(f"  jump table {result.data_bytes} bytes past the span, "
                   f"{where}")
    for index, mnemonic, was, now in result.differing_constants[:8]:
        typer.echo(f"  constant   #{index} {mnemonic}: {was} -> {now}")
    if result.first_divergence is None:
        return
    typer.echo(f"\n  first divergence at instruction "
               f"{result.first_divergence}")
    for original, compiled in zip(*result.context):
        typer.echo(f"    O: {original}")
        typer.echo(f"    C: {compiled}")


# WHAT THE CALL GRAPH IS FOR, and so what it hides by default. The image
# links the CRT and zlib statically, so a function's raw edge list is full of
# `_memset`, `__ftol` and `inflate` - 331 catalogued pieces whose recovery is
# not this project's job, since they can be had from the compiler and from
# upstream. The catalogue's `kind` fact separates them from Alpha Centauri's
# own 5,575, and imports leave the program entirely.
OURS = ("game", "thunk")


@app.command()
def calls(
    target: Annotated[str, typer.Argument(
        help="An address in hex, or a name.")],
    in_file: Annotated[str, typer.Option(
        "--in", help="Disambiguate by file when one address is annotated "
                     "twice.")] = "",
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    every: Annotated[bool, typer.Option(
        "--all", help="Keep the CRT, zlib and the DLL imports too.")] = False,
    as_json: Annotated[bool, typer.Option("--json")] = False,
) -> None:
    """What this function calls, IN THE ORDER THE IMAGE CALLS IT.

    Read from the shipped bytes, not from the annotation's `calls` fact -
    that fact is a sorted SET produced by a pass that no longer runs, so it
    cannot say what happens first, cannot say a target is reached twice, and
    nothing re-derives it when a body changes.

    ALPHA CENTAURI'S OWN CODE ONLY, by default. The image links the CRT and
    zlib statically and every DLL entry point arrives through the import
    table; listing those buries the edges a recovery cares about. `--all`
    keeps them, labelled.

    An edge nothing in the catalogue names is still shown, marked `?`. It is
    either uncatalogued game code or library code nobody annotated, and
    dropping it would quietly shorten the answer.
    """
    records = read(src)
    claimants = _in_file(_matching(records, target), in_file)
    if len(claimants) != 1:
        _ambiguous(claimants, target, "calls", in_file)
    record = claimants[0]

    refusal = span_refusal(record, exe, shared_spans(records))
    if refusal is not None:
        typer.secho(f"{record.address_hex}: {refusal} - no bytes to read",
                    fg=typer.colors.YELLOW)
        raise typer.Exit(1)
    listing = original_asm(record, exe)
    imports = imported_names(exe)
    by_address = {r.address: r for r in records}
    neighbours = sorted(records, key=lambda r: r.address)

    rows = [_edge(site, by_address, imports, neighbours)
            for site in call_sites(listing)]
    kept = rows if every else [r for r in rows if r["ours"]]

    if as_json:
        typer.echo(json.dumps({
            "address": record.address_hex,
            "name": record.name,
            "edges": [{k: v for k, v in row.items() if k != "ours"}
                      for row in kept],
        }, indent=2))
        return

    readable = qualified_name(record.name)
    typer.secho(f"\n{record.address_hex}  {readable or record.name}", bold=True)
    if readable and readable != record.name:
        typer.echo(f"  {record.name}")
    hidden = len(rows) - len(kept)
    typer.echo(f"  {len(kept)} call(s)" + (f", {hidden} hidden (--all shows "
               f"the CRT, zlib and the imports)" if hidden else "") + "\n")
    for row in kept:
        where = row["target"] or row["via"] or "?"
        # THE READABLE NAME HERE, THE MANGLED ONE IN `--json`. Nobody reads a
        # call list to copy a symbol out of it; every other use - `measure`,
        # `record`, a grep - wants the spelling the catalogue holds, and
        # `--json` is what those read.
        typer.echo(f"  {row['at']}  {row['form']:9}{where:12} "
                   f"{row['kind']:8} {qualified_name(row['name'])
                                      or row['name']}")


def _edge(site: CallSite, by_address: dict, imports: dict,
          neighbours: list) -> dict:
    """One call site, resolved as far as the image allows."""
    row = {"at": f"0x{site.at:08X}", "form": site.form, "target": None,
           "via": None, "kind": "", "name": "", "ours": False}
    if site.slot is not None:
        row["via"] = f"[0x{site.slot:08X}]"
        if site.slot in imports:
            row["kind"] = "import"
            row["name"] = imports[site.slot]
            return row
        # A SLOT THAT IS NOT AN IMPORT is this image's own indirection - a
        # vtable, a bound function pointer, a dispatch table. Where it points
        # is a runtime fact, so the slot is the whole answer.
        row["kind"] = "indirect"
        row["name"] = "(this image's own slot)"
        row["ours"] = True
        return row
    if site.target is None:
        row["kind"] = "dynamic"
        row["name"] = "(through a register)"
        row["ours"] = True
        return row
    row["target"] = f"0x{site.target:08X}"
    callee = by_address.get(site.target)
    if callee is not None:
        row["kind"] = callee.kind or "?"
        row["name"] = callee.name
        row["ours"] = callee.kind in OURS or not callee.kind
        return row
    return _unnamed(site.target, row, neighbours)


def _unnamed(target: int, row: dict, neighbours: list) -> dict:
    """An edge the catalogue does not name, described by what surrounds it.

    NOT CLASSIFIED, LOCATED. Naming a kind for an address nobody annotated
    would be an invention, and this tree has been bitten by facts that were
    really guesses. What CAN be said is where the address falls: inside a
    catalogued span - a call into the middle of a known function - or after
    one, in which case the neighbour's own `kind` is evidence about the
    neighbourhood and is reported AS evidence.

    0x00644EF2 is the case that shaped this. It sits between `__initterm`
    and `_abs`, both `library`, and it is the CRT's `free` under no name -
    so hiding it by default is right and asserting `kind library` is not.
    """
    below = None
    for record in neighbours:
        if record.address > target:
            break
        below = record
    if below is None:
        row["kind"] = "?"
        row["name"] = "(before anything the catalogue names)"
        row["ours"] = True
        return row
    inside = any(low <= target < high for low, high in below.image_spans)
    row["kind"] = "?"
    row["ours"] = below.kind in OURS or not below.kind
    row["name"] = (
        f"(inside {below.name}, +0x{target - below.address:X})" if inside else
        f"(unnamed; after {below.name}, kind {below.kind or '?'})")
    return row


def _record_one(job: tuple) -> tuple:
    """Measure one piece in a worker. Returns (piece, result) or (piece, None)."""
    piece, exe, command, shared = job
    try:
        return piece, compare_record(piece, exe, command, FLAG_SETS, shared)
    except (ValueError, CompileFailed) as problem:
        return piece, str(problem)


@app.command()
def record(
    targets: Annotated[list[str], typer.Argument(
        help="Addresses in hex, or names.")],
    in_file: Annotated[str, typer.Option(
        "--in",
        help="Disambiguate by file when one address is annotated twice, "
             "e.g. --in src/text.cpp or --in unrecovered.")] = "",
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
    )] = REPO_ROOT / "build" / "compile_commands.json",
    borrow: Annotated[str, typer.Option(
        help="Compile a file the build does not name with THIS file's "
             "command.")] = "src/buffer.cpp",
    demote: Annotated[bool, typer.Option(
        "--demote",
        help="Also clear a claim that stopped reproducing. Off by default: "
             "the floor is the number of claims.")] = False,
    reconfigure: Annotated[bool, typer.Option(
        "--reconfigure",
        help="Clear the cmake cache and regenerate the build database "
             "first, whether or not it looks stale.")] = False,
) -> None:
    """Measure these pieces and write down what was measured.

    UPDATES ANNOTATIONS THAT EXIST; it does not create them. A piece with no
    annotation cannot be named here, because naming one takes its mangled
    name and its image spans and nothing in this tree can supply those for
    an address nobody has annotated - `src/` IS the catalogue now. Creating
    one is a different operation with different inputs.

    Exits 1 if a claim stopped reproducing, because that is an event and not
    a result: `docs/DECOMP_MAP.md` calls it a tooling change or a lost
    scaffolding, and both need a human.
    """
    _fresh_compile_commands(compile_commands, reconfigure)
    records = read(src)
    shared = shared_spans(records)

    chosen = []
    for target in targets:
        claimants = _in_file(_matching(records, target), in_file)
        if len(claimants) != 1:
            _ambiguous(claimants, target, "record", in_file)
        chosen.append(claimants[0])

    # MEASURED FIRST, WRITTEN AFTER, and grouped by file. Writing one record
    # at a time re-reads its file, and a first write may canonicalise a
    # wrapped lesson onto one line - which moves every line below it and
    # leaves the records read before it pointing at the wrong ones.
    # IN PARALLEL, because a `sweep` hands this hundreds of addresses at once
    # and one compile is a wine invocation: 236 of them serially is the best
    # part of an hour, and the work is independent until the WRITE.
    work = [(piece, exe, _command_for(compile_commands, piece.path, borrow),
             shared) for piece in chosen]
    jobs = max(1, min(os.cpu_count() or 1, WINE_CEILING))
    if len(work) > 1 and jobs > 1:
        pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
        with pool:
            measured = list(pool.map(_record_one, work))
    else:
        measured = [_record_one(job) for job in work]

    updated, regressed, results = [], [], []
    for piece, result in measured:
        if isinstance(result, str):
            typer.secho(f"{piece.address_hex}: {result}", fg=typer.colors.RED)
            raise typer.Exit(2) from None
        results.append((piece, result))
        after = stamped(piece, result, demote)
        if piece.byte_exact and str(result.verdict) != "BYTE_EXACT":
            regressed.append((piece, result))
        if after != piece:
            updated.append(after)

    if updated:
        write_file(updated)
        typer.echo(f"claim floor now {_refresh_floor(src):,}")

    for piece, result in results:
        gained = (not piece.byte_exact
                  and str(result.verdict) == "BYTE_EXACT")
        mark = "  claimed" if gained else ""
        typer.echo(f"{piece.address_hex}  {result.verdict:14}"
                   f"{piece.name}{mark}")
    typer.echo(f"\n{len(updated)} annotation(s) rewritten")

    for piece, result in regressed:
        typer.secho(
            f"{piece.address_hex} claimed BYTE_EXACT and measured "
            f"{result.verdict} - {piece.location}",
            fg=typer.colors.RED)
    if regressed:
        # `--demote` IS THE ACKNOWLEDGEMENT. Exiting non-zero after being
        # told to clear them would make the flag unusable in a script; the
        # demotion is still reported, because a claim disappearing quietly
        # is the thing all of this exists to prevent.
        typer.secho(
            f"{len(regressed)} claim(s) cleared" if demote else
            f"{len(regressed)} claim(s) stopped reproducing and were KEPT; "
            f"--demote clears them once someone has looked",
            fg=typer.colors.RED)
        if not demote:
            raise typer.Exit(1)


@app.command()
def semantic(
    targets: Annotated[list[str], typer.Argument(
        help="Addresses in hex, or names.")],
    in_file: Annotated[str, typer.Option("--in")] = "",
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
    )] = REPO_ROOT / "build" / "compile_commands.json",
    borrow: Annotated[str, typer.Option()] = "src/buffer.cpp",
    withdraw: Annotated[bool, typer.Option(
        "--withdraw", help="Remove the claim instead of granting it.")] = False,
) -> None:
    """Claim that a body is SEMANTICALLY the same as the shipped bytes.

    THE SECOND HALF OF THE GOAL. Some bodies compile to the image's exact
    instruction sequence - same count, same mnemonics, same order - anddiffer only
    in which register holds what. Nothing about the source can move that: every
    processor and optimisation flag VC6 has was measured against ten of them
    and not one changed a single instruction. They are as finished as a source
    tree can make them, and with only BYTE_EXACT to write down they counted as
    unstarted forever.

    GRANTED DELIBERATELY, NEVER SWEPT. `sweep` cannot hand these out, because a
    tier a machine awards to whatever it cannot improve is a place to hide, and
    this tree has been bitten by exactly that. The test is strict and stated:
    identical instruction COUNT, mnemonic similarity of exactly 1.000, and no
    byte-level agreement possible under any flag set - if a body could be
    BYTE_EXACT it must be, and this refuses to cover for it.

    `check` re-proves every one of these and REGRESSES it if it stops holding,
    exactly like a BYTE_EXACT claim. It is reported and counted apart from
    them, so the ratchet's own number never moves because of this command.
    """
    _fresh_compile_commands(compile_commands, False)
    records = read(src)
    shared = shared_spans(records)
    chosen = []
    for target in targets:
        claimants = _in_file(_matching(records, target), in_file)
        if len(claimants) != 1:
            _ambiguous(claimants, target, "semantic", in_file)
        chosen.append(claimants[0])

    updated = []
    if withdraw:
        for piece in chosen:
            if piece.semantic:
                updated.append(replace(piece, semantic=False))
                typer.echo(f"{piece.address_hex}  withdrawn")
        if updated:
            write_file(updated)
            typer.echo(f"claim floor now {_refresh_floor(src):,}")
        typer.echo(f"\n{len(updated)} annotation(s) rewritten")
        return

    # IN PARALLEL, for the same reason `record` is: `sweep --semantic` hands
    # this a list, and one compile is a wine invocation.
    work = [(piece, exe, _command_for(compile_commands, piece.path, borrow),
             shared) for piece in chosen]
    jobs = max(1, min(os.cpu_count() or 1, WINE_CEILING))
    if len(work) > 1 and jobs > 1:
        pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
        with pool:
            measured = list(pool.map(_record_one, work))
    else:
        measured = [_record_one(job) for job in work]

    for piece, result in measured:
        if isinstance(result, str):
            typer.secho(f"{piece.address_hex}: {result}", fg=typer.colors.RED)
            raise typer.Exit(2) from None
        why = _not_semantic(result)
        if why:
            typer.secho(f"{piece.address_hex}  REFUSED: {why}",
                        fg=typer.colors.RED)
            continue
        # SAY WHICH OF THE TWO THIS IS. The agreement summary alone reads
        # like a fresh grant either way, and a caller who stops at the first
        # line - as I did, twice - reports claims that were already there.
        # The count at the end says so, but only after the fact.
        if piece.semantic:
            typer.secho(f"{piece.address_hex}  ALREADY CLAIMED - "
                        f"{result.matching_instructions} of "
                        f"{result.original_instructions} instructions byte "
                        f"identical, the rest allocated differently. Nothing "
                        f"written; this is not a new claim.",
                        fg=typer.colors.YELLOW)
            continue
        typer.secho(f"{piece.address_hex}  GRANTED - "
                    f"{result.matching_instructions} of "
                    f"{result.original_instructions} instructions byte "
                    f"identical, the rest allocated differently",
                    fg=typer.colors.GREEN)
        updated.append(replace(piece, semantic=True))
    if updated:
        write_file(updated)
        typer.echo(f"claim floor now {_refresh_floor(src):,}")
    typer.echo(f"\n{len(updated)} annotation(s) rewritten")


def _not_semantic(result) -> str:
    """Why this measurement does not support a SEMANTIC claim, or ""."""
    if str(result.verdict) == "BYTE_EXACT":
        return ("it is BYTE_EXACT - claim that instead, a weaker claim would "
                "cover for a stronger one")
    # `decomp.asm.allocation_only` IS the test, and it is stricter than any
    # tier: SHAPE_EXACT means a VALUE is wrong, which is a semantic difference
    # and not an allocation. An earlier version of this asked for mnemonic
    # similarity of 1.000 instead, and would have granted the claim to a body
    # with a wrong loop bound.
    return result.allocation_only


# WHAT THE BUILD IS CONFIGURED WITH, read off the cache the first time and
# stated here so a wiped cache can be rebuilt identically. `Debug` is not a
# choice this makes - it is what the tree configures - and the optimisation
# flags a match needs are NOT these: see FLAG_SETS.
CMAKE_ARGUMENTS = (
    "-G", "Ninja",
    "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/vc6.cmake",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
)


def _configure_inputs() -> list[Path]:
    """The files a configure reads. Newer than the database means stale."""
    found = [REPO_ROOT / "CMakeLists.txt"]
    found += sorted((REPO_ROOT / "cmake").rglob("*.cmake"))
    return [p for p in found if p.is_file()]


def _configure(compile_commands: Path, wipe: bool = True) -> None:
    """Regenerate the build database, from a cleared cache by default.

    CLEARED, BECAUSE A CACHE ANSWERS FROM WHAT IT WAS TOLD ONCE. A toolchain
    file that changed, a flag override that moved, a compiler that is no
    longer where it was - cmake will happily reconfigure around a stale
    cache and produce a database that describes a build nobody has. The
    whole point of asking is to be sure, and the cost is under a second
    against the minutes a wrong answer wastes.

    The build TREE is kept; only `CMakeCache.txt` and `CMakeFiles/` go, so
    object files survive.
    """
    build = compile_commands.parent
    if wipe:
        (build / "CMakeCache.txt").unlink(missing_ok=True)
        shutil.rmtree(build / "CMakeFiles", ignore_errors=True)
    result = subprocess.run(
        ["cmake", "-S", str(REPO_ROOT), "-B", str(build), *CMAKE_ARGUMENTS],
        capture_output=True, text=True)
    if result.returncode != 0 or not compile_commands.is_file():
        typer.secho(result.stdout + result.stderr, fg=typer.colors.RED)
        raise typer.Exit(2)


def _fresh_compile_commands(compile_commands: Path,
                            reconfigure: bool = False) -> None:
    """Regenerate the database when it is absent, stale, or asked for.

    STALE MEANS AN INPUT IS NEWER. Measuring against a database that
    describes a build nobody has is the quiet kind of wrong: every verdict
    comes back, and every one of them is about the wrong include path.
    """
    if reconfigure or not compile_commands.is_file():
        _configure(compile_commands)
        return
    stamp = compile_commands.stat().st_mtime
    stale = [p for p in _configure_inputs() if p.stat().st_mtime > stamp]
    if stale:
        typer.secho(f"{stale[0].name} is newer than "
                    f"{compile_commands.name}; reconfiguring",
                    fg=typer.colors.YELLOW)
        _configure(compile_commands)


@app.command()
def configure(
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
    )] = REPO_ROOT / "build" / "compile_commands.json",
) -> None:
    """Clear the cmake cache and regenerate the build database.

    `measure`, `record` and `check` do this for themselves when an input is
    newer than the database; this is how to do it deliberately.
    """
    _configure(compile_commands)
    entries = len(json.loads(compile_commands.read_text()))
    typer.secho(f"{entries} build inputs in {compile_commands}",
                fg=typer.colors.GREEN)


def duplicated_markers(records: list) -> list:
    """Addresses annotated more than once WITHIN one file.

    ALWAYS A DEFECT, unlike the same address in two files - that is the
    body-and-preserved-copy pattern and it is deliberate. Two markers in one
    unit describe one piece twice, so they compile to the same subject and
    can only agree by luck: twenty were found here and seventeen disagreed
    about whether the body reproduced. A sweep for unclaimed bodies that
    already match found the UNCLAIMED HALF of a claimed pair and offered it
    as free work, which is how they surfaced at all.
    """
    seen: dict = {}
    duplicated = []
    for record in records:
        key = (record.path, record.address)
        if key in seen:
            duplicated.append((record, f"also annotated at line {seen[key]}"))
        else:
            seen[key] = record.line
    return duplicated


def redundant_artifacts(records: list) -> list:
    """A claim in `src/recovered/` or `src/unrecovered/` whose address is
    ALSO claimed in product source.

    THE ARTIFACT HAS NOTHING LEFT TO SAY. A preserved unit is proof that a
    body reproduces somewhere; once the SHIPPED source reproduces the same
    address, that proof is a copy, and copies drift. This is not the same
    as an artifact whose product twin does not reproduce - that one is the
    only evidence there is and must stay, which is why the test is on the
    CLAIM and not on the address.

    Eighteen accumulated in one session, every one of them created by
    fixing a body and forgetting its artifact, and the same session had
    already declared the count zero.
    """
    from collections import defaultdict
    grouped: dict = defaultdict(list)
    for record in records:
        grouped[record.address].append(record)

    def preserved(record) -> bool:
        return ("/recovered/" in str(record.path)
                or "/unrecovered/" in str(record.path))

    redundant = []
    for address, group in sorted(grouped.items()):
        product = [r for r in group if not preserved(r) and r.byte_exact]
        copies = [r for r in group if preserved(r) and r.byte_exact]
        if product and copies:
            for copy in copies:
                redundant.append(
                    (copy, f"also claimed in {product[0].path.name} - "
                           f"the artifact is a copy"))
    return redundant


def asm_matches(src: Path) -> list:
    """Bodies that match only because they are written in `__asm`.

    THE ORDER THIS PROJECT WANTS, worst last: byte-exact C++, then C++ that is
    semantically identical, then a byte-exact `__asm` body, then one that does
    not match. A hand-written assembly body is a MATCH but not an ANSWER - it
    reproduces the bytes by restating them, and teaches nothing about what the
    original source said.

    So this does not fail the gate: the claim is true. It reports, so the
    number cannot quietly grow and so each one stays on a list to be rewritten
    in C++ - even at the cost of dropping to a semantic claim, which is the
    trade this project prefers.
    """
    out = []
    for path in sorted(src.glob("*.cpp")):
        text = path.read_text()
        if "__asm" not in text:
            continue
        lines = text.splitlines()
        marker = None
        for number, line in enumerate(lines, 1):
            found = re.match(r"//\s*ORIGINAL:\s*(0x[0-9A-Fa-f]+)(.*)", line)
            if found:
                marker = (found.group(1), "BYTE_EXACT" in found.group(2),
                          number)
            elif line.startswith("}"):
                # END OF A TOP-LEVEL DEFINITION. Without this the scan
                # attributes the NEXT `__asm` in the file to the last marker
                # it saw, however many C++ bodies intervene - which reported
                # StringStruct::seek_id, whose own body is C++, because a
                # helper below it is not.
                marker = None
            elif "__asm" in line and not line.lstrip().startswith("//"):
                if marker and marker[1]:
                    out.append(f"{path.name}:{marker[2]}: {marker[0]} matches "
                               f"in `__asm`, which is a restatement rather "
                               f"than a recovery - rewrite it in C++ even if "
                               f"that costs the byte match")
                    marker = None
    return out


def unread_markers(src: Path) -> list:
    """Markers in files `decomp`'s reader never globs.

    THE FAILURE THIS CATCHES IS SILENCE. The reader takes `*.cpp` and `*.c`;
    a marker written into a header is not a broken claim, it is a claim that
    stops being CHECKED - `check` reported 1,938 of them while the tree held
    1,959, exit 0, nothing red. Moving a body into a header is legitimate and
    now common (`MEASURED inline`); moving its marker is not.
    """
    out = []
    for path in sorted(src.rglob("*")):
        if path.suffix not in (".h", ".hpp", ".hh", ".inl"):
            continue
        for number, line in enumerate(path.read_text().splitlines(), 1):
            match = re.match(r"\s*(?://|\*)\s*ORIGINAL:\s*(0x[0-9A-Fa-f]+)",
                             line)
            if match:
                out.append(
                    f"{path}:{number}: {match.group(1)} is annotated in a "
                    f"header, which the reader does not glob - the claim is "
                    f"not checked by anything. Leave the marker in the .cpp "
                    f"and move only the body.")
    return out


def dangling_bodies(records: list) -> list:
    """Every `body` fact that does not lead to a definition.

    THE FACT HAS TO BE CHECKED OR IT IS A COMMENT WITH A PARSER. `// body
    src/palette.h` says the definition is over there, and nothing about
    measuring the piece depends on it - the marker's own unit is what gets
    compiled. So the only thing that keeps it true is a check: the file it
    names must exist, and must still contain the piece under the name the
    annotation records. A body that moves out of a header, or a header that
    is renamed, leaves the pointer behind, and a pointer nobody verifies is
    how this tree has been misled before.

    Costs no compile: a read per named file, cached.
    """
    text: dict = {}
    broken = []
    for record in records:
        if not record.body:
            continue
        if record.body not in text:
            text[record.body] = _named_file(record)
        contents = text[record.body]
        if contents is None:
            broken.append((record, f"{record.body} does not exist"))
            continue
        identifier = mangled.identifier(record.symbol or record.name)
        if identifier and identifier not in contents:
            broken.append((record,
                           f"{record.body} does not mention {identifier}"))
    return broken


def _named_file(record: DecompilationState) -> str | None:
    """The text of the record's `body` file, or None if it is not there.

    RESOLVED FROM THE RECORD, NOT FROM A ROOT. The fact is repo-relative
    (`src/palette.h`) and the record knows its own absolute path, so the
    repo root is whichever ancestor of that path the fact resolves under.
    Deriving it from `--src` instead looked fine and broke the moment
    someone passed a FILE there: `src/text.cpp`'s parent is `src`, and
    `src/src/text.h` does not exist. The check caught it on its first run.
    """
    for ancestor in record.path.parents:
        candidate = ancestor / record.body
        if candidate.is_file():
            return candidate.read_text(errors="replace")
    return None


def _claims_by_file(records: list, built: set | None = None) -> dict:
    """Every record worth scoring, grouped by file.

    THE CLAIMS AND THE CANDIDATES IN ONE PASS. A claim - byte-exact or
    semantic - is re-proved; an unclaimed body in a file the build compiles is
    measured to see whether it has started reproducing, which used to be a
    second command that compiled the whole tree again.

    What must NOT follow is stamping: a run that recorded its own findings
    would be a `check` that could never fail. It reports them; `record` writes
    them.
    """
    grouped: dict = {}
    for record in records:
        claimed = record.byte_exact or record.semantic
        if claimed:
            grouped.setdefault(record.path, []).append(record)
            continue
        if built is None or record.path not in built:
            continue
        if not record.image_spans or not record.name:
            continue
        grouped.setdefault(record.path, []).append(record)
    return grouped


def _claim_key(record: DecompilationState) -> tuple:
    """What identifies a CLAIM. Not the address.

    ADDRESSES IN THIS TREE ARE ANNOTATED TWICE - the body in `src/` and the
    same piece preserved in `src/recovered/` or `src/unrecovered/` - and
    they are two claims, measured in two translation units, that can
    disagree. Keyed by address, the second result overwrote the first, so
    `check` reported one verdict for two claims and counted the twin it
    never showed among the reproduced.

    THE LINE IS IN THE KEY BECAUSE THE PATH WAS NOT ENOUGH. Twenty
    addresses were annotated twice IN ONE FILE - adjacent comment blocks
    eleven to fourteen lines apart, seventeen of them disagreeing about
    BYTE_EXACT - and (path, address) collapsed those exactly as (address)
    collapsed the others. They are gone from the tree now; the key stays
    honest so the next pair cannot hide.
    """
    return (record.path, record.address, record.line)


TIER_ORDER = {"BYTE_EXACT": 0, "SHAPE_EXACT": 1, "MNEMONIC_ONLY": 2,
              "MISMATCH": 3, "UNRESOLVED": 4, "NO_COMPILE": 5}


def _rank(verdict: str) -> int:
    return TIER_ORDER.get(verdict, 9)


def _semantic_note_holds(note: str) -> bool:
    """Whether a scorer's note still supports a SEMANTIC claim.

    The scorer stamps `[allocation-only]` when `decomp.asm.allocation_only`
    found nothing but register numbers between the two listings. That is the
    whole test, and it is the same one `osmx semantic` applies when granting.
    """
    return "[allocation-only]" in (note or "")


def _report_stale_worktrees() -> None:
    """Name agent worktrees whose agent has finished, if any.

    ADVISORY AND FAILURE-PROOF: this is a convenience on the ratchet's output,
    so anything going wrong with git must leave the gate's verdict untouched.
    It counts only; `tools/reap_worktrees.py` owns the four refusals that
    decide what is actually safe to remove.
    """
    try:
        listing = subprocess.run(
            ("git", "worktree", "list", "--porcelain"),
            cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=20)
        if listing.returncode:
            return
        blocks = listing.stdout.split("\n\n")
        finished = [b for b in blocks
                    if "worktrees/agent-" in b and "locked" not in b]
        if finished:
            typer.secho(
                f"  {len(finished):,} finished agent worktree(s) still on "
                f"disk:", fg=typer.colors.YELLOW)
            typer.echo("    uv run tools/reap_worktrees.py"
                       "        # then --reap")
    except Exception:                                    # noqa: BLE001
        return


def _agreeing(note: str) -> int:
    """The matching-instruction count a scorer put in a claim's note."""
    found = re.match(r"(\d+)/", note or "")
    return int(found.group(1)) if found else -1


# How many instructions short a body may be and still be worth listing as
# nearly done. Three, because two is where the array thunks sat before the
# vector-iterator fix and one is where `Buffer::hline` sits now.
NEAR_MISS = 3


def _agreement(note: str) -> tuple[int, int]:
    """(agreeing, total) from a scorer's note, or (-1, -1)."""
    found = re.match(r"(\d+)/(\d+)", note or "")
    return (int(found.group(1)), int(found.group(2))) if found else (-1, -1)


def _check_one_file(job: tuple) -> list[tuple[tuple, str, str]]:
    """Score every record in one file - claimed and unclaimed alike.

    ONE COMPILE PER FLAG SET, NOT PER RECORD: this tree's records are ~2,100
    across ~90 files, so per record is thousands of compiles and per file is
    at most ten. It stops as soon as every record in the file is BYTE_EXACT,
    which is almost never now that unclaimed bodies are included - the early
    exit was worth more when only claims were scored, and scoring everything
    in ONE pass is worth more than the exit was.
    """
    path, records, exe, command, flag_sets = job
    best: dict = {}
    unresolved: dict = {}
    outstanding = list(records)
    diagnostic = ""
    for flags in flag_sets:
        if not outstanding:
            break
        try:
            obj = compile_unit(path, command, flags)
        except CompileFailed as failed:
            # A UNIT THAT WILL NOT PARSE WILL NOT PARSE SEVEN MORE TIMES.
            # See `decomp.asm.flag_dependent`, and the measurement behind it.
            diagnostic = diagnostic or str(failed)
            if flag_dependent(str(failed)):
                continue
            break
        for record in list(outstanding):
            try:
                compiled = subject_asm(obj, record, flags)
            except ValueError as problem:
                # A SUBJECT THIS INVOCATION DOES NOT EMIT IS NOT A VERDICT.
                # An `inline` function is folded into its callers under /O2
                # and emitted as its own COMDAT under /Ob0, so the record
                # stays outstanding and the next flag set gets its turn;
                # UNRESOLVED is only true if NONE of them emitted it.
                # `decomp.asm.compare_source` learned this first and this
                # copy of the loop did not, which is how `say_num` went from
                # BYTE_EXACT to unverifiable between two sweeps.
                unresolved[_claim_key(record)] = str(problem)
                continue
            result = compare_subject(record, exe, compiled)
            verdict = str(result.verdict)
            # THE NOTE CARRIES THE AGREEMENT, so `sweep --verdicts` can rank
            # within a tier. Most of the remaining work is MISMATCHes, and the
            # word alone cannot tell one that is two instructions out from one
            # nobody has started.
            # THE NOTE CARRIES WHAT A SEMANTIC CLAIM RESTS ON as well as the
            # agreement, because `check` re-proves both from it.
            score = (f"{result.matching_instructions}/"
                     f"{result.original_instructions} instructions, "
                     f"{result.mnemonic_similarity:.3f} similar"
                     + (f", {result.compiled_instructions} compiled"
                        if result.compiled_instructions
                        != result.original_instructions else "")
                     + (" [allocation-only]" if result.allocation_only == ""
                        else ""))
            key = _claim_key(record)
            if key not in best or best[key][0] != "BYTE_EXACT":
                previous = best.get(key)
                better = (previous is None
                          or _rank(verdict) < _rank(previous[0])
                          or (_rank(verdict) == _rank(previous[0])
                              and result.matching_instructions
                              > _agreeing(previous[1])))
                if better:
                    best[key] = (verdict, score)
            if verdict == "BYTE_EXACT":
                outstanding.remove(record)
    for record in outstanding:
        key = _claim_key(record)
        if key in unresolved:
            best.setdefault(key, ("UNRESOLVED", unresolved[key]))
        else:
            best.setdefault(key, ("NO_COMPILE", diagnostic))
    return [(key, verdict, note) for key, (verdict, note) in best.items()]


def _image_pin(src: Path, exe: Path) -> str:
    """Empty if the image matches the pin beside the tree; else what differs.

    THE RULER IS CHECKED BEFORE ANYTHING IS MEASURED WITH IT.
    """
    # NO IMAGE, NO RULER TO VERIFY. `check --exe /nonexistent` with the
    # measurement stubbed is how decomp/tests drives this command, and the
    # pin landed without noticing: fifteen tests went red and stayed red,
    # because nothing runs pytest. A tree that HAS an image still must
    # match its pin - that is the case the positive control exercises - but
    # demanding a pin for an image that is not there fails the wrong thing.
    # A real run without the image cannot measure anything either.
    if not exe.exists():
        # SAID OUT LOUD, not swallowed. Skipping the pin here is right - there
        # is no ruler to verify - but a silent skip is how a check becomes
        # vacuous, so the run says the ruler was never checked.
        typer.secho(f"  no image at {exe}: the pin was not verified, and "
                    f"nothing here was measured against it",
                    fg=typer.colors.YELLOW)
        return ""
    pin = src / "IMAGE_PIN"
    try:
        recorded = re.search(r"^sha256 ([0-9a-f]{64})$", pin.read_text(),
                             re.M)
    except OSError:
        return (f"NO IMAGE PIN at {pin} - write one with the sha256 of the "
                f"image these claims were measured against")
    if not recorded:
        return f"{pin} carries no `sha256 <digest>` line"
    try:
        actual = hashlib.sha256(exe.read_bytes()).hexdigest()
    except OSError as problem:
        return f"cannot read the image at {exe}: {problem}"
    if actual != recorded.group(1):
        return (f"IMAGE IS NOT THE PINNED ONE: {exe} hashes {actual[:16]}..., "
                f"the pin says {recorded.group(1)[:16]}... - every claim in "
                f"this tree was measured against the pinned bytes")
    return ""


@app.command()
def check(
    src: Annotated[Path, typer.Option(
        envvar="OPENSMACX_SRC")] = REPO_ROOT / "src",
    exe: Annotated[Path, typer.Option(
        envvar="OPENSMACX_IMAGE",
    )] = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe",
    compile_commands: Annotated[Path, typer.Option(
        envvar="OPENSMACX_COMPILE_COMMANDS",
    )] = REPO_ROOT / "build" / "compile_commands.json",
    borrow: Annotated[str, typer.Option(
        help="Compile files the build does not name with THIS file's "
             "command.")] = "src/buffer.cpp",
    jobs: Annotated[int, typer.Option(
        help=f"Concurrent compiles. Capped at {WINE_CEILING}: one shared "
             f"wine prefix.")] = 0,
    reconfigure: Annotated[bool, typer.Option(
        "--reconfigure",
        help="Clear the cmake cache and regenerate the build database "
             "first, whether or not it looks stale.")] = False,
    as_json: Annotated[bool, typer.Option("--json")] = False,
) -> None:
    """THE RATCHET. Does every proved body still reproduce?

    Re-measures every `BYTE_EXACT` claim in the tree against the shipped
    image and fails if one stopped. The floor is the number of claims, so
    there is no constant to bump.

    IT WRITES NOTHING, and that is what makes it a gate rather than a pass
    of `record` that happens to fail. A gate that can add a claim changes
    the floor as a side effect of measuring it, and one that can clear a
    broken claim can always be made to pass.

    Exit 1 means a claim REGRESSED - measured, and worse. Exit 3 means some
    claim could not be measured at all, which is a different job and must
    not be mistaken for a green gate. Exit 0 means every claim was asked and
    every one still holds.
    """
    _fresh_compile_commands(compile_commands, reconfigure)
    records = read(src)
    dangling = (dangling_bodies(records)
                + duplicated_markers(records)
                + redundant_artifacts(records))
    unread = unread_markers(src)
    restated = asm_matches(src)

    # THE BUILD DATABASE IS THE AUTHORITY ON WHOSE FAULT A WALL IS - see
    # `unasked_here` below - and on which unclaimed bodies are worth measuring.
    # Read once, not once per file.
    built = build_inputs(compile_commands)
    grouped = _claims_by_file(records, built)
    claimed = {_claim_key(r) for group in grouped.values() for r in group
               if r.byte_exact or r.semantic}
    claims = len(claimed)
    scored = sum(len(v) for v in grouped.values())
    if not claims:
        typer.secho(f"no BYTE_EXACT claims under {src}",
                    fg=typer.colors.RED)
        raise typer.Exit(2)

    # THE FLOOR, before anything is measured: a claim that VANISHED never
    # reaches the measure loop, so no verdict can catch it - only the count
    # against the persisted ledger can. Both directions fail. Below: a marker
    # was deleted or corrupted into prose (`ORIGINAl:` reads as a comment).
    # Above: a claim token exists that `osmx record` never measured.
    pin_broken = _image_pin(src, exe)
    if pin_broken:
        typer.secho(pin_broken, fg=typer.colors.RED, bold=True)
        raise typer.Exit(1)

    floor = _read_floor(src)
    if floor is None:
        floor_broken = (f"NO CLAIM FLOOR at {_floor_path(src)} - "
                        f"run `osmx record` on any address to write it")
    elif claims < floor:
        floor_broken = (f"CLAIMS FELL: {claims:,} live against a floor of "
                        f"{floor:,} - a marker vanished or was edited into "
                        f"prose; `git diff` names it, and a deliberate "
                        f"demotion goes through `osmx record --demote`, "
                        f"which rewrites the floor")
    elif claims > floor:
        floor_broken = (f"CLAIMS ABOVE THE FLOOR: {claims:,} live against "
                        f"{floor:,} - a claim token was written by something "
                        f"other than `osmx record`; measure it "
                        f"(`osmx record <addr>`) or remove it")
    else:
        floor_broken = ""

    jobs = max(1, min(jobs or (os.cpu_count() or 1), WINE_CEILING))

    work = []
    for path, mine in grouped.items():
        command = _command_for(compile_commands, path, borrow)
        work.append((path, mine, exe, command, FLAG_SETS))

    if not as_json:
        # `--json` must emit JSON and nothing else, or it is not
        # machine-readable - which is the only reason it exists.
        typer.echo(f"{scored:,} bodies ({claims:,} claimed) in "
                   f"{len(grouped):,} files, "
                   f"{jobs} at a time")
    measured: dict = {}
    if jobs == 1:
        # SERIAL IS A REAL MODE, not a degenerate pool of one. A failure
        # inside a worker arrives as a re-raised copy with the original
        # traceback lost, so `--jobs 1` is how anything here is debugged -
        # and a pool of one pays setup to gain nothing.
        batches = (_check_one_file(job) for job in work)
    else:
        pool = concurrent.futures.ProcessPoolExecutor(max_workers=jobs)
        with pool:
            batches = [done.result() for done in
                       concurrent.futures.as_completed(
                           [pool.submit(_check_one_file, job)
                            for job in work])]
    for batch in batches:
        for key, verdict, note in batch:
            measured[key] = (verdict, note)

    # A WALL IS NOT A MISS, and one number would bury the difference. A
    # claim measured WORSE has stopped reproducing - that is the ratchet
    # failing. A claim that could not be built or whose subject could not be
    # found was never asked, and reporting it as a regression sends someone
    # to look at a body that is fine. Measured here: 6 against 534.
    by_claim = {_claim_key(r): r for group in grouped.values() for r in group}
    UNASKED = ("UNRESOLVED", "SHARED_TAIL", "REFUSED")

    def unasked_here(record: DecompilationState, verdict: str) -> bool:
        """Was this claim unaskable, or did someone break it?

        A WALL ONLY EXPLAINS ITSELF IN A FILE THE BUILD DOES NOT BUILD -
        and that goes for every unaskable verdict, not just NO_COMPILE.
        `src/recovered/` and `src/unrecovered/` are not build inputs - they
        compile on a borrowed invocation and 313 of them do not compile at
        all, which is the wall this classification was written for. A file
        CMake compiles is a different matter: the build database naming it
        is the project asserting that it builds, so a NO_COMPILE there is a
        broken tree and someone must fix it.

        MEASURED, not reasoned: making `Buffer::dib_` private moved
        `width_` out of reach of `win.cpp`, and 28 claims that had been
        BYTE_EXACT went NO_COMPILE. The gate printed the same "0 REGRESSED"
        and the same exit 3 it prints when everything is fine, because a
        wall someone just built looked exactly like a wall that was always
        there. The verified count fell by 28 and nothing said so.

        THE SAME HOLE IN A DIFFERENT WALL, found 2026-08-23 auditing for
        the class loop: UNRESOLVED - the verdict of a symbol the object no
        longer carries - took the unaskable branch UNCONDITIONALLY. Yet
        declaring a method virtual, renaming one, or re-pointing its
        `// symbol` fact - three things the class passes do on every pass -
        can stop a CLAIMED symbol from being emitted, and the ratchet then
        folded the loss into the standing unverifiable tally as exit 3.
        Same guard, therefore: in a built file these verdicts are
        regressions too.
        """
        if verdict in UNASKED or verdict == "NO_COMPILE":
            return record.path.resolve() not in built
        return False

    def holds(record: DecompilationState, verdict: str, note: str) -> bool:
        """Does the measurement still support the claim this record makes?

        TWO CLAIMS, TWO TESTS. A BYTE_EXACT record needs the bytes. A SEMANTIC
        record needs the same instruction COUNT and a mnemonic similarity of
        1.000 - and BYTE_EXACT satisfies it too, since a body that got better
        has not stopped holding.
        """
        if verdict == "BYTE_EXACT":
            return True
        if not record.semantic:
            return False
        return _semantic_note_holds(note)

    regressed = [(by_claim[k], v, n)
                 for k, (v, n) in sorted(measured.items())
                 if k in claimed and not holds(by_claim[k], v, n)
                 and not unasked_here(by_claim[k], v)]
    unasked = [(by_claim[k], v, n)
               for k, (v, n) in sorted(measured.items())
               if k in claimed and not holds(by_claim[k], v, n)
               and unasked_here(by_claim[k], v)]
    semantic_held = [by_claim[k] for k, (v, n) in measured.items()
                     if k in claimed and by_claim[k].semantic
                     and v != "BYTE_EXACT" and holds(by_claim[k], v, n)]
    # THE OTHER HALF OF THE PASS: bodies nobody has claimed that now
    # reproduce, and bodies that would support a semantic claim. Reported,
    # never stamped - see `_claims_by_file`.
    free = sorted((by_claim[k] for k, (v, _n) in measured.items()
                   if k not in claimed and v == "BYTE_EXACT"),
                  key=lambda r: r.address)
    allocation = sorted((by_claim[k] for k, (v, n) in measured.items()
                         if k not in claimed and v != "BYTE_EXACT"
                         and _semantic_note_holds(n)),
                        key=lambda r: r.address)
    # NEAR MISSES, which this pass already measured and used to throw away.
    # Every unclaimed body is scored here to find the free ones; a body one or
    # two instructions short is the cheapest work on the board and nothing was
    # reporting it. Ranked by how many instructions are missing, then by size,
    # so a 2-instruction gap in a 200-instruction body ranks above the same gap
    # in a 4-instruction thunk - the big one is nearly done, the small one is
    # barely started.
    near = []
    for k, (v, n) in measured.items():
        if k in claimed or v == "BYTE_EXACT" or not n:
            continue
        agreeing, total = _agreement(n)
        if total <= 0 or agreeing < 0:
            continue
        short = total - agreeing
        if 0 < short <= NEAR_MISS:
            near.append((short, -total, by_claim[k], n))
    near.sort(key=lambda row: (row[0], row[1]))
    broken = regressed + unasked

    payload = None
    if as_json:
        def rows(items):
            return [{"address": r.address_hex, "name": r.name,
                     "location": str(r.location), "measured": v, "note": n}
                    for r, v, n in items]
        # BUILT HERE, EMITTED AT THE END, and the move IS the fix. This
        # echoed right here - before the link probe and all five censuses had
        # run - so EIGHT of the ten signals that decide the exit code could
        # not appear in it even in principle: dangling, unread, the link, and
        # every census verdict. A harvest could read `reproduced == claims`,
        # an empty `regressed`, and still exit 1 with nothing saying why.
        # The verdict line was suppressed under `--json` too, in the one mode
        # built for machines - while the comment beside it argued, from a
        # real incident, that a verdict nobody can see through a pipe is how
        # a red gate gets called green. `--json` is a superset now.
        payload = {
            "claims": claims,
            "claim_floor": floor,
            "floor_broken": floor_broken or None,
            "reproduced": claims - len(broken),
            "semantic": [{"address": r.address_hex, "name": r.name}
                         for r in semantic_held],
            "free": [{"address": r.address_hex, "name": r.name,
                      "file": str(r.path)} for r in free],
            "near_miss": [{"address": r.address_hex, "name": r.name,
                           "file": str(r.path), "short": short, "note": n}
                          for short, _, r, n in near],
            "allocation_only": [{"address": r.address_hex, "name": r.name,
                                 "file": str(r.path)} for r in allocation],
            "regressed": rows(regressed),
            "unverifiable": rows(unasked),
            "dangling_bodies": [{"address": r.address_hex,
                                 "location": str(r.location), "note": n}
                                for r, n in dangling],
        }
    else:
        for record, verdict, note in regressed:
            claimed = "SEMANTIC" if record.semantic and not record.byte_exact \
                else "BYTE_EXACT"
            typer.secho(f"REGRESSED {record.address_hex} claims {claimed}, "
                        f"measured {verdict} - {record.location}",
                        fg=typer.colors.RED)
            if note:
                typer.echo(f"    {note}")
        if unasked:
            causes: dict = {}
            for _record, verdict, _note in unasked:
                causes[verdict] = causes.get(verdict, 0) + 1
            typer.secho(
                "  unverifiable: "
                + ", ".join(f"{n} {v}" for v, n in sorted(causes.items())),
                fg=typer.colors.YELLOW)
            # A TALLY IS WHERE A NEW LOSS HIDES. 297 NO_COMPILE against 34
            # UNRESOLVED reads as furniture - which is exactly how 28 claims
            # that went NO_COMPILE in one edit stayed invisible (the incident
            # narrated above). Name them: address and file, nearest-first,
            # so the difference between yesterday's wall and today's break
            # is one diffable list instead of a recount.
            for record, verdict, _note in unasked[:15]:
                typer.secho(f"    {record.address_hex}  {verdict:12s} "
                            f"{record.path.name}", fg=typer.colors.YELLOW)
            if len(unasked) > 15:
                typer.secho(f"    ... and {len(unasked) - 15} more",
                            fg=typer.colors.YELLOW)
            # WHAT "UNVERIFIABLE" MEANS HERE, because the word invites the
            # wrong reading. These are not claims that might be broken and
            # nobody looked; they are almost all ARTIFACT-ONLY bodies under
            # src/recovered/ and src/unrecovered/ that were never
            # self-compiling - bare bodies whose scaffolding lived in the
            # generator's own unit and was not kept beside them. Their own
            # headers say so: "NOT in OPENSMACX_SOURCES and not compiled".
            # Nothing links them, so they cannot hide a regression in
            # anything the build produces - and they cannot be re-asked
            # either, which is why promoting one into product source is the
            # only thing that makes its claim checkable again.
            artifacts = sum(
                1 for record, _v, _n in unasked
                if "/recovered/" in str(record.path)
                or "/unrecovered/" in str(record.path))
            typer.secho(
                f"    {artifacts} of them are artifact-only bodies that were "
                f"never in the build; promoting one is what makes its claim "
                f"checkable", fg=typer.colors.YELLOW)
        # COUNTED APART. A semantic claim is re-proved like any other, but it
        # is a weaker statement and must never be added into the number the
        # ratchet is measured by.
        if semantic_held:
            typer.secho(
                f"  {len(semantic_held):,} semantic: same instructions, "
                f"different registers", fg=typer.colors.CYAN)
        # WHAT THE SAME PASS FOUND WITHOUT BEING ASKED. Printed, never
        # written: `record` and `semantic` are the commands that write.
        if near:
            typer.echo("")
            typer.echo(f"  {len(near):,} unclaimed body(s) within "
                       f"{NEAR_MISS} instruction(s) of the image:")
            for short, _, record, note in near[:25]:
                typer.echo(f"    {record.address_hex}  {short} short   "
                           f"{record.path.name:22s} {record.name or '?'}")
            if len(near) > 25:
                typer.echo(f"    ... and {len(near) - 25:,} more")
        if free:
            typer.secho(
                f"  {len(free):,} unclaimed body(s) already reproduce:",
                fg=typer.colors.GREEN)
            typer.echo("    osmx record "
                       + " ".join(r.address_hex for r in free))
        if allocation:
            typer.secho(
                f"  {len(allocation):,} unclaimed body(s) differ only in "
                f"register allocation:", fg=typer.colors.CYAN)
            typer.echo("    osmx semantic "
                       + " ".join(r.address_hex for r in allocation))
        # FINISHED AGENT WORKTREES, reported HERE because this is the command
        # that always runs. Twelve of them accumulated - nine belonging to
        # agents that had finished hours earlier, each a full checkout - and
        # nothing named them, because reaping them was something the
        # coordinator had to REMEMBER. The gate does not forget.
        _report_stale_worktrees()
        typer.secho(
            f"{claims - len(broken) - len(semantic_held):,} verified, "
            f"{len(semantic_held):,} semantic, {len(regressed):,} "
            f"REGRESSED, {len(unasked):,} unverifiable, of {claims:,} claims",
            fg=typer.colors.RED if regressed else
            typer.colors.YELLOW if unasked else typer.colors.GREEN, bold=True)
    # THREE OUTCOMES, THREE CODES. A regression means a body stopped
    # reproducing and someone must look at it; an unverifiable claim means
    # the gate could not ask, which is a different job - build the unit the
    # way it was built. Passing the second silently would leave 534 claims
    # unchecked behind a green gate, which is the shape this whole ratchet
    # exists to prevent.
    if restated and not as_json:
        for note in restated:
            typer.secho(f"RESTATED  {note}", fg=typer.colors.YELLOW)
    if unread and not as_json:
        for note in unread:
            typer.secho(f"UNREAD    {note}", fg=typer.colors.RED)
    if dangling and not as_json:
        for record, note in dangling:
            typer.secho(f"DUPLICATE {record.address_hex} {note} - "
                        if "annotated" in note or "copy" in note
                        else
                        f"DANGLING  {record.address_hex} body fact: {note} - "
                        f"{record.location}", fg=typer.colors.RED)
    # DOES IT STILL LINK? The gate compiles each body on its own and never
    # built the executable, so a tree could be 2,532-verified green while
    # OpenSMACX.exe did not link at all. That is not hypothetical: declaring
    # two GraphicWin virtuals without defining them produced 2 unresolved
    # externals across 30 objects on 2026-08-22, and every per-function
    # measurement still passed - a virtual has to be IN THE VTABLE, so it needs
    # a definition, and nothing that measures one function at a time can see
    # that. The entry point is the frontier; a tree that cannot link has no
    # entry point.
    #
    # The build is incremental and this is the last thing the gate does, so a
    # clean tree pays almost nothing for it.
    link = subprocess.run(["cmake", "--build", "build"], cwd=REPO_ROOT,
                          capture_output=True, text=True)
    unlinked = [line for line in (link.stdout + link.stderr).splitlines()
                if "LNK" in line or "error" in line.lower()]
    if link.returncode and not as_json:
        typer.secho(f"DOES NOT LINK - {len(unlinked)} error line(s):",
                    fg=typer.colors.RED, bold=True)
        for line in unlinked[:6]:
            typer.secho(f"  {line.strip()[:150]}", fg=typer.colors.RED)

    # WORK THE COMPILER SHOULD BE DOING, eight shapes of it. A hand-installed
    # vtable, a `construct()` standing in for a constructor, an explicit base
    # destructor call, a placement new where a base would do - each is the tree
    # doing VC6's job, and doing it somewhere VC6 would not have put it, which
    # is why they cost bytes rather than taste. Every large gain on 2026-08-22
    # came from deleting one. Ratcheted per shape; none may grow.
    vtables = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "compiler_work.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        typer.secho("  " + vtables.stdout.strip(),
                    fg=typer.colors.RED if vtables.returncode
                    else typer.colors.WHITE)

    # PROSE THAT TELLS THE READER TO RUN SOMETHING THAT IS NOT THERE, held to
    # zero the same way. This failure is invisible from inside the instruction:
    # `.claude/commands/recover-batch.md` named SEVEN deleted tools, so every
    # `uv run` line in it failed at step 1 - and nothing noticed, because the
    # only reader is whoever types the slash command. `tools/stale_references.py`
    # had existed for months and its roots stopped one directory short of that
    # file, and one directory is the whole difference between a check and a
    # check that cannot see the defect it was written for.
    stale = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "stale_references.py")],
        cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = stale.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if stale.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:8]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    # EVERY MARKER NAMES A SYMBOL THE BUILD EMITS, held to a falling floor.
    # A wrong or stale `// symbol` fact sends measurement to the wrong bytes
    # or abandons it - and the class passes rewrite those facts constantly.
    # Six of today's markers await stub conversions and the ordinal
    # initialiser matcher; the floor starts there and only goes down.
    symbols = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "marker_symbols.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = symbols.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if symbols.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:4]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    # ONE ADDRESS, EVERY PLACE IT APPEARS. The three partial duplicate/
    # artifact checks each guarded one boundary and the audit found every
    # boundary leaking: addresses annotated in two artifact files, claims
    # contradicted by the PENDING_BODY forwarder the build actually links,
    # scaffold files nobody ratcheted. This is the whole map, floors only
    # falling; duplicated_markers/redundant_artifacts stay (they encode
    # within-file and claimed-copy RULES) while orphan_artifacts' report is
    # subsumed here.
    index = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "address_index.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = index.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if index.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:4]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    # SEMANTIC DEBT: what a body can carry while measuring BYTE_EXACT - UNK
    # names, function addresses in data clothes, orphan redirects, pointers
    # travelling as int. The bytes cannot see any of it, which is exactly why
    # a census must: palette.cpp measured clean while carrying all four.
    debt = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "class_debt.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = debt.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if debt.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:4]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    # POINTER SCALING: `reinterpret_cast<int *>(parent + 0xcc)` where parent
    # is a `Win *` advances 0xcc whole OBJECTS, not bytes, and reads about
    # 900KB past the one it meant. A byte comparison cannot tell a wrong
    # displacement from a right one - it sees two numbers that differ - so
    # Win::remove_parent_dialog carried eight of them at 12 of 61 and nobody
    # asked why. Ceiling is zero; the correct form casts the POINTER, not
    # the sum.
    scaling = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "pointer_scale.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = scaling.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if scaling.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:6]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    # ANNOTATION IDENTITY: does an annotation survive being rewritten? The
    # writer used to regenerate each lesson run from a lossy parse, so
    # `osmx record` reflowed, truncated or deleted the measured LEVER/TRIED
    # prose of 434 records the moment it touched their file - and the
    # roundtrip test could not see it, because it compared POST-PARSE data.
    # This is the check that had to exist outside the parse.
    annotations = subprocess.run(
        [sys.executable, str(REPO_ROOT / "tools" / "annotation_identity.py"),
         "--check"], cwd=REPO_ROOT, capture_output=True, text=True)
    if not as_json:
        said = annotations.stdout.strip().splitlines() or [""]
        typer.secho("  " + said[-1],
                    fg=typer.colors.RED if annotations.returncode
                    else typer.colors.WHITE)
        for line in said[:-1][:4]:
            typer.secho(f"  {line}", fg=typer.colors.RED)

    if floor_broken and not as_json:
        typer.secho(f"  {floor_broken}", fg=typer.colors.RED, bold=True)

    code = (1 if regressed or dangling or unread or link.returncode
            or vtables.returncode or stale.returncode or symbols.returncode
            or index.returncode or floor_broken or debt.returncode
            or annotations.returncode or scaling.returncode
            else 3 if unasked else 0)
    # THE VERDICT GOES IN THE OUTPUT, not only in the exit code. The brief has
    # told agents for a long time never to pipe this to `tail`, because `cmd |
    # tail` reports TAIL's status and `echo $?` after it is always 0 - and an
    # agent was caught running exactly `osmx.py check 2>&1 | tail -5; echo
    # "EXIT:$?"` anyway, which would have called a red gate green. A rule
    # someone has to remember is not a check. This line is LAST, so it survives
    # any `tail`, and it states the verdict in words rather than leaving it to
    # a status nobody can see through a pipe.
    verdict = ("FAILED: THE TREE DOES NOT LINK" if link.returncode
               else "FAILED: the tree does more of the compiler's work" if vtables.returncode
               else "FAILED: an instruction names something that is not there" if stale.returncode
               else "FAILED: a marker names a symbol the build does not emit" if symbols.returncode
               else "FAILED: the address index grew" if index.returncode
               else "FAILED: semantic debt grew" if debt.returncode
               else "FAILED: an annotation does not survive being rewritten"
               if annotations.returncode
               else "FAILED: the claim floor does not match the tree" if floor_broken
               else "FAILED: regressed, dangling or unread claims" if code == 1
               else "OK, with unverifiable claims present" if code == 3
               else "CLEAN")

    if as_json:
        # Every input to `code` above, by the name the verdict uses.
        payload.update({
            "exit_code": code,
            "verdict": verdict,
            "failed": {
                "link": bool(link.returncode),
                "compiler_work": bool(vtables.returncode),
                "stale_references": bool(stale.returncode),
                "marker_symbols": bool(symbols.returncode),
                "address_index": bool(index.returncode),
                "class_debt": bool(debt.returncode),
                "annotation_identity": bool(annotations.returncode),
                "pointer_scale": bool(scaling.returncode),
                "claim_floor": bool(floor_broken),
                "regressed": bool(regressed),
                "dangling": bool(dangling),
                "unread_markers": bool(unread),
            },
            "unread_markers": [str(m) for m in unread],
            "restated": [str(m) for m in restated],
        })
        typer.echo(json.dumps(payload, indent=2))
        raise typer.Exit(code)

    typer.secho(
        f"GATE EXIT {code} - " + verdict,
        fg=typer.colors.RED if code == 1 else
        typer.colors.YELLOW if code == 3 else typer.colors.GREEN,
        bold=True)
    raise typer.Exit(code)


if __name__ == "__main__":
    app()
