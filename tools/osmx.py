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
import json
import os
import re
import shutil
import subprocess
import statistics
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from decomp import DecompilationState, State, mangled, read, write_file
from decomp.record import stamped
from decomp.asm import (AsmComparison, CompileFailed, build_command,
                        compare_record, compare_source, compare_subject,
                        compile_unit, original_asm, shared_spans,
                        span_refusal, subject_asm)

REPO_ROOT = Path(__file__).resolve().parent.parent

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
    return [r for r in claimants if fragment in str(r.path)]


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
        scoped = f"?{member}@{owner}@@"
        return [r for r in records if r.name.startswith(scoped)
                or (r.symbol and r.symbol.startswith(scoped))]

    return [r for r in records
            if target in (mangled.identifier(r.name),
                          mangled.identifier(r.symbol))]


def _lessons(record: DecompilationState) -> list[tuple[str, str]]:
    out = [("LEVER", f"{key}  {prose}") for key, prose in record.levers]
    out += [("RULED-OUT", p) for p in record.ruled_out]
    out += [("UNRECOVERABLE", p) for p in record.unrecoverable]
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
    if entry["extract_error"]:
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
        typer.secho(
            f"{len(claimants)} pieces match {target!r}; measure names one"
            + ("" if in_file else " - --in <path fragment> picks one"),
            fg=typer.colors.RED)
        for piece in sorted(claimants, key=lambda r: str(r.path)):
            typer.echo(f"  {piece.location}")
        raise typer.Exit(2)
    record = claimants[0]
    command = _command_for(compile_commands, record.path, borrow)
    shared = shared_spans(records)

    if directory is not None:
        _rank_candidates(record, exe, directory, command, shared, jobs,
                         as_json)
        return

    source = body if body is not None else record.path
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
            typer.secho(f"{len(claimants)} pieces match {target!r}; "
                        f"record names one"
                        + ("" if in_file else " - --in picks one"),
                        fg=typer.colors.RED)
            raise typer.Exit(2)
        chosen.append(claimants[0])

    # MEASURED FIRST, WRITTEN AFTER, and grouped by file. Writing one record
    # at a time re-reads its file, and a first write may canonicalise a
    # wrapped lesson onto one line - which moves every line below it and
    # leaves the records read before it pointing at the wrong ones.
    updated, regressed, results = [], [], []
    for piece in chosen:
        command = _command_for(compile_commands, piece.path, borrow)
        try:
            result = compare_record(piece, exe, command, FLAG_SETS, shared)
        except (ValueError, CompileFailed) as problem:
            typer.secho(f"{piece.address_hex}: {problem}",
                        fg=typer.colors.RED)
            raise typer.Exit(2) from None
        results.append((piece, result))
        after = stamped(piece, result, demote)
        if piece.byte_exact and str(result.verdict) != "BYTE_EXACT":
            regressed.append((piece, result))
        if after != piece:
            updated.append(after)

    if updated:
        write_file(updated)

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


