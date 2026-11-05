"""Measured, explicitly bounded contribution units (not historical TUs).

Candidate membership comes only from configured address intervals. Current
source homes describe present organization and never determine candidate
membership or the candidate denominator.
"""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import hashlib
import json
import re

from .application_binary import verify_records
from .components import relative_path
from .graph import DependencyGraph
from .includes import include_graph
from .measurement import Measurement
from .model import Origin, Result, Span
from .ownership_source import project_view
from .pe_coff import PeImage
from .source.reader import read_file
from .source_config import SourceConfiguration
from .object_source import read_objects
from .literal_source import read_literals
from .generated_source import read_generated
from .import_source import read_imports
from .associated_source import read_associated
from .source_inventory import assert_snapshot_current, image_pin


_HEX = re.compile(r"0[xX][0-9a-fA-F]+\Z")
_PADDING = frozenset((0x90, 0xCC))


def _address(value, label):
    if isinstance(value, str) and _HEX.fullmatch(value):
        value = int(value, 16)
    if type(value) is not int or not 0 <= value <= 0xFFFFFFFF:
        raise ValueError(f"{label} must be a PE virtual address")
    return value


def _parse_units_config(raw: bytes) -> list[dict]:
    """Parse schema 1's compact interval declarations; identity ledgers are derived."""
    import json

    value = json.loads(raw)
    if (type(value) is not dict or set(value) != {"schema_version", "units"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1):
        raise ValueError("units config must have exactly schema_version:1 and units")
    if not isinstance(value["units"], list):
        raise ValueError("units must be an array")
    units, names = [], set()
    for index, row in enumerate(value["units"]):
        required = {"name", "start", "end_exclusive", "reason", "evidence", "recovered_tu"}
        if type(row) is not dict or set(row) != required:
            raise ValueError(f"units[{index}] requires name, start, end_exclusive, recovered_tu, reason, evidence")
        name, reason, evidence, recovered_tu = (row["name"], row["reason"], row["evidence"], row["recovered_tu"])
        if not isinstance(name, str) or not name.strip() or name in names:
            raise ValueError(f"units[{index}].name must be nonblank and unique")
        if not isinstance(reason, str) or not reason.strip():
            raise ValueError(f"units[{index}].reason must be nonblank")
        if (not isinstance(evidence, list) or not evidence
                or any(not isinstance(item, str) or not item.strip() for item in evidence)):
            raise ValueError(f"units[{index}].evidence must be a nonempty string array")
        try:
            relative_path(recovered_tu)
        except (TypeError, ValueError) as error:
            raise ValueError(f"units[{index}].recovered_tu must be a canonical project-relative source path") from error
        start = _address(row["start"], f"units[{index}].start")
        end = _address(row["end_exclusive"], f"units[{index}].end_exclusive")
        if end <= start:
            raise ValueError(f"units[{index}] end_exclusive must exceed start")
        names.add(name)
        units.append({"name": name, "start": start, "end_exclusive": end,
                      "recovered_tu": recovered_tu,
                      "reason": reason, "evidence": list(evidence)})
    ordered = sorted(units, key=lambda item: (item["start"], item["end_exclusive"]))
    for left, right in zip(ordered, ordered[1:]):
        if left["end_exclusive"] > right["start"]:
            raise ValueError(f"candidate intervals overlap: {left['name']} and {right['name']}")
    return units


def read_units_config(path: Path) -> list[dict]:
    """Read schema 1's interval declarations from a filesystem path."""
    return _parse_units_config(path.read_bytes())


def _source_records(root, snapshot):
    """Bind source annotations back to their pin-derived original identities."""
    address_space = snapshot["source_configuration"]["address_space"]
    pin = snapshot["image_sha256"]
    by_id = {}
    for relative in sorted(snapshot["files"]):
        for record in read_file(root / relative):
            entity_id = Origin(pin, address_space, record.address).entity_id
            by_id.setdefault(entity_id, []).append((relative, record))
    return by_id


def _intersects(span, start, end):
    return span[0] < end and start < span[1]


def _span_pair(value):
    """Normalize source-snapshot pairs and ownership dataclass dictionaries."""
    if isinstance(value, dict) and set(value) == {"start", "end"}:
        return value["start"], value["end"]
    if isinstance(value, (list, tuple)) and len(value) == 2:
        return value[0], value[1]
    raise ValueError("source inventory contains an invalid span representation")


def _unit_members(view, snapshot, source_rows, unit, image, associated_data=(),
                  associated_data_units=frozenset()):
    start, end = unit["start"], unit["end_exclusive"]
    entities = {}
    for group in view["groups"]:
        for entity in group["entities"]:
            entities[entity["entity_id"]] = entity
    records = {row["entity_id"]: row for row in snapshot["records"]}
    records_by_location = {(row["entity_id"], row["path"], row["line"]): row
                           for row in snapshot["records"]}
    members, selected_records, issues = [], {}, []
    primary_spans, accounted_spans = [], []

    for entity_id, entity in sorted(entities.items(), key=lambda pair: pair[1]["origin"]["address"]):
        address = entity["origin"]["address"]
        raw = records.get(entity_id)
        candidates = entity["candidates"]
        candidate_lists = [[_span_pair(span) for span in candidate["spans"]]
                           for candidate in candidates]
        candidate_spans = [span for group in candidate_lists for span in group]
        candidate_primaries = [group[0] for group in candidate_lists if group]
        invalid = [_span_pair(span) for span in (raw or {}).get("invalid_spans", ())]
        raw_spans = [_span_pair(span) for span in raw["spans"]
                     if span not in (raw or {}).get("invalid_spans", ())] if raw else []
        raw_primary = raw_spans[:1]
        primary_starts = {span[0] for span in candidate_primaries + raw_primary}
        all_spans = sorted(set(candidate_spans + raw_spans))
        in_by_address = start <= address < end
        touches_primary = any(_intersects(span, start, end)
                              for span in candidate_primaries + raw_primary)
        touches_invalid = any(_intersects(span, start, end) for span in invalid)
        touches = touches_primary or touches_invalid or (in_by_address and bool(all_spans))
        if not in_by_address and not touches:
            continue
        if not in_by_address and touches_primary:
            issues.append({"kind": "partial_function_span", "entity_id": entity_id,
                           "address": f"0x{address:08X}",
                           "reason": "a primary function span intersects the interval although its identity starts outside it"})
        if invalid and (in_by_address or any(_intersects(span, start, end) for span in invalid)):
            issues.append({"kind": "invalid_span", "entity_id": entity_id,
                           "invalid_spans": [[f"0x{a:08X}", f"0x{b:08X}"] for a, b in invalid]})
        primary_candidates = sorted(set(candidate_primaries + raw_primary))
        if in_by_address and not primary_candidates:
            issues.append({"kind": "missing_primary_span", "entity_id": entity_id,
                           "address": f"0x{address:08X}"})
        if in_by_address and primary_candidates and address not in primary_starts:
            issues.append({"kind": "primary_span_address_mismatch", "entity_id": entity_id,
                           "address": f"0x{address:08X}",
                           "primary_starts": [f"0x{value:08X}" for value in sorted(primary_starts)]})
        member_primaries = [span for span in primary_candidates
                            if _intersects(span, start, end)]
        for span in member_primaries:
            if span[0] < start or span[1] > end:
                issues.append({"kind": "partial_function_span", "entity_id": entity_id,
                               "span": [f"0x{span[0]:08X}", f"0x{span[1]:08X}"],
                               "reason": "primary span is cut by a candidate interval boundary"})
            else:
                primary_spans.append((span[0], span[1], entity_id))
                accounted_spans.append((span[0], span[1], entity_id, "primary"))
        # Detached secondary EH/associated spans remain full native measurement
        # obligations but do not define the contiguous candidate core boundary.
        if in_by_address:
            for secondary in sorted(set(candidate_spans + raw_spans)):
                if secondary in candidate_primaries or secondary in raw_primary:
                    continue
                if _intersects(secondary, start, end):
                    accounted_spans.append((max(start, secondary[0]),
                                            min(end, secondary[1]), entity_id,
                                            "secondary"))

        mapping_disposition = entity["mapping_disposition"]
        component = next((candidate["component"] for candidate in candidates if candidate.get("component")), None)
        seed_only = bool(candidates) and all(candidate["seed"] for candidate in candidates)
        source_candidates = []
        for candidate in candidates:
            source_record = records_by_location.get((entity_id, candidate["path"], candidate["line"]), {})
            source_candidates.append({"path": candidate["path"],
                                      "context_path": candidate.get("context_path", ""),
                                      "body_path": source_record.get("body_path", ""),
                                      "seed": candidate["seed"],
                                      "unbound": candidate["unbound"],
                                      "component": candidate.get("component"),
                                      "candidate_kind": source_record.get("candidate_kind", "component")})
        member = {"entity_id": entity_id, "address": f"0x{address:08X}",
                  "mapping_disposition": mapping_disposition, "component": component,
                  "seed_only": seed_only,
                  "source_candidates": source_candidates,
                  "spans": [[f"0x{a:08X}", f"0x{b:08X}"] for a, b in all_spans],
                  "secondary_spans_outside_core": [[f"0x{a:08X}", f"0x{b:08X}"]
                                                    for a, b in all_spans if (a, b) not in primary_candidates
                                                    and not _intersects((a, b), start, end)],
                  "current_source_homes": sorted({candidate.get("context_path") or candidate["path"] for candidate in candidates
                                                     if not candidate["seed"] and not candidate["unbound"]
                                                     and not candidate.get("component")}),
                  "unbound": any(candidate["unbound"] for candidate in candidates)}
        members.append(member)
        if member["unbound"]:
            issues.append({"kind": "unbound_identity", "entity_id": entity_id,
                           "address": f"0x{address:08X}",
                           "reason": "retained in candidate count; cannot be treated as compiled code"})
        if component:
            issues.append({"kind": "dependency_identity", "entity_id": entity_id,
                           "component": component,
                           "reason": "retained in candidate count; component identity is not silently dropped or measured as product source"})

        choices = source_rows.get(entity_id, ())
        current = [(path, record) for path, record in choices
                   if not snapshot_record_is_seed(snapshot, path, entity_id)]
        if not current:
            current = [(path, record) for path, record in choices
                       if snapshot_record_is_seed(snapshot, path, entity_id)]
        if len(current) == 1:
            selected_records[entity_id] = current[0][1]
        elif len(current) > 1:
            issues.append({"kind": "multiple_source_annotations", "entity_id": entity_id,
                           "paths": sorted(path for path, _ in current)})

    # A validated RECOVERY_ASSOCIATED interpretation can account for original
    # table bytes only when its source-mapped owner is itself a member of this
    # candidate and both origins use the snapshot's pinned address space.
    # This describes original source ownership; it does not match a current
    # compiler table or contribute code bytes.
    members_by_id = {member["entity_id"]: member for member in members}
    associated_data_spans = []
    for binding in associated_data:
        owner = getattr(binding, "owner", None)
        span = getattr(binding, "span", None)
        if owner is None or span is None:
            continue
        member = members_by_id.get(owner.entity_id)
        if (member is None or member["unbound"]
                or not unit["start"] <= owner.address < unit["end_exclusive"]
                or binding.unit not in associated_data_units
                or owner.image_sha256 != snapshot["image_sha256"]
                or owner.address_space != snapshot["source_configuration"]["address_space"]
                or binding.evidence.image_sha256 != snapshot["image_sha256"]
                or binding.evidence.address_space != snapshot["source_configuration"]["address_space"]):
            continue
        left, right = span.start, span.end
        if (type(left) is not int or type(right) is not int
                or not 0 <= left < right <= 0x100000000):
            issues.append({"kind": "invalid_associated_data_span",
                           "owner_entity_id": owner.entity_id})
            continue
        if not _intersects((left, right), start, end):
            continue
        if left < start or right > end:
            issues.append({"kind": "partial_associated_data_span",
                           "owner_entity_id": owner.entity_id,
                           "span": [f"0x{left:08X}", f"0x{right:08X}"]})
            continue
        if any(left < known_right and known_left < right
               for known_left, known_right, *_rest in accounted_spans):
            issues.append({"kind": "overlapping_associated_data_span",
                           "owner_entity_id": owner.entity_id,
                           "span": [f"0x{left:08X}", f"0x{right:08X}"]})
            continue
        accounted_spans.append((left, right, owner.entity_id, "associated_data"))
        associated_data_spans.append({"owner_entity_id": owner.entity_id,
                                      "kind": binding.kind,
                                      "span": [f"0x{left:08X}", f"0x{right:08X}"]})

    primary_spans.sort()
    for left, right in zip(primary_spans, primary_spans[1:]):
        if left[1] > right[0]:
            issues.append({"kind": "overlapping_function_spans",
                           "left_entity_id": left[2], "right_entity_id": right[2],
                           "left": [f"0x{left[0]:08X}", f"0x{left[1]:08X}"],
                           "right": [f"0x{right[0]:08X}", f"0x{right[1]:08X}"]})

    intervals = sorted((left, right) for left, right, _entity, _kind in accounted_spans)
    merged = []
    for left, right in intervals:
        if merged and left <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(merged[-1][1], right))
        else:
            merged.append((left, right))
    cursor = start
    covered = 0
    def padding_gap(left, right):
        gap_span = Span(left, right)
        try:
            data = image.read(gap_span)
        except ValueError as error:
            issues.append({"kind": "unreadable_original_gap",
                           "span": [f"0x{left:08X}", f"0x{right:08X}"],
                           "reason": str(error)})
            return 0
        mismatch = next((index for index, value in enumerate(data)
                         if value not in _PADDING), None)
        if mismatch is not None:
            issues.append({"kind": "unexplained_gap",
                           "span": [f"0x{left:08X}", f"0x{right:08X}"],
                           "size": len(data),
                           "first_nonpadding_address": f"0x{left + mismatch:08X}",
                           "reason": "unmapped original bytes are not all strict 90/CC padding"})
            return 0
        return len(data)

    for left, right in merged:
        if left > cursor:
            covered += padding_gap(cursor, left)
        cursor = max(cursor, right)
    if not primary_spans:
        issues.append({"kind": "no_mapped_code", "reason": "candidate interval has no fully mapped function spans"})
    elif cursor < end:
        covered += padding_gap(cursor, end)
    primary_bytes = sum(right - left for left, right, _ in primary_spans)
    secondary_bytes = sum(right - left for left, right, _entity, kind in accounted_spans if kind == "secondary")
    associated_data_bytes = sum(right - left for left, right, _entity, kind in accounted_spans
                                if kind == "associated_data")
    code_coverage = {"candidate_size": end - start, "mapped_primary_span_bytes": primary_bytes,
                     "associated_secondary_span_bytes": secondary_bytes,
                     "associated_data_bytes": associated_data_bytes,
                     "associated_data_spans": associated_data_spans,
                     "associated_data_basis": ("source_interpretation_only_not_current_table_match"
                                                if associated_data_spans else "none"),
                     "verified_padding_bytes": covered,
                     "closed": bool(primary_spans) and not issues}
    return members, list(selected_records.values()), issues, code_coverage


