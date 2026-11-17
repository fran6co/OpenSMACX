"""Align a TU's declaration-count pad block so counter-sensitive canaries keep their tie-breaks.

VC6's front end numbers every symbol it creates from one TU-wide counter: each
function declaration and extern variable adds one, each string literal adds one
(its ``$SGn`` label is that number), and type declarations and locals, both in
earlier functions and earlier in the same function, add more. Some register
and reload tie-breaks in the code generator depend on that counter modulo 64
(measured: reportif's ``on_iface_button_clicked`` repeats its exact/inexact
pattern every 64 extra declarations over 260 shifts). A function whose original
bytes need one of those tie-breaks is exact only when its counter residue is in
a fixed set, so any header change before it can flip it.

The residue is read from a fresh object: the number of the first ``$SGn``
label the canary function's body relocates against, modulo 64. Source owns the
target set beside the pad block that tunes it::

    // CANARY: 0x004A6330 ?on_iface_button_clicked@ReportIf@@UAEXH@Z residues 28-30,39-41,54-56
    // CANARY-PADS: function ReportIfDeclFnPad
    void ReportIfDeclFnPad0();
    void ReportIfDeclFnPad1();

``CANARY`` lines belong to the ``CANARY-PADS`` line below them in the same
uninterrupted run of ``//`` comment lines. The pad lines follow it directly and
are numbered from zero; ``function`` pads are ``void <prefix><n>();`` and
``extern`` pads are ``extern int <prefix><n>;``. Each pad moves the counter by
one for everything after the block, so the needed count is arithmetic: the
smallest change that brings every canary of the block into its set. A unit may
have at most one pad block, and its canaries must follow it in the TU.

This is a matching aid, not evidence about the original's declarations: it
keeps tie-breaks stable while the true declaration set is unknown. ``--write``
compiles the proposed pad count as an overlay first and rewrites only the pad
lines when every canary lands in its set. ``--calibrate`` shifts the block over
one full period and reports, per canary, the residues at which its body keeps
its current bytes: a candidate target set for an exact current state, to review
before it replaces the recorded one (a set can move when the canary's own body,
or what it expands, changes).
"""

import re
from pathlib import Path

from .compiler import Compiler
from .mapping_check import build_units
from .measurement import Measurement
from .model import Result
from .pe_coff import Coff
from .records import read_json
from .source.publication import publish
from .source_replica import artifact_parent


PERIOD = 64
STATIC_STRING = re.compile(r"\$SG([0-9]+)", re.ASCII)
CANARY = re.compile(r"[ \t]*// CANARY: (0x[0-9A-Fa-f]{8}) (\S+) residues ([0-9,\-]+)[ \t]*", re.ASCII)
PADS = re.compile(r"[ \t]*// CANARY-PADS: (function|extern) ([A-Za-z_][A-Za-z_0-9]*)[ \t]*", re.ASCII)
COMMENT = re.compile(r"[ \t]*//.*")
FORMS = {"function": "void {prefix}{index}();", "extern": "extern int {prefix}{index};"}


def parse_residues(text: str) -> frozenset[int]:
    """``a-b,c`` ranges of residues modulo 64; every value must be in range."""
    values = set()
    for part in text.split(","):
        match = re.fullmatch(r"([0-9]+)(?:-([0-9]+))?", part)
        if not match:
            raise ValueError(f"invalid residue list: {text}")
        low = int(match.group(1))
        high = int(match.group(2) or low)
        if not 0 <= low <= high < PERIOD:
            raise ValueError(f"residues must be ascending values below {PERIOD}: {text}")
        values.update(range(low, high + 1))
    return frozenset(values)


def format_residues(values) -> str:
    runs, ordered = [], sorted(values)
    for value in ordered:
        if runs and value == runs[-1][1] + 1:
            runs[-1][1] = value
        else:
            runs.append([value, value])
    return ",".join(str(low) if low == high else f"{low}-{high}" for low, high in runs)


