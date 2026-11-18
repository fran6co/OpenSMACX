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
keeps tie-breaks stable while the true declaration set is unknown.

Recorded sets drift: a set measured at one revision moved three times in a day
as other functions in the same unit changed, because the tie depends on more
than the residue at the function's first literal. So the recorded set is only
a prediction, and every verdict here comes from the pinned image:

- the default run measures each canary against the image at the current pad
  count, and when one misses, predicts a count from the recorded sets and
  measures that prediction too;
- ``--write`` publishes a predicted count only when the prediction measures
  BYTE_EXACT against the image, and otherwise refuses and asks for a sweep;
- a body that matches but whose only strict-proof problem is stale ``$SGn``
  literal bindings (every pad renumbers them) counts as exact; run
  ``recover rebind-literals --write`` after writing a count;
- ``--sweep`` measures one full period of pad counts (64 overlay compiles)
  against the image, takes the count nearest the current one at which every
  canary of the block is exact, and with ``--write`` writes that count and the
  measured residue sets into the ``CANARY`` lines.

Run ``--sweep --write`` after each header batch.
"""

import re
from pathlib import Path

from .compiler import Compiler
from .mapping_check import build_units
from .measurement import Measurement
from .model import Result
from .pe_coff import Coff
from .records import read_json
from .source.model import Tier
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


def rewrite_residues(text: str, block: dict, sets: dict) -> str:
    """Replace the residue list of each CANARY line named by ``{line: residues}``."""
    lines = text.split("\n")
    for canary in block["canaries"]:
        if canary["line"] not in sets:
            continue
        index = canary["line"] - 1
        line = lines[index]
        ending = "\r" if line.endswith("\r") else ""
        match = CANARY.fullmatch(line.rstrip("\r"))
        if not match:
            raise ValueError(f"line {canary['line']} is no longer a CANARY line")
        start, end = match.span(3)
        lines[index] = line[:start] + format_residues(sets[canary["line"]]) + line.rstrip("\r")[end:] + ending
    return "\n".join(lines)


def image_judge(root: Path, settings, image: Path, compiler):
    """``judge(unit, data, canaries) -> {address: BYTE_EXACT?}`` against the pinned image.

    ``data`` is the proposed unit text (an overlay; the file is not written),
    or None for the file as it is. Each canary's own source mapping is measured
    with the same per-unit comparer as ``measure``.
    """
    from .source.asm import compare_unit, shared_spans
    from .source_inventory import source_snapshot
    from .source_selection import (eh_fragment_group, measurement_reference_targets, select_records,
                                   selected_source_records, snapshot_mappings)
    snapshot = source_snapshot(root, allow_empty=True)
    mappings = snapshot_mappings(root, snapshot)
    shared = shared_spans(mappings)
    references = measurement_reference_targets(root, settings, snapshot)

    def judge(unit: str, data: bytes | None, canaries) -> dict:
        path = (root / unit).resolve()
        grid = tuple(tuple(flags) for flags in settings.flags_for(unit))
        cache = {}

        def compile_trial(source, arguments, trial):
            key = (Path(source).resolve(), tuple(arguments), tuple(trial))
            if key not in cache:
                if Path(source).resolve() == path and data is not None and not arguments:
                    cache[key] = bytes(compiler.overlay(path, {unit: data}, (), tuple(trial)))
                else:
                    cache[key] = bytes(compiler(source, arguments, tuple(trial)))
            return cache[key]

        verdicts = {}
        for canary in canaries:
            selected = select_records(mappings, [canary["address"]], path, settings.source.symbol_format)
            record, = selected_source_records(root, snapshot, selected)
            group = eh_fragment_group(root, settings, snapshot, mappings, selected[0])
            measured = compare_unit([record], image, path, (), grid, shared, compiler=compile_trial, root=root,
                                    targets=references, fragment_pairs=(group,) if group else (),
                                    diagnostics=True)
            comparison = measured.results[0][1]
            verdicts[canary["address"]] = (not isinstance(comparison, str)
                                           and (comparison.verdict is Tier.BYTE_EXACT
                                                or literal_binding_only(comparison)))
        return verdicts
    return judge


_LITERAL_ONLY = ("strict body proof unknown: unbound compiler artifact relocation(s): ",
                 "strict body proof failed: relocated target(s) differ: ")


def literal_binding_only(comparison) -> bool:
    """True when the body matches and only ``$SGn`` literal bindings disagree.

    Every pad renumbers the literals after the block, so a shifted count leaves
    the unit's RECOVERY_LITERAL lines stale by construction; that is fixed by
    ``recover rebind-literals``, not by the pad count. Anything else in the
    strict diagnostic, a named non-literal target, or masked bytes that are not
    BYTE_EXACT keeps the verdict a miss.
    """
    text = comparison.diagnostic or ""
    if not text.startswith(_LITERAL_ONLY):
        return False
    parts = [part.strip() for part in text.split(";")]
    head, rest = parts[0], parts[1:]
    if any("rebind-literals" not in part for part in rest):
        return False
    named = re.findall(r"\+0x[0-9a-f]+ (\S+?)(?:,|$| and| \()", head)
    if not named or not all(re.fullmatch(r"\$SG[0-9]+", name) for name in named):
        return False
    payload = comparison.diagnostic_comparison or {}
    return payload.get("available") is True and payload.get("masked_verdict") == str(Tier.BYTE_EXACT)


def _residues(compiler, root: Path, unit: str, flags_list, block: dict, data: bytes | None) -> list[int]:
    numbers = []
    objects = _objects(compiler, root, unit, flags_list, data)
    for canary in block["canaries"]:
        found = {first_literal(obj, canary["symbol"]) for obj in objects}
        if None in found:
            raise ValueError(f"{canary['symbol']} relocates against no $SG literal")
        if len(found) != 1:
            raise ValueError("configured compiler trials disagree on the literal counter")
        numbers.append(found.pop() % PERIOD)
    return numbers


def _sweep(compiler, judge, root: Path, unit: str, flags_list, text: str, block: dict) -> dict:
    """Measure one full period of pad counts around the current one against the image."""
    base = max(0, block["count"] - PERIOD // 2)
    rows = []
    for count in range(base, base + PERIOD):
        data = rewrite_pads(text, block, count).encode("utf-8", errors="surrogateescape")
        residues = _residues(compiler, root, unit, flags_list, block, data)
        verdicts = judge(unit, data, block["canaries"])
        rows.append({"count": count, "residues": residues,
                     "exact": [verdicts[canary["address"]] for canary in block["canaries"]]})
    sets = {canary["line"]: {row["residues"][index] for row in rows if row["exact"][index]}
            for index, canary in enumerate(block["canaries"])}
    good = [row["count"] for row in rows if all(row["exact"])]
    best = min(good, key=lambda count: (abs(count - block["count"]), -count)) if good else None
    return {"rows": rows, "sets": sets, "needed": best}


def align_canaries(root: Path, *, units=None, image=None, write=False, sweep=False, compiler=None,
                   judge=None) -> dict:
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
    if judge is None:
        if image is None:
            raise ValueError("canary verdicts need the pinned original image (--image)")
        import hashlib
        from .source_inventory import image_pin
        image = Path(image) if Path(image).is_absolute() else root / image
        if hashlib.sha256(image.read_bytes()).hexdigest() != image_pin(root, settings.source):
            raise ValueError("image does not match the source pin")
        judge = image_judge(root, settings, image, compiler)
    metadata = {"operation": "canary", "units": sorted(blocks)}
    before = settings.snapshot(root, metadata)

    def fresh():
        if settings.snapshot(root, metadata) != before or any(
                (root / unit).read_bytes() != raw for unit, raw in raws.items()):
            raise ValueError("source or compiler inputs changed during canary alignment")

    def publish_unit(unit, replacement):
        nonlocal before
        fresh()
        staging = artifact_parent(root, "build/recovery/publications", excluded_roots=settings.source.roots)
        publish({root / unit: raws[unit]}, {root / unit: replacement}, validate=fresh, staging_directory=staging)
        raws[unit] = replacement
        before = settings.snapshot(root, metadata)
        published.append(unit)

    reports, published = [], []
    for unit in sorted(blocks):
        block, flags = blocks[unit], settings.flags_for(unit)
        text = raws[unit].decode("utf-8", errors="surrogateescape")
        report = {"unit": unit, "line": block["line"], "pads": block["count"]}
        reports.append(report)
        try:
            if sweep:
                found = _sweep(compiler, judge, root, unit, flags, text, block)
                needed = found["needed"]
                report["needed"] = needed
                report["canaries"] = [
                    {"address": canary["address"], "symbol": canary["symbol"], "line": canary["line"],
                     "recorded": format_residues(canary["residues"]),
                     "measured": format_residues(found["sets"][canary["line"]])}
                    for canary in block["canaries"]]
                if needed is None:
                    report.update(result=Result.FAIL, reason="no pad count in one period makes every canary exact")
                    continue
                stale = any(row["recorded"] != row["measured"] for row in report["canaries"])
                if not write:
                    report["result"] = Result.PASS if needed == block["count"] and not stale else Result.FAIL
                    continue
                replacement = rewrite_residues(rewrite_pads(text, block, needed), block,
                                               {line: values for line, values in found["sets"].items() if values})
                replacement = replacement.encode("utf-8", errors="surrogateescape")
                if replacement != raws[unit]:
                    publish_unit(unit, replacement)
                report.update(result=Result.PASS, written=needed,
                              next=f"recover rebind-literals --unit {unit} --write")
                continue
            residues = _residues(compiler, root, unit, flags, block, None)
            verdicts = judge(unit, None, block["canaries"])
            report["canaries"] = [
                {"address": canary["address"], "symbol": canary["symbol"], "line": canary["line"],
                 "targets": format_residues(canary["residues"]), "residue": residue,
                 "aligned": residue in canary["residues"], "exact": verdicts[canary["address"]]}
                for canary, residue in zip(block["canaries"], residues)]
            if all(row["exact"] for row in report["canaries"]):
                report.update(result=Result.PASS, needed=block["count"])
                if not all(row["aligned"] for row in report["canaries"]):
                    report["stale"] = "exact outside a recorded set; run --sweep --write"
                continue
            needed = solve(block["count"], [(row["residue"], canary["residues"])
                                            for row, canary in zip(report["canaries"], block["canaries"])])
            report["needed"] = needed
            if needed is None:
                report.update(result=Result.UNKNOWN, reason="no count fits the recorded sets; run --sweep")
                continue
            replacement = rewrite_pads(text, block, needed).encode("utf-8", errors="surrogateescape")
            predicted = judge(unit, replacement, block["canaries"])
            report["prediction_exact"] = all(predicted.values())
            if not report["prediction_exact"]:
                report.update(result=Result.UNKNOWN,
                              reason="the recorded residue sets are stale: the predicted count does not measure "
                                     "BYTE_EXACT against the image; run --sweep")
            elif not write:
                report["result"] = Result.FAIL
            else:
                publish_unit(unit, replacement)
                report.update(result=Result.PASS, written=needed,
                              next=f"recover rebind-literals --unit {unit} --write")
        except ValueError as error:
            report.update(result=Result.UNKNOWN, reason=str(error))
    fresh()
    results = {report["result"] for report in reports}
    result = (Result.UNKNOWN if Result.UNKNOWN in results else Result.FAIL if Result.FAIL in results
              else Result.PASS)
    return {"schema_version": 1, "operation": "canary-sweep" if sweep else "canary",
            "result": result, "write": bool(write), "published": published, "units": reports,
            "limitations": ["a matching aid for counter-sensitive tie-breaks, not evidence of the "
                            "original declaration set",
                            "verdicts are masked-byte BYTE_EXACT against the pinned image, as measure reports"]}