def snapshot_record_is_seed(snapshot, path, entity_id):
    return any(row["entity_id"] == entity_id and row["path"] == path
               and row["candidate_kind"] == "legacy_seed" for row in snapshot["records"])


def _single_trial(report, members):
    expected = {member["entity_id"] for member in members}
    measured_ids = {row.get("entity_id") for row in report.get("functions", ())}
    all_selected = expected == measured_ids
    trials = report.get("selected_evidence", {}).get("trials", ())
    passing_code_trials, passing_evidence_trials = [], []
    for index, trial in enumerate(trials):
        functions = trial.get("function_results", ())
        # verify_records' selection trial is the native graph snapshot. Never
        # combine per-function PASS variants from different trials.
        if (all_selected and len(functions) == len(expected)
                and all(value == Result.PASS for value in functions)):
            passing_code_trials.append(index)
            if trial.get("result") == Result.PASS:
                passing_evidence_trials.append(index)
    return all_selected, passing_code_trials, passing_evidence_trials


def _unit_trial_status(report, members, coverage_closed):
    """Qualify current-product code and evidence without combining trials."""
    all_selected, code_trials, evidence_trials = _single_trial(report, members)
    nonproduct = any(member["seed_only"] or member["unbound"] or member["component"]
                     for member in members)
    matched = bool(coverage_closed and all_selected and code_trials and not nonproduct)
    closed = bool(matched and evidence_trials)
    return matched, closed, code_trials if matched else [], evidence_trials if closed else []


