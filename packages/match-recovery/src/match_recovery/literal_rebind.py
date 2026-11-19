"""Rebind stale positional ``$SGn`` literal symbols from fresh compiler emission.

Unpooled VC6 string literals are static ``$SGn`` labels numbered by emission
order across the whole TU, headers included. A declaration added to a shared
header renumbers every includer's labels, so a source-owned
``RECOVERY_LITERAL`` binding then names a missing or different label. The
original correspondence (start, end, evidence) is the source author's reviewed
interpretation and is never changed here; only the emitted-symbol fact is
re-identified.

Evidence: a fresh configured compilation of the owning TU, and the pinned
original's bytes for the declared extent. A candidate label must pass the same
storage check as measurement (``literal_storage``: ordinary ``.data``/``.bss``,
no relocation or alias, terminator inside the declared bound and only zero
alignment padding to the next label) and hold exactly the original bytes.
Identical unpooled duplicates are disambiguated only by the TU's existing
bindings: a uniform label shift that every proven binding admits, otherwise a
unique assignment that keeps their relative label order. Anything still
ambiguous, or disagreeing across configured trials, is reported, not guessed.

Before that, identical duplicates are narrowed by the functions that use the
literal: the binding's evidence function (its fourth field) and the mapped
function enclosing its line, when the unit maps and compiles them and their
original bodies address the literal. Each fresh body's ``$SGn`` relocations are
matched against the original body's absolute operand sites for the literal. A
label relocated at the same body offset as such a site decides; otherwise a
label that is the only content-equal candidate the function references at all.
Every such function must decide, and all must agree, or nothing is narrowed.

A pooled ``??_C@`` name is stable while its TU pools strings. When the TU's
recipe stops pooling, the fresh object no longer defines that name: the literal
is then a static label too. Such a stale pooled binding is re-identified by
content, and identical duplicates by layout only: a TU's unpooled literals keep
their relative offsets from object to image, so a candidate is accepted when
its (original address - object offset) equals that of the nearest binding that
is already resolved. A pooled name the fresh object still defines is skipped.
"""

import hashlib
import re
from collections import Counter
from dataclasses import replace
from pathlib import Path

from .compiler import Compiler
from .literal_data import check_literals, literal_storage
from .literal_source import read_literals, validate_literals
from .literal_writer import rewrite_literal_symbols
from .mapping_check import build_units
from .match_scope import affected_units, changed_files
from .measurement import Measurement
from .model import Result, Span
from .pe_coff import Coff, PeImage
from .records import read_json
from .source.publication import publish
from .source.reader import read_file
from .source_inventory import image_pin
from .source_replica import artifact_parent


STATIC_STRING = re.compile(r"\$SG([0-9]+)", re.ASCII)
POOLED_PREFIX = "??_C@"


def _number(symbol: str) -> int:
    return int(STATIC_STRING.fullmatch(symbol).group(1))


def _labels(obj) -> dict[str, object]:
    """Defined static ``$SGn`` labels, by name; duplicates are left to storage checks."""
    return {symbol.name: symbol for symbol in obj.symbols.values()
            if STATIC_STRING.fullmatch(symbol.name) and symbol.section > 0 and symbol.storage == 3}


def _candidates(binding, obj, labels, image) -> tuple[list[int], str | None]:
    """Label numbers whose measured storage holds exactly the original extent."""
    try:
        if image.absolute_sites(binding.span):
            return [], "original extent contains relocations; not a string literal"
        original = image.read_initial(binding.span)
    except ValueError as error:
        return [], str(error)
    size = binding.span.end - binding.span.start
    found = []
    for name, symbol in labels.items():
        section = obj.sections[symbol.section - 1]
        # Cheap byte prefilter before the complete storage proof.
        if section.data and section.data[symbol.value:symbol.value + size] != original:
            continue
        try:
            contents, _ = literal_storage(obj, name, size)
        except ValueError:
            continue
        if contents == original:
            found.append(_number(name))
    if not found:
        return [], "no fresh $SG label holds exactly the original bytes with only zero padding"
    return sorted(found), None


