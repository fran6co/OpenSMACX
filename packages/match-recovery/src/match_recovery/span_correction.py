"""Guarded correction of mappings' original span sets.

A mapping's identity is its entry address, catalogue name, mode, state and
historical claims; its span list is an interpretation of where that original
function's bytes lie. Seed catalogues sometimes attached one shared chunk to
several entries (an IDA function chunk tail-jumped into by several stubs).
This operation rewrites only the span field of selected `ORIGINAL` markers in
one host, through the canonical annotation writer, and checks the result line
for line: no other byte, fact, lesson, binding or claim may change.

Evidence binding: every corrected span set keeps its primary span at the entry
address, is backed by executable file bytes of the pinned image, and overlaps
no other current owner. Every byte a correction removes must, after the same
publication, be owned by another non-seed mapping: either one that already
lists it, or a new unclaimed binding (`assign`) added in this operation. A
mapping carrying BYTE_EXACT/SEMANTIC history may only drop secondary spans
(its primary span and remaining spans unchanged) and only when the caller
explicitly allows it. This never measures original bytes or grants a claim.
"""

from dataclasses import replace
import hashlib
from pathlib import Path

from .binding import binding_report, prepare_binding
from .binding_source import render
from .compiler import Compiler
from .measurement import Measurement
from .model import Origin, Result, Span, require_digest
from .pe_coff import Coff, PeImage
from .source import read_text
from .source.publication import publish
from .source.writer import write_bytes, write_symbol
from .source_config import SourceConfiguration
from .source_inventory import assert_snapshot_current, source_snapshot
from .source_replica import artifact_parent
from .symbol_check import function_definition


def parse_spans(value: str) -> tuple[Span, ...]:
    """`start-end[;start-end...]` (also `,`-separated), end exclusive."""
    spans = []
    for part in value.replace(",", ";").split(";"):
        bounds = part.strip().split("-")
        if len(bounds) != 2:
            raise ValueError(f"span must be start-end: {part!r}")
        spans.append(Span(*(int(item, 0) for item in bounds)))
    return tuple(spans)


def _validate_spans(address: int, spans) -> tuple[Span, ...]:
    if not isinstance(spans, (tuple, list)) or not spans or not all(isinstance(span, Span) for span in spans):
        raise ValueError(f"0x{address:08X}: span correction requires explicit original spans")
    spans = tuple(spans)
    if spans[0].start != address:
        raise ValueError(f"0x{address:08X}: the primary span must begin at the mapping's entry address")
    if any(span.end > 0x100000000 for span in spans):
        raise ValueError(f"0x{address:08X}: spans must lie in the PE32 address space")
    ordered = sorted(spans)
    if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
        raise ValueError(f"0x{address:08X}: corrected spans overlap each other")
    return spans


def _uncovered(span: tuple[int, int], owners) -> list[tuple[int, int]]:
    """Subranges of `span` not covered by the union of `owners`."""
    low, high = span
    gaps, cursor = [], low
    for start, end in sorted(owners):
        if end <= cursor or start >= high:
            continue
        if start > cursor:
            gaps.append((cursor, min(start, high)))
        cursor = max(cursor, end)
        if cursor >= high:
            break
    if cursor < high:
        gaps.append((cursor, high))
    return gaps


def _emits(root: Path, unit: str, symbol: str, compiler) -> dict:
    """Fresh configured compilation of `unit` must define `symbol` as code."""
    settings = Measurement.read(root)
    if Path(unit).suffix not in settings.unit_extensions:
        raise ValueError(f"assigned symbol host is not a configured measurement unit: {unit}")
    trials = []
    for flags in settings.flags_for(unit):
        compiler = compiler or Compiler(root, profile=settings.compiler_profile)
        try:
            data = compiler(root / unit, (), flags)
            if not isinstance(data, bytes) or not data:
                raise ValueError("compiler returned no object bytes")
            found = function_definition(Coff(data), symbol)
        except (OSError, ValueError) as error:
            trials.append({"flags": list(flags), "result": Result.UNKNOWN, "reason": str(error)})
            continue
        trials.append({"flags": list(flags), "result": found["result"], "reason": found["reason"]})
        if found["result"] == Result.PASS:
            return {"flags": list(flags), "trials": trials}
    raise ValueError(f"no configured compilation of {unit} defines {symbol}: {trials}")