def _qualified_evidence_status(native_evidence_closed, evidence_trials,
                               owned_evidence_covered):
    """Qualify native aggregate closure only with same-trial owned receipts."""
    if native_evidence_closed is None:
        return None, []
    closed = bool(native_evidence_closed and owned_evidence_covered is True)
    return closed, list(evidence_trials) if closed else []


def _fully_byte_exact_status(represented, matched, evidence_closed,
                             owned_evidence_covered):
    return bool(represented and matched and evidence_closed
                and owned_evidence_covered is True)


def _native_current_translation_units(report, members):
    """Use native selected-function owner rows, not annotation source homes."""
    expected = {member["entity_id"] for member in members}
    rows_by_id = {}
    for row in (report or {}).get("functions", ()):
        rows_by_id.setdefault(row.get("entity_id"), []).append(row)
    owners, unknowns = set(), []
    for entity_id in sorted(expected):
        rows = rows_by_id.get(entity_id, ())
        if len(rows) != 1:
            owners.update(row["unit"] for row in rows
                          if isinstance(row.get("unit"), str) and row["unit"])
            unknowns.append({"entity_id": entity_id,
                             "reason": "native verifier did not return exactly one selected-function row"})
            continue
        owner = rows[0].get("unit")
        if not isinstance(owner, str) or not owner:
            unknowns.append({"entity_id": entity_id,
                             "reason": "native function row has no current source-unit owner"})
            continue
        owners.add(owner)
    if len(rows_by_id) != len(expected):
        for entity_id in sorted(set(rows_by_id) - expected):
            unknowns.append({"entity_id": entity_id,
                             "reason": "native verifier returned an unexpected function identity"})
    known = bool(expected) and not unknowns and len(rows_by_id) == len(expected)
    return sorted(owners), (len(owners) == 1 if known else None), unknowns