def _context_choice(binding, candidates, obj, image, context) -> tuple[int | None, str]:
    """One candidate proven by one original function's own relocations, else a reason.

    ``context`` is (emitted symbol, original body start, original body end).
    Only content-equal candidates are considered, so the choice can never name
    bytes that differ from the image.
    """
    name, start, end = context
    defined = {(item.section, item.value) for item in obj.symbols.values()
               if item.name == name and item.section > 0 and item.storage in (2, 3)}
    if len(defined) != 1:
        return None, f"{name} is not uniquely defined in the fresh object"
    (number, value), = defined
    section = obj.sections[number - 1]
    others = sorted(item.value for item in obj.symbols.values()
                    if item.section == number and item.storage in (2, 3) and item.value > value)
    limit = min(others, default=len(section.data))
    wanted = set(candidates)
    referenced = {}
    for offset, target, kind in section.relocations:
        match = STATIC_STRING.fullmatch(obj.symbols[target].name)
        if (kind == 6 and match and value <= offset < limit and int(match.group(1)) in wanted
                and int.from_bytes(section.data[offset:offset + 4], "little") == 0):
            referenced.setdefault(int(match.group(1)), set()).add(offset - value)
    exact = sorted(label for label, offsets in referenced.items()
                   if offsets & context_sites(binding, image, start, end))
    if len(exact) == 1:
        return exact[0], "context_relocation"
    if not exact and len(referenced) == 1:
        label, = referenced
        return label, "context_reference"
    return None, (f"{name} relocations do not separate the candidates: "
                  + (", ".join(f"$SG{label}" for label in sorted(referenced)) or "none referenced"))


def context_sites(binding, image, start, end) -> set[int]:
    """Body offsets of the original function's absolute operands addressing the literal."""
    return {offset for offset in image.absolute_sites(Span(start, end))
            if int.from_bytes(image.read(Span(start + offset, start + offset + 4)), "little")
            == binding.span.start}


def _narrowed(binding, candidates, obj, image, contexts, row) -> list[int]:
    """Narrow identical duplicates by the functions that use the literal.

    Two original functions can witness a binding: its evidence function (the
    fourth field) and the mapped function whose annotation encloses the
    binding's line. Each one whose original body addresses the literal must
    decide, and all deciders must agree; otherwise nothing is narrowed and the
    unit-wide shift/order rules (which refuse ambiguity) still apply.
    """
    if len(candidates) < 2 or contexts is None:
        return candidates
    by_address, markers = contexts
    addresses = [binding.evidence.address]
    before = [address for line, address in markers if line < binding.line]
    if before and before[-1] not in addresses:
        addresses.append(before[-1])
    choices, notes = set(), []
    for address in addresses:
        context = by_address.get(address)
        if context is None:
            notes.append(f"0x{address:08X} is not mapped in this unit")
            continue
        try:
            if not context_sites(binding, image, context[1], context[2]):
                notes.append(f"0x{address:08X} has no original operand addressing the literal")
                continue
        except ValueError as error:
            notes.append(f"0x{address:08X} original body is unreadable: {error}")
            continue
        choice, note = _context_choice(binding, candidates, obj, image, context)
        if choice is None:
            row["context"] = note
            return candidates
        choices.add(choice)
        notes.append(f"{note} 0x{address:08X}")
    if len(choices) != 1:
        row["context"] = ("context functions disagree: " if choices else "") + "; ".join(notes)
        return candidates
    choice, = choices
    row["context"] = f"$SG{choice} by " + "; ".join(note for note in notes if note.startswith("context_"))
    return [choice]


def _ordered(sets: list[set[int]]) -> list[set[int]]:
    """Prune candidates to those in some strictly increasing assignment."""
    sets = [set(item) for item in sets]
    floor = -1
    for item in sets:
        item.difference_update({value for value in item if value <= floor})
        floor = min(item) if item else floor
    ceiling = None
    for item in reversed(sets):
        if ceiling is not None:
            item.difference_update({value for value in item if value >= ceiling})
        ceiling = max(item) if item else ceiling
    return sets