# WHAT THE BUILD IS CONFIGURED WITH, read off the cache the first time and
# stated here so a wiped cache can be rebuilt identically. `Debug` is not a
# choice this makes - it is what the tree configures - and the optimisation
# flags a match needs are NOT these: see FLAG_SETS.
CMAKE_ARGUMENTS = (
    "-G", "Unix Makefiles",
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


def _claims_by_file(records: list) -> dict:
    grouped: dict = {}
    for record in records:
        if record.byte_exact:
            grouped.setdefault(record.path, []).append(record)
    return grouped


def _claim_key(record: DecompilationState) -> tuple:
    """What identifies a CLAIM. Not the address.

    THIRTY-FOUR ADDRESSES IN THIS TREE ARE ANNOTATED TWICE - the body in
    `src/` and the same piece preserved in `src/recovered/` or
    `src/unrecovered/` - and they are two claims, measured in two
    translation units, that can disagree. Keyed by address, the second
    result overwrote the first, so `check` reported one verdict for two
    claims and counted the twin it never showed among the reproduced.
    """
    return (record.path, record.address)


def _check_one_file(job: tuple) -> list[tuple[tuple, str, str]]:
    """Score every claim in one file. Runs in a worker process.

    ONE COMPILE PER FLAG SET, NOT PER RECORD, and it stops as soon as every
    claim in the file has been proved: this tree's claims are 626 across 96
    files, so per record is 2,504 compiles and per file is at most 384 -
    usually far fewer, because most bodies settle on the first invocation.
    """
    path, records, exe, command, flag_sets = job
    best: dict = {}
    outstanding = list(records)
    diagnostic = ""
    for flags in flag_sets:
        if not outstanding:
            break
        try:
            obj = compile_unit(path, command, flags)
        except CompileFailed as failed:
            diagnostic = diagnostic or str(failed)
            continue
        for record in list(outstanding):
            try:
                compiled = subject_asm(obj, record, flags)
            except ValueError as problem:
                best[_claim_key(record)] = ("UNRESOLVED",
                                            str(problem))
                outstanding.remove(record)
                continue
            result = compare_subject(record, exe, compiled)
            verdict = str(result.verdict)
            if (_claim_key(record) not in best
                    or best[_claim_key(record)][0] != "BYTE_EXACT"):
                best[_claim_key(record)] = (verdict, "")
            if verdict == "BYTE_EXACT":
                outstanding.remove(record)
    for record in outstanding:
        best.setdefault(_claim_key(record), ("NO_COMPILE", diagnostic))
    return [(key, verdict, note) for key, (verdict, note) in best.items()]


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
    grouped = _claims_by_file(records)
    claims = sum(len(v) for v in grouped.values())
    if not claims:
        typer.secho(f"no BYTE_EXACT claims under {src}",
                    fg=typer.colors.RED)
        raise typer.Exit(2)

    jobs = max(1, min(jobs or (os.cpu_count() or 1), WINE_CEILING))

    work = []
    for path, mine in grouped.items():
        command = _command_for(compile_commands, path, borrow)
        work.append((path, mine, exe, command, FLAG_SETS))

    if not as_json:
        # `--json` must emit JSON and nothing else, or it is not
        # machine-readable - which is the only reason it exists.
        typer.echo(f"{claims:,} claims in {len(grouped):,} files, "
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
    UNASKED = ("NO_COMPILE", "UNRESOLVED", "SHARED_TAIL", "REFUSED")
    regressed = [(by_claim[k], v, n)
                 for k, (v, n) in sorted(measured.items())
                 if v != "BYTE_EXACT" and v not in UNASKED]
    unasked = [(by_claim[k], v, n)
               for k, (v, n) in sorted(measured.items()) if v in UNASKED]
    broken = regressed + unasked

    if as_json:
        def rows(items):
            return [{"address": r.address_hex, "name": r.name,
                     "location": str(r.location), "measured": v, "note": n}
                    for r, v, n in items]
        typer.echo(json.dumps({
            "claims": claims,
            "reproduced": claims - len(broken),
            "regressed": rows(regressed),
            "unverifiable": rows(unasked),
        }, indent=2))
    else:
        for record, verdict, note in regressed:
            typer.secho(f"REGRESSED {record.address_hex} claims BYTE_EXACT, "
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
        typer.secho(
            f"{claims - len(broken):,} verified, {len(regressed):,} "
            f"REGRESSED, {len(unasked):,} unverifiable, of {claims:,} claims",
            fg=typer.colors.RED if regressed else
            typer.colors.YELLOW if unasked else typer.colors.GREEN, bold=True)
    # THREE OUTCOMES, THREE CODES. A regression means a body stopped
    # reproducing and someone must look at it; an unverifiable claim means
    # the gate could not ask, which is a different job - build the unit the
    # way it was built. Passing the second silently would leave 534 claims
    # unchecked behind a green gate, which is the shape this whole ratchet
    # exists to prevent.
    raise typer.Exit(1 if regressed else 3 if unasked else 0)


if __name__ == "__main__":
    app()