def _identity_counts(members):
    return {
        "known_function_count": len(members),
        "known_product_function_count": sum(not member["seed_only"] and not member["unbound"]
                                             and not member["component"] for member in members),
        "seed_only_function_count": sum(member["seed_only"] for member in members),
        "unbound_function_count": sum(member["unbound"] for member in members),
        "dependency_component_count": sum(bool(member["component"]) for member in members),
    }


def _candidate_home(candidate):
    return candidate.get("context_path") or candidate.get("path", "")


def _counterpart_representation(root, unit, members, view, snapshot, source,
                                unit_extensions, build_units, include_closure):
    """Prove current source-map correspondence without requiring body success."""
    recovered = unit["recovered_tu"]
    issues = []
    relative = Path(recovered)
    try:
        target = source.path(root, recovered)
    except ValueError as error:
        target = None
        issues.append({"kind": "invalid_recovered_tu", "reason": str(error)})
    if (target is None or relative.is_absolute()
            or not any(relative.is_relative_to(Path(directory)) for directory in source.roots)
            or relative.suffix not in unit_extensions
            or relative.suffix in source.interface_extensions
            or source.is_seed(recovered)
            or recovered not in build_units
            or not target.is_file() or target.is_symlink()
            or not target.resolve().is_relative_to(root)
            or target.resolve().relative_to(root).as_posix() != recovered
            or recovered not in snapshot["files"]):
        issues.append({"kind": "recovered_tu_not_built",
                       "path": recovered,
                       "reason": "recovered_tu must be a canonical, configured, built nonseed translation unit"})

    closure = include_closure.prerequisites((recovered,)) if recovered in snapshot["files"] else frozenset()
    pending_unbound_count = 0
    for member in members:
        address = int(member["address"], 16)
        if not unit["start"] <= address < unit["end_exclusive"]:
            issues.append({"kind": "primary_identity_outside_candidate", "entity_id": member["entity_id"],
                           "address": member["address"]})
        product = [candidate for candidate in member["source_candidates"]
                   if not candidate["seed"] and not candidate["component"]
                   and not candidate["unbound"]]
        unbound = [candidate for candidate in member["source_candidates"]
                   if candidate["unbound"]]
        if unbound:
            pending_unbound_count += len(unbound)
            if len(unbound) != 1 or len(member["source_candidates"]) != 1:
                issues.append({"kind": "unbound_source_cardinality", "entity_id": member["entity_id"],
                               "reason": "a pending UNBOUND identity cannot compete with another source candidate"})
                continue
            candidate, = unbound
            home = _candidate_home(candidate)
            if (candidate["seed"] or candidate["component"] or home != recovered
                    or Path(candidate["path"]).suffix in source.interface_extensions
                    or candidate.get("body_path") or candidate.get("context_path")):
                issues.append({"kind": "unbound_host_mismatch", "entity_id": member["entity_id"],
                               "path": candidate["path"], "recovered_tu": recovered,
                               "reason": "a pending note must be bodyless in the built product TU counterpart"})
            continue
        component_only = bool(member["source_candidates"]) and not product and any(
            candidate["component"] for candidate in member["source_candidates"])
        if component_only:
            continue
        if len(product) != 1:
            issues.append({"kind": "member_source_cardinality", "entity_id": member["entity_id"],
                           "product_candidate_count": len(product),
                           "reason": "each known original member needs one current nonseed product source mapping"})
            continue
        if any(candidate["component"] for candidate in member["source_candidates"]):
            issues.append({"kind": "mixed_product_component_identity", "entity_id": member["entity_id"],
                           "reason": "a product source mapping conflicts with a dependency-component interpretation"})
        candidate, = product
        home = _candidate_home(candidate)
        if home != recovered:
            issues.append({"kind": "member_home_mismatch", "entity_id": member["entity_id"],
                           "home": home, "recovered_tu": recovered})
        annotation_path = candidate["path"]
        if Path(annotation_path).suffix in source.interface_extensions and annotation_path not in closure:
            issues.append({"kind": "header_not_in_recovered_tu", "entity_id": member["entity_id"],
                           "header": annotation_path, "recovered_tu": recovered})
        body_path = candidate["body_path"]
        if body_path:
            if source.is_seed(body_path):
                issues.append({"kind": "seed_body_path", "entity_id": member["entity_id"],
                               "body_path": body_path})
            elif body_path not in snapshot["files"]:
                issues.append({"kind": "body_path_outside_source_coverage", "entity_id": member["entity_id"],
                               "body_path": body_path})
            elif body_path != recovered and Path(body_path).suffix in unit_extensions and body_path not in closure:
                issues.append({"kind": "implementation_in_other_translation_unit",
                               "entity_id": member["entity_id"], "body_path": body_path,
                               "recovered_tu": recovered})

    # A source function outside the proposed original interval must not be
    # smuggled into the same current C++ TU, which would make the interval an
    # incomplete representation of that contribution.
    member_ids = {member["entity_id"] for member in members}
    for group in view["groups"]:
        for entity in group["entities"]:
            entity_id = entity["entity_id"]
            address = entity["origin"]["address"]
            if entity_id in member_ids or unit["start"] <= address < unit["end_exclusive"]:
                continue
            for candidate in entity["candidates"]:
                if candidate["seed"] or candidate.get("component"):
                    continue
                home = _candidate_home(candidate)
                if home == recovered:
                    issues.append({"kind": "external_original_identity_in_recovered_tu",
                                   "entity_id": entity_id, "address": f"0x{address:08X}",
                                   "recovered_tu": recovered,
                                   "reason": "a known primary identity outside this candidate maps to its counterpart C++ unit"})
                    break

    return {"recovered_tu": recovered, "represented_cpp": not issues,
            "pending_unbound_count": pending_unbound_count,
            "counterpart_placeholder": not issues and not members,
            "representation_issues": issues,
            "representation_basis": "configured source homes/context paths and intended build membership; no body match or native verifier PASS is required"}