def plan_unit(bindings, obj, image, contexts=None) -> tuple[list[dict], int | None]:
    """Return one row per ``$SG`` binding and the uniform shift applied, if any.

    Row status: ``valid`` (keeps its symbol), ``rebind`` (new symbol proven)
    or ``unproven`` (reason given; source is left alone).
    """
    bindings = sorted(bindings, key=lambda item: _number(item.symbol))
    labels = _labels(obj)
    rows, proven = [], []
    for binding in bindings:
        candidates, reason = _candidates(binding, obj, labels, image)
        row = {"line": binding.line, "start": f"0x{binding.span.start:08X}",
               "end": f"0x{binding.span.end:08X}", "previous": binding.symbol,
               "symbol": binding.symbol, "candidates": [f"$SG{value}" for value in candidates]}
        if reason is not None:
            row.update(status="unproven", reason=reason)
        else:
            proven.append((binding, _narrowed(binding, candidates, obj, image, contexts, row), row))
        rows.append(row)
    if not proven:
        return rows, None
    shifts = set.intersection(*({value - _number(binding.symbol) for value in candidates}
                                for binding, candidates, _ in proven))
    if 0 in shifts:
        # The current population is consistent; never move a valid binding.
        for _, _, row in proven:
            row["status"] = "valid"
        return rows, 0
    if len(shifts) == 1:
        shift, = shifts
        for binding, _, row in proven:
            row.update(status="rebind", symbol=f"$SG{_number(binding.symbol) + shift}",
                       basis="uniform_shift")
        return rows, shift
    if shifts:
        for _, _, row in proven:
            row.update(status="unproven", reason="several uniform label shifts fit identical duplicates: "
                       + ", ".join(f"{value:+d}" for value in sorted(shifts)))
        return rows, None
    # No single shift: keep valid bindings, then require a unique
    # order-preserving assignment for the rest.
    feasible = _ordered([set(candidates) for _, candidates, _ in proven])
    for (binding, _, _), item in zip(proven, feasible):
        if _number(binding.symbol) in item:
            item.intersection_update({_number(binding.symbol)})
    feasible = _ordered(feasible)
    if any(not item for item in feasible):
        # The labels no longer rise in the bindings' old order (definitions
        # were reordered in the source). Order then proves nothing, but a
        # binding whose bytes (or whose context function's relocation) leave
        # exactly one fresh label, claimed by no other binding, is still
        # identified by content alone. Identical duplicates stay unproven.
        claims = Counter(value for _, candidates, _ in proven for value in set(candidates))
        for binding, candidates, row in proven:
            if len(candidates) == 1 and claims[candidates[0]] == 1:
                value, = candidates
                if value == _number(binding.symbol):
                    row["status"] = "valid"
                else:
                    row.update(status="rebind", symbol=f"$SG{value}", basis="unique_content")
            else:
                row.update(status="unproven", reason="no assignment keeps the TU's existing binding order"
                           " and identical duplicate labels remain ambiguous: "
                           + ", ".join(f"$SG{value}" for value in sorted(candidates)))
        return rows, None
    for (binding, _, row), item in zip(proven, feasible):
        if len(item) != 1:
            row.update(status="unproven", reason="identical duplicate labels remain ambiguous after order constraints: "
                       + ", ".join(f"$SG{value}" for value in sorted(item)))
            continue
        value, = item
        if value == _number(binding.symbol):
            row["status"] = "valid"
        else:
            row.update(status="rebind", symbol=f"$SG{value}", basis="order_preserving")
    return rows, None


def _stale_pooled(bindings, obj) -> list:
    """Pooled-name bindings whose COMDAT the fresh object no longer defines."""
    defined = {symbol.name for symbol in obj.symbols.values() if symbol.section > 0}
    return [item for item in bindings
            if item.symbol.startswith(POOLED_PREFIX) and item.symbol not in defined]


def plan_pooled(bindings, static_rows, static_bindings, obj, image, contexts=None) -> list[dict]:
    """Rows for stale pooled bindings; ``static_rows`` supply layout anchors.

    A unique content match is proven by content. Identical duplicates are
    resolved only when exactly one candidate shares the displacement
    (original start - object offset, same section) of the nearest resolved
    binding by original address; anything else stays unproven.
    """
    labels = _labels(obj)

    def place(number):
        symbol = labels[f"$SG{number}"]
        return symbol.section, symbol.value

    resolved = []  # (original start, section, displacement)
    by_line = {item.line: item for item in static_bindings}
    for row in static_rows:
        if row["status"] in ("valid", "rebind") and row["symbol"] in labels:
            section, value = place(_number(row["symbol"]))
            resolved.append((by_line[row["line"]].span.start, section, by_line[row["line"]].span.start - value))
    rows, pending = [], []
    for binding in sorted(bindings, key=lambda item: item.span.start):
        candidates, reason = _candidates(binding, obj, labels, image)
        row = {"line": binding.line, "start": f"0x{binding.span.start:08X}",
               "end": f"0x{binding.span.end:08X}", "previous": binding.symbol,
               "symbol": binding.symbol, "candidates": [f"$SG{value}" for value in candidates]}
        rows.append(row)
        if reason is None:
            candidates = _narrowed(binding, candidates, obj, image, contexts, row)
        if reason is not None:
            row.update(status="unproven", reason=reason)
        elif len(candidates) == 1:
            section, value = place(candidates[0])
            row.update(status="rebind", symbol=f"$SG{candidates[0]}", basis="unpooled_content")
            resolved.append((binding.span.start, section, binding.span.start - value))
        else:
            pending.append((binding, candidates, row))
    progress = True
    while pending and progress:
        progress = False
        for entry in list(pending):
            binding, candidates, row = entry
            start = binding.span.start
            below = max((item for item in resolved if item[0] < start), default=None)
            above = min((item for item in resolved if item[0] > start), default=None)
            anchors = {item[1:] for item in (below, above) if item is not None}
            fits = [value for value in candidates
                    if (place(value)[0], start - place(value)[1]) in anchors]
            if len(fits) == 1:
                section, value = place(fits[0])
                row.update(status="rebind", symbol=f"$SG{fits[0]}", basis="unpooled_layout")
                resolved.append((start, section, start - value))
                pending.remove(entry)
                progress = True
    for _, _, row in pending:
        row.update(status="unproven",
                   reason="identical duplicate labels are not separated by the layout of a resolved neighbour: "
                   + ", ".join(row["candidates"]))
    return rows


