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
import statistics
from enum import StrEnum
from pathlib import Path
from typing import Annotated

import typer

from decomp import DecompilationState, State, read

REPO_ROOT = Path(__file__).resolve().parent.parent

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


if __name__ == "__main__":
    app()