def _summary_identity_counts(report_units, *, mapping_only=False):
    """Count unique identities only inside the declared candidate intervals."""
    all_ids, matched_ids, closed_ids = set(), set(), set()
    for row in report_units:
        ids = {member["entity_id"] for member in row["members"]}
        all_ids.update(ids)
        if row["matched_code_unit"]:
            matched_ids.update(ids)
        if row["evidence_closed_unit"]:
            closed_ids.update(ids)
    return {"candidate_identity_count": len(all_ids),
            "matched_code_unit_identity_count": None if mapping_only else len(matched_ids),
            "evidence_closed_unit_identity_count": None if mapping_only else len(closed_ids),
            "native_measurement": "not_run" if mapping_only else "measured",
            "identity_count_scope": "unique mapped identities inside declared candidate ranges; may include compiler entries or fragments and is not a global function or historical-TU denominator"}


def _index_owned_obligations(items, path_of, value_of):
    """Index rendered obligation rows by source path without changing order."""
    indexed = {}
    for order, item in enumerate(items):
        indexed.setdefault(path_of(item), []).append((order, value_of(item)))
    return {path: tuple(rows) for path, rows in indexed.items()}


def _owned_obligation_snapshot(root, pin, associated_bindings=None):
    """Read and validate each source-owned obligation family once per report."""
    objects = read_objects(root, pin)
    literals = read_literals(root, pin)
    generated = read_generated(root, pin)
    imports = read_imports(root)
    associated = (read_associated(root, pin) if associated_bindings is None
                  else associated_bindings)
    return {
        "objects": _index_owned_obligations(
            objects, lambda item: item.unit,
            lambda item: {"kind": "object", "unit": item.unit,
                          "symbol": item.symbol, "bounded": item.span is not None}),
        "literals": _index_owned_obligations(
            literals, lambda item: item.unit,
            lambda item: {"kind": "literal", "unit": item.unit,
                          "symbol": item.symbol}),
        "generated": _index_owned_obligations(
            generated, lambda item: item.path,
            lambda item: {"kind": "generated", "path": item.path,
                          "symbol": item.symbol}),
        "imports": _index_owned_obligations(
            imports, lambda item: item.path,
            lambda item: {"kind": "import", "path": item.path,
                          "symbol": item.symbol}),
        "associated": _index_owned_obligations(
            associated, lambda item: item.unit,
            lambda item: {"kind": "associated", "unit": item.unit,
                          "owner_entity_id": item.owner.entity_id}),
    }