def _verify(unit_bindings, rows, obj, image) -> None:
    """Recheck the proposed unit population exactly as measurement would."""
    changed = {row["line"]: row["symbol"] for row in rows if row["status"] == "rebind"}
    proposed = [replace(item, symbol=changed.get(item.line, item.symbol)) for item in unit_bindings]
    try:
        validate_literals(proposed)
    except ValueError as error:
        for row in rows:
            if row["status"] == "rebind":
                row.update(status="unproven", symbol=row["previous"],
                           reason=f"proposed population is invalid: {error}")
        return
    checked = [item for item in proposed if item.line in changed]
    _, results = check_literals(checked, {item.unit: obj for item in checked}, image)
    failed = {item.line: result.get("reason", "content check failed")
              for item, result in zip(checked, results) if result["content"] != Result.PASS}
    for row in rows:
        if row["line"] in failed:
            row.update(status="unproven", symbol=row["previous"],
                       reason=f"proposed label fails the literal check: {failed[row['line']]}")


def _unit_contexts(path: Path):
    """({original address: (emitted symbol, body start, body end)}, [(marker line, address)]) in ``path``."""
    by_address, markers = {}, []
    try:
        records = read_file(path)
    except (OSError, ValueError):
        return by_address, markers
    for record in records:
        if record.image_spans and (record.symbol or record.name):
            start, end = record.image_spans[0]
            by_address.setdefault(record.address, (record.symbol or record.name, start, end))
            markers.append((record.line, record.address))
    return by_address, sorted(markers)