def correct_spans(root: Path, *, image: Path, source: str, source_sha256: str, corrections,
                  reason: str, evidence, assign: dict | None = None, allow_claimed: bool = False,
                  compiler=None) -> dict:
    root = root.resolve()
    require_digest(source_sha256)
    rationale = render(reason, evidence)  # Validates single-line reason and evidence references.
    if not isinstance(corrections, dict) or not corrections:
        raise ValueError("span correction requires at least one address")
    corrections = {address: _validate_spans(address, spans) for address, spans in corrections.items()}
    image = image if image.is_absolute() else root / image
    snapshot = source_snapshot(root, image)
    config = SourceConfiguration.from_dict(snapshot["source_configuration"])
    if config.address_space != "pe:va":
        raise ValueError("span correction requires pe:va source configuration")
    path = config.path(root, source)
    if source not in snapshot["files"] or config.is_seed(source) or not path.is_file():
        raise ValueError("span correction host must be an existing non-seed source file")
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != source_sha256:
        raise ValueError("span correction source digest is stale")
    text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
    records = read_text(text, path)
    original = PeImage(image.read_bytes(), snapshot["image_sha256"])

    def backed(span: Span) -> bool:
        original.read(span)
        return any(flags & 0x20000000 and region.start <= span.start and span.end <= region.end
                   for (region, _, _), flags in zip(original.sections, original.section_flags))

    changes, updated = [], []
    for address, spans in corrections.items():
        selected = [row for row in records if row.address == address]
        if len(selected) != 1:
            raise ValueError(f"expected one mapping for 0x{address:08X} in {source}, found {len(selected)}")
        record, = selected
        old = tuple(record.image_spans)
        new = tuple((span.start, span.end) for span in spans)
        if new == old:
            raise ValueError(f"0x{address:08X}: mapping already lists the requested spans")
        if not old or old[0][0] != address:
            raise ValueError(f"0x{address:08X}: current primary span does not begin at the entry; review it by hand")
        if not all(backed(span) for span in spans):
            raise ValueError(f"0x{address:08X}: corrected spans require executable file-backed storage")
        removed = [span for span in old if span not in new]
        added = [span for span in new if span not in old]
        claimed = record.byte_exact or record.semantic
        if claimed and not allow_claimed:
            raise ValueError(f"0x{address:08X}: mapping carries historical BYTE_EXACT/SEMANTIC claims; "
                             "only an explicitly allowed secondary-span removal may change its spans")
        if claimed and (added or new[0] != old[0] or not removed):
            raise ValueError(f"0x{address:08X}: a claimed mapping may only drop whole secondary spans")
        changes.append({"record": record, "old": old, "new": new, "removed": removed, "added": added})
        updated.append(replace(record, image_spans=new))

    changed = write_bytes(raw, updated)
    after = read_text(changed.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n"), path)
    before_lines, after_lines = raw.splitlines(keepends=True), changed.splitlines(keepends=True)
    markers = {change["record"].line - 1 for change in changes}
    if (len(after) != len(records) or len(before_lines) != len(after_lines)
            or any(left != right for index, (left, right) in enumerate(zip(before_lines, after_lines))
                   if index not in markers)):
        raise ValueError("span correction would change more than the selected marker lines")
    wanted = {change["record"].line: change["new"] for change in changes}
    for old_row, new_row in zip(records, after):
        expected = wanted.get(old_row.line, old_row.image_spans)
        if (new_row.image_spans != expected
                or replace(new_row, image_spans=old_row.image_spans, region=old_row.region,
                           origin=old_row.origin) != old_row):
            raise ValueError("span correction would change identity, facts, lessons or historical claims")

    # Ownership after this publication: every other non-seed mapping as it
    # stands, the corrected mappings with their new spans, plus the assignment.
    corrected = {(source, change["record"].address) for change in changes}
    owners = []  # (entity_id, path, address, spans)
    for row in snapshot["records"]:
        if row["candidate_kind"] == "legacy_seed":
            continue
        address = row["origin"]["address"]
        if (row["path"], address) in corrected:
            continue
        owners.append((row["entity_id"], row["path"], address,
                       [tuple(span) for span in row["spans"] if span not in row["invalid_spans"]]))
    entity = {address: Origin(snapshot["image_sha256"], config.address_space, address).entity_id
              for _, address in corrected}
    for change in changes:
        address = change["record"].address
        owners.append((entity[address], source, address, list(change["new"])))

    prepared = None
    if assign is not None:
        required = {"address", "name", "spans", "source", "line", "source_sha256"}
        if not isinstance(assign, dict) or not required <= assign.keys() or assign.keys() - required - {"symbol", "mode"}:
            raise ValueError(f"assignment requires {sorted(required)} and optional symbol/mode")
        released = frozenset((entity[change["record"].address], *span)
                             for change in changes for span in change["removed"])
        same = assign["source"] == source
        prepared = prepare_binding(root, image=image, address=assign["address"], name=assign["name"],
                                   spans=list(assign["spans"]), source=assign["source"], line=assign["line"],
                                   source_sha256=assign["source_sha256"], reason=reason, evidence=evidence,
                                   mode=assign.get("mode", "body"), released=released,
                                   base=changed if same else None)
        if prepared["snapshot"]["revision"] != snapshot["revision"]:
            raise ValueError("source inventory changed during span correction")
        assigned = prepared["changed"]
        if assign.get("symbol"):
            unit = assign["source"]
            emitted = _emits(root, unit, assign["symbol"], compiler)
            reread = read_text(assigned.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n"),
                               prepared["path"])
            new_record, = [row for row in reread if row.address == assign["address"] and row.line == assign["line"]]
            assigned = write_symbol(assigned, new_record, assign["symbol"])
            prepared["emission"] = emitted
        prepared["changed"] = assigned
        if same:
            changed = assigned
        owners.append((prepared["origin"].entity_id, assign["source"], assign["address"],
                       [(span.start, span.end) for span in assign["spans"]]))

    for change in changes:
        address = change["record"].address
        others = [span for owner, _, _, spans in owners if owner != entity[address] for span in spans]
        for span in change["added"]:
            clash = [other for other in others if span[0] < other[1] and other[0] < span[1]]
            if clash:
                raise ValueError(f"0x{address:08X}: added span {span[0]:#010x}-{span[1]:#010x} overlaps "
                                 "another owner; shared ownership needs separate review")
        for span in change["removed"]:
            gaps = _uncovered(span, others)
            if gaps:
                raise ValueError(f"0x{address:08X}: removed span {span[0]:#010x}-{span[1]:#010x} would leave "
                                 f"{', '.join(f'{low:#010x}-{high:#010x}' for low, high in gaps)} unowned; "
                                 "assign it to another mapping in the same operation")
        change["new_owners"] = sorted({f"{p}:0x{a:08X}" for owner, p, a, spans in owners
                                       if owner != entity[address]
                                       for span in change["removed"] for other in spans
                                       if span[0] < other[1] and other[0] < span[1]})

    def fresh():
        assert_snapshot_current(root, snapshot)
        with image.open("rb") as stream:
            if hashlib.file_digest(stream, "sha256").hexdigest() != snapshot["image_sha256"]:
                raise ValueError("original image changed before span correction publication")
        if path.read_bytes() != raw:
            raise ValueError("span correction source changed before publication")
        if prepared is not None:
            prepared["fresh"]()

    expected, replacements = {path: raw}, {path: changed}
    if prepared is not None and prepared["path"].resolve() != path.resolve():
        expected[prepared["path"]] = prepared["disk"]
        replacements[prepared["path"]] = prepared["changed"]
    fresh()
    staging = artifact_parent(root, "build/recovery/publications", excluded_roots=config.roots)
    publish(expected, replacements, validate=fresh, staging_directory=staging)
    report = {"schema_version": 1, "operation": "correct-spans", "path": source,
              "corrections": [{"entity_id": entity[change["record"].address],
                               "address": f"0x{change['record'].address:08X}", "line": change["record"].line,
                               "original_name": change["record"].name,
                               "previous_spans": [f"0x{low:08X}-0x{high:08X}" for low, high in change["old"]],
                               "spans": [f"0x{low:08X}-0x{high:08X}" for low, high in change["new"]],
                               "removed_owned_by": change["new_owners"],
                               "historical_claims": {"byte_exact": change["record"].byte_exact,
                                                     "instruction_shape": change["record"].semantic}}
                              for change in changes],
              "reason": reason, "evidence": list(evidence), "rationale": rationale,
              "verification": "ownership_and_backing_only", "persisted": True, "ready_for_collection": False,
              "next_action": "measure the corrected mappings; bank an exact result only with `recover record`; "
                             "carry the reason in the commit message",
              "limitations": ["spans are caller-supplied interpretations; only image backing and current "
                              "source ownership were checked",
                              "existing RECOVERY_BINDING rationale and prose comments are not rewritten"]}
    if prepared is not None:
        report["assignment"] = {**binding_report(prepared), "address": f"0x{assign['address']:08X}",
                                "name": assign["name"], "symbol": assign.get("symbol"),
                                "emission": prepared.get("emission")}
    return report