def _owned_obligations(snapshot, recovered, closure):
    """Find source-declared evidence owned by a TU or one of its included headers.

    The current selected-evidence report does not retain stable IDs for every
    category in each graph trial. Until it does, any such obligation makes the
    whole-unit byte-exact stage unknown rather than being inferred from aggregate
    PASS counts.
    """
    paths = set(closure) | {recovered}
    values = []
    for category in ("objects", "literals", "generated", "imports", "associated"):
        rows = [row for path in paths for row in snapshot[category].get(path, ())]
        values.extend(dict(value) for _order, value in sorted(rows, key=lambda row: row[0]))
    return values


def units_report(root: Path, image_path: Path, units_config: Path, *, mapping_only=False) -> dict:
    root = root.resolve()
    image_path = image_path if image_path.is_absolute() else root / image_path
    config_path = units_config if units_config.is_absolute() else root / units_config
    config_bytes = config_path.read_bytes()
    config_digest = hashlib.sha256(config_bytes).hexdigest()
    units = _parse_units_config(config_bytes)
    settings = Measurement.read(root)
    measurement_epoch = settings.snapshot(root, {})
    pin = image_pin(root)
    view, snapshot = project_view(root, image_path)
    if snapshot["image_sha256"] != pin:
        raise ValueError("source snapshot does not describe configured pinned image")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, pin)
    source_rows = _source_records(root, snapshot)
    source = SourceConfiguration.from_dict(snapshot["source_configuration"])
    project_path = root / "recovery/project.json"
    project_bytes = project_path.read_bytes()
    project = json.loads(project_bytes)
    from .mapping_check import build_units
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or not isinstance(mappings.get("compile_commands"), str):
        raise ValueError("unit representation requires configured compile_commands")
    database = source.path(root, mappings["compile_commands"])
    database_bytes = database.read_bytes()
    build_units_set, _external = build_units(root, database_bytes)
    associated_bindings = read_associated(root, pin) if units else ()
    import base64
    decoded_source = {name: base64.b64decode(row["content_base64"]).decode("utf-8", errors="replace")
                      for name, row in snapshot["files"].items()}
    include_graph_all, _widened = include_graph(decoded_source, source.include_paths)
    include_closure = DependencyGraph(edge for edge in include_graph_all.edges
                                      if edge.relationship == "includes")
    report_units = []
    owned_obligation_snapshot = None

    for unit in units:
        associated_source_units = (include_closure.prerequisites((unit["recovered_tu"],))
                                   | {unit["recovered_tu"]}
                                   if unit["recovered_tu"] in snapshot["files"] else frozenset())
        members, records, coverage_issues, coverage = _unit_members(
            view, snapshot, source_rows, unit, image, associated_bindings,
            associated_source_units)
        selected_ids = {Origin(pin, snapshot["source_configuration"]["address_space"], record.address).entity_id
                        for record in records}
        report = None
        verification_error = None
        if mapping_only:
            verification_error = "native measurement skipped by --mapping-only"
        elif records:
            try:
                report = verify_records(root, records, image_path, flag_sets=settings.flag_sets)
            except (OSError, ValueError) as error:
                verification_error = str(error)
        else:
            verification_error = "no configured source annotations can be selected by native verification"
        if mapping_only:
            native_matched = native_closed = None
            code_trials = evidence_trials = []
        else:
            native_matched, native_closed, code_trials, evidence_trials = _unit_trial_status(
                report or {}, members, coverage["closed"])
        representation = _counterpart_representation(
            root, unit, members, view, snapshot, source, settings.unit_extensions,
            build_units_set, include_closure)
        if owned_obligation_snapshot is None:
            owned_obligation_snapshot = _owned_obligation_snapshot(root, pin, associated_bindings)
        owned_obligations = _owned_obligations(
            owned_obligation_snapshot, unit["recovered_tu"],
            include_closure.prerequisites((unit["recovered_tu"],))
            if unit["recovered_tu"] in snapshot["files"] else frozenset())
        matched = None if mapping_only else bool(representation["represented_cpp"] and native_matched)
        native_closed = None if mapping_only else bool(representation["represented_cpp"] and native_closed)
        # The existing trial record has aggregate data/import/generated results
        # but no stable per-obligation identity receipts. Do not claim stage 3
        # for a TU with owned data obligations until those can be tied to the
        # same passing trial. Function-only candidates can use native closure.
        owned_evidence_covered = (None if mapping_only else
                                  True if not owned_obligations else None)
        closed, qualified_evidence_trials = _qualified_evidence_status(
            native_closed, evidence_trials, owned_evidence_covered)
        byte_exact = (None if mapping_only else
                      _fully_byte_exact_status(representation["represented_cpp"], matched,
                                               closed, owned_evidence_covered))
        if mapping_only:
            current_tus, single_current_tu, owner_unknowns = [], None, []
        else:
            current_tus, single_current_tu, owner_unknowns = _native_current_translation_units(
                report, members)
        result = {"name": unit["name"], "start": f"0x{unit['start']:08X}",
                  "end_exclusive": f"0x{unit['end_exclusive']:08X}",
                  "reason": unit["reason"], "evidence": unit["evidence"],
                  **_identity_counts(members),
                  **representation,
                  "current_source_homes": sorted({path for member in members
                                                   for path in member["current_source_homes"]}),
                  "current_translation_units": current_tus,
                  "current_translation_unit_count": None if mapping_only else len(current_tus),
                  "single_current_tu": single_current_tu,
                  "current_translation_unit_unknowns": owner_unknowns,
                  "code_coverage": coverage, "coverage_issues": coverage_issues,
                  "selected_function_count": None if mapping_only else len(selected_ids),
                  "unselected_identity_count": (None if mapping_only else
                      len({member["entity_id"] for member in members} - selected_ids)),
                  "matched_code_unit": matched, "code_matching_trial_indexes": code_trials,
                  "native_evidence_closed_unit": native_closed,
                  "native_evidence_closed_trial_indexes": evidence_trials,
                  "evidence_closed_unit": closed,
                  "evidence_closed_trial_indexes": qualified_evidence_trials,
                  "owned_evidence_obligations": owned_obligations,
                  "owned_evidence_covered_in_passing_trial": owned_evidence_covered,
                  "owned_evidence_status": ("no_owned_obligations" if not owned_obligations
                                            else "unknown_missing_per_obligation_trial_receipts"),
                  "fully_byte_exact_unit": byte_exact,
                  "native_result": (report or {}).get("result"),
                  "native_counts": (report or {}).get("counts"),
                  "verification_error": verification_error,
                  "native_measurement": "not_run" if mapping_only else "measured",
                  "measurement_report": report,
                  "members": members}
        report_units.append(result)

    # Each verifier invocation has its own native freshness guard. This outer
    # epoch prevents a report from combining individually fresh units measured
    # against different sources, image, measurement flags or unit config.
    assert_snapshot_current(root, snapshot)
    if (settings.snapshot(root, {}) != measurement_epoch
            or image_path.read_bytes() != image_data
            or hashlib.sha256(config_path.read_bytes()).hexdigest() != config_digest
            or project_path.read_bytes() != project_bytes
            or database.read_bytes() != database_bytes):
        raise ValueError("source, measurement inputs, image, project/build config, or units config changed during unit measurement; retry")

    count = Counter()
    count["candidate_units"] = len(report_units)
    count["identified_original_tu_candidates"] = len(report_units)
    count["represented_cpp_units"] = sum(row["represented_cpp"] for row in report_units)
    count["pending_unbound_mappings"] = sum(row.get("pending_unbound_count", 0) for row in report_units)
    count["matched_code_units"] = None if mapping_only else sum(row["matched_code_unit"] for row in report_units)
    count["evidence_closed_units"] = None if mapping_only else sum(row["evidence_closed_unit"] for row in report_units)
    count["fully_byte_exact_units"] = (None if mapping_only else
                                        sum(row["fully_byte_exact_unit"] for row in report_units))
    count["placeholder_units"] = sum(row["counterpart_placeholder"] for row in report_units)
    count["single_current_tu"] = None if mapping_only else sum(row["single_current_tu"] is True for row in report_units)
    count["single_current_tu_unknown"] = (None if mapping_only else
                                          sum(row["single_current_tu"] is None for row in report_units))
    return {"schema": "match-recovery.units.v1", "image_sha256": pin,
            "source_revision": snapshot["revision"],
            "original_translation_unit_denominator": "unknown",
            "global_recovery_percentage": "unknown",
            "historical_translation_unit_proof": False,
            "game_acceptance": False,
            "metric_mode": "mapping_only" if mapping_only else "native_measurement",
            "counts": dict(count), "identity_counts": _summary_identity_counts(report_units, mapping_only=mapping_only),
            "units": report_units,
            "limitations": ["explicit ranges are inferred contribution candidates, not proven historical translation units",
                            "source homes describe current organization and do not define candidate membership",
                            "native diagnostics do not establish whole-project linked preservation or game acceptance",
                            "code matching requires every selected implementation identity to PASS in one native graph trial; pending UNBOUND notes remain unmeasurable",
                            "evidence closure requires the same trial's native aggregate result to PASS",
                            "source-associated original data intervals are reported separately and do not prove a matching current table",
                            "whole-unit byte exactness remains unknown when owned object, literal, import, generated, or associated obligations cannot be tied to that same trial",
                            "the original translation-unit denominator and global recovery percentage are unknown",
                            *(["mapping-only mode did not run native verification; byte-exact and all other native counts are unmeasured"]
                              if mapping_only else [])]}