def rebind_literals(root: Path, image_path: Path, *, units=None, since=None, write=False,
                    compiler=None) -> dict:
    root = root.resolve()
    settings = Measurement.read(root)
    config = settings.source
    pin = image_pin(root, config)
    image_bytes = Path(image_path).read_bytes()
    if hashlib.sha256(image_bytes).hexdigest() != pin:
        raise ValueError("image does not match the source pin")
    image = PeImage(image_bytes, pin)
    literals = read_literals(root, pin)
    by_unit = {}
    for item in literals:
        by_unit.setdefault(item.unit, []).append(item)
    owners = {unit for unit, items in by_unit.items()
              if any(STATIC_STRING.fullmatch(item.symbol)
                     or (item.symbol.startswith(POOLED_PREFIX) and not item.merged) for item in items)}

    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("literal rebinding requires a configured compilation database")
    database = config.path(root, mappings["compile_commands"]).read_bytes()
    build, _ = build_units(root, database)
    scope = {"requested_units": list(units or ()), "since": since}
    selected = None  # every TU that owns a $SG binding
    requested = {Path(name).as_posix() for name in units or ()}
    unknown = sorted(name for name in requested
                     if name not in build or Path(name).suffix not in settings.unit_extensions)
    if unknown:
        raise ValueError("not configured build translation units: " + ", ".join(unknown))
    if since is not None:
        affected = affected_units(root, settings, database, changed_files(root, since), since=since)
        scope["widened"] = affected.get("widened")
        if affected["units"] is not None:
            selected = set(affected["units"]) | requested
    elif requested:
        selected = requested
    selected = sorted(owners if selected is None else owners & selected)
    if requested:
        scope["without_static_literals"] = sorted(requested - owners)  # no $SG or pooled binding

    metadata = {"operation": "rebind-literals", "units": selected}
    before = settings.snapshot(root, metadata)
    raws = {unit: (root / unit).read_bytes() for unit in selected}

    def fresh():
        if (settings.snapshot(root, metadata) != before or Path(image_path).read_bytes() != image_bytes
                or any((root / unit).read_bytes() != raw for unit, raw in raws.items())):
            raise ValueError("source, compiler inputs or image changed during literal rebinding")

    if compiler is None:
        compiler = Compiler(root, profile=settings.compiler_profile)
    reports = []
    for unit in selected:
        static = [item for item in by_unit[unit] if STATIC_STRING.fullmatch(item.symbol)]
        # A v2 merged declaration states linker merging; it is not a TU-private label.
        pooled = [item for item in by_unit[unit]
                  if item.symbol.startswith(POOLED_PREFIX) and not item.merged]
        contexts = _unit_contexts(root / unit)
        trials, plans, error = [], [], None
        for flags in settings.flags_for(unit):
            try:
                data = compiler(root / unit, (), flags)
                if not isinstance(data, bytes) or not data:
                    raise ValueError("compiler returned no object bytes")
                obj = Coff(data)
            except (OSError, ValueError) as failure:
                error = str(failure)
                trials.append({"flags": list(flags), "result": Result.UNKNOWN, "reason": error})
                break
            rows, shift = plan_unit(static, obj, image, contexts) if static else ([], None)
            rows = rows + plan_pooled(_stale_pooled(pooled, obj), rows, static, obj, image, contexts)
            rows.sort(key=lambda row: row["line"])
            _verify(by_unit[unit], rows, obj, image)
            plans.append((rows, shift))
            trials.append({"flags": list(flags), "result": Result.PASS, "shift": shift})
        report = {"unit": unit, "trials": trials}
        if error is not None or not plans:
            report.update(result=Result.UNKNOWN, reason=error or "no configured compiler trial",
                          bindings=[{"line": item.line, "previous": item.symbol, "symbol": item.symbol,
                                     "status": "unproven", "reason": "TU did not compile"}
                                    for item in sorted(static + pooled, key=lambda item: item.line)])
            reports.append(report)
            continue
        rows, shift = plans[0]
        for other, _ in plans[1:]:
            peers = {peer["line"]: peer for peer in other}
            for row in rows:
                peer = peers.get(row["line"])
                if peer is None or (row["status"], row["symbol"]) != (peer["status"], peer["symbol"]):
                    row.update(status="unproven", symbol=row["previous"],
                               reason="configured compiler trials disagree on this label")
        rows.sort(key=lambda row: row["line"])
        statuses = {row["status"] for row in rows}
        report.update(bindings=rows, shift=shift if all(other == shift for _, other in plans) else None,
                      result=Result.UNKNOWN if "unproven" in statuses
                      else Result.FAIL if "rebind" in statuses else Result.PASS)
        reports.append(report)

    changes = {unit: {row["line"]: (row["previous"], row["symbol"]) for row in report["bindings"]
                      if row["status"] == "rebind"} for unit, report in zip(selected, reports)}
    changes = {unit: lines for unit, lines in changes.items() if lines}
    published = []
    if write and changes:
        fresh()
        replacements = {}
        for unit, lines in changes.items():
            replacements[root / unit] = rewrite_literal_symbols(
                raws[unit], expected=tuple(by_unit[unit]), changes=lines, path=unit, pin=pin,
                address_space=config.address_space)
        staging = artifact_parent(root, "build/recovery/publications", excluded_roots=config.roots)
        publish({root / unit: raws[unit] for unit in changes}, replacements, validate=fresh,
                staging_directory=staging)
        published = sorted(changes)
        for report in reports:
            if report["unit"] in changes and report["result"] == Result.FAIL:
                report["result"] = Result.PASS
            for row in report["bindings"]:
                if row["status"] == "rebind" and report["unit"] in changes:
                    row["status"] = "rebound"
    else:
        fresh()
    counts = {}
    for report in reports:
        for row in report["bindings"]:
            counts[row["status"]] = counts.get(row["status"], 0) + 1
    results = {report["result"] for report in reports}
    result = (Result.UNKNOWN if Result.UNKNOWN in results else Result.FAIL if Result.FAIL in results
              else Result.PASS)
    return {"schema_version": 1, "operation": "rebind-literals", "result": result,
            "write": bool(write), "persisted": bool(published), "published": published,
            "scope": scope, "units": reports, "counts": counts,
            "ready_for_collection": False,
            "next_action": ("rerun with --write to publish proven rebinds" if changes and not write
                            else "measure affected functions; bank exact results only with `recover record`"),
            "limitations": ["`$SGn` static string labels, and pooled `??_C@` names the fresh object no longer defines; "
                            "a pooled name the object still defines is stable and skipped",
                            "original extents, evidence addresses and claims are never changed",
                            "identical duplicates are resolved only by a uniform shift or the bindings' order",
                            "emission and storage evidence only; not function, linker or runtime proof"]}
