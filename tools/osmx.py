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

import json
import re
import statistics
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from decomp import DecompilationState, State, mangled, read

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
    claimants = _matching(records, target)
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
    from decomp.asm import original_asm, shared_spans, span_refusal
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


if __name__ == "__main__":
    app()