def parse_block(text: str, path: str = "<source>") -> dict | None:
    """The unit's pad block, or None; malformed or repeated blocks are refused."""
    lines = text.split("\n")
    blocks = [index for index, line in enumerate(lines) if PADS.fullmatch(line.rstrip("\r"))]
    if not blocks:
        if any(CANARY.fullmatch(line.rstrip("\r")) for line in lines):
            raise ValueError(f"{path}: CANARY lines without a CANARY-PADS block")
        return None
    if len(blocks) > 1:
        raise ValueError(f"{path}: more than one CANARY-PADS block")
    at = blocks[0]
    kind, prefix = PADS.fullmatch(lines[at].rstrip("\r")).groups()
    canaries, index = [], at - 1
    while index >= 0 and COMMENT.fullmatch(lines[index].rstrip("\r")):
        match = CANARY.fullmatch(lines[index].rstrip("\r"))
        if match:
            canaries.append({"line": index + 1, "address": match.group(1), "symbol": match.group(2),
                             "residues": parse_residues(match.group(3))})
        index -= 1
    attached = {canary["line"] for canary in canaries}
    stray = [number + 1 for number, line in enumerate(lines)
             if CANARY.fullmatch(line.rstrip("\r")) and number + 1 not in attached]
    if stray:
        raise ValueError(f"{path}: CANARY lines not attached to the pad block: {stray}")
    if not canaries:
        raise ValueError(f"{path}: CANARY-PADS block has no CANARY line above it")
    pattern = re.compile(re.escape(FORMS[kind]).replace(r"\{prefix\}", re.escape(prefix))
                         .replace(r"\{index\}", "([0-9]+)"))
    count = 0
    while at + 1 + count < len(lines):
        match = pattern.fullmatch(lines[at + 1 + count].rstrip("\r"))
        if not match:
            break
        if int(match.group(1)) != count:
            raise ValueError(f"{path}: pad {prefix}{match.group(1)} out of sequence at line {at + 2 + count}")
        count += 1
    canaries.reverse()
    return {"line": at + 1, "kind": kind, "prefix": prefix, "count": count, "canaries": canaries}


def rewrite_pads(text: str, block: dict, count: int) -> str:
    """Replace exactly the block's pad lines with ``count`` pads, keeping the line ending."""
    lines = text.split("\n")
    start = block["line"]
    ending = "\r" if lines[start - 1].endswith("\r") else ""
    pads = [FORMS[block["kind"]].format(prefix=block["prefix"], index=index) + ending for index in range(count)]
    return "\n".join(lines[:start] + pads + lines[start + block["count"]:])


def first_literal(obj: Coff, symbol: str) -> int | None:
    """Number of the first ``$SGn`` label the named function body relocates against."""
    definitions = [entry for entry in obj.symbols.values()
                   if entry.name == symbol and entry.section > 0 and entry.type == 0x20]
    if len(definitions) != 1:
        raise ValueError(f"object defines {symbol} {len(definitions)} times")
    section = obj.sections[definitions[0].section - 1]
    for offset, index, _ in sorted(section.relocations):
        match = STATIC_STRING.fullmatch(obj.symbols[index].name)
        if match:
            return int(match.group(1))
    return None


def solve(current: int, residues: list[tuple[int, frozenset[int]]]) -> int | None:
    """Pad count nearest ``current`` putting every ``(residue, targets)`` pair in its set."""
    for distance in range(PERIOD):
        for count in (current + distance, current - distance):
            if count >= 0 and all((residue + count - current) % PERIOD in targets
                                  for residue, targets in residues):
                return count
    return None


def _objects(compiler, root: Path, unit: str, flags_list, data: bytes | None) -> list[Coff]:
    """Fresh objects for every configured trial; ``data`` compiles proposed unit bytes as an overlay."""
    objects = []
    for flags in flags_list:
        raw = (compiler(root / unit, (), flags) if data is None
               else compiler.overlay(root / unit, {unit: data}, (), flags))
        if not isinstance(raw, bytes) or not raw:
            raise ValueError("compiler returned no object bytes")
        objects.append(Coff(raw))
    if not objects:
        raise ValueError("no configured compiler trial")
    return objects


def _body(obj: Coff, symbol: str) -> bytes:
    definitions = [entry for entry in obj.symbols.values()
                   if entry.name == symbol and entry.section > 0 and entry.type == 0x20]
    if len(definitions) != 1:
        raise ValueError(f"object defines {symbol} {len(definitions)} times")
    return obj.sections[definitions[0].section - 1].data


def _measure(objects: list[Coff], block: dict) -> list[dict]:
    """Each canary's residue; configured trials must agree."""
    rows = []
    for canary in block["canaries"]:
        numbers = {first_literal(obj, canary["symbol"]) for obj in objects}
        if None in numbers:
            raise ValueError(f"{canary['symbol']} relocates against no $SG literal")
        if len(numbers) != 1:
            raise ValueError("configured compiler trials disagree on the literal counter")
        number = numbers.pop()
        rows.append({"address": canary["address"], "symbol": canary["symbol"], "line": canary["line"],
                     "targets": format_residues(canary["residues"]), "literal": f"$SG{number}",
                     "residue": number % PERIOD, "aligned": number % PERIOD in canary["residues"]})
    return rows


def _calibrate(compiler, root: Path, unit: str, flags_list, text: str, block: dict) -> list[dict]:
    """Residues at which each canary compiles to its current bytes, over one full period.

    The current source is assumed to be the exact state; the result is a
    candidate target set to review, not a measurement against the image.
    """
    current = _objects(compiler, root, unit, flags_list, None)
    expected = [_body(current[0], canary["symbol"]) for canary in block["canaries"]]
    found = [set() for _ in block["canaries"]]
    for shift in range(PERIOD):
        data = rewrite_pads(text, block, block["count"] + shift).encode("utf-8", errors="surrogateescape")
        objects = _objects(compiler, root, unit, flags_list, data)
        for index, canary in enumerate(block["canaries"]):
            number = first_literal(objects[0], canary["symbol"])
            if number is None:
                raise ValueError(f"{canary['symbol']} relocates against no $SG literal")
            if all(_body(obj, canary["symbol"]) == expected[index] for obj in objects):
                found[index].add(number % PERIOD)
    return [{"address": canary["address"], "symbol": canary["symbol"], "line": canary["line"],
             "recorded": format_residues(canary["residues"]), "calibrated": format_residues(values)}
            for canary, values in zip(block["canaries"], found)]


def align_canaries(root: Path, *, units=None, write=False, calibrate=False, compiler=None) -> dict:
    if write and calibrate:
        raise ValueError("--calibrate reports candidate target sets and never writes")
    root = root.resolve()
    settings = Measurement.read(root)
    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("canary alignment requires a configured compilation database")
    build, _ = build_units(root, settings.source.path(root, mappings["compile_commands"]).read_bytes())
    requested = sorted({Path(name).as_posix() for name in units or ()})
    unknown = [name for name in requested if name not in build]
    if unknown:
        raise ValueError("not configured build translation units: " + ", ".join(unknown))
    candidates = requested or sorted(name for name in build if Path(name).suffix in settings.unit_extensions)
    raws, blocks = {}, {}
    for unit in candidates:
        raw = (root / unit).read_bytes()
        if b"CANARY" not in raw:
            continue
        block = parse_block(raw.decode("utf-8", errors="surrogateescape"), unit)
        if block is not None:
            raws[unit], blocks[unit] = raw, block
    missing = [name for name in requested if name not in blocks]
    if missing:
        raise ValueError("no CANARY-PADS block in: " + ", ".join(missing))
    if compiler is None:
        compiler = Compiler(root, profile=settings.compiler_profile)
    metadata = {"operation": "canary", "units": sorted(blocks)}
    before = settings.snapshot(root, metadata)

    def fresh():
        if settings.snapshot(root, metadata) != before or any(
                (root / unit).read_bytes() != raw for unit, raw in raws.items()):
            raise ValueError("source or compiler inputs changed during canary alignment")

    reports, published = [], []
    for unit in sorted(blocks):
        block, flags = blocks[unit], settings.flags_for(unit)
        text = raws[unit].decode("utf-8", errors="surrogateescape")
        report = {"unit": unit, "line": block["line"], "pads": block["count"]}
        try:
            if calibrate:
                report["canaries"] = _calibrate(compiler, root, unit, flags, text, block)
                report["result"] = (Result.PASS if all(row["recorded"] == row["calibrated"]
                                                       for row in report["canaries"]) else Result.FAIL)
                reports.append(report)
                continue
            rows = _measure(_objects(compiler, root, unit, flags, None), block)
        except ValueError as error:
            report.update(result=Result.UNKNOWN, reason=str(error))
            reports.append(report)
            continue
        report["canaries"] = rows
        needed = solve(block["count"], [(row["residue"], canary["residues"])
                                        for row, canary in zip(rows, block["canaries"])])
        report["needed"] = needed
        if needed is None:
            report.update(result=Result.FAIL, reason="no pad count aligns every canary of the block")
        elif needed == block["count"]:
            report["result"] = Result.PASS
        elif not write:
            report["result"] = Result.FAIL
        else:
            replacement = rewrite_pads(text, block, needed).encode("utf-8", errors="surrogateescape")
            try:
                verified = _measure(_objects(compiler, root, unit, flags, replacement),
                                    parse_block(replacement.decode("utf-8", errors="surrogateescape"), unit))
            except ValueError as error:
                verified, reason = None, str(error)
            else:
                reason = None if all(row["aligned"] for row in verified) else \
                    "the proposed pad count did not land every canary in its set"
            report["canaries_after"] = verified
            if reason:
                report.update(result=Result.UNKNOWN, reason=reason)
            else:
                fresh()
                staging = artifact_parent(root, "build/recovery/publications",
                                          excluded_roots=settings.source.roots)
                publish({root / unit: raws[unit]}, {root / unit: replacement}, validate=fresh,
                        staging_directory=staging)
                raws[unit] = replacement
                before = settings.snapshot(root, metadata)
                published.append(unit)
                report.update(result=Result.PASS, written=needed)
        reports.append(report)
    fresh()
    results = {report["result"] for report in reports}
    result = (Result.UNKNOWN if Result.UNKNOWN in results else Result.FAIL if Result.FAIL in results
              else Result.PASS)
    return {"schema_version": 1, "operation": "canary-calibrate" if calibrate else "canary",
            "result": result, "write": bool(write), "published": published, "units": reports,
            "limitations": ["a matching aid for counter-sensitive tie-breaks, not evidence of the "
                            "original declaration set",
                            *(["calibration assumes the current source compiles each canary exactly; "
                               "it reports candidate sets and never compares with the image"] if calibrate else [])]}
